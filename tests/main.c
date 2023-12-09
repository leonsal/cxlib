#include <stdio.h>
#include <stdio.h>
#include <assert.h>
#include "cx_alloc.h"
#include "cx_alloc_pool.h"

#include "logger.h"
#include "alloc.h"
#include "array.h"
#include "hmap.h"
#include "string.h"
#include "queue.h"

// const CxAllocator* testAllocator;
//
// #define cx_array_name arri32
// #define cx_array_type int
// #define cx_array_allocator testAllocator
// #define cx_array_implement
// #include "cx_array.h"

#define IFACE(IF,MT,...) IF->MT(IF->ctx,__VA_ARGS__)

int main() {
    LOG_INIT();
    LOGW("START");

    cxAllocPoolTest(10, 256, 40);

    LOGW("END");

    // // Use pool allocator
    // CxAllocPool* pa = cxAllocPoolCreate(4*1024, NULL);
    // testAllocator = cxAllocPoolGetAllocator(pa);
    // arri32 a1 = {};
    // arri32_push(&a1, 2);
    // arri32_free(&a1);
    // cxAllocPoolDestroy(pa);
    //
    // LOG_INIT();
    // LOGW("START");
    // cxAllocBlockTests();
    // cxArrayTests();
    // cxHmapTests();
    // cxStrTests();
    // cxQueueTests();
    // LOGW("END");
    return 0;
}


// #include "logger.h"
// #include "alloc.h"
// #include "array.h"
// #include "hmap.h"
// #include "string.h"
// #include "queue.h"
//
// #include "chan.h"
//
// #define cx_chan_name chb
// #define cx_chan_type int
// #define cx_chan_cap  8
// #define cx_chan_static
// #define cx_chan_implement
// #include "cx_chan.h"
//
// static void* writer(void* d) {
//
//     LOGI("writer started");
//     chb* c = d;
//     nanosleep(&(struct timespec){.tv_sec=1}, NULL);
//     chb_send(c, 1);
//     LOGI("writer ended");
//     return NULL;
// }
//
// int main() {
//
//     LOG_INIT();
//     LOGW("START");
//     chb c1 = chb_init(1);
//     chb c2 = chb_init(1);
//     chb_send(&c1, 1);
//     chb_send(&c2, 2);
//     //chb_send(&c2, 2);
//     // TODO THESE CHANNELS ARE NOT READY TO WRITE...
//     //
//     pthread_t t;
//     //pthread_create(&t, NULL, writer, &c1);
//     //pthread_create(&t, NULL, writer, &c2);
//     //int sel = chb_select(2, (int[]){chb_rfd(&c1), chb_rfd(&c2)}, 0, NULL);
//     int sel = chb_select(0, NULL, 2, (int[]){chb_wfd(&c1), chb_wfd(&c2)});
//     LOGI("sel:%d", sel);
//
//
//     // cxAllocBlockTests();
//     // cxArrayTests();
//     // cxHmapTests();
//     // cxStrTests();
//     // cxQueueTests();
//     //cxChanTests();
//     LOGW("END");
// }


