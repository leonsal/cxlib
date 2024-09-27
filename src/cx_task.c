#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>
#include "cx_tpool.h"
#include "cx_task.h"

typedef struct CxTaskRunner {
    const CxAllocator*  alloc;
    size_t              nthreads;
    CxThreadPool*       tpool;
    bool                started;
    size_t              cycles;
    size_t              run_cycles;
} CxTaskRunner;

CxTaskRunner* cx_task_runner_new(const CxAllocator* alloc, size_t nthreads) {

    CXCHK(nthreads > 1);
    CxTaskRunner* tr = cx_alloc_mallocz(alloc, sizeof(CxTaskRunner));
    tr->alloc = alloc;
    tr->nthreads = nthreads;
    return tr;
}

CxError cx_task_runner_del(CxTaskRunner* tr) {

    cx_alloc_free(tr->alloc, tr, sizeof(CxTaskRunner));
    return CXOK();
}

CxError cx_task_runner_start(CxTaskRunner* ts, size_t cycles) {

    if (ts->started) {
        return CXERR("CxTaskRunner already started");
    }

    return CXOK();
}

CxError cx_task_runner_stop(CxTaskRunner* ts) {

    return CXOK();
}

size_t cx_task_runner_cycles(CxTaskRunner* ts) {

    return 0;
}

CxError cx_task_new(CxTaskRunner* ts, CxTaskRunnerTask t, void* arg, CxTask* task) {

    return CXOK();
}

CxError cx_task_depends(CxTask* t, CxTask* other) {

    return CXOK();
}



