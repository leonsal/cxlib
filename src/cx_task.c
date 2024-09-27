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

// Define internal array of pointers to CxTaskFlowTask
#define cx_array_name arr_ptask
#define cx_array_type CxTaskFlowTask*
#define cx_array_static
#define cx_array_implement
#include "cx_array.h"

// Task information
typedef struct CxTaskFlowTask {
    CxTaskFlow*         tf;             // Associated Task Flow
    cxstr               name;           // Task unique name
    CxTaskFlowTaskFn    task_fn;        // Task function pointer
    void*               task_arg;       // Task argument pointer
    arr_ptask           inps;           // Array of pointers to tasks which are inputs of this one (dependencies)
    arr_ptask           outs;           // Array of pointers to task which depends on this one (dependants)
    size_t              cycles;         // Task last cycle executed
    void*               udata;          // Optional associated user data 
} CxTaskFlowTask;

// Define internal array of CxTaskFlowTask
#define cx_array_name arr_task
#define cx_array_type CxTaskFlowTask
#define cx_array_static
#define cx_array_implement
#include "cx_array.h"


typedef struct CxTaskFlow {
    pthread_mutex_t     lock;           // For exclusive access
    const CxAllocator*  alloc;          // Custom allocator
    size_t              nthreads;
    CxThreadPool*       tpool;          // Thread pool
    bool                started;
    size_t              cycles;         // Number of cycles to run (0=unlimited)
    size_t              run_cycles;     // Number of cycles run
    arr_task            tasks;          // Array with all tasks
} CxTaskFlow;

CxTaskFlow* cx_task_flow_new(const CxAllocator* alloc, size_t nthreads) {

    CXCHK(nthreads > 0);
    CxTaskFlow* tr = cx_alloc_mallocz(alloc, sizeof(CxTaskFlow));
    tr->alloc = alloc;
    tr->nthreads = nthreads;
    tr->tpool = cx_tpool_new(alloc, nthreads, 32);
    CXCHKZ(pthread_mutex_init(&tr->lock, NULL));
    return tr;
}

CxError cx_task_flow_del(CxTaskFlow* tr) {

    CXCHKZ(pthread_mutex_destroy(&tr->lock));
    cx_tpool_del(tr->tpool);
    cx_alloc_free(tr->alloc, tr, sizeof(CxTaskFlow));
    return CXOK();
}

static void cx_task_wrapper(void* arg) {

    CxTaskFlowTask* tinfo = arg;
    CxTaskFlow* tf = tinfo->tf;
    // Executes user task
    tinfo->task_fn(tinfo->task_arg);
    tinfo->cycles++;

    CXCHKZ(pthread_mutex_lock(&tf->lock));

    // If this task has no outputs it is sink task
    if (arr_ptask_len(&tinfo->outs) == 0) {


        CXCHKZ(pthread_mutex_unlock(&tf->lock));
        return;
    }

    // For each current task output, checks if its inputs are satisfied.
    for (size_t to = 0; to < arr_ptask_len(&tinfo->outs); to++) {
        CxTaskFlowTask* tout = &tf->tasks.data[to];

        // Checks if all the current output task inputs are satisfied
        bool inputs_ok = true;
        for (size_t ti = 0; to < arr_ptask_len(&tout->inps); to++) {
            CxTaskFlowTask* tinp = &tf->tasks.data[ti];
            if (tinp->cycles != tinfo->cycles) {
                inputs_ok = false;
            }
        }

        // Runs current output task
        if (inputs_ok) {
            CXCHKZ(cx_tpool_run(tf->tpool, cx_task_wrapper, tout));
        }
    }
    CXCHKZ(pthread_mutex_unlock(&tf->lock));
}

CxError cx_task_flow_start(CxTaskFlow* ts, size_t cycles) {

    if (ts->started) {
        return CXERR("CxTaskFlow already started");
    }

    if (arr_task_len(&ts->tasks) == 0) {
        return CXERR("No tasks have been created");
    }

    // Start tasks with no inputs (no dependencies)
    for (size_t i = 0; i < arr_task_len(&ts->tasks); i++) {
        CxTaskFlowTask* tinfo = &ts->tasks.data[i];
        if (arr_ptask_len(&tinfo->inps) == 0) {
            const int res = cx_tpool_run(ts->tpool, cx_task_wrapper, tinfo);
            if (res) {
                return CXERR2(res, "Error starting thread");
            }
        }
    }

    return CXOK();
}

CxError cx_task_flow_stop(CxTaskFlow* ts) {

    return CXOK();
}

size_t cx_task_flow_cycles(CxTaskFlow* ts) {

    return 0;
}

CxError cx_task_flow_add_task(CxTaskFlow* tf, const char* name, CxTaskFlowTaskFn fn, void* arg, void* udata, CxTaskFlowTask** ptask) {

    // The task runner must be stopped

    // Checks if new task name is not being used
    for (size_t i = 0; i < arr_task_len(&tf->tasks); i++) {
        CxTaskFlowTask* task = &tf->tasks.data[i];
        if (cxstr_cmp(&task->name, name) == 0) {
            return CXERR("Task name already present");
        }
    }

    CxTaskFlowTask task = {
        .tf = tf,
        .name = cxstr_initc(name),
        .task_fn = fn,
        .task_arg = arg,
        .udata = udata,
    };
    arr_task_push(&tf->tasks, task);
    const size_t idx = arr_task_len(&tf->tasks) - 1;
    *ptask = &tf->tasks.data[idx];

    return CXOK();
}

CxError cx_task_flow_set_task_dep(CxTaskFlowTask* task, CxTaskFlowTask* dep) {

    // Checks if both task pointers are valid
    CxTaskFlow* tf = task->tf;
    bool task_found = false;
    bool dep_found = false;
    for (size_t i = 0; i < arr_task_len(&tf->tasks); i++) {
        CxTaskFlowTask* curr = &tf->tasks.data[i];
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
    for (size_t i = 0; i < arr_ptask_len(&task->inps); i++) {
        if (task->inps.data[i] == dep) {
            return CXERR("Dependency already set");
        }
    }

    arr_ptask_push(&task->inps, dep);
    return CXOK();
}



