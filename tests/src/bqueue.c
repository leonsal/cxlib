#include <stdint.h>
#include <stdbool.h>
#include "cx_logger.h"
#include "cx_bqueue.h"
#include "logger.h"
#include "bqueue.h"

static bool chk_block(uint8_t* b, size_t size, uint8_t val) {

    for (size_t i = 0; i < size; i++) {
        if (b[i] != val) {
            return false;
        }
    }
    return true;
}
void test_bqueue(size_t count, const CxAllocator* alloc) {

    LOGI("%s: count=%lu alloc=%p", __func__, count, alloc);

    CxBqueue* q = cx_bqueue_new(alloc);
    CXCHK(cx_bqueue_count(q) == 0);

    // Creates blocks
    void* b = cx_bqueue_put(q, 10);
    CXCHK(cx_bqueue_count(q) == 1);
    memset(b, 10, 10);

    b = cx_bqueue_put(q, 20);
    CXCHK(cx_bqueue_count(q) == 2);
    memset(b, 20, 20);

    b = cx_bqueue_put(q, 30);
    CXCHK(cx_bqueue_count(q) == 3);
    memset(b, 30, 30);

    // Remove first 2 blocks
    size_t bsize;
    b = cx_bqueue_get(q, &bsize);
    CXCHK(cx_bqueue_count(q) == 2);
    CXCHK(bsize == 10);
    CXCHK(chk_block(b, 10, 10));

    b = cx_bqueue_get(q, &bsize);
    CXCHK(cx_bqueue_count(q) == 1);
    CXCHK(bsize == 20);
    CXCHK(chk_block(b, 20, 20));

    // Insert another block
    b = cx_bqueue_put(q, 8);
    CXCHK(cx_bqueue_count(q) == 2);
    memset(b, 8, 8);

    // Remove
    b = cx_bqueue_get(q, &bsize);
    CXCHK(cx_bqueue_count(q) == 1);
    CXCHK(bsize == 30);
    CXCHK(chk_block(b, 30, 30));

    b = cx_bqueue_get(q, &bsize);
    CXCHK(cx_bqueue_count(q) == 0);
    CXCHK(bsize == 8);
    CXCHK(chk_block(b, 8, 8));

    cx_bqueue_pstats(q);
    cx_bqueue_del(q);
}

