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

// Timer function argument
typedef struct FuncArgs {
    struct timespec delay;
} FuncArgs;

// Timer function rsult
typedef struct Result {
    struct timespec delay;
} Result;

// Define array with records the result of each timer function execution
#define cx_array_name arr_res
#define cx_array_type Result
#define cx_array_instance_allocator
#define cx_array_implement
#define cx_array_static
#include "cx_array.h"

typedef struct State {
    const CxAllocator* alloc;
    arr_res results;
} State;

static void timer_func(CxTimer* timer, void* arg);

void cx_timer_tests() {

    cx_timer_test(cxDefaultAllocator());
}

void cx_timer_test(const CxAllocator* alloc) {

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
        FuncArgs* args = cx_alloc_malloc(alloc, sizeof(FuncArgs));
        args->delay = cx_timer_ts_from_secs(0.01*(i+1));
        CHK(cx_timer_set(tm, args->delay, timer_func, args, NULL) == 0);
    }
    while (cx_timer_count(tm) > 0) {
        usleep(10000);
    }
    CHK(arr_res_len(&s.results) == task_count);
    // Checks result
    for (size_t i = 0; i < task_count; i++) {
        if (i > 0) {
            CHK(cx_timer_cmp_ts(s.results.data[i-1].delay, s.results.data[i].delay) < 0);
        }
    }
    arr_res_clear(&s.results);

    // Schedule tasks in decreasing delay
    task_count = 10;
    for (size_t i = 0; i < task_count; i++) {
        FuncArgs* args = cx_alloc_malloc(alloc, sizeof(FuncArgs));
        args->delay = cx_timer_ts_from_secs(0.01*(task_count - i + 1));
        CHK(cx_timer_set(tm, args->delay, timer_func, args, NULL) == 0);
    }
    while (cx_timer_count(tm) > 0) {
        usleep(10000);
    }
    CHK(arr_res_len(&s.results) == task_count);
    // Checks result
    for (size_t i = 0; i < task_count; i++) {
        if (i > 0) {
            CHK(cx_timer_cmp_ts(s.results.data[i-1].delay, s.results.data[i].delay) < 0);
        }
    }
    arr_res_clear(&s.results);

    cx_timer_destroy(tm);
    arr_res_free(&s.results);
}

static void timer_func(CxTimer* timer, void* arg) {

    FuncArgs* a = arg;
    State* s = cx_timer_get_userdata(timer);
    arr_res_push(&s->results, (Result){
        .delay = a->delay,
    });
    //LOGD("timer_func: %.3f", cx_timer_secs_from_ts(a->delay));
    cx_alloc_free(s->alloc, a, sizeof(FuncArgs));
}

