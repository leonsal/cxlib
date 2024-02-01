#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include "cx_alloc.h"
#include "cx_pool_allocator.h"
#include "logger.h"
#include "util.h"
#include "cx_timer.h"
#include "timer.h"


static void timer_func(CxTimer* timer, void* arg);

void cx_timer_tests() {

    cx_timer_test(cxDefaultAllocator());
}

void cx_timer_test(const CxAllocator* alloc) {

    LOGI("timer. alloc:%p", alloc);
    CxTimer* tm = cx_timer_create(alloc);

    size_t task_id;
    struct timespec delay = {.tv_sec = 4};
    CHK(cx_timer_set(tm, delay, timer_func, NULL, &task_id) == 0);
    printf("set timer:%zu\n", delay.tv_sec);

    sleep(10);

    printf("TEST: destroy\n");
    cx_timer_destroy(tm);

}

static void timer_func(CxTimer* timer, void* arg) {

    printf("timer_func: %p/%p\n", timer, arg);
}

