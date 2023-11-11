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

// Use custom instance allocator
#ifdef cx_queue_allocator
    #define cx_queue_alloc_field_\
        const CxAllocator* alloc;
    #define cx_queue_alloc_global_
    #define cx_queue_alloc_(s,n)\
        cx_alloc_alloc(s->alloc, n)
    #define cx_queue_free_(s,p,n)\
        cx_alloc_free(s->alloc, p, n)
// Use global type allocator
#else
    #define cx_queue_alloc_field_
    #define cx_queue_alloc_global_\
        static const CxAllocator* cx_queue_name_(_allocator) = NULL;
    #define cx_queue_alloc_(s,n)\
        cx_alloc_alloc(cx_queue_name_(_allocator),n)
    #define cx_queue_free_(s,p,n)\
        cx_alloc_free(cx_queue_name_(_allocator),p,n)
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

#ifdef cx_queue_allocator
    cx_queue_api_ cx_queue_name cx_queue_name_(_init)(const CxAllocator*);
#else
    cx_queue_api_ cx_queue_name cx_queue_name_(_init)(void);
#endif
cx_queue_api_ void cx_queue_name_(_free)(cx_queue_name* a);
cx_queue_api_ void cx_queue_name_(_clear)(cx_queue_name* a);
cx_queue_api_ cx_queue_name cx_queue_name_(_clone)(cx_queue_name* a);
cx_queue_api_ size_t cx_queue_name_(_cap)(cx_queue_name* a);
cx_queue_api_ size_t cx_queue_name_(_len)(cx_queue_name* a);
cx_queue_api_ bool cx_queue_name_(_empty)(cx_queue_name* a);
cx_queue_api_ void cx_queue_name_(_setcap)(cx_queue_name* a, size_t cap);

cx_queue_api_ void cx_queue_name_(_pushbn)(cx_queue_name* a, const cx_queue_type* v, size_t n);
cx_queue_api_ void cx_queue_name_(_pushb)(cx_queue_name* a, cx_queue_type v);
cx_queue_api_ void cx_queue_name_(_pushba)(cx_queue_name* a, const cx_queue_name* src);
cx_queue_api_ cx_queue_type cx_queue_name_(_popb)(cx_queue_name* a);

cx_queue_api_ void cx_queue_name_(_pushfn)(cx_queue_name* a, const cx_queue_type* v, size_t n);
cx_queue_api_ void cx_queue_name_(_pushf)(cx_queue_name* a, cx_queue_type v);
cx_queue_api_ void cx_queue_name_(_pushfa)(cx_queue_name* a, const cx_queue_name* src);
cx_queue_api_ cx_queue_type cx_queue_name_(_popf)(cx_queue_name* a);

cx_queue_api_ cx_queue_type* cx_queue_name_(_at)(cx_queue_name* a, size_t idx);
cx_queue_api_ cx_queue_type cx_queue_name_(_first)(const cx_queue_name* a);
cx_queue_api_ cx_queue_type cx_queue_name_(_last)(const cx_queue_name* a);
cx_queue_api_ void cx_queue_name_(_reserve)(cx_queue_name* a, size_t n);

//
// Implementations
//
#ifdef cx_queue_implement
    cx_queue_alloc_global_;

    // Internal queue reallocation function
static void cx_queue_name_(_grow_)(cx_queue_name* a, size_t addLen, size_t minCap) {

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

#ifdef cx_queue_error_handler
    if (minCap > cx_queue_max_cap_) {
        cx_queue_error_handler("capacity exceeded", __func__);
        return;
    }
#endif

    // Allocates new capacity
    const size_t elemSize = sizeof(*(a->data));
    const size_t allocSize = elemSize * minCap;
    void* new = cx_queue_alloc_(a, allocSize);
    if (new == NULL) {
        return;
    }

    // Copy current data to new area and free previous
    memcpy(new, a->data, a->len_ * elemSize);
    cx_queue_free_(a, a->data, a->len_ * elemSize);
    a->data = new;
    a->cap_ = minCap;
}


#ifdef cx_queue_allocator

    cx_queue_api_ cx_queue_name cx_queue_name_(_init)(const CxAllocator* alloc) {
        return (cx_queue_name) {
            .alloc = alloc,
        };
    }
#else

    cx_queue_api_ cx_queue_name cx_queue_name_(_init)(void) {
        if (cx_queue_name_(_allocator) == NULL) {
            cx_queue_name_(_allocator) = cxDefaultAllocator();
        }
        return (cx_queue_name) {
        };
    }
#endif

cx_queue_api_ void cx_queue_name_(_free)(cx_queue_name* a) {
    cx_queue_free_(a, a->data, a->cap_ * sizeof(*(a->data)));
    a->cap_ = 0;
    a->in_ = 0;
    a->out_ = 0;
    a->data = NULL;
}

cx_queue_api_ void cx_queue_name_(_clear)(cx_queue_name* a) {
    a->in_ = 0;
    a->out_ = 0;
}

cx_queue_api_ cx_queue_name cx_queue_name_(_clone)(cx_queue_name* a) {
    const size_t alloc_size = a->len_ * sizeof(*(a->data));
    cx_queue_name cloned = *a;
    cloned.data  = cx_queue_alloc_(a, alloc_size),
    memcpy(cloned.data, a->data, alloc_size);
    return cloned;
}

cx_queue_api_ size_t cx_queue_name_(_cap)(cx_queue_name* a) {
    return a->cap_;
}

cx_queue_api_ size_t cx_queue_name_(_len)(cx_queue_name* a) {
    if (a->in_ >= a->out_) {
        return a->in_ - a->_out;
    }
    return a->in_ + (a->cap_ - a->out_);
}

cx_queue_api_ bool cx_queue_name_(_empty)(cx_queue_name* a) {
    return a->in_ == a->out_;
} 

cx_queue_api_ void cx_queue_name_(_setcap)(cx_queue_name* a, size_t cap) {
    cx_queue_name_(_grow_)(a, 0, cap);
}

cx_queue_api_ void cx_queue_name_(_setlen)(cx_queue_name* a, size_t len) {
    if (a->cap_ < len) {
        cx_queue_name_(_grow_)(a, len, 0);
    }
    a->len_ = len;
}

cx_queue_api_ void cx_queue_name_(_pushn)(cx_queue_name* a, const cx_queue_type* v, size_t n) {
    if (a->len_ + n > a->cap_) {
        cx_queue_name_(_grow_)(a, n, 0);
    }
    memcpy(a->data + a->len_, v, n * sizeof(*(a->data)));
    a->len_ += n;
}

cx_queue_api_ void cx_queue_name_(_push)(cx_queue_name* a, cx_queue_type v) {
    if (a->len_ >= a->cap_) {
        cx_queue_name_(_grow_)(a, 1, 0);
    }
    a->data[a->len_++] = v;
}

cx_queue_api_ void cx_queue_name_(_pusha)(cx_queue_name* a, const cx_queue_name* src) {
    cx_queue_name_(_pushn)(a, src->data, src->len_);
}
 
cx_queue_api_ cx_queue_type cx_queue_name_(_pop)(cx_queue_name* a) {
#ifdef cx_queue_error_handler
    if (a->len_ == 0) {
        cx_queue_type el = {0};
        cx_queue_error_handler("queue empty",__func__);
        return el;
    }
#endif
    a->len_--;
    return a->data[a->len_];
}

cx_queue_api_ cx_queue_type* cx_queue_name_(_at)(cx_queue_name* a, size_t idx) {
#ifdef cx_queue_error_handler
    if (idx > a->len_) {
        cx_queue_error_handler("invalid index",__func__);
        return NULL;
    }
#endif
    return &a->data[idx];
}

cx_queue_api_ cx_queue_type cx_queue_name_(_last)(const cx_queue_name* a) {
#ifdef cx_queue_error_handler
    if (!a->len_) {
        cx_queue_type el = {0};
        cx_queue_error_handler("queue empty",__func__);
        return el;
    }
#endif
    return a->data[a->len_-1];
}

cx_queue_api_ void cx_queue_name_(_reserve)(cx_queue_name* a, size_t n) {
    if (a->len_ + n > a->cap_ ) {
        cx_queue_name_(_grow_)(a, n, 0);
    }
}

cx_queue_api_ void cx_queue_name_(_insn)(cx_queue_name* a, const cx_queue_type* src, size_t n, size_t idx) {
#ifdef cx_queue_error_handler
    if (idx > a->len_) {
        cx_queue_error_handler("invalid index",__func__);
        return;
    }
#endif
    if (a->len_ + n > a->cap_) {
        cx_queue_name_(_grow_)(a, n, 0);
    }
    a->len_ += n;
    memmove(a->data + idx + n, a->data + idx, sizeof(*(a->data)) * (a->len_-n-idx));
    memcpy(a->data + idx, src, n * sizeof(*(a->data)));
}

cx_queue_api_ void cx_queue_name_(_ins)(cx_queue_name* a, cx_queue_type v, size_t idx) {
    cx_queue_name_(_insn)(a, &v, 1, idx);
}

cx_queue_api_ void cx_queue_name_(_insa)(cx_queue_name* a, const cx_queue_name* src, size_t idx) {
    cx_queue_name_(_insn)(a, src->data, src->len_, idx);
}

cx_queue_api_ void cx_queue_name_(_deln)(cx_queue_name* a, size_t idx, size_t n) {
#ifdef cx_queue_error_handler
    if (idx > a->len_) {
        cx_queue_error_handler("invalid index",__func__);
        return;
    }
#endif
    n = n > a->len_ - idx ? a->len_ - idx : n;
    memmove(a->data + idx, a->data + idx + n, sizeof(*(a->data)) * (a->len_- n - idx));
    a->len_ -= n;
}

cx_queue_api_ void cx_queue_name_(_del)(cx_queue_name* a, size_t idx) {
    cx_queue_name_(_deln)(a, idx, 1);
}

cx_queue_api_ void cx_queue_name_(_delswap)(cx_queue_name* a, size_t idx) {
#ifdef cx_queue_error_handler
    if (idx >= a->len_) {
        cx_queue_error_handler("invalid index",__func__);
        return;
    }
#endif
    a->data[idx] = cx_queue_name_(_last)(a);
    a->len_--;
}

cx_queue_api_ void cx_queue_name_(_sort)(cx_queue_name* a, int (*f)(const cx_queue_type*, const cx_queue_type*)) {
    qsort(a->data,a->len_,sizeof(*(a->data)),(int (*)(const void*,const void*))f);
}

#endif

// Undefine config  macros
#undef cx_queue_name
#undef cx_queue_type
#undef cx_queue_cap
#undef cx_queue_error_handler
#undef cx_queue_allocator
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



