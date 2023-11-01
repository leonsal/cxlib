#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "cx_alloc.h"

// Default string type name
#ifndef cx_str_name
    #define cx_str_name CxStr
#endif

// String capacity in number of bits
#define cx_str_cap8     8
#define cx_str_cap16    16
#define cx_str_cap32    32
#ifndef cx_str_cap
    #define cx_str_cap cx_str_cap8
#endif
#if cx_str_cap == cx_str_cap8
    #define cx_str_cap_type uint8_t
#elif cx_str_cap == cx_str_cap16
    #define cx_str_cap_type uint16_t
#elif cx_str_cap == cx_str_cap32
    #define cx_str_cap_type uint32_t
#else
    #error "invalid cx string capacity bits"
#endif

// Custom allocator
#ifndef cx_str_allocator
    #define cx_str_alloc
    #define str_alloc(s,n)  malloc(n)
    #define str_free(s,p,n) free(p)
#else
    #define cx_str_alloc        const CxAllocator* alloc;
    #define str_alloc(s,n)      s->alloc->alloc(s->alloc->ctx, n)
    #define str_free(s,p,n)     s->alloc->free(s->alloc->ctx, p, n)
#endif

#define concat2_(a, b) a ## b
#define concat1_(a, b) concat2_(a, b)
#define type_name(name) concat1_(cx_str_name, name)

#ifdef cx_str_static
    #define linkage static
#else
    #define linkage
#endif

//
// Function names
//
#ifdef cx_array_camel_case
    #define name_init           Init
    #define name_init2          Init2
#else
    #define name_init           _init
    #define name_init2          _init2
#endif

//
// Declarations
//
typedef struct cx_str_name {
    cx_str_alloc
    cx_str_cap_type len;
    cx_str_cap_type cap;
    char* data;
} cx_str_name;

linkage cx_str_name type_name(name_init)(void);
linkage cx_str_name type_name(name_init2)(const CxAllocator*);

//
// Implementations
//
#ifdef cx_str_implement

void cxStrGrowFn(cx_str_name* s, size_t addLen, size_t minCap) {

    size_t minLen = s->len + addLen;
    if (minLen > minCap) {
        minCap = minLen;
    }
    if (minCap <= s->cap) {
        return;
    }

    // Increase needed capacity to guarantee O(1) amortized
    if (minCap < 2 * s->cap) {
        minCap = 2 * s->cap;
    }
    else if (minCap < 4) {
        minCap = 4;
    }

    // Allocates new capacity
    size_t allocSize = sizeof(char) * minCap;
    void* new = str_alloc(s, allocSize);
    if (new == NULL) {
        return;
    }

    // Copy current data to new area and free previous
    memcpy(new, s->data, s->len * sizeof(char));
    str_free(s, s->data, s->len * sizeof(char));
    s->data = new;
    s->cap = minCap;
}


linkage cx_str_name type_name(name_init)(void) {
    return (cx_str_name) {
        .len = 0,
        .cap = 0,
        .data = NULL,
    };
}

#ifdef cx_str_allocator
linkage cx_str_name type_name(name_init2)(const CxAllocator* alloc) {
    return (cx_str_name) {
        .alloc = alloc,
        .len = 0,
        .cap = 0,
        .data = NULL,
    };
}
#endif

#endif // cx_str_implement

