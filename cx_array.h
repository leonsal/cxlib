/* Dynamic Array Implementation
 *
Example
-------

// Defines array of 'ints' using global type allocator
#define cx_array_name ai32
#define cx_array_type int
#define cx_array_static
#define cx_array_inline
#define cx_array_implement
#include "cx_str.h"

// Defines array of 'doubles' using instance allocator
#define cx_array_name af64
#define cx_array_type double
#define cx_array_static
#define cx_array_inline
#define cx_array_allocator
#define cx_array_implement
#include "cx_str.h"

int main() {

    ai32 a1 = ai32_init();
    
    return 0;
}
 
Configuration before including header file
------------------------------------------

Define the name of the array type (mandatory):
    #define cx_array_name <name>

Define the type of the array elements (mandatory):
    #define cx_array_type <name>

Define the array maximum capacity (optional, default = 32):
    #define cx_array_cap <8|16|32|64>

Define error handler function (optional):
    #define cx_array_error_handler <func>

Sets if array uses custom allocator per instance
    #define cx_array_allocator

Sets if all array functions are prefixed with 'static'
    #define cx_array_static

Sets if all array functions are prefixed with 'inline'
    #define cx_array_inline

Sets to implement functions in this translation unit:
    #define cx_array_implement

Returns current array capacity in number of elements
    size_t cxarray_cap(const cxarray* s);

Returns current array length in number of elements
    size_t cxarray_len(const cxarray* s);

API
---
Assuming:
#define cx_array_name cxarray   // Array type
#define cx_array_type cxtype    // Type of elements of the array

Initialize array defined with custom allocator
    cxarray cxarray_init(const CxAllocator* a);

Initialize string NOT defined with custom allocator
    cxarray cxarray_init();

Free array allocated memory
    void cxarray_free(cxarray* s);

Clone array returning a copy
    cxarray cxarray_clone(const cxarray a);

Returns it the array is empty (length == 0)
    bool cxarray_empty(const cxarray* a);

Sets the capacity of the array at least 'cap'
    void cxarray_setcap(cxarray* a, size_t cap);

Sets the length of the array to 'len'
    void cxarray_setlen(cxarray* a, size_t len);

Pushes one element at the back of the array
    void cxarray_push(cxarray* a, cxtype v);

Pushes 'n' elements from 'src' at the back of the array
    void cxarray_pushn(cxarray* a, const cxtype* src, size_t n);

Pushes all elements from 'src' array at the back of this array
    void cxarray_pusha(cxarray* a, const cxarray* src);

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

// Array maximum capacity in number of bits
#define cx_array_cap8_     8
#define cx_array_cap16_    16
#define cx_array_cap32_    32
#define cx_array_cap64_    64

// Default capacity
#ifndef cx_array_cap
    #define cx_array_cap  cx_array_cap32_
#endif
#if cx_array_cap == cx_array_cap8_
    #define cx_array_cap_type_ uint8_t
    #define cx_array_max_cap_  (UINT8_MAX)
#elif cx_array_cap == cx_array_cap16_
    #define cx_array_cap_type_ uint16_t
    #define cx_array_max_cap_  (UINT16_MAX)
#elif cx_array_cap == cx_array_cap32_
    #define cx_array_cap_type_ uint32_t
    #define cx_array_max_cap_  (UINT32_MAX)
#elif cx_array_cap == cx_array_cap64_
    #define cx_array_cap_type_ uint64_t
    #define cx_array_max_cap_  (UINT64_MAX)
#else
    #error "invalid cx array capacity bits"
#endif

// Error handler
#ifndef cx_array_error_handler
    #define cx_array_error_handler(msg)\
        printf("CXLIB ARRAY ERROR:%s\n",msg);abort()
#endif

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

// Use custom instance allocator
#ifdef cx_array_allocator
    #define cx_array_alloc_field_\
        const CxAllocator* alloc;
    #define cx_array_alloc_global_
    #define cx_array_alloc_(s,n)\
        cx_alloc_alloc(s->alloc, n)
    #define cx_array_free_(s,p,n)\
        cx_alloc_free(s->alloc, p, n)
// Use global type allocator
#else
    #define cx_array_alloc_field_
    #define cx_array_alloc_global_\
        static const CxAllocator* cx_array_name_(_allocator) = NULL;
    #define cx_array_alloc_(s,n)\
        cx_alloc_alloc(cx_array_name_(_allocator),n)
    #define cx_array_free_(s,p,n)\
        cx_alloc_free(cx_array_name_(_allocator),p,n)
#endif

//
// Declarations
//
typedef struct cx_array_name {
    cx_array_alloc_field_
    cx_array_cap_type_  len_;
    cx_array_cap_type_  cap_;
    cx_array_type*      data;
} cx_array_name;

#ifdef cx_str_allocator
    cx_array_api_ cx_array_name cx_array_name_(_init)(const CxAllocator*);
#else
    cx_array_api_ cx_array_name cx_array_name_(_init)(void);
#endif
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
    cx_array_alloc_global_;
    void cxArrayGrowFn(void* ag, size_t elemsize, size_t addlen, size_t min_cap);

#ifdef cx_array_allocator

    cx_array_api_ cx_array_name cx_array_name_(_init2)(const CxAllocator* alloc) {
        return (cx_array_name) {
            .alloc = alloc,
            .len_ = 0,
            .cap_ = 0,
            .data = NULL,
        };
    }

#else

    cx_array_api_ cx_array_name cx_array_name_(_init)(void) {
        return (cx_array_name) {
            .len_ = 0,
            .cap_ = 0,
            .data = NULL,
        };
    }

#endif

cx_array_api_ void cx_array_name_(_free)(cx_array_name* a) {
    cx_array_free_(a, a->data, a->cap_ * sizeof(*(a->data)));
    a->len_ = 0;
    a->cap_ = 0;
    a->data = NULL;
}

cx_array_api_ void cx_array_name_(_clear)(cx_array_name* a) {
    a->len_ = 0;
}

cx_array_api_ cx_array_name cx_array_name_(_clone)(cx_array_name* a) {
    const size_t alloc_size = a->len_ * sizeof(*(a->data));
    cx_array_name cloned = *a;
    cloned.data  = cx_array_alloc_(a, alloc_size),
    memcpy(cloned.data, a->data, alloc_size);
    return cloned;
}

cx_array_api_ ptrdiff_t cx_array_name_(_cap)(cx_array_name* a) {
    return a->cap_;
}

cx_array_api_ ptrdiff_t cx_array_name_(_len)(cx_array_name* a) {
    return a->len_;
}

cx_array_api_ bool cx_array_name_(_empty)(cx_array_name* a) {
    return a->len_ == 0;
} 

cx_array_api_ void cx_array_name_(_setcap)(cx_array_name* a, size_t cap) {
    cxArrayGrowFn(a, sizeof(*(a->data)), 0, cap);
}

cx_array_api_ void cx_array_name_(_setlen)(cx_array_name* a, size_t len) {
    if (a->cap_ < len) {
        cxArrayGrowFn(a, sizeof(*(a->data)), len, 0);
    }
    a->len_ = len;
}

cx_array_api_ void cx_array_name_(_push)(cx_array_name* a, cx_array_type v) {
    if (a->len_ >= a->cap_) {
        cxArrayGrowFn(a, sizeof(*(a->data)), 1, 0);
    }
    a->data[a->len_++] = v;
}
 
cx_array_api_ cx_array_type cx_array_name_(_pop)(cx_array_name* a) {
    a->len_--;
    return a->data[a->len_];
}

cx_array_api_ void cx_array_name_(_append)(cx_array_name* a, cx_array_type* p, size_t n) {
    if (a->len_ + n > a->cap_) {
        cxArrayGrowFn(a, sizeof(*(a->data)), n, 0);
    }
    memcpy(&a->data[a->len_], p, n * sizeof(*(a->data)));
    a->len_ += n;
}

cx_array_api_ void cx_array_name_(_append_array)(cx_array_name* a, const cx_array_name* src) {
    cx_array_name_(_append)(a, src->data, src->len_);
}

cx_array_api_ cx_array_type* cx_array_name_(_at)(cx_array_name* a, size_t idx) {
    if (idx > a->len_) {
        abort();
    }
    return &a->data[idx];
}

cx_array_api_ cx_array_type cx_array_name_(_last)(const cx_array_name* a) {
    return a->data[a->len_-1];
}

cx_array_api_ void cx_array_name_(_reserve)(cx_array_name* a, size_t n) {
    if (a->cap_ < a->len_ + n) {
        cxArrayGrowFn(a, sizeof(*(a->data)), 0, a->len_+n);
    }
}

cx_array_api_ void cx_array_name_(_insn)(cx_array_name* a, size_t i, size_t n) {
    if (a->len_ + n > a->cap_) {
        cxArrayGrowFn(a, sizeof(*(a->data)),n,0);
    }
    a->len_ += n;
    memmove(&a->data[i+n], &a->data[i], sizeof(*(a->data)) * (a->len_-n-i));
}

cx_array_api_ void cx_array_name_(_ins)(cx_array_name* a, size_t i, cx_array_type v) {
    cx_array_name_(_insn)(a, i, 1);
    a->data[i] = v;
}

cx_array_api_ void cx_array_name_(_deln)(cx_array_name* a, size_t i, size_t n) {
    memmove(&a->data[i], &a->data[i+n], sizeof(*(a->data)) * (a->len_-n-i));
    a->len_ -= n;
}

cx_array_api_ void cx_array_name_(_del)(cx_array_name* a, size_t i) {
    cx_array_name_(_deln)(a, i, 1);
}

cx_array_api_ void cx_array_name_(_delswap)(cx_array_name* a, size_t i) {
    a->data[i] = cx_array_name_(_last)(a);
    a->len_--;
}

cx_array_api_ void cx_array_name_(_sort)(cx_array_name* a, int (*f)(const cx_array_type*, const cx_array_type*)) {
    qsort(a->data,a->len_,sizeof(*(a->data)),(int (*)(const void*,const void*))f);
}

#endif

// Undefine config  macros
#undef cx_array_name
#undef cx_array_type
#undef cx_array_static
#undef cx_array_inline

// Undefine internal macros
#undef cx_array_concat2_
#undef cx_array_concat1_
#undef cx_array_name_
#undef cx_array_api_

