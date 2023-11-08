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

// Auxiliary internal macros
#define cx_hmap_concat2_(a, b) a ## b
#define cx_hmap_concat1_(a, b) cx_hmap_concat2_(a, b)
#define cx_hmap_name_(name) cx_hmap_concat1_(cx_hmap_name, name)

// API attributes
#if defined(cx_hmap_static) && defined(cx_hmap_inline)
    #define cx_hmap_api_ static inline
#elif defined(cx_hmap_static)
    #define cx_hmap_api_ static
#elif defined(cx_hmap_inline)
    #define cx_hmap_api_ inline
#else
    #define cx_hmap_api_
#endif

//
// Declarations (only generated once)
//
#ifndef CX_HMAP_H 
#define CX_HMAP_H

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

typedef struct cx_hmap_name_(_entry) {
    struct cx_hmap_name_(_entry)* n;
    cx_hmap_key key;
    cx_hmap_val val;
} cx_hmap_name_(_entry);

typedef struct cx_hmap_name {
    CxHmapState s;
    cx_hmap_name_(_entry)* buckets;
} cx_hmap_name;

// Declare hmap iter type
typedef CxHmapIter cx_hmap_name_(_iter);

cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init)(void);
cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init2)(size_t nbuckets, const CxAllocator* alloc);
cx_hmap_api_ void cx_hmap_name_(_free)(cx_hmap_name* m);
cx_hmap_api_ void cx_hmap_name_(_set)(cx_hmap_name* m, cx_hmap_key k, cx_hmap_val v);
cx_hmap_api_ cx_hmap_val* cx_hmap_name_(_get)(cx_hmap_name* m, cx_hmap_key k);
cx_hmap_api_ bool cx_hmap_name_(_del)(cx_hmap_name* m, cx_hmap_key k);
cx_hmap_api_ size_t cx_hmap_name_(_count)(cx_hmap_name* m);
cx_hmap_api_ cx_hmap_name_(_entry)* cx_hmap_name_(_next)(cx_hmap_name* m, cx_hmap_name_(_iter)* iter);
cx_hmap_api_ cx_hmap_name cx_hmap_name_(_clone)(cx_hmap_name* src, size_t nbuckets, const CxAllocator* alloc);

#endif

//
// Implementations
//
#ifdef cx_hmap_implement

cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init)(void) {
    return cx_hmap_name_(_init2)(cx_hmap_def_nbuckets, cxDefaultAllocator());
}

cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init2)(size_t nbuckets, const CxAllocator* alloc) {
    return (cx_hmap_name){
        .s.alloc = alloc == NULL ? cxDefaultAllocator() : alloc,
        .s.bucketCount = nbuckets == 0 ? cx_hmap_def_nbuckets : nbuckets,
        .s.entrySize = sizeof(cx_hmap_name_(_entry)),
        .s.keySize = sizeof(((cx_hmap_name_(_entry)*)0)->key),
        .s.entryCount = 0,
        .buckets = NULL,
    };
}

cx_hmap_api_ void cx_hmap_name_(_free)(cx_hmap_name* m) {
    cxHmapFreeFn(&m->s);
}

cx_hmap_api_ void cx_hmap_name_(_set)(cx_hmap_name* m, cx_hmap_key k, cx_hmap_val v) {
    cx_hmap_name_(_entry)* e = cxHmapOperFn(&m->s, cx_hmap_op_set, &k);
    e->val = v;
}

cx_hmap_api_ cx_hmap_val* cx_hmap_name_(_get)(cx_hmap_name* m, cx_hmap_key k) {
    cx_hmap_name_(_entry)* e = cxHmapOperFn(&m->s, cx_hmap_op_get, &k);
    return e == NULL ? NULL : &e->val;
}

cx_hmap_api_ bool cx_hmap_name_(_del)(cx_hmap_name* m, cx_hmap_key k) {
    cx_hmap_name_(_entry)* e = cxHmapOperFn(&m->s, cx_hmap_op_del, &k);
    return e == NULL ? false : true;
}

cx_hmap_api_ size_t cx_hmap_name_(_count)(cx_hmap_name* m) {
    return m->s.entryCount;
}

cx_hmap_api_ cx_hmap_name_(_entry)* cx_hmap_name_(_next)(cx_hmap_name* m, cx_hmap_name_(_iter)* iter) {
    return cxHmapNext(&m->s, iter);
}

cx_hmap_api_ cx_hmap_name cx_hmap_name_(_clone)(cx_hmap_name* src, size_t nbuckets, const CxAllocator* alloc) {
    cx_hmap_name dst = cx_hmap_name_(_init2)(nbuckets, alloc == NULL ? src->s.alloc : alloc);
    cx_hmap_name_(_iter) iter = {0};
    while (true) {
        cx_hmap_name_(_entry)* e = cx_hmap_name_(_next)(src, &iter);
        if (e == NULL) {
            return dst;
        }
        cx_hmap_name_(_set)(&dst, e->key, e->val);
    }
    return dst;
}

#endif

// Undefine config  macros
#undef cx_hmap_name
#undef cx_hmap_key
#undef cx_hmap_val
#undef cx_hmap_implement

// Undefine internal macros
#undef cx_hmap_concat2_
#undef cx_hmap_concat1_
#undef cx_hmap_name_
#undef cx_hmap_api_



