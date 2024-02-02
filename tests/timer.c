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

#define MILLISEC    (1000000)

static void timer_func(CxTimer* timer, void* arg);

void cx_timer_tests() {

    cx_timer_test(cxDefaultAllocator());
}

void cx_timer_test(const CxAllocator* alloc) {

    LOGI("timer. alloc:%p", alloc);
    CxTimer* tm = cx_timer_create(alloc);

    size_t task_id;

    struct timespec delay1 = {.tv_sec = 2};
    CHK(cx_timer_set(tm, delay1, timer_func, &delay1, &task_id) == 0);

    struct timespec delay2 = {.tv_sec = 1};
    CHK(cx_timer_set(tm, delay2, timer_func, &delay2, &task_id) == 0);

    struct timespec delay3 = {.tv_sec = 3};
    CHK(cx_timer_set(tm, delay3, timer_func, &delay3, &task_id) == 0);

    struct timespec delay4 = {.tv_sec = 2, .tv_nsec=MILLISEC};
    CHK(cx_timer_set(tm, delay4, timer_func, &delay4, &task_id) == 0);

    struct timespec delay5 = {.tv_sec = 0, .tv_nsec=400*MILLISEC};
    CHK(cx_timer_set(tm, delay5, timer_func, &delay5, &task_id) == 0);

    sleep(10);

    printf("TEST: destroy\n");
    cx_timer_destroy(tm);

}

static void timer_func(CxTimer* timer, void* arg) {

    struct timespec* delay = arg;
    double secs = delay->tv_sec + delay->tv_nsec/1000000000.0;
    LOGD("timer_func: %.3f", secs);
}

