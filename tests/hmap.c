#include "cx_alloc.h"
#include "cx_alloc_pool.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include "util.h"

//
// Map of int key -> double
//
#define cx_hmap_name mapt1
#define cx_hmap_key int
#define cx_hmap_val double
#define cx_hmap_instance_allocator
#define cx_hmap_stats
#define cx_hmap_implement
#include "cx_hmap.h"

//
// Map of uint64_t key -> double
//
#define cx_hmap_name mapt2
#define cx_hmap_key uint64_t
#define cx_hmap_val double
#define cx_hmap_instance_allocator
#define cx_hmap_stats
#define cx_hmap_implement
#include "cx_hmap2.h"

//
// Map of fixed size C string key -> double
//
static int cmp_fixedstr_keys(const void* k1, const void* k2, size_t size) {
    return strcmp(k1, k2);
}
static size_t hash_fixedstr_key(void* key, size_t keySize) {
    return cxHashFNV1a32(key, strlen((char*)key));
}
typedef struct strkey_{char data[32];} strkey;
#define cx_hmap_name mapt3
#define cx_hmap_key  strkey
#define cx_hmap_val  double
#define cx_hmap_cmp_key  cmp_fixedstr_keys
#define cx_hmap_hash_key hash_fixedstr_key
#define cx_hmap_instance_allocator
#define cx_hmap_implement
#include "cx_hmap.h"

//
// Map of allocated C string key -> double
// Must use custom allocator for clone to work
//
static int cmp_str_keys(const void* k1, const void* k2, size_t size) {
    return strcmp(*(char**)k1, *(char**)k2);
}
static size_t hash_str_key(void* key, size_t keySize) {
    return cxHashFNV1a32(*((char**)key), strlen(*(char**)key));
}
#define cx_hmap_name mapt4
#define cx_hmap_key  char*
#define cx_hmap_val  double
#define cx_hmap_cmp_key  cmp_str_keys
#define cx_hmap_hash_key hash_str_key
#define cx_hmap_instance_allocator
#define cx_hmap_implement
#include "cx_hmap2.h"

//
// Map of cx_str -> double
//
#define cx_str_name cxstr
#define cx_str_cap 8
#define cx_str_static
#define cx_str_error_handler(msg,func) printf("CXSTR ERROR:%s at %s\n", msg, func);abort()
#define cx_str_instance_allocator
#define cx_str_implement
#include "cx_str.h"

static int cmp_cxstr_keys(const void* k1, const void* k2, size_t size) {
    return cxstr_cmps((cxstr*)k1, (cxstr*)k2);
}
static size_t hash_cxstr_key(void* key, size_t keySize) {
    return cxHashFNV1a32(((cxstr*)key)->data, cxstr_len((cxstr*)key));
}

#define cx_hmap_name mapt5
#define cx_hmap_key  cxstr
#define cx_hmap_val  double
#define cx_hmap_cmp_key  cmp_cxstr_keys
#define cx_hmap_hash_key hash_cxstr_key
#define cx_hmap_instance_allocator
#define cx_hmap_implement
#include "cx_hmap2.h"

#include "hmap.h"
#include "logger.h"

void cxHmapTests(void) {

    cxHmapTest1(100, 40, cxDefaultAllocator());
    cxHmapTest2(100, 40, cxDefaultAllocator());
    cxHmapTest3(100, 40, cxDefaultAllocator());

    // Use pool allocator because the keys are dynamically allocated
    CxAllocPool* pa = cxAllocPoolCreate(4*1024, NULL);
    cxHmapTest4(100, 40, cxAllocPoolGetAllocator(pa));
    cxAllocPoolDestroy(pa);

    // Use pool allocator because the keys are dynamically allocated
    pa = cxAllocPoolCreate(4*1024, NULL);
    cxHmapTest5(100, 40, cxAllocPoolGetAllocator(pa));
    cxAllocPoolDestroy(pa);
}

void cxHmapTest1(size_t size, size_t nbuckets, const CxAllocator* alloc) {

    LOGI("hmap1 int->double, size=%lu nbuckets=%lu alloc=%p", size, nbuckets, alloc);
    // Initializes map type 1 and sets entries
    mapt1 m1 = mapt1_init(alloc, nbuckets);
    for (size_t  i = 0; i < size; i++) {
        mapt1_set(&m1, i, i*2.0);
    }
    //mapt1_stats s = mapt1_get_stats(&m1);
    //mapt1_print_stats(&s);
    CHK(mapt1_count(&m1) == size);
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt1_get(&m1, i) == i * 2.0);
    }
    // Checks entries using iterator
    mapt1_iter iter1 = {};
    size_t m1Count = 0;
    while (true) {
        mapt1_entry* e = mapt1_next(&m1, &iter1);
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
        mapt1_set(&m1, i, i*3.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt1_get(&m1, i) == i * 3.0);
    }

    // Delete even keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2 == 0) {
            mapt1_del(&m1, i);
        }
    }
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        if (i % 2 == 0) {
            CHK(mapt1_get(&m1, i) == NULL);
        }
        else {
            CHK(*mapt1_get(&m1, i) == i * 3.0);
        }
    }

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        mapt1_set(&m1, i, i*4.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt1_get(&m1, i) == i * 4.0);
    }

    // Delete odd keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2) {
            mapt1_del(&m1, i);
        }
    }
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        if (i % 2) {
            CHK(mapt1_get(&m1, i) == NULL);
        }
        else {
            CHK(*mapt1_get(&m1, i) == i * 4.0);
        }
    }

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        mapt1_set(&m1, i, i*5.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt1_get(&m1, i) == i * 5.0);
    }

    // Clones map and checks entries of cloned map
    mapt1 m2 = mapt1_clone(&m1, nbuckets*2);
    iter1 = (mapt1_iter){};
    while (true) {
        mapt1_entry* e = mapt1_next(&m2, &iter1);
        if (e == NULL) {
            break;
        }
        CHK(*mapt1_get(&m2, e->key) == *mapt1_get(&m1, e->key));
    }
    // Frees mapt1 1
    mapt1_free(&m1);
    CHK(mapt1_count(&m1) == 0);

    // Removes all the keys from map 2
    for (size_t  i = 0; i < size; i++) {
        mapt1_del(&m2, i);
    }
    CHK(mapt1_count(&m2) == 0);
    // Fill again
    for (size_t i = 0; i < size; i++) {
        mapt1_set(&m2, i, i*6.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt1_get(&m2, i) == i * 6.0);
    }
    // Clears the map
    mapt1_clear(&m2);
    CHK(mapt1_count(&m2) == 0);
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(mapt1_get(&m2, i) == NULL);
    }
    mapt1_free(&m2);
}

void cxHmapTest2(size_t size, size_t nbuckets, const CxAllocator* alloc) {

    LOGI("hmap2 uint32_t->double, size=%lu nbuckets=%lu alloc=%p", size, nbuckets, alloc);
    // Initializes map type 1 and sets entries
    mapt2 m1 = mapt2_init(alloc, nbuckets);
    for (size_t  i = 0; i < size; i++) {
        mapt2_set(&m1, i, i*2.0);
    }
    //mapt2_stats s = mapt2_get_stats(&m1);
    //mapt2_print_stats(&s);
    CHK(mapt2_count(&m1) == size);
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt2_get(&m1, i) == i * 2.0);
    }
    // Checks entries using iterator
    mapt2_iter iter1 = {};
    size_t m1Count = 0;
    while (true) {
        mapt2_entry* e = mapt2_next(&m1, &iter1);
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
        mapt2_set(&m1, i, i*3.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt2_get(&m1, i) == i * 3.0);
    }

    // Delete even keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2 == 0) {
            mapt2_del(&m1, i);
        }
    }
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        if (i % 2 == 0) {
            CHK(mapt2_get(&m1, i) == NULL);
        }
        else {
            CHK(*mapt2_get(&m1, i) == i * 3.0);
        }
    }

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        mapt2_set(&m1, i, i*4.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt2_get(&m1, i) == i * 4.0);
    }

    // Delete odd keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2) {
            mapt2_del(&m1, i);
        }
    }
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        if (i % 2) {
            CHK(mapt2_get(&m1, i) == NULL);
        }
        else {
            CHK(*mapt2_get(&m1, i) == i * 4.0);
        }
    }

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        mapt2_set(&m1, i, i*5.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt2_get(&m1, i) == i * 5.0);
    }

    // Clones map and checks entries of cloned map
    mapt2 m2 = mapt2_clone(&m1, nbuckets*2);
    iter1 = (mapt2_iter){};
    while (true) {
        mapt2_entry* e = mapt2_next(&m2, &iter1);
        if (e == NULL) {
            break;
        }
        CHK(*mapt2_get(&m2, e->key) == *mapt2_get(&m1, e->key));
    }
    // Frees mapt2 1
    mapt2_free(&m1);
    CHK(mapt2_count(&m1) == 0);

    // Removes all the keys from map 2
    for (size_t  i = 0; i < size; i++) {
        mapt2_del(&m2, i);
    }
    CHK(mapt2_count(&m2) == 0);
    // Fill again
    for (size_t i = 0; i < size; i++) {
        mapt2_set(&m2, i, i*6.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt2_get(&m2, i) == i * 6.0);
    }
    // Clears the map
    mapt2_clear(&m2);
    CHK(mapt2_count(&m2) == 0);
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(mapt2_get(&m2, i) == NULL);
    }
    mapt2_free(&m2);
}


// Converts integer to fixed ASCII string
static strkey num2key(size_t val) {
   
    strkey k;
    snprintf(k.data, sizeof(k), "%zu", val);
    return k;
}

void cxHmapTest3(size_t size, size_t nbuckets, const CxAllocator* alloc) {

    LOGI("hmap3 char[32]->double, size=%lu nbuckets=%lu alloc=%p", size, nbuckets, alloc);
    // Initializes map type 1 and sets entries
    mapt3 m1 = mapt3_init(alloc, nbuckets);
    for (size_t  i = 0; i < size; i++) {
        mapt3_set(&m1, num2key(i), i*2.0);
    }
    CHK(mapt3_count(&m1) == size);

    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt3_get(&m1, num2key(i)) == i * 2.0);
    }

    // Checks entries using iterator
    mapt3_iter iter1 = {};
    size_t m1Count = 0;
    while (true) {
        mapt3_entry* e = mapt3_next(&m1, &iter1);
        if (e == NULL) {
            break;
        }
        m1Count++;
        CHK(e->val == atoi(e->key.data) * 2.0);
    }
    CHK(m1Count == size);

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        mapt3_set(&m1, num2key(i), i*3.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt3_get(&m1, num2key(i)) == i * 3.0);
    }

    // Delete even keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2 == 0) {
            mapt3_del(&m1, num2key(i));
        }
    }
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        if (i % 2 == 0) {
            CHK(mapt3_get(&m1, num2key(i)) == NULL);
        }
        else {
            CHK(*mapt3_get(&m1, num2key(i)) == i * 3.0);
        }
    }

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        mapt3_set(&m1, num2key(i), i*4.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt3_get(&m1, num2key(i)) == i * 4.0);
    }

    // Delete odd keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2) {
            mapt3_del(&m1, num2key(i));
        }
    }
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        if (i % 2) {
            CHK(mapt3_get(&m1, num2key(i)) == NULL);
        }
        else {
            CHK(*mapt3_get(&m1, num2key(i)) == i * 4.0);
        }
    }

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        mapt3_set(&m1, num2key(i), i*5.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt3_get(&m1, num2key(i)) == i * 5.0);
    }

    // Clones map and checks entries of cloned map
    mapt3 m2 = mapt3_clone(&m1, nbuckets*2);
    iter1 = (mapt3_iter){};
    while (true) {
        mapt3_entry* e = mapt3_next(&m2, &iter1);
        if (e == NULL) {
            break;
        }
        CHK(*mapt3_get(&m2, e->key) == *mapt3_get(&m1, e->key));
    }
    // Frees mapt3 1
    mapt3_free(&m1);
    CHK(mapt3_count(&m1) == 0);

    // Removes all the keys from map 2
    for (size_t  i = 0; i < size; i++) {
        mapt3_del(&m2, num2key(i));
    }
    CHK(mapt3_count(&m2) == 0);

    // Fill again
    for (size_t i = 0; i < size; i++) {
        mapt3_set(&m2, num2key(i), i*6.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt3_get(&m2, num2key(i)) == i * 6.0);
    }

    // Clears the map
    mapt3_clear(&m2);
    CHK(mapt3_count(&m2) == 0);
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(mapt3_get(&m2, num2key(i)) == NULL);
    }
    mapt3_free(&m2);
}

// Converts integer to allocated ASCII string 
static char* num2str(size_t val, const CxAllocator* alloc) {

    char buf[32];
    snprintf(buf, sizeof(buf), "%zu", val);
    char* s  = cx_alloc_malloc(alloc, strlen(buf)+1);
    strcpy(s, buf);
    return s;
}

void cxHmapTest4(size_t size, size_t nbuckets, const CxAllocator* alloc) {

    LOGI("hmap4 char*->double, size=%lu nbuckets=%lu alloc=%p", size, nbuckets, alloc);
    // Initializes map and sets entries
    mapt4 m1 = mapt4_init(alloc, nbuckets);
    for (size_t  i = 0; i < size; i++) {
        mapt4_set(&m1, num2str(i,alloc), i*2.0);
    }
    CHK(mapt4_count(&m1) == size);

    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt4_get(&m1, num2str(i,alloc)) == i * 2.0);
    }

    // Checks entries using iterator
    mapt4_iter iter1 = {};
    size_t m1Count = 0;
    while (true) {
        mapt4_entry* e = mapt4_next(&m1, &iter1);
        if (e == NULL) {
            break;
        }
        m1Count++;
        CHK(e->val == atoi(e->key) * 2.0);
    }
    CHK(m1Count == size);

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        mapt4_set(&m1, num2str(i,alloc), i*3.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt4_get(&m1, num2str(i,alloc)) == i * 3.0);
    }

    // Delete even keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2 == 0) {
            mapt4_del(&m1, num2str(i,alloc));
        }
    }
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        if (i % 2 == 0) {
            CHK(mapt4_get(&m1, num2str(i,alloc)) == NULL);
        }
        else {
            CHK(*mapt4_get(&m1, num2str(i,alloc)) == i * 3.0);
        }
    }

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        mapt4_set(&m1, num2str(i,alloc), i*4.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt4_get(&m1, num2str(i,alloc)) == i * 4.0);
    }

    // Delete odd keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2) {
            mapt4_del(&m1, num2str(i,alloc));
        }
    }
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        if (i % 2) {
            CHK(mapt4_get(&m1, num2str(i,alloc)) == NULL);
        }
        else {
            CHK(*mapt4_get(&m1, num2str(i,alloc)) == i * 4.0);
        }
    }

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        mapt4_set(&m1, num2str(i,alloc), i*5.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt4_get(&m1, num2str(i,alloc)) == i * 5.0);
    }

    // Clones map and checks entries of cloned map
    // NOTE: the key's string pointers are copied not allocated.
    mapt4 m2 = mapt4_clone(&m1, nbuckets*2);
    iter1 = (mapt4_iter){};
    while (true) {
        mapt4_entry* e = mapt4_next(&m2, &iter1);
        if (e == NULL) {
            break;
        }
        CHK(*mapt4_get(&m2, e->key) == *mapt4_get(&m1, e->key));
    }
    // Frees mapt4 1
    mapt4_free(&m1);
    CHK(mapt4_count(&m1) == 0);

    // Removes all the keys from map 2
    for (size_t  i = 0; i < size; i++) {
        mapt4_del(&m2, num2str(i, alloc));
    }
    CHK(mapt4_count(&m2) == 0);

    // Fill again
    for (size_t i = 0; i < size; i++) {
        mapt4_set(&m2, num2str(i,alloc), i*6.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt4_get(&m2, num2str(i,alloc)) == i * 6.0);
    }

    // Clears the map
    mapt4_clear(&m2);
    CHK(mapt4_count(&m2) == 0);
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(mapt4_get(&m2, num2str(i,alloc)) == NULL);
    }
    mapt4_free(&m2);
}

// Converts integer to dynamically allocated cxstr
static cxstr num2cxstr(size_t val, const CxAllocator* alloc) {
  
    cxstr s = cxstr_init(alloc);
    cxstr_printf(&s, "%u", val);
    return s;
}

void cxHmapTest5(size_t size, size_t nbuckets, const CxAllocator* alloc) {

    LOGI("hmap5 cxstr->double, size=%lu nbuckets=%lu alloc=%p", size, nbuckets, alloc);
    // Initializes map and sets entries
    mapt5 m1 = mapt5_init(alloc, nbuckets);
    for (size_t  i = 0; i < size; i++) {
        mapt5_set(&m1, num2cxstr(i,alloc), i*2.0);
    }
    CHK(mapt5_count(&m1) == size);

    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt5_get(&m1, num2cxstr(i,alloc)) == i * 2.0);
    }

    // Checks entries using iterator
    mapt5_iter iter1 = {};
    size_t m1Count = 0;
    while (true) {
        mapt5_entry* e = mapt5_next(&m1, &iter1);
        if (e == NULL) {
            break;
        }
        m1Count++;
        CHK(e->val == atoi(e->key.data) * 2.0);
    }
    CHK(m1Count == size);

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        mapt5_set(&m1, num2cxstr(i,alloc), i*3.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt5_get(&m1, num2cxstr(i,alloc)) == i * 3.0);
    }

    // Delete even keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2 == 0) {
            mapt5_del(&m1, num2cxstr(i,alloc));
        }
    }
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        if (i % 2 == 0) {
            CHK(mapt5_get(&m1, num2cxstr(i,alloc)) == NULL);
        }
        else {
            CHK(*mapt5_get(&m1, num2cxstr(i,alloc)) == i * 3.0);
        }
    }

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        mapt5_set(&m1, num2cxstr(i,alloc), i*4.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt5_get(&m1, num2cxstr(i,alloc)) == i * 4.0);
    }

    // Delete odd keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2) {
            mapt5_del(&m1, num2cxstr(i,alloc));
        }
    }
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        if (i % 2) {
            CHK(mapt5_get(&m1, num2cxstr(i,alloc)) == NULL);
        }
        else {
            CHK(*mapt5_get(&m1, num2cxstr(i,alloc)) == i * 4.0);
        }
    }

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        mapt5_set(&m1, num2cxstr(i,alloc), i*5.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt5_get(&m1, num2cxstr(i,alloc)) == i * 5.0);
    }

    // Clones map and checks entries of cloned map
    // NOTE: the key's cxstr are copied
    mapt5 m2 = mapt5_clone(&m1, nbuckets*2);
    iter1 = (mapt5_iter){};
    while (true) {
        mapt5_entry* e = mapt5_next(&m2, &iter1);
        if (e == NULL) {
            break;
        }
        CHK(*mapt5_get(&m2, e->key) == *mapt5_get(&m1, e->key));
    }
    // Frees mapt4 1
    mapt5_free(&m1);
    CHK(mapt5_count(&m1) == 0);

    // Removes all the keys from map 2
    for (size_t  i = 0; i < size; i++) {
        mapt5_del(&m2, num2cxstr(i, alloc));
    }
    CHK(mapt5_count(&m2) == 0);

    // Fill again
    for (size_t i = 0; i < size; i++) {
        mapt5_set(&m2, num2cxstr(i,alloc), i*6.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(*mapt5_get(&m2, num2cxstr(i,alloc)) == i * 6.0);
    }

    // Clears the map
    mapt5_clear(&m2);
    CHK(mapt5_count(&m2) == 0);
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        CHK(mapt5_get(&m2, num2cxstr(i,alloc)) == NULL);
    }
    mapt5_free(&m2);
}


