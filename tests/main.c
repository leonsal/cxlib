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

#include "bench_hmap.h"

int main() {

    LOG_INIT();
    LOGW("START");
    cxAllocPoolTests();
    // test_array();
    // test_hmap();
    // cxStrTests();
    // cxQueueTests();
    // cx_list_tests();
    // cx_timer_tests();
    // cx_var_tests();
    // json_build_tests();
    // json_parse_tests();

    bench_hmap();
    LOGW("END");
}

