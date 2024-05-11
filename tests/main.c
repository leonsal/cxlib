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
#include "tpool.h"
#include "bench_hmap.h"
#include "cx_error.h"

#include "cx_logger.h"
#include "log.h"

CxLogger* g_logger = NULL;

int main() {

    g_logger = cx_logger_new(NULL, "CXLIB_TESTS");
    cx_logger_set_flags(g_logger, CxLoggerFlagDate|CxLoggerFlagTime|CxLoggerFlagUs|CxLoggerFlagColor);
    cx_logger_add_handler(g_logger, cx_logger_console_handler, NULL);
    // cx_logger_log(g_logger, CxLoggerDebug, "message:%d", 34);
    GLOGD("debug message: %s", "tests");
    GLOGI("debug message: %s", "info");
    GLOGW("debug message: %s", "warn");
    GLOGE("debug message: %s", "err");


    // CxError
    CxError err1 = CXERROR(1, "static error message");
    CXERROR_FREE(err1);
    CxError err2 = CXERRORF(1, "dynamic error message: %d / %f / %s", 10, 3.1415, "string");
    CXERROR_FREE(err2);

    LOG_INIT();
    LOGW("START");
    cxAllocPoolTests();
    test_array();
    test_hmap();
    cxStrTests();
    cxQueueTests();
    cx_list_tests();
    //// cx_timer_tests();
    cx_var_tests();
    ////json_build_tests();
    json_parse_tests();
    //bench_hmap();
    tpool_tests();
    LOGW("END");
    cx_logger_del(g_logger);
}

