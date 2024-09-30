#include <bits/time.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>

#include "cx_alloc.h"
#include "cx_pool_allocator.h"
#include "logger.h"
#include "util.h"
#include "cx_timer.h"
#include "registry.h"

// Timer function result
typedef struct Result {
    struct timespec abstime;    // Time when timer function executed
} Result;

// Define array with results of each timer function execution
#define cx_array_name arr_res
#define cx_array_type Result
#define cx_array_instance_allocator
#define cx_array_implement
#define cx_array_static
#include "cx_array.h"

// Test state passed to timer functions
typedef struct State {
    const CxAllocator* alloc;
    arr_res results;
    _Atomic uintptr_t periodic_count;
} State;

static void timer_func(CxTimer* timer, void* arg);
static void periodic_func(CxTimer* tm, void* arg);


void test_timer1(const CxAllocator* alloc) {

    LOGI("timer. alloc:%p", alloc);

    // Creates timer manager and sets its userdata to our test state
    CxTimer* tm = cx_timer_create(alloc);
    State s = {
        .alloc = alloc,
        .results = arr_res_init(alloc),
    };
    cx_timer_set_userdata(tm, &s);

    // Schedule tasks in increasing delay
    size_t task_count = 10;
    for (size_t i = 0; i < task_count; i++) {
        struct timespec delay = cx_timer_ts_from_secs(0.01*(i+1));
        CHK(cx_timer_set(tm, delay, timer_func, NULL, NULL) == 0);
    }
    while (cx_timer_count(tm) > 0) {
        usleep(10000);
    }
    CHK(arr_res_len(&s.results) == task_count);
    // Checks result
    for (size_t i = 0; i < task_count; i++) {
        if (i > 0) {
            CHK(cx_timer_cmp_ts(s.results.data[i-1].abstime, s.results.data[i].abstime) < 0);
        }
    }
    arr_res_clear(&s.results);

    // Schedule tasks in decreasing delay
    task_count = 10;
    for (size_t i = 0; i < task_count; i++) {
        struct timespec delay = cx_timer_ts_from_secs(0.01*(task_count - i + 1));
        CHK(cx_timer_set(tm, delay, timer_func, NULL, NULL) == 0);
    }
    while (cx_timer_count(tm) > 0) {
        usleep(10000);
    }
    CHK(arr_res_len(&s.results) == task_count);
    // Checks result
    for (size_t i = 0; i < task_count; i++) {
        if (i > 0) {
            CHK(cx_timer_cmp_ts(s.results.data[i-1].abstime, s.results.data[i].abstime) < 0);
        }
    }
    arr_res_clear(&s.results);

    // Schedule tasks in increasing delay
    for (size_t i = 0; i < task_count; i++) {
        struct timespec delay = cx_timer_ts_from_secs(0.01*(i+1));
        CHK(cx_timer_set(tm, delay, timer_func, NULL, NULL) == 0);
    }
    // Clear all tasks and checks
    usleep(1000);
    CHK(cx_timer_clear_all(tm) == 0);
    CHK(arr_res_len(&s.results) == 0);

    // Schedule periodic task which schedules itself.
    struct timespec delay = cx_timer_ts_from_secs(0.01);
    uintptr_t count = 5;
    s.periodic_count = 0;
    CHK(cx_timer_set(tm, delay, periodic_func, (void*)count, NULL) == 0);
    // Wait for all activations
    while (s.periodic_count < count) {
        usleep(10000);
    }
    CHK(arr_res_len(&s.results) == count);

    CHK(cx_timer_destroy(tm) == 0);
    arr_res_free(&s.results);
}

static void timer_func(CxTimer* tm, void* arg) {

    State* s = cx_timer_get_userdata(tm);
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    arr_res_push(&s->results, (Result){
        .abstime = now,
    });
}

static void periodic_func(CxTimer* tm, void* arg) {

    uintptr_t count = (uintptr_t)(arg);
    State* s = cx_timer_get_userdata(tm);
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);
    arr_res_push(&s->results, (Result){
        .abstime = now,
    });
    if (count == 0) {
        return;
    }
    count--;
    s->periodic_count++;

    //LOGD("periodic_func:%zu", count);
    // Reactivates itself
    struct timespec delay = cx_timer_ts_from_secs(0.01);
    cx_timer_set(tm, delay, periodic_func, (void*)count, NULL);
}

void test_timer() {

    // Tests with default allocator
    test_timer1(cx_def_allocator());

    // Tests with pool allocator
    CxPoolAllocator* pa = cx_pool_allocator_create(4*1024, NULL);
    test_timer1(cx_pool_allocator_iface(pa));
    cx_pool_allocator_destroy(pa);
}

__attribute__((constructor))
static void reg_timer(void) {

    // DISABLED (HAS BUG)
    //    reg_add_test("timer", test_timer);
}

