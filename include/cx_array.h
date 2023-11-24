/* Dynamic Array Implementation
 
Example
-------

// Defines array of 'ints' using global type allocator
#define cx_array_name ai32
#define cx_array_type int
#define cx_array_static
#define cx_array_inline
#define cx_array_implement
#include "cx_array.h"

int main() {

    ai32 a1 = ai32_init();
    ai32_push(&a1, 1);
    ai32_push(&a2, 2);
    assert(ai32_len(&a1) = 2);
    assert(*ai32_at(&a1, 1) = 1);
    return 0;
}
 
Array configuration defines
---------------------------

Define the name of the array type (mandatory):
    #define cx_array_name <name>

Define the type of the array elements (mandatory):
    #define cx_array_type <name>

Define optional array maximum capacity (default = 32):
    #define cx_array_cap <8|16|32>

Define optional error handler function with type:
void (*handler)(const char* err_msg, const char* func_name)
which will be called if error is detected  (default = no handler):
    #define cx_array_error_handler <func>

Define optional custom allocator pointer or function which return pointer to allocator.
Uses default allocator if not defined.
This allocator will be used for all instances of this array type.
    #define cx_array_allocator <allocator>

Sets if array uses custom allocator per instance.
If set, it is necessary to initialize each array with the desired allocator.
    #define cx_array_instance_allocator

Defines optional array element comparison function used in find().
If not defined, uses 'memcmp()'
    #define cx_array_cmp_el <cmp_func>

Sets if all array functions are prefixed with 'static'
    #define cx_array_static

Sets if all array functions are prefixed with 'inline'
    #define cx_array_inline

Sets to implement functions in this translation unit:
    #define cx_array_implement


Array API
---------

Assuming:
#define cx_array_name cxarray   // Array type
#define cx_array_type cxtype    // Type of elements of the array

Initialize array defined with custom allocator
    cxarray cxarray_init(const CxAllocator* a);

Initialize array NOT defined with custom allocator.
It is equivalent to zero initialize the array struct.
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

Pushes 'n' elements from 'src' at the back of the array
    void cxarray_pushn(cxarray* a, const cxtype* src, size_t n);

Pushes one element at the back of the array
    void cxarray_push(cxarray* a, cxtype v);

Pushes all elements from 'src' array at the back of this array
    void cxarray_pusha(cxarray* a, const cxarray* src);

Pops and returns the last element of the array.
Error handler is called if defined and array is empty.
    cxtype cxarray_pop(cxarray* a);

Returns pointer to array the element at the specified index 'idx'.
Error handler is called if defined and index is invalid,
otherwise NULL is returned.
    cxtype* cxarray_at(const cxarray* a, size_t idx);

Returns the last element of the array without removing it.
Error handler is called if defined and array is empty.
    cxtype cxrray_last(const cxarray* a);

Reserve capacity for at least new 'n' elements in the array.
    void cxarray_reserve(cxarray* a, size_t n);

Inserts 'n' elements from 'src' into the array at index 'idx'.
Error handler is called if defined and index is invalid,
    void cxarray_insn(cxarray* a, const cxtype* src, size_t n, size_t idx);

Inserts specified value into the array at index 'idx'
Error handler is called if defined and index is invalid,
    void cxarray_ins(cxarray* a, cxtype v, size_t idx);

Inserts array 'src' into this array at index 'idx'
Error handler is called if defined and index is invalid,
    void cxarray_insa(cxarray* a, const cxarray* src, size_t idx);

Deletes 'n' elements from this array starting at index 'idx'
Error handler is called if defined and index is invalid,
    void cxarray_deln(cxarray* a, size_t idx, size_t n);

Deletes element at index 'idx'
Error handler is called if defined and index is invalid,
    void cxarray_del(cxarray* a, size_t idx);

Deletes element at index 'idx' swaping with last element of the array.
Error handler is called if defined and index is invalid,
    void cxarray_delswap(cxarray* a, size_t idx);

Sorts the array using the specified sort function.
For ascending order the function should return zero if the elements are equal,
greater than 0 if first element greater than the second or less than zero otherwise.
    void cxarray_sort(cxarray* a, int (*f)(const cxtype*, const cxtype*));

Finds element in the array returning its index or -1 if not found.
    ssize_t cxarray_find(cxarray* a, cxtype v);

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
#else
    #error "invalid cx array capacity bits"
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

// Default element comparison function
#ifndef cx_array_cmp_el
    #define cx_array_cmp_el memcmp
#endif

// Default array allocator
#ifndef cx_array_allocator
    #define cx_array_allocator cxDefaultAllocator()
#endif

// Use custom instance allocator
#ifdef cx_array_instance_allocator
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
    #define cx_array_alloc_(s,n)\
        cx_alloc_alloc(cx_array_allocator,n)
    #define cx_array_free_(s,p,n)\
        cx_alloc_free(cx_array_allocator,p,n)
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

#ifdef cx_array_instance_allocator
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
cx_array_api_ void cx_array_name_(_pushn)(cx_array_name* a, cx_array_type* v, size_t n);
cx_array_api_ void cx_array_name_(_push)(cx_array_name* a, cx_array_type v);
cx_array_api_ void cx_array_name_(_pusha)(cx_array_name* a, const cx_array_name* src);
cx_array_api_ cx_array_type cx_array_name_(_pop)(cx_array_name* a);
cx_array_api_ cx_array_type* cx_array_name_(_at)(cx_array_name* a, size_t idx);
cx_array_api_ cx_array_type cx_array_name_(_last)(const cx_array_name* a);
cx_array_api_ void cx_array_name_(_reserve)(cx_array_name* a, size_t n);
cx_array_api_ void cx_array_name_(_insn)(cx_array_name* a, cx_array_type* src, size_t n, size_t idx);
cx_array_api_ void cx_array_name_(_ins)(cx_array_name* a, cx_array_type v, size_t idx);
cx_array_api_ void cx_array_name_(_insa)(cx_array_name* a, const cx_array_name* src, size_t idx);
cx_array_api_ void cx_array_name_(_deln)(cx_array_name* a, size_t idx, size_t n);
cx_array_api_ void cx_array_name_(_del)(cx_array_name* a, size_t idx);
cx_array_api_ void cx_array_name_(_delswap)(cx_array_name* a, size_t i);
cx_array_api_ void cx_array_name_(_sort)(cx_array_name* a, int (*f)(const cx_array_type*, const cx_array_type*));
cx_array_api_ ssize_t cx_array_name_(_find)(cx_array_name* a, cx_array_type v);

//
// Implementations
//
#ifdef cx_array_implement

    // Internal array reallocation function
static void cx_array_name_(_grow_)(cx_array_name* a, size_t addLen, size_t minCap) {

    // Compute the minimum capacity needed
    size_t minLen = a->len_ + addLen;
    if (minLen > minCap) {
        minCap = minLen;
    }
    if (minCap <= a->cap_) {
        return;
    }

    // Increase needed capacity to guarantee O(1) amortized
    if (minCap < 2 * a->cap_) {
        minCap = 2 * a->cap_;
    }
    else if (minCap < 4) {
        minCap = 4;
    }

#ifdef cx_array_error_handler
    if (minCap > cx_array_max_cap_) {
        cx_array_error_handler("capacity exceeded", __func__);
        return;
    }
#endif

    // Allocates new capacity
    const size_t elemSize = sizeof(*(a->data));
    const size_t allocSize = elemSize * minCap;
    void* new = cx_array_alloc_(a, allocSize);
    if (new == NULL) {
        return;
    }

    // Copy current data to new area and free previous
    memcpy(new, a->data, a->len_ * elemSize);
    cx_array_free_(a, a->data, a->len_ * elemSize);
    a->data = new;
    a->cap_ = minCap;
}


#ifdef cx_array_instance_allocator

    cx_array_api_ cx_array_name cx_array_name_(_init)(const CxAllocator* alloc) {
        return (cx_array_name) {
            .alloc = alloc,
        };
    }
#else

    cx_array_api_ cx_array_name cx_array_name_(_init)(void) {
        return (cx_array_name){0};
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
    cx_array_name_(_grow_)(a, 0, cap);
}

cx_array_api_ void cx_array_name_(_setlen)(cx_array_name* a, size_t len) {
    if (a->cap_ < len) {
        cx_array_name_(_grow_)(a, len, 0);
    }
    a->len_ = len;
}

cx_array_api_ void cx_array_name_(_pushn)(cx_array_name* a, cx_array_type* v, size_t n) {
    if (a->len_ + n > a->cap_) {
        cx_array_name_(_grow_)(a, n, 0);
    }
    memcpy(a->data + a->len_, v, n * sizeof(*(a->data)));
    a->len_ += n;
}

cx_array_api_ void cx_array_name_(_push)(cx_array_name* a, cx_array_type v) {
    if (a->len_ >= a->cap_) {
        cx_array_name_(_grow_)(a, 1, 0);
    }
    a->data[a->len_++] = v;
}

cx_array_api_ void cx_array_name_(_pusha)(cx_array_name* a, const cx_array_name* src) {
    cx_array_name_(_pushn)(a, src->data, src->len_);
}
 
cx_array_api_ cx_array_type cx_array_name_(_pop)(cx_array_name* a) {
#ifdef cx_array_error_handler
    if (a->len_ == 0) {
        cx_array_type el = {0};
        cx_array_error_handler("array empty",__func__);
        return el;
    }
#endif
    a->len_--;
    return a->data[a->len_];
}

cx_array_api_ cx_array_type* cx_array_name_(_at)(cx_array_name* a, size_t idx) {
#ifdef cx_array_error_handler
    if (idx > a->len_) {
        cx_array_error_handler("invalid index",__func__);
        return NULL;
    }
#endif
    return &a->data[idx];
}

cx_array_api_ cx_array_type cx_array_name_(_last)(const cx_array_name* a) {
#ifdef cx_array_error_handler
    if (!a->len_) {
        cx_array_type el = {0};
        cx_array_error_handler("array empty",__func__);
        return el;
    }
#endif
    return a->data[a->len_-1];
}

cx_array_api_ void cx_array_name_(_reserve)(cx_array_name* a, size_t n) {
    if (a->len_ + n > a->cap_ ) {
        cx_array_name_(_grow_)(a, n, 0);
    }
}

cx_array_api_ void cx_array_name_(_insn)(cx_array_name* a, cx_array_type* src, size_t n, size_t idx) {
#ifdef cx_array_error_handler
    if (idx > a->len_) {
        cx_array_error_handler("invalid index",__func__);
        return;
    }
#endif
    if (a->len_ + n > a->cap_) {
        cx_array_name_(_grow_)(a, n, 0);
    }
    a->len_ += n;
    memmove(a->data + idx + n, a->data + idx, sizeof(*(a->data)) * (a->len_-n-idx));
    memcpy(a->data + idx, src, n * sizeof(*(a->data)));
}

cx_array_api_ void cx_array_name_(_ins)(cx_array_name* a, cx_array_type v, size_t idx) {
    cx_array_name_(_insn)(a, &v, 1, idx);
}

cx_array_api_ void cx_array_name_(_insa)(cx_array_name* a, const cx_array_name* src, size_t idx) {
    cx_array_name_(_insn)(a, src->data, src->len_, idx);
}

cx_array_api_ void cx_array_name_(_deln)(cx_array_name* a, size_t idx, size_t n) {
#ifdef cx_array_error_handler
    if (idx > a->len_) {
        cx_array_error_handler("invalid index",__func__);
        return;
    }
#endif
    n = n > a->len_ - idx ? a->len_ - idx : n;
    memmove(a->data + idx, a->data + idx + n, sizeof(*(a->data)) * (a->len_- n - idx));
    a->len_ -= n;
}

cx_array_api_ void cx_array_name_(_del)(cx_array_name* a, size_t idx) {
    cx_array_name_(_deln)(a, idx, 1);
}

cx_array_api_ void cx_array_name_(_delswap)(cx_array_name* a, size_t idx) {
#ifdef cx_array_error_handler
    if (idx >= a->len_) {
        cx_array_error_handler("invalid index",__func__);
        return;
    }
#endif
    a->data[idx] = cx_array_name_(_last)(a);
    a->len_--;
}

cx_array_api_ void cx_array_name_(_sort)(cx_array_name* a, int (*f)(const cx_array_type*, const cx_array_type*)) {
    qsort(a->data,a->len_,sizeof(*(a->data)),(int (*)(const void*,const void*))f);
}

cx_array_api_ ssize_t cx_array_name_(_find)(cx_array_name* a, cx_array_type v) {
    for (ssize_t i = 0; i < a->len_; i++) {
        if (cx_array_cmp_el(&a->data[i], &v, sizeof(v)) == 0) {
            return i;
        }
    }
    return -1;
}

#endif

// Undefine config  macros
#undef cx_array_name
#undef cx_array_type
#undef cx_array_cap
#undef cx_array_error_handler
#undef cx_array_allocator
#undef cx_array_instance_allocator
#undef cx_array_static
#undef cx_array_inline
#undef cx_array_implement

// Undefine internal macros
#undef cx_array_concat2_
#undef cx_array_concat1_
#undef cx_array_name_
#undef cx_array_cap8_
#undef cx_array_cap16_
#undef cx_array_cap32_
#undef cx_array_cap64_
#undef cx_array_cap_type_
#undef cx_array_max_cap_
#undef cx_array_api_
#undef cx_array_alloc_field_
#undef cx_array_alloc_global_
#undef cx_array_alloc_
#undef cx_array_freec_


