#include <stdio.h>
#include <asm-generic/errno.h>
#include <stdio.h>
#include "cx_alloc.h"
#include "cx_pool_allocator.h"

#include "logger.h"
#include "alloc.h"
#include "array.h"
#include "hmap.h"
#include "string.h"
#include "queue.h"
#include "list.h"
#include "timer.h"
#include "json_build.h"

int main() {

    LOG_INIT();
    LOGW("START");
    // cxAllocPoolTests();
    // cxArrayTests();
    // cxHmapTests();
    // cxStrTests();
    // cxQueueTests();
    // cx_list_tests();
    // cx_timer_tests();
    json_build_tests();
    LOGW("END");
}

