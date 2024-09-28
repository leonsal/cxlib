#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include "cx_tpool.h"
#include "cx_tflow.h"

// Define internal dynamic string
#define cx_str_name cxstr
#define cx_str_static
#define cx_str_implement
#include "cx_str.h"

// Define internal array of pointers to CxTFlowTask
#define cx_array_name arr_task
#define cx_array_type CxTFlowTask*
#define cx_array_static
#define cx_array_implement
#include "cx_array.h"

// Task information
typedef struct CxTFlowTask {
    CxTFlow*        tf;             // Associated Task Flow
    cxstr           name;           // Task unique name
    CxTFlowTaskFn   task_fn;        // Task function pointer
    void*           task_arg;       // Task argument pointer
    arr_task        inps;           // Array of pointers to tasks which are inputs of this one (dependencies)
    arr_task        outs;           // Array of pointers to taskd which are outputs of this one (dependants)
    size_t          cycles;         // Task last cycle executed
    void*           udata;          // Optional associated user data 
} CxTFlowTask;

// Task Flow state
typedef struct CxTFlow {
    pthread_mutex_t     lock;           // For exclusive access
    pthread_cond_t      stop_cv;        // Conditional variable signaled when stopped
    const CxAllocator*  alloc;          // Custom allocator
    CxThreadPool*       tpool;          // Thread pool
    size_t              cycles;         // Number of cycles to run (0=unlimited)
    size_t              run_cycles;     // Number of cycles run
    size_t              run_sinks;      // Number of sinks run in the cycle
    arr_task            tasks;          // Array of pointers to all tasks
    arr_task            sources;        // Array of pointers to source tasks
    arr_task            sinks;          // Array of pointers to sink tasks
    bool                stop;           // Stop request flag
    bool                running;        // Running flag
} CxTFlow;

// Forward declarations of local functions
static inline bool cx_tflow_is_running(CxTFlow* tf);
static void cx_tflow_wrapper(void* arg);
static CxError cx_tflow_restart(CxTFlow* tf);


CxTFlow* cx_tflow_new(const CxAllocator* alloc, size_t nthreads) {

    CXCHK(nthreads > 0);
    CxTFlow* tf = cx_alloc_mallocz(alloc, sizeof(CxTFlow));
    CXCHKZ(pthread_mutex_init(&tf->lock, NULL));
    CXCHKZ(pthread_cond_init(&tf->stop_cv, NULL));
    tf->alloc = alloc;
    tf->tpool = cx_tpool_new(alloc, nthreads, 32);
    return tf;
}

CxError cx_tflow_del(CxTFlow* tf) {

    CXCHKZ(pthread_cond_destroy(&tf->stop_cv));
    CXCHKZ(pthread_mutex_destroy(&tf->lock));

    for (size_t i = 0; i < arr_task_len(&tf->tasks); i++) {
        CxTFlowTask* tinfo = tf->tasks.data[i];
        cxstr_free(&tinfo->name);
        arr_task_free(&tinfo->inps);
        arr_task_free(&tinfo->outs);
        cx_alloc_free(tf->alloc, tinfo, sizeof(CxTFlowTask));
    }

    arr_task_free(&tf->tasks);
    arr_task_free(&tf->sources);
    arr_task_free(&tf->sinks);

    cx_tpool_del(tf->tpool);
    cx_alloc_free(tf->alloc, tf, sizeof(CxTFlow));
    return CXOK();
}


CxError cx_tflow_start(CxTFlow* tf, size_t cycles) {

    if (cx_tflow_is_running(tf)) {
        return CXERR("CxTaskFlow already started");
    }

    if (arr_task_len(&tf->tasks) == 0) {
        return CXERR("No tasks have been created");
    }

    // Find source and sink tasks
    arr_task_clear(&tf->sources);
    arr_task_clear(&tf->sinks);
    for (size_t i = 0; i < arr_task_len(&tf->tasks); i++) {
        CxTFlowTask* tinfo = tf->tasks.data[i];
        // If no inputs is a source task
        if (arr_task_len(&tinfo->inps) == 0) {
            arr_task_push(&tf->sources, tinfo);
        }
        // If no inputs is a sink task
        if (arr_task_len(&tinfo->outs) == 0) {
            arr_task_push(&tf->sinks, tinfo);
        }
    }

    tf->cycles = cycles;
    tf->run_cycles = 0;
    tf->running = true;
    tf->stop = false;

    // Start source tasks (tasks with no dependencies)
    return cx_tflow_restart(tf);
}

CxError cx_tflow_stop(CxTFlow* tf, struct timespec timeout) {

    CXCHKZ(pthread_mutex_lock(&tf->lock));
    if (!tf->running) {
        CXCHKZ(pthread_mutex_unlock(&tf->lock));
        return CXERR("Task Flow is not running");
    }

    // Sets stop request
    tf->stop = true;
    CXCHKZ(pthread_mutex_unlock(&tf->lock));

    // Waits for task flow to stop
    return cx_tflow_wait(tf, timeout);
}

CxTFlowStatus cx_tflow_status(CxTFlow* tf) {

    CxTFlowStatus status;
    CXCHKZ(pthread_mutex_lock(&tf->lock));
    status.running = tf->running;
    status.cycles = tf->cycles;
    status.run_cycles = tf->run_cycles;
    CXCHKZ(pthread_mutex_unlock(&tf->lock));
    return status;
}

#define NANOSECS_PER_SEC    (1000000000)

CxError cx_tflow_wait(CxTFlow* tf, struct timespec reltime) {

    // Calculates absolute time from now from the specified relative time
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);
    abstime.tv_sec += reltime.tv_sec;
    abstime.tv_nsec += reltime.tv_nsec;
    if (abstime.tv_nsec >= NANOSECS_PER_SEC) {
        abstime.tv_nsec -= NANOSECS_PER_SEC;
        abstime.tv_sec += 1;
    }

    CxError error = {0};
    CXCHKZ(pthread_mutex_lock(&tf->lock));
    while (tf->running) {
        const int res = pthread_cond_timedwait(&tf->stop_cv, &tf->lock, &abstime);
        if (res) {
            if (res == ETIMEDOUT) {
                error = CXERR("Timeout waiting for Task Flow to stop");
            } else {
                error = CXERR2(res, strerror(res));
            }
            break;
        }
    }
    CXCHKZ(pthread_mutex_unlock(&tf->lock));
    return error;
}

CxError cx_tflow_add_task(CxTFlow* tf, const char* name, CxTFlowTaskFn fn, void* arg, CxTFlowTask** ptask) {

    if (cx_tflow_is_running(tf)) {
        return CXERR("Task flow is running");
    }

    // Checks if new task name is not being used
    for (size_t i = 0; i < arr_task_len(&tf->tasks); i++) {
        CxTFlowTask* task = tf->tasks.data[i];
        if (cxstr_cmp(&task->name, name) == 0) {
            return CXERR("Task name already present");
        }
    }

    CxTFlowTask* task = cx_alloc_mallocz(tf->alloc, sizeof(CxTFlowTask));
    *task = (CxTFlowTask){
        .tf = tf,
        .name = cxstr_initc(name),
        .task_fn = fn,
        .task_arg = arg,
    };
    arr_task_push(&tf->tasks, task);
    *ptask = task;
    return CXOK();
}

CxError cx_tflow_set_task_dep(CxTFlowTask* task, CxTFlowTask* dep) {

    if (cx_tflow_is_running(task->tf)) {
        return CXERR("Task flow is running");
    }

    // Checks if both task pointers are valid
    CxTFlow* tf = task->tf;
    bool task_found = false;
    bool dep_found = false;
    for (size_t i = 0; i < arr_task_len(&tf->tasks); i++) {
        CxTFlowTask* curr = tf->tasks.data[i];
        if (curr == task) {
            task_found = true;
            continue;
        }
        if (curr == dep) {
            dep_found = true;
            continue;
        }

    }
    if (!task_found) {
        return CXERR("Task not found");
    }
    if (!dep_found) {
        return CXERR("Task dependency not found");
    }
    if (task == dep) {
        return CXERR("Task cannot depend on itself");
    }

    // Checks if dependency already set
    for (size_t i = 0; i < arr_task_len(&task->inps); i++) {
        if (task->inps.data[i] == dep) {
            return CXERR("Dependency already set");
        }
    }

    arr_task_push(&task->inps, dep);
    arr_task_push(&dep->outs, task);
    return CXOK();
}

CxTFlowTask* cx_tflow_find_task(CxTFlow* tf, const char* name) {

    for (size_t i = 0; i < arr_task_len(&tf->tasks); i++) {
        CxTFlowTask* task = tf->tasks.data[i];
        if (cxstr_cmp(&task->name, name) == 0) {
            return task;
        }
    }
    return NULL;
}


void cx_tflow_set_task_udata(CxTFlowTask* task, void* udata) {

    task->udata = udata;
}

static inline bool cx_tflow_is_running(CxTFlow* tf) {

    bool running;
    CXCHKZ(pthread_mutex_lock(&tf->lock));
    running = tf->running;
    CXCHKZ(pthread_mutex_unlock(&tf->lock));
    return running;
}

static void cx_tflow_wrapper(void* arg) {

    CxTFlowTask* tinfo = arg;
    CxTFlow* tf = tinfo->tf;

    // Executes user task
    printf("%s: name:%s cycles:%zu run_cycles:%zu total:%zu\n", __func__, tinfo->name.data, tinfo->cycles, tf->run_cycles, tf->cycles);
    tinfo->task_fn(tinfo->task_arg);
    tinfo->cycles++;

    // If this task has no outputs it is a sink task
    if (arr_task_len(&tinfo->outs) == 0) {
        CXCHKZ(pthread_mutex_lock(&tf->lock));
        tf->run_sinks++;
        printf("\t%s: name:%s run_sinks:%zu total_sinks:%zu\n", __func__, tinfo->name.data, tf->run_sinks, arr_task_len(&tf->sinks));
        // If all sinks have run, a cycle has completed
        if (tf->run_sinks == arr_task_len(&tf->sinks)) {
            tf->run_cycles++;
            // Checks for stop request or number of cycles run
            if (tf->stop || (tf->cycles && tf->run_cycles >= tf->cycles)) {
                tf->running = false;
                CXCHKZ(pthread_cond_signal(&tf->stop_cv));
                CXCHKZ(pthread_mutex_unlock(&tf->lock));
                return;
            }
            // Restart source tasks beginning a new cycle
            CXCHKZ(pthread_mutex_unlock(&tf->lock));
            cx_tflow_restart(tf);
            return;
        }
        // Not all sinks have run in the cycle
        CXCHKZ(pthread_mutex_unlock(&tf->lock));
        return;
    }

    // For each current task output, checks if its inputs are satisfied.
    for (size_t to = 0; to < arr_task_len(&tinfo->outs); to++) {
        CxTFlowTask* tout = tf->tasks.data[to];

        // Checks if all the current output task inputs are satisfied
        bool inputs_ok = true;
        for (size_t ti = 0; to < arr_task_len(&tout->inps); to++) {
            CxTFlowTask* tinp = tf->tasks.data[ti];
            CXCHKZ(pthread_mutex_lock(&tf->lock));
            if (tinp->cycles != tinfo->cycles) {
                inputs_ok = false;
            }
            CXCHKZ(pthread_mutex_unlock(&tf->lock));
        }

        // Runs current output task
        if (inputs_ok) {
            CXCHKZ(cx_tpool_run(tf->tpool, cx_tflow_wrapper, tout));
        }
    }
}

static CxError cx_tflow_restart(CxTFlow* tf) {

    tf->run_sinks = 0;
    for (size_t i = 0; i < arr_task_len(&tf->sources); i++) {
        CxTFlowTask* tinfo = tf->sources.data[i];
        int res = cx_tpool_run(tf->tpool, cx_tflow_wrapper, tinfo);
        if (res) {
            return CXERR2(res, "returned by cx_tpool_run{}");
        }
    }
    return CXOK();
}

