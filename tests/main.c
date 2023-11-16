#include <stdio.h>

#define cx_log_name logger
#define cx_log_max_handlers 8
#define cx_log_tsafe
#include "cx_log.h"

int main() {

    logger log = logger_init();
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
