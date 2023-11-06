/* Dynamic Array Implementation
 *
Example
-------

#define cx_array_name cxarr
#define cx_array_type int
#define cx_array_static
#define cx_array_inline
#define cx_array_implement
#include "cx_str.h"

int main() {

    cxarr a1 = cxarr_init();

    return 0;
}
 
Configuration before including header file
------------------------------------------

Define the name of the array type (mandatory):
    #define cx_array_name <name>

Define the type of the array elements (mandatory):
    #define cx_array_type <name>

*/ 
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "cx_alloc.h"

// Array type name must be defined
#ifndef cx_array_name
    #error "cx_array_name not defined"
#endif
// Array element type name must be defined
#ifndef cx_array_type
    #error "cx_array_type not defined"
#endif

// Auxiliary internal macros
#define cx_array_concat2_(a, b) a ## b
#define cx_array_concat1_(a, b) cx_array_concat2_(a, b)
#define cx_array_name_(name) cx_array_concat1_(cx_array_name, name)

// API attributes
#if defined(cx_array_static) && defined(cx_array_inline)
    #define cx_array_api_ static inline
#elif defined(cx_array_static)
    #define cx_array_api_ static
#elif defined(cx_array_inline)
    #define cx_array_api_ inline
#else
    #define cx_array_api_
#endif

//
// Declarations
//
typedef struct cx_array_name {
    const CxAllocator* alloc;
    ptrdiff_t   len;
    ptrdiff_t   cap;
    cx_array_type*  data;
} cx_array_name;

cx_array_api_ cx_array_name cx_array_name_(_init)(void);
cx_array_api_ cx_array_name cx_array_name_(_init2)(const CxAllocator*);
cx_array_api_ void cx_array_name_(_free)(cx_array_name* a);
cx_array_api_ void cx_array_name_(_clear)(cx_array_name* a);
cx_array_api_ cx_array_name cx_array_name_(_clone)(cx_array_name* a);
cx_array_api_ ptrdiff_t cx_array_name_(_cap)(cx_array_name* a);
cx_array_api_ ptrdiff_t cx_array_name_(_len)(cx_array_name* a);
cx_array_api_ bool cx_array_name_(_empty)(cx_array_name* a);
cx_array_api_ void cx_array_name_(_setcap)(cx_array_name* a, size_t cap);
cx_array_api_ void cx_array_name_(_setlen)(cx_array_name* a, size_t len);
cx_array_api_ void cx_array_name_(_push)(cx_array_name* a, cx_array_type v);
cx_array_api_ cx_array_type cx_array_name_(_pop)(cx_array_name* a);
cx_array_api_ void cx_array_name_(_append)(cx_array_name* a, cx_array_type* p, size_t n);
cx_array_api_ void cx_array_name_(_append_array)(cx_array_name* a, const cx_array_name* src);
cx_array_api_ cx_array_type* cx_array_name_(_at)(cx_array_name* a, size_t idx);
cx_array_api_ cx_array_type cx_array_name_(_last)(const cx_array_name* a);
cx_array_api_ void cx_array_name_(_reserve)(cx_array_name* a, size_t n);
cx_array_api_ void cx_array_name_(_insn)(cx_array_name* a, size_t i, size_t n);
cx_array_api_ void cx_array_name_(_ins)(cx_array_name* a, size_t i, cx_array_type v);
cx_array_api_ void cx_array_name_(_deln)(cx_array_name* a, size_t i, size_t n);
cx_array_api_ void cx_array_name_(_del)(cx_array_name* a, size_t i);
cx_array_api_ void cx_array_name_(_delswap)(cx_array_name* a, size_t i);
cx_array_api_ void cx_array_name_(_sort)(cx_array_name* a, int (*f)(const cx_array_type*, const cx_array_type*));

//
// Implementations
//
#ifdef cx_array_implement
void cxArrayGrowFn(void* ag, size_t elemsize, size_t addlen, size_t min_cap);

cx_array_api_ cx_array_name cx_array_name_(_init)(void) {
    return (cx_array_name) {
        .alloc = cxDefaultAllocator(),
        .len = 0,
        .cap = 0,
        .data = NULL,
    };
}

cx_array_api_ cx_array_name cx_array_name_(_init2)(const CxAllocator* alloc) {
    return (cx_array_name) {
        .alloc = alloc,
        .len = 0,
        .cap = 0,
        .data = NULL,
    };
}

cx_array_api_ void cx_array_name_(_free)(cx_array_name* a) {
    a->alloc->free(a->alloc->ctx, a->data, a->cap * sizeof(*(a->data)));
    a->len = 0;
    a->cap = 0;
    a->data = NULL;
}

cx_array_api_ void cx_array_name_(_clear)(cx_array_name* a) {
    a->len = 0;
}

cx_array_api_ cx_array_name cx_array_name_(_clone)(cx_array_name* a) {
    const size_t alloc_size = a->len * sizeof(*(a->data));
    cx_array_name cloned = {
        .alloc = a->alloc,
        .len   = a->len,
        .cap   = a->len,
        .data  = a->alloc->alloc(a->alloc->ctx, alloc_size),
    };\
    memcpy(cloned.data, a->data, alloc_size);
    return cloned;
}

cx_array_api_ ptrdiff_t cx_array_name_(_cap)(cx_array_name* a) {
    return a->cap;
}

cx_array_api_ ptrdiff_t cx_array_name_(_len)(cx_array_name* a) {
    return a->len;
}

cx_array_api_ bool cx_array_name_(_empty)(cx_array_name* a) {
    return a->len == 0;
} 

cx_array_api_ void cx_array_name_(_setcap)(cx_array_name* a, size_t cap) {
    cxArrayGrowFn(a, sizeof(*(a->data)), 0, cap);
}

cx_array_api_ void cx_array_name_(_setlen)(cx_array_name* a, size_t len) {
    if (a->cap < len) {
        cxArrayGrowFn(a, sizeof(*(a->data)), len, 0);
    }
    a->len = len;
}

cx_array_api_ void cx_array_name_(_push)(cx_array_name* a, cx_array_type v) {
    if (a->len >= a->cap) {
        cxArrayGrowFn(a, sizeof(*(a->data)), 1, 0);
    }
    a->data[a->len++] = v;
}
 
cx_array_api_ cx_array_type cx_array_name_(_pop)(cx_array_name* a) {
    a->len--;
    return a->data[a->len];
}

cx_array_api_ void cx_array_name_(_append)(cx_array_name* a, cx_array_type* p, size_t n) {
    if (a->len + n > a->cap) {
        cxArrayGrowFn(a, sizeof(*(a->data)), n, 0);
    }
    memcpy(&a->data[a->len], p, n * sizeof(*(a->data)));
    a->len += n;
}

cx_array_api_ void cx_array_name_(_append_array)(cx_array_name* a, const cx_array_name* src) {
    cx_array_name_(_append)(a, src->data, src->len);
}

cx_array_api_ cx_array_type* cx_array_name_(_at)(cx_array_name* a, size_t idx) {
    if (idx > a->len) {
        abort();
    }
    return &a->data[idx];
}

cx_array_api_ cx_array_type cx_array_name_(_last)(const cx_array_name* a) {
    return a->data[a->len-1];
}

cx_array_api_ void cx_array_name_(_reserve)(cx_array_name* a, size_t n) {
    if (a->cap < a->len + n) {
        cxArrayGrowFn(a, sizeof(*(a->data)), 0, a->len+n);
    }
}

cx_array_api_ void cx_array_name_(_insn)(cx_array_name* a, size_t i, size_t n) {
    if (a->len + n > a->cap) {
        cxArrayGrowFn(a, sizeof(*(a->data)),n,0);
    }
    a->len += n;
    memmove(&a->data[i+n], &a->data[i], sizeof(*(a->data)) * (a->len-n-i));
}

cx_array_api_ void cx_array_name_(_ins)(cx_array_name* a, size_t i, cx_array_type v) {
    cx_array_name_(_insn)(a, i, 1);
    a->data[i] = v;
}

cx_array_api_ void cx_array_name_(_deln)(cx_array_name* a, size_t i, size_t n) {
    memmove(&a->data[i], &a->data[i+n], sizeof(*(a->data)) * (a->len-n-i));
    a->len -= n;
}

cx_array_api_ void cx_array_name_(_del)(cx_array_name* a, size_t i) {
    cx_array_name_(_deln)(a, i, 1);
}

cx_array_api_ void cx_array_name_(_delswap)(cx_array_name* a, size_t i) {
    a->data[i] = cx_array_name_(_last)(a);
    a->len--;
}

cx_array_api_ void cx_array_name_(_sort)(cx_array_name* a, int (*f)(const cx_array_type*, const cx_array_type*)) {
    qsort(a->data,a->len,sizeof(*(a->data)),(int (*)(const void*,const void*))f);
}

#endif

#undef cx_array_name
#undef cx_array_type
#undef cx_array_static
#undef cx_array_camel_case
#undef concat2_
#undef concat1_
#undef cx_array_name_
#undef cx_array_api_
#undef name_init
#undef name_init2
#undef name_free
#undef name_clear
#undef name_clone
#undef name_cap
#undef name_len
#undef name_empty
#undef name_setcap
#undef name_setlen
#undef name_push
#undef name_pop
#undef name_append
#undef name_append_array
#undef name_at
#undef name_last
#undef name_reserve
#undef name_insn
#undef name_ins
#undef name_deln
#undef name_del
#undef name_delswap
#undef name_sort


