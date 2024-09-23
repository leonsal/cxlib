#include "cx_pool_allocator.h"
#include "cx_error.h"
#include "cx_logger.h"

#include "logger.h"
#include "alloc.h"
#include "array.h"
#include "hmap.h"
#include "string.h"
#include "cqueue.h"
#include "list.h"
#include "timer.h"
#include "var.h"
#include "json_build.h"
#include "json_parse.h"
#include "tpool.h"
#include "bqueue.h"
#include "bench_hmap.h"

CxLogger* g_logger = NULL;

int main() {

    // Initialize global logger
    g_logger = cx_logger_new(NULL, NULL);
    cx_logger_set_flags(g_logger, CxLoggerFlagTime|CxLoggerFlagUs|CxLoggerFlagColor);
    cx_logger_add_handler(g_logger, cx_logger_console_handler, NULL);

    LOGW("START");
    test_bqueue(10, NULL);

    cxAllocPoolTests();
    test_array();
    test_hmapii(1000, 0, NULL);
    test_hmapss(1000, 0, NULL);
    test_hmapcc(1000, 0, NULL);
    cxStrTests();
    cx_list_tests();
    cx_var_tests();
    json_build_tests();
    json_parse_tests();
    cxQueueTests();
    //// //// cx_timer_tests();
    //// //bench_hmap();
    tpool_tests();
    LOGW("END");

    cx_logger_del(g_logger);
}

