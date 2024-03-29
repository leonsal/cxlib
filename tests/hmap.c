#include "cx_alloc.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include "util.h"

// String used as value for the following map
#define cx_str_name cxstr
#define cx_str_instance_allocator
#define cx_str_implement
#include "cx_str.h"

// Map type of allocated C string key -> allocated cxstr value
#define cx_hmap_name hmap1
#define cx_hmap_key  char*
#define cx_hmap_val  cxstr
#define cx_hmap_cmp_key(k1,k2,s)    cx_hmap_cmp_key_str(k1,k2)
#define cx_hmap_hash_key(k,s)       cx_hmap_hash_key_str(k,s)
#define cx_hmap_free_key(k)         free(*k)
#define cx_hmap_free_val(k)         cxstr_free(k)
#define cx_hmap_instance_allocator
#define cx_hmap_stats
#define cx_hmap_implement
#include "cx_hmap.h"

#include "hmap.h"
#include "logger.h"


void test_hmap1(size_t size, size_t nbuckets, const CxAllocator* alloc);

void test_hmap(void) {

    test_hmap1(100, 50, cx_def_allocator());
}


// Creates new C string from number
// Used to set new keys
static char* newstr(size_t val, const CxAllocator* alloc) {

    char buf[32];
    snprintf(buf, sizeof(buf), "%zu", val);
    char* s  = cx_alloc_malloc(alloc, strlen(buf)+1);
    strcpy(s, buf);
    return s;
}

// Creates new cxstr from number
// Used to set new values
static cxstr newcxstr(size_t val, const CxAllocator* alloc) {

    char buf[32];
    snprintf(buf, sizeof(buf), "%zu", val);
    return cxstr_initc(alloc, buf);
}

// Returns statically allocated string from number
// Used to get values from map
static char* numstr(size_t val) {

    static char buf[32];
    snprintf(buf, sizeof(buf), "%zu", val);
    return buf;
}

void test_hmap1(size_t size, size_t nbuckets, const CxAllocator* alloc) {

    LOGI("%s: size=%lu nbuckets=%lu alloc=%p", __func__, size, nbuckets, alloc);
    // Initializes map and sets entries
    hmap1 m = hmap1_init(alloc, nbuckets);
    // Tests clearing empty map
    hmap1_clear(&m);
    CHK(hmap1_count(&m) == 0);

    // Fill map
    for (size_t  i = 0; i < size; i++) {
        // The key and val are allocated and 'MOVED' to the map
        char* key = newstr(i, alloc);
        cxstr val = newcxstr(i*2, alloc);
        hmap1_set(&m, key, val);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        cxstr* val = hmap1_get(&m, numstr(i));
        CHK(val && cxstr_cmp(val, numstr(i*2))==0);
    }
    CHK(hmap1_count(&m) == size);

    // Checks entries using iterator
    hmap1_iter iter1 = {};
    size_t counter = 0;
    while (true) {
        hmap1_entry* e = hmap1_next(&m, &iter1);
        if (e == NULL) {
            break;
        }
        CHK(cxstr_cmp(&e->val, numstr(atoi(e->key)*2))==0);
        counter++;
    }
    CHK(counter == hmap1_count(&m));

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        char* key = newstr(i, alloc);
        cxstr val = newcxstr(i*3, alloc);
        hmap1_set(&m, key, val);
    }
    CHK(hmap1_count(&m) == size);

    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        cxstr* val = hmap1_get(&m, numstr(i));
        CHK(val && cxstr_cmp(val, numstr(i*3))==0);
    }

    // Delete even keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2 == 0) {
            hmap1_del(&m, numstr(i));
        }
    }
    CHK(hmap1_count(&m) == size/2);

    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        cxstr* val = hmap1_get(&m, numstr(i));
        if (i % 2 == 0) {
            CHK(val == NULL);
        }
        else {
            CHK(val && cxstr_cmp(val, numstr(i*3))==0);
        }
    }

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        hmap1_set(&m, newstr(i,alloc), newcxstr(i*4.0, alloc));
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        cxstr* val = hmap1_get(&m, numstr(i));
        CHK(val && cxstr_cmp(val, numstr(i*4))==0);
    }
    CHK(hmap1_count(&m) == size);

    // Delete odd keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2) {
            hmap1_del(&m, numstr(i));
        }
    }
    CHK(hmap1_count(&m) == size/2);

    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        cxstr* val = hmap1_get(&m, numstr(i));
        if (i % 2) {
            CHK(val == NULL);
        }
        else {
            CHK(val && cxstr_cmp(val, numstr(i*4))==0);
        }
    }
    if (0) {
        hmap1_stats stats = hmap1_get_stats(&m);
        hmap1_print_stats(&stats);
    }

    hmap1_clear(&m);
    CHK(hmap1_count(&m) == 0);
    hmap1_free(&m);
}

