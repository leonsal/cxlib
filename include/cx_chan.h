#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <threads.h>
#include "cx_alloc.h"

// chan type name must be defined
#ifndef cx_chan_name
    #error "cx_chan_name not defined"
#endif
// chan element type name must be defined
#ifndef cx_chan_type
    #error "cx_queue_type not defined"
#endif

// Auxiliary internal macros
#define cx_chan_concat2_(a, b) a ## b
#define cx_chan_concat1_(a, b) cx_chan_concat2_(a, b)
#define cx_chan_name_(name) cx_chan_concat1_(cx_chan_name, name)

// chan maximum capacity in number of bits
#define cx_chan_cap8_     8
#define cx_chan_cap16_    16
#define cx_chan_cap32_    32

// Default capacity
#ifndef cx_chan_cap
    #define cx_chan_cap  cx_chan_cap32_
#endif
#if cx_chan_cap == cx_chan_cap8_
    #define cx_chan_cap_type_ uint8_t
    #define cx_chan_max_cap_  (UINT8_MAX)
#elif cx_chan_cap == cx_chan_cap16_
    #define cx_chan_cap_type_ uint16_t
    #define cx_chan_max_cap_  (UINT16_MAX)
#elif cx_chan_cap == cx_chan_cap32_
    #define cx_chan_cap_type_ uint32_t
    #define cx_chan_max_cap_  (UINT32_MAX)
#else
    #error "invalid cx chan capacity bits"
#endif

// API attributes
#if defined(cx_chan_static) && defined(cx_chan_inline)
    #define cx_chan_api_ static inline
#elif defined(cx_chan_static)
    #define cx_chan_api_ static
#elif defined(cx_chan_inline)
    #define cx_chan_api_ inline
#else
    #define cx_chan_api_
#endif

// Use custom instance allocator
#ifdef cx_chan_allocator
    #define cx_chan_alloc_field_\
        const CxAllocator* alloc;
    #define cx_chan_alloc_global_
// Use global type allocator
#else
    #define cx_chan_alloc_field_
    #define cx_chan_alloc_global_\
        static const CxAllocator* cx_chan_name_(_allocator) = NULL;
#endif

//
// Declarations
//
typedef struct cx_chan_name {
    mtx_t              mut_;
    cnd_t              wcnd_;
    cnd_t              rcnd_;
    bool               closed_;
    cx_chan_type       data_;
    cx_chan_cap_type_  cap_;   // current capacity
    cx_chan_cap_type_  in_;    // input index
    cx_chan_cap_type_  out_;   // output index
    cx_chan_type*      queue_;
} cx_chan_name;

#ifdef cx_chan_allocator
    cx_chan_api_ cx_chan_name cx_chan_name_(_init)(const CxAllocator*, size_t cap);
#else
    cx_chan_api_ cx_chan_name cx_chan_name_(_init)(size_t cap);
#endif
cx_chan_api_ void cx_chan_name_(_free)(cx_chan_name* q);
cx_chan_api_ void cx_chan_name_(_close)(cx_chan_name* q);
cx_chan_api_ bool cx_chan_name_(_isclosed)(cx_chan_name* q);
cx_chan_api_ size_t cx_chan_name_(_len)(cx_chan_name* q);
cx_chan_api_ bool cx_chan_name_(_send)(cx_chan_name* q, cx_chan_type v);
cx_chan_api_ cx_chan_type cx_chan_name_(_recv)(cx_chan_name* q);

//
// Implementation
//
#define cx_chan_implement
#ifdef cx_chan_implement
    cx_chan_alloc_global_;

    cx_chan_api_ cx_chan_name cx_chan_name_(_init_)(const CxAllocator* alloc, size_t size) {
        cx_chan_name ch = {0};
        if (size > 0) {
            ch.queue_ = alloc->alloc(alloc->ctx, size * sizeof(*ch.queue_));
            ch.cap_ = size;
        }
        assert(mtx_init(&ch.mut_, mtx_plain) == thrd_success);
        assert(cnd_init(&ch.rcnd_) == thrd_success);
        assert(cnd_init(&ch.wcnd_) == thrd_success);
        return ch;
    }

#ifdef cx_chan_allocator

    cx_chan_api_ cx_chan_name cx_chan_name_(_init)(const CxAllocator* alloc, size_t size) {
        return cx_chan_name_(_init_)(alloc, size);
    }

#else

    cx_chan_api_ cx_chan_name cx_chan_name_(_init)(size_t size) {
        if (cx_chan_name_(_allocator) == NULL) {
            cx_chan_name_(_allocator) = cxDefaultAllocator();
        }
        return cx_chan_name_(_init_)(cx_chan_name_(_allocator), size);
    }

#endif

cx_chan_api_ bool cx_chan_name_(_send)(cx_chan_name* c, cx_chan_type v) {

    // Unbuffered channel
    if (c->cap_ == 0) {
        assert(mtx_lock(&c->mut_) == thrd_success);
        if (c->closed_) {
            assert(mtx_unlock(&c->mut_) == thrd_success);
            return false;
        }

        // Wait for space to store data
        while (c->in_ != 0) {
            assert(cnd_wait(&c->wcnd_, &c->mut_) == thrd_success);
        }
        c->data_ = v;
        c->in_ = 1;
        cnd_broadcast(&c->rcnd_);

        // Wait for reader to read data
        while (c->in_ > 0) {
            assert(cnd_wait(&c->rcnd_, &c->mut_) == thrd_success);
        }
        cnd_broadcast(&c->wcnd_);   // for other writers
        assert(mtx_unlock(&c->mut_) == thrd_success);
        return true;
    }

    // const size_t len = cx_queue_name_(_len)(q);
    // if (len + n + 1 > q->cap_) {
    //     cx_queue_name_(_grow_)(q, n, 0);
    // }
    // if (q->in_ >= q->out_) {
    //     const size_t free = q->cap_ - q->in_;
    //     const size_t cpy = n > free ? free : n;
    //     memcpy(q->data_ + q->in_, v, cpy * sizeof(*(q->data_)));
    //     if (n > cpy) {
    //         memcpy(q->data_, v + cpy, (n - cpy) * sizeof(*(q->data_)));
    //     }
    // } else {
    //     memcpy(q->data_ + q->in_, v, n * sizeof(*(q->data_)));
    // }
    // q->in_ += n;
    // q->in_ %= q->cap_;


}

cx_chan_api_ cx_chan_type cx_chan_name_(_recv)(cx_chan_name* c) {

    assert(mtx_lock(&c->mut_) == thrd_success);
    if (c->cap_ == 0) {
        if (c->closed_) {
            assert(mtx_unlock(&c->mut_) == thrd_success);
            return (cx_chan_type){0};
        }
        // Waits for data
        while (c->in_ == 0) {
            assert(cnd_wait(&c->rcnd_, &c->mut_) == thrd_success);
        }
        cx_chan_type data = c->data_;
        c->in_ = 0;
        cnd_signal(&c->wcnd_);   // for other writer



    }

}



#endif

