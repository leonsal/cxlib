#include <asm-generic/errno.h>
#include "cx_pool_allocator.h"

#include "logger.h"
#include "alloc.h"
#include "array.h"
#include "hmap.h"
#include "string.h"
#include "queue.h"
#include "list.h"
#include "timer.h"
#include "var.h"
#include "json_build.h"
#include "json_parse.h"

int main() {

    LOG_INIT();
    LOGW("START");
    cxAllocPoolTests();
    cxArrayTests();
    cxHmapTests();
    cxStrTests();
    cxQueueTests();
    cx_list_tests();
    cx_timer_tests();
    cx_var_tests();
    json_build_tests();
    json_parse_tests();
    LOGW("END");
}

