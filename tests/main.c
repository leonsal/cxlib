#include "cx_logger.h"
#include "logger.h"
#include "registry.h"

CxLogger* g_logger = NULL;

int main(int argc, char* argv[]) {

    // Initialize global logger
    g_logger = cx_logger_new(NULL, NULL);
    cx_logger_set_flags(g_logger, CxLoggerFlagTime|CxLoggerFlagUs|CxLoggerFlagColor);
    cx_logger_add_handler(g_logger, cx_logger_console_handler, NULL);

    // Get optional test name
    const char* test_name = NULL;
    TestFunc fn = NULL;
    if (argc > 1) {
        test_name = argv[1];
        fn = reg_get_test(test_name);
        if (fn == NULL) {
            printf("INVALID TEST NAME: %s\n", test_name);
            return 1;
        }
    }

    // Run test(s)
    LOGI("START");
    if (test_name) {
        fn();
    } else {
        size_t test_count = reg_get_count();
        for (size_t idx = 0; idx < test_count; idx++) {
            test_name = reg_get_name(idx);
            fn = reg_get_test(test_name);
            fn();
        }
    }
    LOGI("END");
    cx_logger_del(g_logger);
}

