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
#include "registry.h"

typedef struct Work {
    atomic_int counter;
} Work;

static void tpool_worker(void* arg);


void test_tpool1(const CxAllocator* alloc, size_t nthreads, size_t nworks) {

    LOGI("%s: alloc:%p nthreads=%zu nworks=%zu", __func__, alloc, nthreads, nworks);

    // Creates and destroy thread pool
    CxThreadPool* tp = cx_tpool_new(alloc, nthreads, nworks);
    cx_tpool_del(tp);

    tp = cx_tpool_new(alloc, nthreads, nworks);
    Work work = {};
    for (size_t i = 0; i < nworks; i++) {
        CHKZ(cx_tpool_run(tp, tpool_worker, &work));
    }
    cx_tpool_del(tp);
    CHK(work.counter == nworks);
}

static void tpool_worker(void *arg) {

    Work* work = arg;
    work->counter++;
}

static void test_tpool(void) {

    // Use default allocator
    test_tpool1(NULL, 1, 20);
    test_tpool1(NULL, 8, 20);

    // Use pool allocator
    CxPoolAllocator* pa = cx_pool_allocator_create(4*1024, NULL);
    test_tpool1(cx_pool_allocator_iface(pa), 1, 20);
    test_tpool1(cx_pool_allocator_iface(pa), 8, 20);
    cx_pool_allocator_destroy(pa);
}

__attribute__((constructor))
static void reg_tpool(void) {

    reg_add_test("tpool", test_tpool);
}

