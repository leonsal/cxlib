#include <stdlib.h>
#include <assert.h>
#include "queue.h"

#define cx_queue_name queue
#define cx_queue_type int
#define cx_queue_cap 16
#define cx_queue_static
#define cx_queue_inline
#define cx_queue_allocator
#define cx_queue_implement
#include "cx_queue.h"


void cxQueueTest(const CxAllocator* alloc) {

    queue q1 = queue_init(cxDefaultAllocator());
    size_t size = 1000;
    for (size_t i = 0; i < size; i++) {
        queue_pushb(&q1, i);
    }
    assert(queue_len(&q1) == size);
    for (size_t i = 0; i < size; i++) {
        assert(queue_popf(&q1) == i);
    }
    queue_free(&q1);






}


