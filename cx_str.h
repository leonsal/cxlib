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
    #define cx_str_max_cap  (UINT8_MAX)
#elif cx_str_cap == cx_str_cap16
    #define cx_str_cap_type uint16_t
    #define cx_str_max_cap  (UINT16_MAX)
#elif cx_str_cap == cx_str_cap32
    #define cx_str_cap_type uint32_t
    #define cx_str_max_cap  (UINT32_MAX)
#else
    #error "invalid cx string capacity bits"
#endif

// Custom allocator
#ifndef cx_str_allocator
    #define cx_str_alloc_field
    #define str_alloc(s,n)      malloc(n)
    #define str_free(s,p,n)     free(p)
#else
    #define cx_str_alloc_field  const CxAllocator* alloc;
    #define str_alloc(s,n)      s->alloc->alloc(s->alloc->ctx, n)
    #define str_free(s,p,n)     s->alloc->free(s->alloc->ctx, p, n)
#endif

// Auxiliary macros
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
#ifdef cx_str_camel_case
    #define name_init           Init
    #define name_init2          Init2
    #define name_free           Free
    #define name_clear          Clear
    #define name_clone          Clone
    #define name_cap            Cap
    #define name_len            Len
    #define name_empty          Empty
    #define name_setcap         SetCap
    #define name_setlen         SetLen
    #define name_set            Set
    #define name_setn           Setn
    #define name_sets           Sets
    #define name_cat            Cat
    #define name_catn           Catn
    #define name_cats           Cats
    #define name_insn           Insn
    #define name_ins            Ins
    #define name_inss           Inss
#else
    #define name_init           _init
    #define name_init2          _init2
    #define name_free           _free
    #define name_clear          _clear
    #define name_clone          _clone
    #define name_cap            _cap
    #define name_len            _len
    #define name_empty          _empty
    #define name_setcap         _setcap
    #define name_setlen         _setlen
    #define name_set            _set
    #define name_setn           _setn
    #define name_sets           _sets
    #define name_cat            _cat
    #define name_catn           _catn
    #define name_cats           _cats
    #define name_insn           _insn
    #define name_ins            _ins
    #define name_inss           _inss
#endif

//
// Declarations
//
typedef struct cx_str_name {
    cx_str_alloc_field;
    cx_str_cap_type len;
    cx_str_cap_type cap;
    char* data;
} cx_str_name;

linkage cx_str_name type_name(name_init)(void);
#ifdef cx_str_allocator
    linkage cx_str_name type_name(name_init2)(const CxAllocator*);
#endif
linkage void type_name(name_set)(cx_str_name* s, const char* src);


//
// Implementations
//
#ifdef cx_str_implement

static void type_name(_grow_)(cx_str_name* s, size_t addLen, size_t minCap) {

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
    if (minCap + 1 > cx_str_max_cap) {
        abort();
    }

    // Allocates new capacity
    const size_t elemSize = sizeof(*(s->data));
    const size_t allocSize = elemSize * (minCap + 1);
    void* new = str_alloc(s, allocSize);
    if (new == NULL) {
        return;
    }

    // Copy current data to new area and free previous
    if (s->data) {
        memcpy(new, s->data, (s->len + 1) * elemSize);
        str_free(s, s->data, (s->len + 1) * elemSize);
    }
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

linkage void type_name(name_free)(cx_str_name* s) {

    str_free(s, s->data, s->cap);
    s->cap = 0;
    s->len = 0;
}

linkage void type_name(name_clear)(cx_str_name* s) {

    s->len = 0;
}

linkage cx_str_name type_name(name_clone)(const cx_str_name* s) {

    cx_str_name cloned = *s;
    cloned.data = str_alloc(s, s->cap + 1);
    memcpy(cloned.data, s->data, s->len + 1);
    return cloned;
}

linkage size_t type_name(name_cap)(const cx_str_name* s) {

    return s->cap;
}

linkage size_t type_name(name_len)(const cx_str_name* s) {

    return s->len;
}

linkage bool type_name(name_empty)(const cx_str_name* s) {

    return s->len == 0;
}

linkage void type_name(name_setcap)(cx_str_name* s, size_t cap) {

    type_name(_grow_)(s, 0, cap);
}

linkage void type_name(name_setlen)(cx_str_name* s, size_t len) {

    if (len > s->cap) {
        type_name(_grow_)(s, len, 0);
    }
    s->len = len;
    s->data[s->len] = 0;
}

linkage void type_name(name_set)(cx_str_name* s, const char* src) {

    size_t srcLen = strlen(src);
    if (srcLen > s->cap) {
        type_name(_grow_)(s, 0, srcLen);
    }
    strcpy(s->data, src);
    s->len = srcLen;
}

linkage void type_name(name_setn)(cx_str_name* s, const char* src, size_t n) {

    if (n > s->cap) {
        type_name(_grow_)(s, 0, n);
    }
    memcpy(s->data, src, n);
    s->data[n] = 0;
    s->len = n;
}

linkage void type_name(name_sets)(cx_str_name* s, const cx_str_name* src) {

    if (src->cap > s->cap) {
        type_name(_grow_)(s, 0, src->cap);
    }
    memcpy(s->data, src->data, src->len);
    s->len = src->len;
    s->data[s->len] = 0;
}

linkage void type_name(name_catn)(cx_str_name* s, const char* src, size_t n) {

    if (s->len + n > s->cap) {
        type_name(_grow_)(s, s->len + n, 0);
    }
    memcpy(s->data + s->len, src, n);
    s->len += n;
    s->data[s->len] = 0;
}

linkage void type_name(name_cat)(cx_str_name* s, const char* src) {

    type_name(name_catn)(s, src, strlen(src));
}

linkage void type_name(name_cats)(cx_str_name* s, const cx_str_name* src) {

    type_name(name_catn)(s, src->data, src->len);
}

linkage void type_name(name_insn)(cx_str_name* s, const char* src, size_t n, size_t idx) {

    if (idx > s->len) {
        abort();
    }
    if (s->len + n > s->cap) {
        type_name(_grow_)(s, s->len + n, 0);
    }
    memmove(s->data + idx + n, s->data + idx, s->len - idx);
    memcpy(s->data + idx, src, n);
    s->len += n;
    s->data[s->len] = 0;
}

linkage void type_name(name_ins)(cx_str_name* s, const char* src, size_t idx) {

    type_name(name_insn)(s, src, strlen(src), idx);
}

linkage void type_name(name_inss)(cx_str_name* s, const cx_str_name* src, size_t idx) {

    type_name(name_insn)(s, src->data, src->len, idx);
}

#endif // cx_str_implement


#undef cx_str_name
#undef cx_str_cap8
#undef cx_str_cap16
#undef cx_str_cap32
#undef cx_str_cap
#undef cx_str_cap_type
#undef cx_str_max_cap
#undef cx_str_allocator
#undef cx_str_alloc_field
#undef cx_str_camel_case
#undef str_alloc
#undef str_free

