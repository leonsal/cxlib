#include <stdio.h>
#include <asm-generic/errno.h>
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

// typedef struct url_{char data[32];} url;
// #define cx_hmap_name map
// #define cx_hmap_key url
// #define cx_hmap_val  int
// #define cx_hmap_implement
// #include "cx_hmap.h"

int main() {

    // map m1 = map_init(17);
    // url k1 = {.data="123"};
    // map_set(&m1, k1, 10);
    //
    // url k2 = {.data="123"};
    // int* v = map_get(&m1, k2);


    LOG_INIT();
    LOGW("START");
    cxAllocPoolTests();
    cxArrayTests();
    cxHmapTests();
    cxStrTests();
    cxQueueTests();
    LOGW("END");
}


