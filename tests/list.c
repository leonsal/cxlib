#include <stdio.h>
#include <time.h>

#include "cx_alloc.h"
#include "cx_pool_allocator.h"
#include "util.h"
#include "list.h"
#include "logger.h"

#define cx_list_name list
#define cx_list_type uint64_t
#define cx_list_implement
#define cx_list_static
#define cx_list_error_handler(msg,func)\
    printf("CXLIST ERROR:%s at %s\n", msg, func);abort()
#define cx_list_instance_allocator
#include "cx_list.h"

void cx_list_tests(void) {

    // Use default 'malloc/free' allocator
    cx_list_test(cxDefaultAllocator());

    // Use pool allocator
    CxPoolAllocator* pa = cx_pool_allocator_create(4*1024, NULL);
    cx_list_test(cx_pool_allocator_iface(pa));
    cx_pool_allocator_destroy(pa);
}

void cx_list_test(const CxAllocator* alloc) {

    LOGI("list. alloc:%p", alloc);
    list l1 = list_init(alloc);
    CHK(list_empty(&l1));
    CHK(list_count(&l1) == 0);
    list_free(&l1);
    list_clear(&l1);

    // Push elements at back
    const size_t size = 128;
    for (size_t i = 0; i < size; i++) {
        list_push(&l1, i);
    }
    CHK(list_empty(&l1) == false);
    CHK(list_count(&l1) == size);
    // Pop elements from the front and check
    for (size_t i = 0; i < size; i++) {
        uint64_t v = list_popf(&l1);
        CHK((size_t)v ==  v);
    }
    CHK(list_empty(&l1));
    CHK(list_count(&l1) == 0);

    // Push elements at front
    for (size_t i = 0; i < size; i++) {
        list_pushf(&l1, i * 2);
    }
    CHK(list_empty(&l1) == false);
    CHK(list_count(&l1) == size);
    // Pop elements from the back and check
    for (size_t i = 0; i < size; i++) {
        uint64_t v = list_pop(&l1);
        CHK(v == i * 2.0);
    }
    CHK(list_empty(&l1));
    CHK(list_count(&l1) == 0);

    // Push elements at back
    for (size_t i = 0; i < size; i++) {
        list_push(&l1, i * 3);
    }
    CHK(list_empty(&l1) == false);
    CHK(list_count(&l1) == size);
    // Iterates forward and check
    list_iter iter;
    ssize_t idx = 0;
    uint64_t* curr = list_first(&l1, &iter);
    while (curr) {
        CHK(*curr == idx * 3);
        idx++;
        curr = list_next(&iter);
    }
    CHK(idx == size);
    // Iterates backward and check
    idx = size-1;
    curr = list_last(&l1, &iter);
    while (curr) {
        CHK(*curr == idx * 3);
        idx--;
        curr = list_prev(&iter);
    }
    CHK(idx == -1);

    // Clears the list (keeps allocated memory)
    list_clear(&l1);
    CHK(list_empty(&l1));
    CHK(list_count(&l1) == 0);

    // Push elements at back
    for (size_t i = 0; i < size; i++) {
        list_push(&l1, i * 4);
    }
    CHK(list_empty(&l1) == false);
    CHK(list_count(&l1) == size);
    // Changes all elements using iterator
    curr = list_first(&l1, &iter);
    while (curr) {
        *curr = *curr * 2;
        curr = list_next(&iter);
    }
    // Pops elements from the front and check values
    for (size_t i = 0; i < size; i++) {
        uint64_t v = list_popf(&l1);
        CHK(v ==  i * 4 * 2);
    }

    // Frees the list
    list_free(&l1);
    CHK(list_empty(&l1));
    CHK(list_count(&l1) == 0);

    // Insert before first
    list_push(&l1, 1);
    list_push(&l1, 2);
    curr = list_first(&l1, &iter);
    list_ins_before(&iter, 0);
    // Check
    curr = list_first(&l1, &iter);
    CHK(list_count(&l1) == 3);
    CHK(list_popf(&l1) == 0);
    CHK(list_popf(&l1) == 1);
    CHK(list_popf(&l1) == 2);

    // Insert after first
    list_push(&l1, 1);
    list_push(&l1, 2);
    curr = list_first(&l1, &iter);
    list_ins_after(&iter, 10);
    // Check
    curr = list_first(&l1, &iter);
    CHK(list_count(&l1) == 3);
    CHK(list_popf(&l1) == 1);
    CHK(list_popf(&l1) == 10);
    CHK(list_popf(&l1) == 2);

    // Insert after last
    list_push(&l1, 1);
    list_push(&l1, 2);
    curr = list_last(&l1, &iter);
    list_ins_after(&iter, 3);
    // Check
    curr = list_first(&l1, &iter);
    CHK(list_count(&l1) == 3);
    CHK(list_popf(&l1) == 1);
    CHK(list_popf(&l1) == 2);
    CHK(list_popf(&l1) == 3);

    // Clears the list (keeps allocated memory)
    list_clear(&l1);
    CHK(list_empty(&l1));
    CHK(list_count(&l1) == 0);

    // Push elements at back
    for (size_t i = 0; i < size; i++) {
        list_push(&l1, i);
    }
    CHK(list_count(&l1) == size);
    // Insert elements before
    curr = list_first(&l1, &iter);
    while (curr) {
        list_ins_before(&iter, *curr);
        curr = list_next(&iter);
    }
    CHK(list_count(&l1) == size*2);
    // Checks
    curr = list_first(&l1, &iter);
    idx = 0;
    while (curr) {
        assert(*curr == idx);
        curr = list_next(&iter);
        assert(*curr == idx);
        curr = list_next(&iter);
        idx++;
    }

    // Clears the list (keeps allocated memory)
    list_clear(&l1);
    CHK(list_empty(&l1));
    CHK(list_count(&l1) == 0);

    // Push elements at back
    for (size_t i = 0; i < size; i++) {
        list_push(&l1, i);
    }
    CHK(list_count(&l1) == size);
    // Insert elements after
    curr = list_first(&l1, &iter);
    while (curr) {
        list_ins_after(&iter, *curr * 2);
        list_next(&iter); // skip inserted element
        curr = list_next(&iter);
    }
    CHK(list_count(&l1) == size*2);
    // Checks
    curr = list_first(&l1, &iter);
    idx = 0;
    while (curr) {
        CHK(*curr == idx);
        curr = list_next(&iter);
        CHK(*curr == idx * 2);
        curr = list_next(&iter);
        idx++;
    }
    CHK(idx == size);

    // Clears the list (keeps allocated memory)
    list_clear(&l1);
    CHK(list_empty(&l1));
    CHK(list_count(&l1) == 0);

    // Delete first
    list_push(&l1, 1);
    list_push(&l1, 2);
    list_push(&l1, 3);
    curr = list_first(&l1, &iter);
    list_del(&iter, true);
    // Check
    CHK(list_count(&l1) == 2);
    CHK(list_popf(&l1) == 2);
    CHK(list_popf(&l1) == 3);

    // Delete middle 
    list_push(&l1, 1);
    list_push(&l1, 2);
    list_push(&l1, 3);
    curr = list_first(&l1, &iter);
    curr = list_next(&iter);
    list_del(&iter, true);
    // Check
    CHK(list_count(&l1) == 2);
    CHK(list_popf(&l1) == 1);
    CHK(list_popf(&l1) == 3);

    // Delete last 
    list_push(&l1, 1);
    list_push(&l1, 2);
    list_push(&l1, 3);
    curr = list_first(&l1, &iter);
    curr = list_next(&iter);
    curr = list_next(&iter);
    list_del(&iter, true);
    // Check
    CHK(list_count(&l1) == 2);
    CHK(list_popf(&l1) == 1);
    CHK(list_popf(&l1) == 2);

    // Push elements at back
    for (size_t i = 0; i < size; i++) {
        list_push(&l1, i);
    }
    CHK(list_count(&l1) == size);
    // Delete even elements
    curr = list_first(&l1, &iter);
    while (curr) {
        if (*curr % 2 == 0) {
            curr = list_del(&iter, true);
        } else {
            curr = list_next(&iter);
        }
    }
    // Check
    CHK(list_count(&l1) == size/2);
    uint64_t expected = 1;
    while (list_count(&l1)) {
         uint64_t v = list_popf(&l1);
         CHK(v == expected);
         expected += 2;
    }

    // Clears the list (keeps allocated memory)
    list_clear(&l1);
    CHK(list_empty(&l1));
    CHK(list_count(&l1) == 0);

    // Push elements at back
    for (size_t i = 0; i < size; i++) {
        list_push(&l1, i * 10);
    }
    CHK(list_count(&l1) == size);
    // Finds first element
    curr = list_find(&l1, 0, &iter);
    CHK(*curr == 0);
    CHK(list_prev(&iter) == NULL);
    CHK(*list_next(&iter) == 10);
    CHK(*list_prev(&iter) == 0);
    // Finds last element
    curr = list_find(&l1, (size-1)*10, &iter);
    CHK(*curr = (size-1)*10);
    CHK(list_next(&iter) == NULL);
    CHK(*list_prev(&iter) == (size-2)*10);
    // Finds non existent element
    curr = list_find(&l1, size*10, &iter);
    CHK(curr == NULL);

    list_free(&l1);
}


