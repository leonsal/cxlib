#include "cx_alloc.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include "logger.h"
#include "registry.h"

// Map int -> int
#define cx_hmap_name                mapii
#define cx_hmap_key                 int
#define cx_hmap_val                 int
#define cx_hmap_cmp_key(pk1,pk2)    memcmp(pk1,pk2,sizeof(int))
#define cx_hmap_hash_key(pk)        cx_hmap_hash_fnv1a32((char*)pk,sizeof(*pk))
#define cx_hmap_static
#define cx_hmap_instance_allocator
#define cx_hmap_implement
#include "cx_hmap.h"

// Test map int -> int
void test_hmapii(size_t size, size_t nbuckets, const CxAllocator* alloc) {

    LOGI("%s: size=%lu nbuckets=%lu alloc=%p", __func__, size, nbuckets, alloc);
    // Initializes map and sets entries
    mapii m = mapii_init(alloc, nbuckets);
    // Tests clearing empty map
    mapii_clear(&m);
    CXCHK(mapii_count(&m) == 0);

    // Fill map
    for (size_t  i = 0; i < size; i++) {
        mapii_set(&m, i, i*2);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        int* val = mapii_get(&m, i);
        CXCHK(*val == i * 2);
    }
    CXCHK(mapii_count(&m) == size);

    // Checks entries using iterator
    mapii_iter iter1 = {};
    size_t counter = 0;
    while (true) {
        mapii_entry* e = mapii_next(&m, &iter1);
        if (e == NULL) {
            break;
        }
        CXCHK(e->val == e->key * 2);
        counter++;
    }
    CXCHK(counter == mapii_count(&m));

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        mapii_set(&m, i, i * 3);
    }
    CXCHK(mapii_count(&m) == size);
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        int* val = mapii_get(&m, i);
        CXCHK(*val == i * 3);
    }

    // Delete even keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2 == 0) {
            mapii_del(&m, i);
        }
    }
    CXCHK(mapii_count(&m) == size/2);
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        int* val = mapii_get(&m, i);
        if (i % 2 == 0) {
            CXCHK(val == NULL);
        }
        else {
            CXCHK(*val == i * 3);
        }
    }

    // Overwrites all keys with value = key * 4
    for (size_t i = 0; i < size; i++) {
        mapii_set(&m, i, i * 4);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        int* val = mapii_get(&m, i);
        CXCHK(val && *val == i * 4);
    }
    CXCHK(mapii_count(&m) == size);

    // Delete odd keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2) {
            mapii_del(&m, i);
        }
    }
    CXCHK(mapii_count(&m) == size/2);
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        int* val = mapii_get(&m, i);
        if (i % 2) {
            CXCHK(val == NULL);
        }
        else {
            CXCHK(val && *val == i * 4);
        }
    }

    mapii_clear(&m);
    CXCHK(mapii_count(&m) == 0);
    mapii_free(&m);
}

// Define dynamic string
#define cx_str_name cxstr
#define cx_str_implement
#define cx_str_static
#define cx_str_instance_allocator
#define cx_str_implement
#include "cx_str.h"

// Define hashmap from cxstr to cxstr
#define cx_hmap_name                mapss
#define cx_hmap_key                 cxstr
#define cx_hmap_val                 cxstr
#define cx_hmap_cmp_key(pk1,pk2)    cxstr_cmps(pk1,pk2)
#define cx_hmap_hash_key(pk)        cx_hmap_hash_fnv1a32((pk)->data, (pk)->len_)
#define cx_hmap_free_key(pk)        cxstr_free(pk)
#define cx_hmap_free_val(pv)        cxstr_free(pv)
#define cx_hmap_instance_allocator
#define cx_hmap_static
#define cx_hmap_implement
#include "cx_hmap.h"

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


void test_hmapss(size_t size, size_t nbuckets, const CxAllocator* alloc) {

    LOGI("%s: size=%lu nbuckets=%lu alloc=%p", __func__, size, nbuckets, alloc);
    // Initializes map and sets entries
    mapss m = mapss_init(alloc, nbuckets);
    // Tests clearing empty map
    mapss_clear(&m);
    CXCHK(mapss_count(&m) == 0);

    // Fill map with value = key * 2
    for (size_t  i = 0; i < size; i++) {
        // The key and val are allocated and 'MOVED' to the map
        cxstr key = newcxstr(i, alloc);
        cxstr val = newcxstr(i*2, alloc);
        mapss_set(&m, key, val);
    }
    // Checks entries directly
    cxstr keyaux = newcxstr(0, alloc);
    for (size_t  i = 0; i < size; i++) {
        cxstr_clear(&keyaux);
        cxstr_printf(&keyaux, "%d", i);
        cxstr* val = mapss_get(&m, keyaux);
        CXCHK(val && cxstr_cmp(val, numstr(i*2))==0);
    }
    CXCHK(mapss_count(&m) == size);

    // Checks entries using iterator
    mapss_iter iter1 = {};
    size_t counter = 0;
    while (true) {
        mapss_entry* e =mapss_next(&m, &iter1);
        if (e == NULL) {
            break;
        }
        CXCHK(cxstr_cmp(&e->val, numstr(atoi(e->key.data)*2))==0);
        counter++;
    }
    CXCHK(counter == mapss_count(&m));

    // Overwrites all keys with value = key * 3
    for (size_t i = 0; i < size; i++) {
        cxstr key = newcxstr(i, alloc);
        cxstr val = newcxstr(i*3, alloc);
        mapss_set(&m, key, val);
    }
    CXCHK(mapss_count(&m) == size);
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        cxstr_clear(&keyaux);
        cxstr_printf(&keyaux, "%d", i);
        cxstr* val = mapss_get(&m, keyaux);
        CXCHK(val && cxstr_cmp(val, numstr(i*3))==0);
    }

    // Delete even keys
    for (size_t i = 0; i < size; i++) {
        cxstr_clear(&keyaux);
        cxstr_printf(&keyaux, "%d", i);
        if (i % 2 == 0) {
            mapss_del(&m, keyaux);
        }
    }
    CXCHK(mapss_count(&m) == size/2);
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        cxstr_clear(&keyaux);
        cxstr_printf(&keyaux, "%d", i);
        cxstr* val = mapss_get(&m, keyaux);
        if (i % 2 == 0) {
            CXCHK(val == NULL);
        }
        else {
            CXCHK(val && cxstr_cmp(val, numstr(i*3))==0);
        }
    }

    // Overwrites all keys with value = key * 4
    for (size_t i = 0; i < size; i++) {
        cxstr key = newcxstr(i, alloc);
        cxstr val = newcxstr(i*4, alloc);
        mapss_set(&m, key, val);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        cxstr_clear(&keyaux);
        cxstr_printf(&keyaux, "%d", i);
        cxstr* val = mapss_get(&m, keyaux);
        CXCHK(val && cxstr_cmp(val, numstr(i*4))==0);
    }
    CXCHK(mapss_count(&m) == size);

    // Delete odd keys
    for (size_t i = 0; i < size; i++) {
        cxstr_clear(&keyaux);
        cxstr_printf(&keyaux, "%d", i);
        if (i % 2) {
            mapss_del(&m, keyaux);
        }
    }
    CXCHK(mapss_count(&m) == size/2);
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        cxstr_clear(&keyaux);
        cxstr_printf(&keyaux, "%d", i);
        cxstr* val = mapss_get(&m, keyaux);
        if (i % 2) {
            CXCHK(val == NULL);
        }
        else {
            CXCHK(val && cxstr_cmp(val, numstr(i*4))==0);
        }
    }

    cxstr_free(&keyaux);
    mapss_clear(&m);
    CXCHK(mapss_count(&m) == 0);
    mapss_free(&m);
}


// Map type of allocated C string key -> allocated C string
#define cx_hmap_name                mapcc
#define cx_hmap_key                 char*
#define cx_hmap_val                 char*
#define cx_hmap_cmp_key(pk1,pk2)    strcmp(*pk1,*pk2)
#define cx_hmap_hash_key(pk)        cx_hmap_hash_fnv1a32(*pk, strlen(*pk))
#define cx_hmap_free_key(pk)        free(*pk)
#define cx_hmap_free_val(pk)        free(*pk)
#define cx_hmap_instance_allocator
#define cx_hmap_implement
#define cx_hmap_stats
#include "cx_hmap.h"

void test_hmapcc(size_t size, size_t nbuckets, const CxAllocator* alloc) {

    // Initializes map and sets entries
    LOGI("%s: size=%lu nbuckets=%lu alloc=%p", __func__, size, nbuckets, alloc);
    mapcc m = mapcc_init(alloc, nbuckets);

    // Tests clearing empty map
    mapcc_clear(&m);
    CXCHK(mapcc_count(&m) == 0);

    // Fill map
    for (size_t  i = 0; i < size; i++) {
        // The key and val are allocated and 'MOVED' to the map
        char* key = newstr(i, alloc);
        char* val = newstr(i*2, alloc);
        mapcc_set(&m, key, val);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        char** val = mapcc_get(&m, numstr(i));
        CXCHK(val && strcmp(*val, numstr(i*2))==0);
    }
    CXCHK(mapcc_count(&m) == size);

    // Checks entries using iterator
    mapcc_iter iter1 = {};
    size_t counter = 0;
    while (true) {
        mapcc_entry* e = mapcc_next(&m, &iter1);
        if (e == NULL) {
            break;
        }
        CXCHK(strcmp(e->val, numstr(atoi(e->key)*2))==0);
        counter++;
    }
    CXCHK(counter == mapcc_count(&m));

    // Overwrites all keys with value = key * 3
    for (size_t i = 0; i < size; i++) {
        char* key = newstr(i, alloc);
        char* val = newstr(i*3, alloc);
        mapcc_set(&m, key, val);
    }
    CXCHK(mapcc_count(&m) == size);
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        char** val = mapcc_get(&m, numstr(i));
        CXCHK(val && strcmp(*val, numstr(i*3))==0);
    }

    // Delete even keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2 == 0) {
            mapcc_del(&m, numstr(i));
        }
    }
    CXCHK(mapcc_count(&m) == size/2);
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        char** val = mapcc_get(&m, numstr(i));
        if (i % 2 == 0) {
            CXCHK(val == NULL);
        }
        else {
            CXCHK(val && strcmp(*val, numstr(i*3))==0);
        }
    }

    // Overwrites all keys with value = key * 4
    for (size_t i = 0; i < size; i++) {
        mapcc_set(&m, newstr(i,alloc), newstr(i*4.0, alloc));
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        char** val = mapcc_get(&m, numstr(i));
        CXCHK(val && strcmp(*val, numstr(i*4))==0);
    }
    CXCHK(mapcc_count(&m) == size);

    // Delete odd keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2) {
            mapcc_del(&m, numstr(i));
        }
    }
    CXCHK(mapcc_count(&m) == size/2);
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        char** val = mapcc_get(&m, numstr(i));
        if (i % 2) {
            CXCHK(val == NULL);
        }
        else {
            CXCHK(val && strcmp(*val, numstr(i*4))==0);
        }
    }

    // Stats
    if (0) {
        mapcc_stats stats = mapcc_get_stats(&m);
        mapcc_print_stats(&stats);
    }

    mapcc_clear(&m);
    CXCHK(mapcc_count(&m) == 0);
    mapcc_free(&m);
}

void test_hmap(void) {

    test_hmapii(1000, 0, NULL);
    test_hmapss(1000, 0, NULL);
    test_hmapcc(1000, 0, NULL);
}

__attribute__((constructor))
static void reg_hmap(void) {

    reg_add_test("hmap", test_hmap);
}

