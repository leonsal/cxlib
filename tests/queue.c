#include <stdlib.h>
#include <assert.h>
#include "cx_alloc_pool.h"
#include "queue.h"

#define cx_queue_name queue
#define cx_queue_type int
#define cx_queue_cap 16
#define cx_queue_static
#define cx_queue_inline
#define cx_queue_instance_allocator
#define cx_queue_implement
#include "cx_queue.h"

#include "logger.h"

void cxQueueTests(void) {

    // Use default allocator
    cxQueueTest(cxDefaultAllocator());

    // Use pool allocator
    CxAllocPool* ba = cxAllocPoolCreate(4*1024, NULL);
    cxQueueTest(cxAllocPoolGetAllocator(ba));
    cxAllocPoolDestroy(ba);
}

void cxQueueTest(const CxAllocator* alloc) {

    LOGI("queue. alloc=%p", alloc);

    // push back
    queue q1 = queue_init(cxDefaultAllocator());
    size_t size = 1000;
    for (size_t i = 0; i < size; i++) {
        queue_pushb(&q1, i);
    }
    assert(queue_len(&q1) == size);
    assert(*queue_front(&q1) == 0);
    assert(*queue_back(&q1) == size-1);
    // pop front
    for (size_t i = 0; i < size; i++) {
        assert(queue_popf(&q1) == i);
    }
    queue_free(&q1);

    size = 10;
    for (size_t i = 0; i < size; i++) {
        queue_pushb(&q1, i);
    }
    for (size_t i = 0; i < size/2; i++) {
        assert(queue_popf(&q1) == i);
    }
    for (size_t i = size; i < 2*size; i++) {
        queue_pushb(&q1, i);
    }
    for (size_t i = size; i < size/2; i++) {
        assert(queue_popf(&q1) == i);
    }
    queue_free(&q1);
}


