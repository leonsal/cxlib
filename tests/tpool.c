#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <stdatomic.h>

#include "cx_alloc.h"
#include "cx_pool_allocator.h"
#include "util.h"
#include "logger.h"
#include "cx_tpool.h"
#include "tpool.h"

typedef struct Work {
    atomic_int counter;
} Work;

static void tpool_worker(void* arg);

void tpool_tests(void) {

    // Use default allocator
    tpool_test(NULL, 1, 20);
    tpool_test(NULL, 8, 20);

    // Use pool allocator
    CxPoolAllocator* pa = cx_pool_allocator_create(4*1024, NULL);
    tpool_test(cx_pool_allocator_iface(pa), 1, 20);
    tpool_test(cx_pool_allocator_iface(pa), 8, 20);
    cx_pool_allocator_destroy(pa);
}

void tpool_test(const CxAllocator* alloc, size_t nthreads, size_t nworks) {

    LOGI("%s: alloc:%p nthreads=%zu nworks=%zu", __func__, alloc, nthreads, nworks);

    // Creates and destroy thread pool
    CxThreadPool* tp = cx_tpool_new(alloc, nthreads, nworks);
    cx_tpool_del(tp);

    tp = cx_tpool_new(alloc, nthreads, nworks);
    Work work = {};
    for (size_t i = 0; i < nworks; i++) {
        int res = cx_tpool_run(tp, tpool_worker, &work);
        assert(res == 0);
    }
    cx_tpool_del(tp);
    CHK(work.counter == nworks);
}

static void tpool_worker(void *arg) {

    Work* work = arg;
    work->counter++;
}

