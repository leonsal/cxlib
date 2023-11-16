#include <stdio.h>

#define cx_log_name logger
#define cx_log_max_handlers 8
#define cx_log_tsafe
#include "cx_log.h"

// #define cx_log_name xxx
// #define cx_log_max_handlers 8
// #define cx_log_tsafe
// #include "cx_log.h"

int main() {

    logger log = logger_init();
    logger_set_flags(&log, CX_LOG_FLAG_TIME | CX_LOG_FLAG_US | CX_LOG_FLAG_COLOR);
    logger_add_handler(&log, logger_console_handler, NULL, CX_LOG_DEBUG);
    logger_deb(&log, "Log initialized OK: %s %d %f", "df", 1, 2.3);
    logger_info(&log, "Log info: %s %d %f", "xxx", 1, 2.3);
    logger_warn(&log, "Log warn: %s %d %f", "xxx", 1, 2.3);
    logger_error(&log, "Log error: %s %d %f", "xxx", 1, 2.3);
    logger_fatal(&log, "Log fatal: %s %d %f", "xxx", 1, 2.3);
    return 0;
}


// #include <stdio.h>
// #include <assert.h>
// #include "alloc.h"
// #include "array.h"
// #include "hmap.h"
// #include "string.h"
// #include "queue.h"
// #include "chan.h"
//
// int main() {
//
//     cxAllocBlockTests();
//     cxArrayTests();
//     cxHmapTests();
//     cxStrTests();
//     cxQueueTests();
//     cxChanTests();
//     return 0;
// }
//
