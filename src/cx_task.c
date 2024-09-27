#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include "cx_tpool.h"
#include "cx_task.h"

// Define internal dynamic string
#define cx_str_name cxstr
#define cx_str_static
#define cx_str_implement
#include "cx_str.h"

// Define internal array of task ids
#define cx_array_name arr_tid
#define cx_array_type size_t
#define cx_array_static
#define cx_array_implement
#include "cx_array.h"

// Task information
typedef struct TaskInfo {
    CxTaskRunner*       tr;             // Associated Task Runner
    cxstr               name;           // Task unique name
    CxTaskRunnerTask    task_fn;        // Task function pointer
    void*               task_arg;       // Task argument pointer
    arr_tid             inps;           // Array of task ids which are inputs of this one (dependencies)
    arr_tid             outs;           // Array of task ids which depends on this one (dependants)
    size_t              cycles;         // Task last cycle executed
} TaskInfo;

// Define internal array of TaskInfo
#define cx_array_name arr_task
#define cx_array_type TaskInfo
#define cx_array_static
#define cx_array_implement
#include "cx_array.h"

typedef struct CxTaskRunner {
    pthread_mutex_t     lock;           // For exclusive access
    const CxAllocator*  alloc;          // Custom allocator
    size_t              nthreads;
    CxThreadPool*       tpool;          // Thread pool
    bool                started;
    size_t              cycles;
    size_t              run_cycles;
    arr_task            tasks;          // Array with all tasks
} CxTaskRunner;

CxTaskRunner* cx_task_runner_new(const CxAllocator* alloc, size_t nthreads) {

    CXCHK(nthreads > 0);
    CxTaskRunner* tr = cx_alloc_mallocz(alloc, sizeof(CxTaskRunner));
    tr->alloc = alloc;
    tr->nthreads = nthreads;
    tr->tpool = cx_tpool_new(alloc, nthreads, 32);
    CXCHKZ(pthread_mutex_init(&tr->lock, NULL));
    return tr;
}

CxError cx_task_runner_del(CxTaskRunner* tr) {

    CXCHKZ(pthread_mutex_destroy(&tr->lock));
    cx_tpool_del(tr->tpool);
    cx_alloc_free(tr->alloc, tr, sizeof(CxTaskRunner));
    return CXOK();
}

static void cx_task_wrapper(void* arg) {

    TaskInfo* tinfo = arg;
    CxTaskRunner* tr = tinfo->tr;
    // Executes user task
    tinfo->task_fn(tinfo->task_arg);
    tinfo->cycles++;

    CXCHKZ(pthread_mutex_lock(&tr->lock));

    // If this task has no outputs it is sink task
    if (arr_tid_len(&tinfo->outs) == 0) {


        CXCHKZ(pthread_mutex_unlock(&tr->lock));
        return;
    }

    // For each current task output, checks if its inputs are satisfied.
    for (size_t to = 0; to < arr_tid_len(&tinfo->outs); to++) {
        const size_t tid = tinfo->outs.data[to];
        TaskInfo* tout = &tr->tasks.data[tid];

        // Checks if all the current output task inputs are satisfied
        bool inputs_ok = true;
        for (size_t ti = 0; to < arr_tid_len(&tout->inps); to++) {
            TaskInfo* tinp = &tr->tasks.data[ti];
            if (tinp->cycles != tinfo->cycles) {
                inputs_ok = false;
            }
        }

        // Runs current output task
        if (inputs_ok) {
            CXCHKZ(cx_tpool_run(tr->tpool, cx_task_wrapper, tout));
        }
    }
    CXCHKZ(pthread_mutex_unlock(&tr->lock));
}

CxError cx_task_runner_start(CxTaskRunner* ts, size_t cycles) {

    if (ts->started) {
        return CXERR("CxTaskRunner already started");
    }

    if (arr_task_len(&ts->tasks) == 0) {
        return CXERR("No tasks have been created");
    }

    // Start tasks with no inputs (no dependencies)
    for (size_t i = 0; i < arr_task_len(&ts->tasks); i++) {
        TaskInfo* tinfo = &ts->tasks.data[i];
        if (arr_tid_len(&tinfo->inps) == 0) {
            const int res = cx_tpool_run(ts->tpool, cx_task_wrapper, tinfo);
            if (res) {
                return CXERR2(res, "Error starting thread");
            }
        }
    }

    return CXOK();
}

CxError cx_task_runner_stop(CxTaskRunner* ts) {

    return CXOK();
}

size_t cx_task_runner_cycles(CxTaskRunner* ts) {

    return 0;
}

CxError cx_task_new(CxTaskRunner* tr, const char* name, CxTaskRunnerTask fn, void* arg, size_t* tid) {

    // The task runner must be stopped

    // Checks if new task name is not being used
    for (size_t i = 0; i < arr_task_len(&tr->tasks); i++) {
        TaskInfo* pinfo = &tr->tasks.data[i];
        if (cxstr_cmp(&pinfo->name, name) == 0) {
            return CXERR("Task name already present");
        }
    }

    TaskInfo tinfo = {
        .tr = tr,
        .name = cxstr_initc(name),
        .task_fn = fn,
        .task_arg = arg,
    };
    arr_task_push(&tr->tasks, tinfo);
    *tid = arr_task_len(&tr->tasks) - 1;

    return CXOK();
}

CxError cx_task_depends(CxTaskRunner* tr, size_t tid, size_t other) {

    const size_t ntasks = arr_task_len(&tr->tasks);
    if (tid >= ntasks) {
        return CXERR("Invalid task id");
    }
    if (other >= ntasks) {
        return CXERR("Invalid dependency task id");
    }
    if (tid == other) {
        return CXERR("Task cannot depend on itself");
    }

    TaskInfo* tinfo = &tr->tasks.data[tid];
    for (size_t i = 0; i < arr_tid_len(&tinfo->inps); i++) {
        if (tinfo->inps.data[i] == other) {
            return CXERR("Dependency already set");
        }
    }
    arr_tid_push(&tinfo->inps, other);

    tinfo = &tr->tasks.data[other];
    arr_tid_push(&tinfo->outs, tid);
    return CXOK();
}



