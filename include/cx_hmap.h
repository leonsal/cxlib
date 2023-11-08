#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "cx_alloc.h"

#ifndef cx_hmap_internal
    #ifndef cx_hmap_name
        #error "cx_hmap_name not defined"
    #endif
    #ifndef cx_hmap_key
        #error "cx_hmap_key not defined"
    #endif
    #ifndef cx_hmap_val
        #error "cx_hmap_val not defined"
    #endif
#endif

#ifndef cx_hmap_def_nbuckets
    #define cx_hmap_def_nbuckets (17)
#endif

// Default key comparison function
#ifndef cx_hmap_cmp_key
    #define cx_hmap_cmp_key cxHmapCmpKey
#endif

// Default key hash function
#ifndef cx_hmap_hash_key
    #define cx_hmap_hash_key cxHmapHashKey
#endif

#define concat2_(a, b) a ## b
#define concat1_(a, b) concat2_(a, b)
#define type_name(name) concat1_(cx_hmap_name, name)

#ifdef cx_array_static
    #define linkage static
#else
    #define linkage
#endif

//
// Function names
//
#ifdef cx_hmap_camel_case
    #define name_init           Init
    #define name_init2          Init2
    #define name_free           Free
    #define name_set            Set
    #define name_get            Get
    #define name_del            Del
    #define name_count          Count
    #define name_next           Next
    #define name_clone          Clone
#else
    #define name_init           _init
    #define name_init2          _init2
    #define name_free           _free
    #define name_set            _set
    #define name_get            _get
    #define name_del            _del
    #define name_count          _count
    #define name_next           _next
    #define name_clone          _clone
#endif

//
// Declarations (only generated once)
//
#ifndef cx_HMAP_H 
#define cx_HMAP_H

typedef struct CxHmapState {
    const       CxAllocator* alloc;
    void*       userdata;
    size_t      entrySize;
    size_t      keySize;
    size_t      entryCount;
    size_t      bucketCount;
} CxHmapState;

typedef struct CxHmapIter {
    size_t bucket;
    void*  next;
} CxHmapIter;

typedef enum {
    cx_hmap_op_set,
    cx_hmap_op_get,
    cx_hmap_op_del,
} cx_hmap_op;

// Declaration of generic implementation functions
int cxHmapCmpKey(void* k1, void* k2, size_t size);
size_t cxHmapHashKey(char* key, size_t keySize);
void cxHmapFreeFn(CxHmapState *ms);
void* cxHmapOperFn(CxHmapState* ms, cx_hmap_op op, void *key);
void* cxHmapNext(const CxHmapState* ms, CxHmapIter* iter);

#endif

//
// Internal declarations used only by cx_hmap.c
//
#ifndef cx_hmap_internal

typedef struct type_name(_entry) {
    struct type_name(_entry)* n;
    cx_hmap_key key;
    cx_hmap_val val;
} type_name(_entry);

typedef struct cx_hmap_name {
    CxHmapState s;
    type_name(_entry)* buckets;
} cx_hmap_name;

// Declare hmap iter type
typedef CxHmapIter type_name(_iter);

linkage cx_hmap_name type_name(name_init)(void);
linkage cx_hmap_name type_name(name_init2)(size_t nbuckets, const CxAllocator* alloc);
linkage void type_name(name_free)(cx_hmap_name* m);
linkage void type_name(name_set)(cx_hmap_name* m, cx_hmap_key k, cx_hmap_val v);
linkage cx_hmap_val* type_name(name_get)(cx_hmap_name* m, cx_hmap_key k);
linkage bool type_name(name_del)(cx_hmap_name* m, cx_hmap_key k);
linkage size_t type_name(name_count)(cx_hmap_name* m);
linkage type_name(_entry)* type_name(name_next)(cx_hmap_name* m, type_name(_iter)* iter);
linkage cx_hmap_name type_name(_clone)(cx_hmap_name* src, size_t nbuckets, const CxAllocator* alloc);

#endif

//
// Implementations
//
#ifdef cx_hmap_implement

linkage cx_hmap_name type_name(name_init)(void) {
    return type_name(_init2)(cx_hmap_def_nbuckets, cxDefaultAllocator());
}

linkage cx_hmap_name type_name(name_init2)(size_t nbuckets, const CxAllocator* alloc) {
    return (cx_hmap_name){
        .s.alloc = alloc == NULL ? cxDefaultAllocator() : alloc,
        .s.bucketCount = nbuckets == 0 ? cx_hmap_def_nbuckets : nbuckets,
        .s.entrySize = sizeof(type_name(_entry)),
        .s.keySize = sizeof(((type_name(_entry)*)0)->key),
        .s.entryCount = 0,
        .buckets = NULL,
    };
}

linkage void type_name(name_free)(cx_hmap_name* m) {
    cxHmapFreeFn(&m->s);
}

linkage void type_name(name_set)(cx_hmap_name* m, cx_hmap_key k, cx_hmap_val v) {
    type_name(_entry)* e = cxHmapOperFn(&m->s, cx_hmap_op_set, &k);
    e->val = v;
}

linkage cx_hmap_val* type_name(name_get)(cx_hmap_name* m, cx_hmap_key k) {
    type_name(_entry)* e = cxHmapOperFn(&m->s, cx_hmap_op_get, &k);
    return e == NULL ? NULL : &e->val;
}

linkage bool type_name(name_del)(cx_hmap_name* m, cx_hmap_key k) {
    type_name(_entry)* e = cxHmapOperFn(&m->s, cx_hmap_op_del, &k);
    return e == NULL ? false : true;
}

linkage size_t type_name(name_count)(cx_hmap_name* m) {
    return m->s.entryCount;
}

linkage type_name(_entry)* type_name(name_next)(cx_hmap_name* m, type_name(_iter)* iter) {
    return cxHmapNext(&m->s, iter);
}

linkage cx_hmap_name type_name(_clone)(cx_hmap_name* src, size_t nbuckets, const CxAllocator* alloc) {
    cx_hmap_name dst = type_name(_init2)(nbuckets, alloc == NULL ? src->s.alloc : alloc);
    type_name(_iter) iter = {0};
    while (true) {
        type_name(_entry)* e = type_name(_next)(src, &iter);
        if (e == NULL) {
            return dst;
        }
        type_name(_set)(&dst, e->key, e->val);
    }
    return dst;
}

#endif

#undef cx_hmap_name
#undef cx_hmap_key
#undef cx_hmap_val
#undef cx_hmap_camel_case
#undef cx_hmap_implement
#undef concat2_
#undef concat1_
#undef type_name
#undef linkage
#undef name_init
#undef name_init2
#undef name_free
#undef name_set
#undef name_get
#undef name_del
#undef name_count
#undef name_next
#undef name_clone



