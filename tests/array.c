#include <stdio.h>
#include <time.h>

#include "cx_alloc.h"
#include "cx_pool_allocator.h"
#include "registry.h"
#include "util.h"

// Define array of integers
#define cx_array_name cxarray
#define cx_array_type int
#define cx_array_implement
#define cx_array_static
#define cx_array_cap 32
#define cx_array_error_handler(msg,func)     printf("CXARRAY ERROR:%s at %s\n", msg, func);abort()
#define cx_array_instance_allocator
#include "cx_array.h"

// Define array of pointers to allocated C strings
#define cx_array_name                       arrc
#define cx_array_type                       char*
#define cx_array_cmp_el(el1,el2,s)          strcmp(*el1,*el2)
#define cx_array_free_el(el)                free(*el)
#define cx_array_static
#define cx_array_instance_allocator
#define cx_array_error_handler(msg,func)    printf("CXARRAY ERROR:%s at %s\n", msg, func);abort()
#define cx_array_implement
#include "cx_array.h"

// Define cx str used for next array
#define cx_str_name cxstr
#define cx_str_static
#define cx_stry_instance_allocator
#define cx_str_implement
#include "cx_str.h"

// Define array of cx_str
#define cx_array_name                       arrs
#define cx_array_type                       cxstr
#define cx_array_cmp_el(el1,el2,s)          cxstr_cmps(el1, el2)
#define cx_array_free_el(el)                cxstr_free(el)
#define cx_array_static
#define cx_array_instance_allocator
#define cx_array_error_handler(msg,func)    printf("CXARRAY ERROR:%s at %s\n", msg, func);abort()
#define cx_array_implement
#include "cx_array.h"

#include "logger.h"

// Sort function
static int sort_int_desc(int* v1, int* v2) {
    return *v2 > *v1;
}


void test_array_int(size_t size, const CxAllocator* alloc) {

    LOGI("%s: size=%lu alloc:%p", __func__, size, alloc);
    cxarray a1 = cxarray_init(alloc);
    cxarray a2 = cxarray_init(alloc);

    // push
    CHK(cxarray_len(&a1) == 0);
    CHK(cxarray_cap(&a1) == 0);
    CHK(cxarray_empty(&a1));
    cxarray_free(&a1);
    for (size_t i = 0; i < size; i++) {
        cxarray_push(&a1, i);
    }
    CHK(cxarray_len(&a1) == size);
    CHK(!cxarray_empty(&a1));
    CHK(cxarray_last(&a1) == size-1);
    // Check
    for (size_t i = 0; i < size; i++) {
        CHK(*cxarray_at(&a1, i) == i);
        CHK(a1.data[i] == i);
    }
    // Pop
    for (size_t i = 0; i < size; i++) {
        CHK(cxarray_pop(&a1) == size-i-1);
    }
    cxarray_free(&a1);
    CHK(cxarray_len(&a1) == 0);
    CHK(cxarray_cap(&a1) == 0);

    // Set length and fill array
    cxarray_setlen(&a1, size);
    CHK(cxarray_len(&a1) == size);
    CHK(!cxarray_empty(&a1));
    for (size_t i = 0; i < size; i++) {
        a1.data[i] = i * 2;
    }
    // Check
    for (size_t i = 0; i < size; i++) {
        CHK(*cxarray_at(&a1, i) == i*2);
        CHK(a1.data[i] == i*2);
    }
    // Clone and check
    a2 = cxarray_clone(&a1);
    CHK(cxarray_len(&a2) == cxarray_len(&a1));
    for (size_t i = 0; i < size; i++) {
        CHK(*cxarray_at(&a2, i) == *cxarray_at(&a1, i));
        CHK(a2.data[i] == a1.data[i]);
    }
    cxarray_free(&a1);
    CHK(cxarray_len(&a1) == 0);
    CHK(cxarray_cap(&a1) == 0);
    cxarray_free(&a2);
    CHK(cxarray_len(&a2) == 0);
    CHK(cxarray_cap(&a2) == 0);
    
    // Set capacity and fill array
    cxarray_setcap(&a1, size);
    CHK(cxarray_len(&a1) == 0);
    CHK(cxarray_cap(&a1) == size);
    CHK(cxarray_empty(&a1));
    for (size_t i = 0; i < size; i++) {
        cxarray_push(&a1, i*3);
    }
    CHK(cxarray_len(&a1) == size);
    CHK(cxarray_cap(&a1) == size);
    for (size_t i = 0; i < size; i++) {
        CHK(*cxarray_at(&a1, i) == i*3);
        CHK(a1.data[i] == i*3);
    }
    cxarray_clear(&a1);
    CHK(cxarray_len(&a1) == 0);
    CHK(cxarray_cap(&a1) == size);
    cxarray_free(&a1);
    CHK(cxarray_cap(&a1) == 0);

    // reserve
    cxarray_push(&a1, 1);
    size_t cap = cxarray_cap(&a1);
    cxarray_reserve(&a1, 2);
    CHK(cxarray_cap(&a1) == cap);

    // insn
    int buf[] = {1,2,3,4,5,6,7,8,9};
    size_t insCount = 5;
    cxarray_free(&a1);
    cxarray_push(&a1, 100);
    cxarray_insn(&a1, buf, insCount, 0);
    CHK(cxarray_len(&a1) == 1+insCount);
    for (size_t i = 0; i < insCount; i++) {
        CHK(*cxarray_at(&a1, i) == buf[i]);
        CHK(a1.data[i] == buf[i]);
    }
    CHK(a1.data[insCount] == 100);

    // ins
    cxarray_free(&a1);
    cxarray_push(&a1, 0);
    cxarray_push(&a1, 3);
    cxarray_ins(&a1, 1, 1);
    cxarray_ins(&a1, 2, 2);
    for (size_t i = 0; i < 4; i++) {
        CHK(*cxarray_at(&a1, i) == i);
        CHK(a1.data[i] == i);
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
        CHK(*cxarray_at(&a1, i) == i);
        CHK(a1.data[i] == i);

    }

    // pushn
    size_t buf_size = sizeof(buf)/sizeof(int);
    cxarray_free(&a1);
    cxarray_push(&a1, 0);
    cxarray_pushn(&a1, buf, buf_size);
    CHK(cxarray_len(&a1) == buf_size+1);
    for (size_t i = 0; i < buf_size+1; i++) {
        CHK(*cxarray_at(&a1, i) == i);
        CHK(a1.data[i] == i);
    }

    // pusha
    cxarray_free(&a1);
    cxarray_free(&a2);
    cxarray_push(&a1, 100);
    cxarray_push(&a2, 0);
    cxarray_push(&a2, 1);
    cxarray_pusha(&a1, &a2);
    CHK(cxarray_len(&a1) == 3);
    CHK(a1.data[0] == 100 && a1.data[1] == 0 && a1.data[2] == 1);
    cxarray_free(&a1);
    cxarray_free(&a2);

    // deln
    cxarray_free(&a1);
    const size_t bufSize = sizeof(buf)/sizeof(buf[0]);
    cxarray_pushn(&a1, buf, bufSize);
    cxarray_deln(&a1, 1, 7);
    CHK(a1.data[0] == 1);
    CHK(a1.data[1] == 9);
    CHK(cxarray_len(&a1) == 2);
    
    // Sorts data in descending order
    cxarray_free(&a1);
    cxarray_pushn(&a1, buf, bufSize);
    cxarray_sort(&a1, sort_int_desc);
    for (size_t i = 0; i < bufSize; i++) {
        CHK(*cxarray_at(&a1, i) == bufSize-i);
        CHK(a1.data[i] == bufSize-i);
    }
    // Finds element
    CHK(cxarray_find(&a1, 1) == 8);
    CHK(cxarray_find(&a1, 0) == -1);
    cxarray_free(&a1);
}

// Creates new string from number using the specified allocator
static char* newstr(size_t n, const CxAllocator* alloc) {

    char buf[32];
    snprintf(buf, sizeof(buf), "%zu", n);
    char* s = cx_alloc_malloc(alloc, strlen(buf) + 1);
    strcpy(s, buf);
    return s;
}

// Returns statically allocated string used for comparisons only
static char* num2str(size_t n) {

    static char buf[32];
    snprintf(buf, sizeof(buf), "%zu", n);
    return buf;
}

static size_t str2num(char* s) {

    return strtol(s, NULL, 10);
}

void test_array_str(size_t size, const CxAllocator* alloc) {

    LOGI("%s: size=%lu alloc:%p", __func__, size, alloc);
    arrc a = arrc_init(alloc);
    // push
    CHK(arrc_len(&a) == 0);
    CHK(arrc_cap(&a) == 0);
    CHK(arrc_empty(&a));
    arrc_free(&a);
    for (size_t i = 0; i < size; i++) {
        arrc_push(&a, newstr(i, alloc));
    }
    CHK(arrc_len(&a) == size);
    CHK(!arrc_empty(&a));
    CHK(strcmp(arrc_last(&a), num2str(size-1)) == 0);
    // Check
    for (size_t i = 0; i < size; i++) {
        CHK(strcmp(*arrc_at(&a, i), num2str(i)) == 0);
    }
    // Free all strings and array buffer
    arrc_free(&a);
    CHK(arrc_len(&a) == 0);
    CHK(arrc_cap(&a) == 0);
    CHK(arrc_empty(&a));

    // push
    for (size_t i = 0; i < size; i++) {
        arrc_push(&a, newstr(i, alloc));
    }
    CHK(arrc_len(&a) == size);
    // Delete odd elements
    size_t idx = 0;
    while (idx < arrc_len(&a)) {
        if (str2num(*arrc_at(&a, idx)) % 2 == 0) {
            idx++;
            continue;
        }
        arrc_del(&a, idx);
    }
    CHK(arrc_len(&a) == size/2);
    // Checks
    for (size_t idx = 0; idx < size/2; idx++) {
        CHK(str2num(*arrc_at(&a, idx)) == idx*2);
    }
    arrc_free(&a);
    CHK(arrc_len(&a) == 0);

    // push
    for (size_t i = 0; i < size; i++) {
        arrc_push(&a, newstr(i, alloc));
    }
    CHK(arrc_len(&a) == size);
    // set all elements
    for (size_t i = 0; i < size; i++) {
        arrc_set(&a, i, newstr(i*2, alloc));
    }
    // Check
    for (size_t i = 0; i < size; i++) {
        CHK(strcmp(*arrc_at(&a, i), num2str(i*2)) == 0);
    }
    arrc_free(&a);
}

void test_array_cxstr(size_t size, const CxAllocator* alloc) {

    LOGI("%s: size=%lu alloc:%p", __func__, size, alloc);
    arrs a = arrs_init(alloc);
    // push
    CHK(arrs_len(&a) == 0);
    CHK(arrs_cap(&a) == 0);
    CHK(arrs_empty(&a));
    arrs_free(&a);
    for (size_t i = 0; i < size; i++) {
        arrs_push(&a, cxstr_initc(num2str(i)));
    }
    CHK(arrs_len(&a) == size);
    CHK(!arrs_empty(&a));
    // Check
    for (size_t i = 0; i < size; i++) {
        CHK(cxstr_cmp(arrs_at(&a, i), num2str(i)) == 0);
    }
    // Free all strings and array buffer
    arrs_free(&a);
    CHK(arrs_len(&a) == 0);
    CHK(arrs_cap(&a) == 0);
    CHK(arrs_empty(&a));

    // push
    for (size_t i = 0; i < size; i++) {
        arrs_push(&a, cxstr_initc(num2str(i)));
    }
    CHK(arrs_len(&a) == size);
    // Delete odd elements
    size_t idx = 0;
    while (idx < arrs_len(&a)) {
        if (str2num(arrs_at(&a, idx)->data) % 2 == 0) {
            idx++;
            continue;
        }
        arrs_del(&a, idx);
    }
    CHK(arrs_len(&a) == size/2);
    // Checks
    for (size_t idx = 0; idx < size/2; idx++) {
        CHK(str2num(arrs_at(&a, idx)->data) == idx*2);
    }
    arrs_free(&a);
    CHK(arrs_len(&a) == 0);

    // push
    for (size_t i = 0; i < size; i++) {
        arrs_push(&a, cxstr_initc(num2str(i)));
    }
    CHK(arrs_len(&a) == size);
    // set all elements
    for (size_t i = 0; i < size; i++) {
        arrs_set(&a, i, cxstr_initc(num2str(i*2)));
    }
    // Check
    for (size_t i = 0; i < size; i++) {
        CHK(strcmp(arrs_at(&a, i)->data, num2str(i*2)) == 0);
    }
    arrs_free(&a);
}

void test_array(void) {

    // Use default allocator
    const size_t size = 1000;
    test_array_int(size, cx_def_allocator());
    test_array_str(size, cx_def_allocator());
    test_array_cxstr(size, cx_def_allocator());

    // Use pool allocator
    CxPoolAllocator* ba = cx_pool_allocator_create(4*1024, NULL);
    test_array_int(size, cx_pool_allocator_iface(ba));
    test_array_cxstr(size, cx_pool_allocator_iface(ba));
    cx_pool_allocator_destroy(ba);
}

__attribute__((constructor))
static void reg_array(void) {

    reg_add_test("array", test_array);
}


