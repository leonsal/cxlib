#include <stdio.h>
#include <pthread.h>

#include "cx_alloc.h"
#include "cx_tpool.h"

typedef struct Work {
    CxThreadPoolWorker f;   // User work function
    void* param;            // User work pointer to function parameter
} Work;

// Define queue of Work
#define cx_queue_name queue
#define cx_queue_type Work
#define cx_queue_static
#define cx_queue_inline
#define cx_queue_instance_allocator
#define cx_queue_implement
#include "cx_queue.h"

// Thread pool state
typedef struct CxThreadPool {
    const CxAllocator*  alloc;      // Allocator
    queue               work;       // Work queue
    size_t              nthreads;   // Number of threads                            
    pthread_t*          threads;    // Array of threads ids
} CxThreadPool;

static void* cx_tpool_worker(void* arg);

CxThreadPool* cx_tpool_new(const CxAllocator* alloc, size_t nthreads, size_t wsize) {

    // Allocates thread pool state
    if (alloc == NULL) {
        alloc = cx_def_allocator();
    }
    CxThreadPool* tp = cx_alloc_malloc(alloc, sizeof(CxThreadPool));
    if (tp == NULL) {
        return NULL;
    }
    tp->alloc = alloc;
    tp->nthreads = nthreads;

    // Creates work queue with specified size
    tp->work = queue_init(alloc, wsize);

    // Creates worker threads
    tp->threads = cx_alloc_malloc(alloc, sizeof(pthread_t) * tp->nthreads);
    if (tp->threads == NULL) {
        return NULL;
    }
    for (size_t i = 0; i < tp->nthreads; i++) {
        int res = pthread_create(&tp->threads[i], NULL, cx_tpool_worker, tp);
        if (res) {
            return NULL;
        }
    }
    return tp;
}

int cx_tpool_del(CxThreadPool* tp) {

    // Close queue and wait for threads to finish
    queue_close(&tp->work);
    int res = 0;
    for (size_t i = 0; i < tp->nthreads; i++) {
        res = pthread_join(tp->threads[i], NULL);
    }

    queue_free(&tp->work);
    cx_alloc_free(tp->alloc, tp->threads, sizeof(pthread_t) * tp->nthreads);
    cx_alloc_free(tp->alloc, tp, sizeof(CxThreadPool));
    return res;
}

size_t cx_tpool_work_len(CxThreadPool* tp) {

    return queue_len(&tp->work);
}

int cx_tpool_run(CxThreadPool* tp, CxThreadPoolWorker worker, void* param) {

    Work work = {.f = worker, .param = param};
    return queue_put(&tp->work, work);
}

static void* cx_tpool_worker(void* arg) {

    CxThreadPool* tp = arg;
    while (1) {
        Work work;
        int res = queue_get(&tp->work, &work);
        if (res == ECANCELED) {
            break;
        }
        work.f(work.param);
    }
    return NULL;
}


