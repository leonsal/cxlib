#include <stdio.h>
#include <stdio.h>
#include <assert.h>
#include "cx_alloc.h"
#include "cx_alloc_pool.h"

#include "logger.h"
// #include "alloc.h"
// #include "array.h"
// #include "hmap.h"
// #include "string.h"
#include "queue.h"

#define cx_queue_name qu64
#define cx_queue_type uint64_t
#include "cx_queue.h"

int main() {

    LOG_INIT();
    LOGW("START");

    qu64 q = qu64_init(5);
    assert(qu64_cap(&q) == 5);
    assert(qu64_len(&q) == 0);


    // cxAllocPoolTests();
    // cxArrayTests();
    // cxHmapTests();
    // cxStrTests();
    // cxQueueTests();
    // //cxChanTests();
    // LOGW("END");
}


