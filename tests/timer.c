#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>

#include "cx_alloc.h"
#include "cx_pool_allocator.h"
#include "logger.h"
#include "util.h"
#include "cx_timer.h"
#include "timer.h"

void cx_timer_tests() {

    cx_timer_test(cxDefaultAllocator());
}

void cx_timer_test(const CxAllocator* alloc) {

    LOGI("timer. alloc:%p", alloc);
    CxTimerMan* tm = cx_timer_man_create(alloc);


    cx_timer_man_destroy(tm);

}

