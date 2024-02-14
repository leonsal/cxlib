#include "cx_alloc.h"
#include "cx_pool_allocator.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include "util.h"

// Map type 1 of uint64_t -> double
#define cx_hmap_name map1u64
#define cx_hmap_key uint64_t
#define cx_hmap_val double
#define cx_hmap_instance_allocator
#define cx_hmap_stats
#define cx_hmap_implement
#include "cx_hmap.h"

// // Map type 2 of uint64_t key -> double
// #define cx_hmap_name map2u64
// #define cx_hmap_key uint64_t
// #define cx_hmap_val double
// #define cx_hmap_instance_allocator
// #define cx_hmap_stats
// #define cx_hmap_implement
// #include "cx_hmap2.h"

// String used as value for the following maps
#define cx_str_name cxstr
#define cx_str_instance_allocator
#define cx_str_implement
#include "cx_str.h"

// Map type 1 of allocated C string key -> double
#define cx_hmap_name map1str
#define cx_hmap_key  char*
#define cx_hmap_val  cxstr
#define cx_hmap_cmp_key(k1,k2,s)    strcmp(*(char**)k1,*(char**)k2)
#define cx_hmap_hash_key(k,s)       cx_hmap_hash_fnv1a32(*((char**)k), strlen(*(char**)k))
#define cx_hmap_free_key(k)         free(*k)
#define cx_hmap_free_val(k)         cxstr_free(k)
#define cx_hmap_instance_allocator
#define cx_hmap_implement
#include "cx_hmap.h"

// // Map type 2 of allocated C string key -> double
// #define cx_hmap_name map2str
// #define cx_hmap_key  char*
// #define cx_hmap_val  int64_t
// #define cx_hmap_cmp_key(k1,k2,s)    strcmp(*(char**)k1,*(char**)k2)
// #define cx_hmap_hash_key(k,s)       cx_hmap_hash_fnv1a32(*((char**)k), strlen(*(char**)k))
// #define cx_hmap_free_key(k)         free(*k)
// #define cx_hmap_instance_allocator
// #define cx_hmap_implement
// #include "cx_hmap2.h"

// //
// // Map of cx_str -> double
// //
// #define cx_str_name cxstr
// #define cx_str_cap 8
// #define cx_str_static
// #define cx_str_error_handler(msg,func) printf("CXSTR ERROR:%s at %s\n", msg, func);abort()
// #define cx_str_instance_allocator
// #define cx_str_implement
// #include "cx_str.h"

// static int cmp_cxstr_keys(const void* k1, const void* k2, size_t size) {
//     return cxstr_cmps((cxstr*)k1, (cxstr*)k2);
// }
// static size_t hash_cxstr_key(void* key, size_t keySize) {
//     return cx_hmap_hash_fnv1a32(((cxstr*)key)->data, cxstr_len((cxstr*)key));
// }

// #define cx_hmap_name mapt5
// #define cx_hmap_key  cxstr
// #define cx_hmap_val  double
// #define cx_hmap_cmp_key  cmp_cxstr_keys
// #define cx_hmap_hash_key hash_cxstr_key
// #define cx_hmap_instance_allocator
// #define cx_hmap_implement
// #include "cx_hmap2.h"

#include "hmap.h"
#include "logger.h"

void test_map1u64(size_t size, size_t nbuckets, const CxAllocator* alloc);
void test_map1str(size_t size, size_t nbuckets, const CxAllocator* alloc);

void cxHmapTests(void) {

    //test_map1u64(100, 50, cxDefaultAllocator());
    test_map1str(100, 50, cxDefaultAllocator());

    // // Use pool allocator because the keys are dynamically allocated
    // CxPoolAllocator* pa = cx_pool_allocator_create(4*1024, NULL);
    // cxHmapTest4(100, 40, cx_pool_allocator_iface(pa));
    // cx_pool_allocator_destroy(pa);
    //
    // // Use pool allocator because the keys are dynamically allocated
    // pa = cx_pool_allocator_create(4*1024, NULL);
    // cxHmapTest5(100, 40, cx_pool_allocator_iface(pa));
    // cx_pool_allocator_destroy(pa);
}

void test_map1u64(size_t size, size_t nbuckets, const CxAllocator* alloc) {

    LOGI("%s: size=%lu nbuckets=%lu alloc=%p", __func__, size, nbuckets, alloc);
    // Initializes map type 1 and sets entries
    map1u64 m = map1u64_init(alloc, nbuckets);
    // Tests clearing empty map
    map1u64_clear(&m);
    for (size_t  i = 0; i < size; i++) {
        map1u64_set(&m, i, i*2.0);
    }
    //map1u64_stats s = map1u64_get_stats(&m1);
    //map1u64_print_stats(&s);
    CHK(map1u64_count(&m) == size);
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*map1u64_get(&m, i) == i * 2.0);
    }
    // Checks entries using iterator
    map1u64_iter iter1 = {};
    size_t m1Count = 0;
    while (true) {
        map1u64_entry* e = map1u64_next(&m, &iter1);
        if (e == NULL) {
            break;
        }
        m1Count++;
        CHK(e->val == e->key * 2.0);
        //fprintf(stderr, "k:%u v:%f\n", e->key, e->val);
    }
    CHK(m1Count == size);

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        map1u64_set(&m, i, i*3.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*map1u64_get(&m, i) == i * 3.0);
    }

    // Delete even keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2 == 0) {
            map1u64_del(&m, i);
        }
    }
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        if (i % 2 == 0) {
            CHK(map1u64_get(&m, i) == NULL);
        }
        else {
            CHK(*map1u64_get(&m, i) == i * 3.0);
        }
    }

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        map1u64_set(&m, i, i*4.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*map1u64_get(&m, i) == i * 4.0);
    }

    // Delete odd keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2) {
            map1u64_del(&m, i);
        }
    }
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        if (i % 2) {
            CHK(map1u64_get(&m, i) == NULL);
        }
        else {
            CHK(*map1u64_get(&m, i) == i * 4.0);
        }
    }

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        map1u64_set(&m, i, i*5.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*map1u64_get(&m, i) == i * 5.0);
    }

    map1u64_clear(&m);
    CHK(map1u64_count(&m) == 0);
    map1u64_free(&m);
}

// Creates new C string from number
// Used to set new values
static char* newstr(size_t val, const CxAllocator* alloc) {

    char buf[32];
    snprintf(buf, sizeof(buf), "%zu", val);
    char* s  = cx_alloc_malloc(alloc, strlen(buf)+1);
    strcpy(s, buf);
    //printf("alloc:%s\n", s);
    return s;
}

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

void test_map1str(size_t size, size_t nbuckets, const CxAllocator* alloc) {

    LOGI("%s: size=%lu nbuckets=%lu alloc=%p", __func__, size, nbuckets, alloc);
    // Initializes map and sets entries
    map1str m = map1str_init(alloc, nbuckets);
    // Tests clearing empty map
    map1str_clear(&m);
    CHK(map1str_count(&m) == 0);

    // Fill map
    for (size_t  i = 0; i < size; i++) {
        // The key and val are allocated and 'MOVED' to the map
        char* key = newstr(i, alloc);
        cxstr val = newcxstr(i*2, alloc);
        map1str_set(&m, key, val);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        cxstr* val = map1str_get(&m, numstr(i));
        CHK(val && cxstr_cmp(val, numstr(i*2))==0);
    }
    CHK(map1str_count(&m) == size);

    // Checks entries using iterator
    map1str_iter iter1 = {};
    size_t counter = 0;
    while (true) {
        map1str_entry* e = map1str_next(&m, &iter1);
        if (e == NULL) {
            break;
        }
        CHK(cxstr_cmp(&e->val, numstr(atoi(e->key)*2))==0);
        counter++;
    }
    CHK(counter == map1str_count(&m));

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        char* key = newstr(i, alloc);
        cxstr val = newcxstr(i*3, alloc);
        map1str_set(&m, key, val);
    }
    CHK(map1str_count(&m) == size);

    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        cxstr* val = map1str_get(&m, numstr(i));
        CHK(val && cxstr_cmp(val, numstr(i*3))==0);
    }

    // Delete even keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2 == 0) {
            map1str_del(&m, numstr(i));
        }
    }
    CHK(map1str_count(&m) == size/2);

    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        cxstr* val = map1str_get(&m, numstr(i));
        if (i % 2 == 0) {
            CHK(val == NULL);
        }
        else {
            CHK(val && cxstr_cmp(val, numstr(i*3))==0);
        }
    }

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        map1str_set(&m, newstr(i,alloc), newcxstr(i*4.0, alloc));
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        cxstr* val = map1str_get(&m, numstr(i));
        CHK(val && cxstr_cmp(val, numstr(i*4))==0);
    }
    CHK(map1str_count(&m) == size);

    // Delete odd keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2) {
            map1str_del(&m, numstr(i));
        }
    }
    CHK(map1str_count(&m) == size/2);

    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        cxstr* val = map1str_get(&m, numstr(i));
        if (i % 2) {
            CHK(val == NULL);
        }
        else {
            CHK(val && cxstr_cmp(val, numstr(i*4))==0);
        }
    }
    map1str_clear(&m);
    CHK(map1str_count(&m) == 0);
    map1str_free(&m);
}

