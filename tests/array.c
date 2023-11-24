#include <stdio.h>
#include <assert.h>
#include <time.h>
#include "cx_alloc.h"
#include "cx_alloc_pool.h"
#include "array.h"

#define cx_array_name cxarray
#define cx_array_type int
#define cx_array_implement
#define cx_array_static
#define cx_array_cap 32
#define cx_array_error_handler(msg,func)\
    printf("CXARRAY ERROR:%s at %s\n", msg, func);abort()
#define cx_array_instance_allocator
#include "cx_array.h"

#include "logger.h"

// Sort function
static int sort_int_desc(const int* v1, const int* v2) {
    return *v2 > *v1;
}

void cxArrayTests(void) {

    // Use default allocator
    const size_t size = 100000;
    cxArrayTest(size, cxDefaultAllocator());

    // Use pool allocator
    CxAllocPool* ba = cxAllocPoolCreate(4*1024, NULL);
    cxArrayTest(size, cxAllocPoolGetAllocator(ba));
    cxAllocPoolDestroy(ba);
}

void cxArrayTest(size_t size, const CxAllocator* alloc) {

    LOGI("array. size=%lu alloc:%p", size, alloc);
    cxarray a1 = cxarray_init(alloc);
    cxarray a2 = cxarray_init(alloc);

    // push
    assert(cxarray_len(&a1) == 0);
    assert(cxarray_cap(&a1) == 0);
    assert(cxarray_empty(&a1));
    cxarray_free(&a1);
    for (size_t i = 0; i < size; i++) {
        cxarray_push(&a1, i);
    }
    assert(cxarray_len(&a1) == size);
    assert(!cxarray_empty(&a1));
    assert(cxarray_last(&a1) == size-1);
    // Check
    for (size_t i = 0; i < size; i++) {
        assert(*cxarray_at(&a1, i) == i);
        assert(a1.data[i] == i);
    }
    // Pop
    for (size_t i = 0; i < size; i++) {
        assert(cxarray_pop(&a1) == size-i-1);
    }
    cxarray_free(&a1);
    assert(cxarray_len(&a1) == 0);
    assert(cxarray_cap(&a1) == 0);

    // Set length and fill array
    cxarray_setlen(&a1, size);
    assert(cxarray_len(&a1) == size);
    assert(!cxarray_empty(&a1));
    for (size_t i = 0; i < size; i++) {
        a1.data[i] = i * 2;
    }
    // Check
    for (size_t i = 0; i < size; i++) {
        assert(*cxarray_at(&a1, i) == i*2);
        assert(a1.data[i] == i*2);
    }
    // Clone and check
    a2 = cxarray_clone(&a1);
    assert(cxarray_len(&a2) == cxarray_len(&a1));
    for (size_t i = 0; i < size; i++) {
        assert(*cxarray_at(&a2, i) == *cxarray_at(&a1, i));
        assert(a2.data[i] == a1.data[i]);
    }
    cxarray_free(&a1);
    assert(cxarray_len(&a1) == 0);
    assert(cxarray_cap(&a1) == 0);
    cxarray_free(&a2);
    assert(cxarray_len(&a2) == 0);
    assert(cxarray_cap(&a2) == 0);
    
    // Set capacity and fill array
    cxarray_setcap(&a1, size);
    assert(cxarray_len(&a1) == 0);
    assert(cxarray_cap(&a1) == size);
    assert(cxarray_empty(&a1));
    for (size_t i = 0; i < size; i++) {
        cxarray_push(&a1, i*3);
    }
    assert(cxarray_len(&a1) == size);
    assert(cxarray_cap(&a1) == size);
    for (size_t i = 0; i < size; i++) {
        assert(*cxarray_at(&a1, i) == i*3);
        assert(a1.data[i] == i*3);
    }
    cxarray_clear(&a1);
    assert(cxarray_len(&a1) == 0);
    assert(cxarray_cap(&a1) == size);
    cxarray_free(&a1);
    assert(cxarray_cap(&a1) == 0);

    // reserve
    cxarray_push(&a1, 1);
    size_t cap = cxarray_cap(&a1);
    cxarray_reserve(&a1, 2);
    assert(cxarray_cap(&a1) == cap);

    // insn
    int buf[] = {1,2,3,4,5,6,7,8,9};
    size_t insCount = 5;
    cxarray_free(&a1);
    cxarray_push(&a1, 100);
    cxarray_insn(&a1, buf, insCount, 0);
    assert(cxarray_len(&a1) == 1+insCount);
    for (size_t i = 0; i < insCount; i++) {
        assert(*cxarray_at(&a1, i) == buf[i]);
        assert(a1.data[i] == buf[i]);
    }
    assert(a1.data[insCount] == 100);

    // ins
    cxarray_free(&a1);
    cxarray_push(&a1, 0);
    cxarray_push(&a1, 3);
    cxarray_ins(&a1, 1, 1);
    cxarray_ins(&a1, 2, 2);
    for (size_t i = 0; i < 4; i++) {
        assert(*cxarray_at(&a1, i) == i);
        assert(a1.data[i] == i);
    }

    // insa
    cxarray_free(&a1);
    cxarray_free(&a2);
    cxarray_push(&a1, 0);
    cxarray_push(&a1, 3);
    cxarray_push(&a2, 1);
    cxarray_push(&a2, 2);
    cxarray_insa(&a1, &a2, 1);
    for (size_t i = 0; i < 4; i++) {
        assert(*cxarray_at(&a1, i) == i);
        assert(a1.data[i] == i);

    }

    // pushn
    size_t buf_size = sizeof(buf)/sizeof(int);
    cxarray_free(&a1);
    cxarray_push(&a1, 0);
    cxarray_pushn(&a1, buf, buf_size);
    assert(cxarray_len(&a1) == buf_size+1);
    for (size_t i = 0; i < buf_size+1; i++) {
        assert(*cxarray_at(&a1, i) == i);
        assert(a1.data[i] == i);
    }

    // pusha
    cxarray_free(&a1);
    cxarray_free(&a2);
    cxarray_push(&a1, 100);
    cxarray_push(&a2, 0);
    cxarray_push(&a2, 1);
    cxarray_pusha(&a1, &a2);
    assert(cxarray_len(&a1) == 3);
    assert(a1.data[0] == 100 && a1.data[1] == 0 && a1.data[2] == 1);
    cxarray_free(&a1);
    cxarray_free(&a2);

    // deln
    cxarray_free(&a1);
    const size_t bufSize = sizeof(buf)/sizeof(buf[0]);
    cxarray_pushn(&a1, buf, bufSize);
    cxarray_deln(&a1, 1, 7);
    assert(a1.data[0] == 1);
    assert(a1.data[1] == 9);
    assert(cxarray_len(&a1) == 2);
    
    // Sorts data in descending order
    cxarray_free(&a1);
    cxarray_pushn(&a1, buf, bufSize);
    cxarray_sort(&a1, sort_int_desc);
    for (size_t i = 0; i < bufSize; i++) {
        assert(*cxarray_at(&a1, i) == bufSize-i);
        assert(a1.data[i] == bufSize-i);
    }
    // Finds element
    assert(cxarray_find(&a1, 1) == 8);
    assert(cxarray_find(&a1, 0) == -1);
    cxarray_free(&a1);
}

