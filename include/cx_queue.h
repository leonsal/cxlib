#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "cx_alloc.h"

// queue type name must be defined
#ifndef cx_queue_name
    #error "cx_queue_name not defined"
#endif
// queue element type name must be defined
#ifndef cx_queue_type
    #error "cx_queue_type not defined"
#endif

// Auxiliary internal macros
#define cx_queue_concat2_(a, b) a ## b
#define cx_queue_concat1_(a, b) cx_queue_concat2_(a, b)
#define cx_queue_name_(name) cx_queue_concat1_(cx_queue_name, name)

// queue maximum capacity in number of bits
#define cx_queue_cap8_     8
#define cx_queue_cap16_    16
#define cx_queue_cap32_    32

// Default capacity
#ifndef cx_queue_cap
    #define cx_queue_cap  cx_queue_cap32_
#endif
#if cx_queue_cap == cx_queue_cap8_
    #define cx_queue_cap_type_ uint8_t
    #define cx_queue_max_cap_  (UINT8_MAX)
#elif cx_queue_cap == cx_queue_cap16_
    #define cx_queue_cap_type_ uint16_t
    #define cx_queue_max_cap_  (UINT16_MAX)
#elif cx_queue_cap == cx_queue_cap32_
    #define cx_queue_cap_type_ uint32_t
    #define cx_queue_max_cap_  (UINT32_MAX)
#else
    #error "invalid cx queue capacity bits"
#endif

// API attributes
#if defined(cx_queue_static) && defined(cx_queue_inline)
    #define cx_queue_api_ static inline
#elif defined(cx_queue_static)
    #define cx_queue_api_ static
#elif defined(cx_queue_inline)
    #define cx_queue_api_ inline
#else
    #define cx_queue_api_
#endif

// Default allocator
#ifndef cx_queue_allocator
    #define cx_queue_allocator cxDefaultAllocator()
#endif

// Use custom instance allocator
#ifdef cx_queue_instance_allocator
    #define cx_queue_alloc_field_\
        const CxAllocator* alloc;
    #define cx_queue_alloc_(s,n)\
        cx_alloc_malloc(s->alloc, n)
    #define cx_queue_free_(s,p,n)\
        cx_alloc_free(s->alloc, p, n)
// Use global type allocator
#else
    #define cx_queue_alloc_field_
    #define cx_queue_alloc_(s,n)\
        cx_alloc_malloc(cx_queue_allocator,n)
    #define cx_queue_free_(s,p,n)\
        cx_alloc_free(cx_queue_allocator,p,n)
#endif

//
// Declarations
//
typedef struct cx_queue_name {
    cx_queue_alloc_field_
    cx_queue_cap_type_  cap_;   // current capacity
    cx_queue_cap_type_  in_;    // input index
    cx_queue_cap_type_  out_;   // output index
    cx_queue_type*      data_;
} cx_queue_name;

#ifdef cx_queue_instance_allocator
    cx_queue_api_ cx_queue_name cx_queue_name_(_init)(const CxAllocator*);
#else
    cx_queue_api_ cx_queue_name cx_queue_name_(_init)(void);
#endif
cx_queue_api_ void cx_queue_name_(_free)(cx_queue_name* q);
cx_queue_api_ void cx_queue_name_(_clear)(cx_queue_name* q);
cx_queue_api_ cx_queue_name cx_queue_name_(_clone)(cx_queue_name* q);
cx_queue_api_ size_t cx_queue_name_(_cap)(cx_queue_name* q);
cx_queue_api_ size_t cx_queue_name_(_len)(cx_queue_name* q);
cx_queue_api_ bool cx_queue_name_(_empty)(cx_queue_name* q);
cx_queue_api_ void cx_queue_name_(_setcap)(cx_queue_name* q, size_t cap);
cx_queue_api_ void cx_queue_name_(_pushbn)(cx_queue_name* q, const cx_queue_type* v, size_t n);
cx_queue_api_ void cx_queue_name_(_pushb)(cx_queue_name* q, cx_queue_type v);
cx_queue_api_ size_t cx_queue_name_(_popfn)(cx_queue_name* q, cx_queue_type* v, size_t n);
cx_queue_api_ cx_queue_type cx_queue_name_(_popf)(cx_queue_name* q);
cx_queue_api_ cx_queue_type* cx_queue_name_(_at)(cx_queue_name* q, size_t idx);
cx_queue_api_ cx_queue_type* cx_queue_name_(_back)(const cx_queue_name* q);
cx_queue_api_ cx_queue_type* cx_queue_name_(_front)(const cx_queue_name* q);
cx_queue_api_ void cx_queue_name_(_reserve)(cx_queue_name* q, size_t n);

//
// Implementations
//
#ifdef cx_queue_implement

    // Internal queue reallocation function
static void cx_queue_name_(_grow_)(cx_queue_name* q, size_t addLen, size_t minCap) {

    // Compute the minimum capacity needed
    size_t len = cx_queue_name_(_len)(q);
    size_t minLen = len + addLen;
    if (minLen > minCap) {
        minCap = minLen;
    }
    if (minCap + 1 <= q->cap_) {
        return;
    }

    // Increase needed capacity to guarantee O(1) amortized
    if (minCap < 2 * q->cap_) {
        minCap = 2 * q->cap_;
    }
    else if (minCap < 4) {
        minCap = 4;
    }

#ifdef cx_queue_error_handler
    if (minCap > cx_queue_max_cap_) {
        cx_queue_error_handler("capacity exceeded", __func__);
        return;
    }
#endif

    // Allocates new capacity
    const size_t elemSize = sizeof(*(q->data_));
    const size_t allocSize = elemSize * minCap;
    void* new = cx_queue_alloc_(q, allocSize);
    if (new == NULL) {
        return;
    }

    // Copy current data to new area and free previous
    if (q->in_ >= q->out_) {
        memcpy(new + q->out_, q->data_ + q->out_, len * elemSize);
    } else {
        memcpy(new, q->data_, q->in_ * elemSize);
        memcpy(new + minCap - q->out_, q->data_ + q->out_, (q->cap_ - q->out_) * elemSize);
        q->out_ += minCap - q->cap_;
    }
    cx_queue_free_(q, q->data_, len * elemSize);
    q->data_ = new;
    q->cap_ = minCap;
}


#ifdef cx_queue_instance_allocator

    cx_queue_api_ cx_queue_name cx_queue_name_(_init)(const CxAllocator* alloc) {
        return (cx_queue_name) {
            .alloc = alloc,
        };
    }
#else

    cx_queue_api_ cx_queue_name cx_queue_name_(_init)(void) {
        return (cx_queue_name){0};
    }
#endif

cx_queue_api_ void cx_queue_name_(_free)(cx_queue_name* q) {
    cx_queue_free_(q, q->data_, q->cap_ * sizeof(*(q->data_)));
    q->cap_ = 0;
    q->in_ = 0;
    q->out_ = 0;
    q->data_ = NULL;
}

cx_queue_api_ void cx_queue_name_(_clear)(cx_queue_name* q) {
    q->in_ = 0;
    q->out_ = 0;
}

cx_queue_api_ cx_queue_name cx_queue_name_(_clone)(cx_queue_name* q) {

    const size_t len = cx_queue_name_(_len)(q);
    const size_t alloc_size = len * sizeof(*(q->data_));
    cx_queue_name cloned = *q;
    cloned.data_  = cx_queue_alloc_(q, alloc_size),
    memcpy(cloned.data_, q->data_, alloc_size);
    return cloned;
}

cx_queue_api_ size_t cx_queue_name_(_cap)(cx_queue_name* q) {
    return q->cap_;
}

cx_queue_api_ size_t cx_queue_name_(_len)(cx_queue_name* q) {

    if (q->in_ >= q->out_) {
        return q->in_ - q->out_;
    }
    return q->in_ + (q->cap_ - q->out_);
}

cx_queue_api_ bool cx_queue_name_(_empty)(cx_queue_name* q) {
    return q->in_ == q->out_;
} 

cx_queue_api_ void cx_queue_name_(_setcap)(cx_queue_name* q, size_t cap) {
    cx_queue_name_(_grow_)(q, 0, cap);
}

cx_queue_api_ void cx_queue_name_(_pushbn)(cx_queue_name* q, const cx_queue_type* v, size_t n) {

    const size_t len = cx_queue_name_(_len)(q);
    if (len + n + 1 > q->cap_) {
        cx_queue_name_(_grow_)(q, n, 0);
    }
    if (q->in_ >= q->out_) {
        const size_t free = q->cap_ - q->in_;
        const size_t cpy = n > free ? free : n;
        memcpy(q->data_ + q->in_, v, cpy * sizeof(*(q->data_)));
        if (n > cpy) {
            memcpy(q->data_, v + cpy, (n - cpy) * sizeof(*(q->data_)));
        }
    } else {
        memcpy(q->data_ + q->in_, v, n * sizeof(*(q->data_)));
    }
    q->in_ += n;
    q->in_ %= q->cap_;
}

cx_queue_api_ void cx_queue_name_(_pushb)(cx_queue_name* q, cx_queue_type v) {

    cx_queue_name_(_pushbn)(q, &v, 1);
}

cx_queue_api_ size_t cx_queue_name_(_popfn)(cx_queue_name* q, cx_queue_type* v, size_t n) {

    const size_t len = cx_queue_name_(_len)(q);
    n = n <= len ? n : len;
    if (q->in_ >= q->out_) {
        memcpy(v, q->data_ + q->out_,  n);
    } else {
        const size_t cpy = q->cap_ - q->out_;
        memcpy(v, q->data_ + q->out_,  cpy);
        if (n > cpy) {
            memcpy(v + cpy, q->data_,  n - cpy);
        }
    }
    q->out_ += n;
    q->out_ %= q->cap_;
    return n;
}
 
cx_queue_api_ cx_queue_type cx_queue_name_(_popf)(cx_queue_name* q) {

#ifdef cx_queue_error_handler
    if (q->in_ == q->out_ {
        cx_queue_type el = {0};
        cx_queue_error_handler("queue empty",__func__);
        return el;
    }
#endif
    cx_queue_type el = q->data_[q->out_++];
    q->out_ %= q->cap_;
    return el;
}

cx_queue_api_ cx_queue_type* cx_queue_name_(_at)(cx_queue_name* q, size_t idx) {

    if (q->in_ == q->out_) {
        return NULL;
    }
    return q->data_ + q->out_ + idx;
}

cx_queue_api_ cx_queue_type* cx_queue_name_(_back)(const cx_queue_name* q) {

    if (q->in_ == q->out_) {
        return NULL;
    }
    const ssize_t in = (ssize_t)q->in_ - 1 < 0 ? q->cap_ - 1 : q->in_ - 1;
    return q->data_ + in;
}

cx_queue_api_ cx_queue_type* cx_queue_name_(_front)(const cx_queue_name* q) {

    if (q->in_ == q->out_) {
        return NULL;
    }
    return q->data_ + q->out_;
}

cx_queue_api_ void cx_queue_name_(_reserve)(cx_queue_name* a, size_t n) {
    // if (a->len_ + n > a->cap_ ) {
    //     cx_queue_name_(_grow_)(a, n, 0);
    // }
}

#endif

// Undefine config  macros
#undef cx_queue_name
#undef cx_queue_type
#undef cx_queue_cap
#undef cx_queue_error_handler
#undef cx_queue_allocator
#undef cx_queue_instance_allocator
#undef cx_queue_static
#undef cx_queue_inline
#undef cx_queue_implement

// Undefine internal macros
#undef cx_queue_concat2_
#undef cx_queue_concat1_
#undef cx_queue_name_
#undef cx_queue_cap8_
#undef cx_queue_cap16_
#undef cx_queue_cap32_
#undef cx_queue_cap64_
#undef cx_queue_cap_type_
#undef cx_queue_max_cap_
#undef cx_queue_api_
#undef cx_queue_alloc_field_
#undef cx_queue_alloc_global_
#undef cx_queue_alloc_
#undef cx_queue_freec_



