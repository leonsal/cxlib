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
    #define cx_chan_alloc_(s,n)\
        cx_alloc_alloc(s->alloc, n)
    #define cx_chan_free_(s,p,n)\
        cx_alloc_free(s->alloc, p, n)
// Use global type allocator
#else
    #define cx_chan_alloc_field_
    #define cx_chan_alloc_global_\
        static const CxAllocator* cx_chan_name_(_allocator) = NULL;
    #define cx_chan_alloc_(s,n)\
        cx_alloc_alloc(cx_chan_name_(_allocator),n)
    #define cx_chan_free_(s,p,n)\
        cx_alloc_free(cx_chan_name_(_allocator),p,n)
#endif

//
// Declarations
//
typedef struct cx_chan_name {
    cx_chan_alloc_field_
    mtx_t              mut_;
    mtx_t              rmut_;
    mtx_t              wmut_;
    cnd_t              rcnd_;
    cnd_t              wcnd_;
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
cx_chan_api_ void cx_chan_name_(_free)(cx_chan_name* c);
cx_chan_api_ size_t cx_chan_name_(_len)(cx_chan_name* c);
cx_chan_api_ size_t cx_chan_name_(_cap)(cx_chan_name* c);
cx_chan_api_ void cx_chan_name_(_close)(cx_chan_name* c);
cx_chan_api_ bool cx_chan_name_(_isclosed)(cx_chan_name* c);
cx_chan_api_ size_t cx_chan_name_(_len)(cx_chan_name* c);
cx_chan_api_ bool cx_chan_name_(_send)(cx_chan_name* c, cx_chan_type v);
cx_chan_api_ cx_chan_type cx_chan_name_(_recv)(cx_chan_name* c);

//
// Implementation
//
#define cx_chan_implement
#ifdef cx_chan_implement
    cx_chan_alloc_global_;

    // Internal initializer
    cx_chan_api_ void cx_chan_name_(_init_)(cx_chan_name* ch, size_t size) {

        if (size > 0) {
            ch->queue_ = cx_chan_alloc_(ch, (size + 1) * sizeof(*ch->queue_));
            ch->cap_ = size + 1;
        }
        assert(mtx_init(&ch->mut_, mtx_plain) == thrd_success);
        assert(mtx_init(&ch->rmut_, mtx_plain) == thrd_success);
        assert(mtx_init(&ch->wmut_, mtx_plain) == thrd_success);
        assert(cnd_init(&ch->rcnd_) == thrd_success);
        assert(cnd_init(&ch->wcnd_) == thrd_success);
    }

    // Internal queue length
    cx_chan_api_ size_t cx_chan_name_(_len_)(cx_chan_name* c) {
        if (c->in_ >= c->out_) {
            return c->in_ - c->out_;
        }
        return c->in_ + (c->cap_ - c->out_);
    }

#ifdef cx_chan_allocator

    cx_chan_api_ cx_chan_name cx_chan_name_(_init)(const CxAllocator* alloc, size_t size) {
        cx_chan_name ch = {.alloc = alloc};
        cx_chan_name_(_init_)(&ch, size);
        return ch;
    }

#else

    cx_chan_api_ cx_chan_name cx_chan_name_(_init)(size_t size) {
        if (cx_chan_name_(_allocator) == NULL) {
            cx_chan_name_(_allocator) = cxDefaultAllocator();
        }
        cx_chan_name ch = {0};
        cx_chan_name_(_init_)(&ch, size);
        return ch;
    }

#endif

cx_chan_api_ void cx_chan_name_(_free)(cx_chan_name* c) {

    if (c->cap_) {


    }
}

cx_chan_api_ size_t cx_chan_name_(_len)(cx_chan_name* c) {

    assert(mtx_lock(&c->mut_) == thrd_success);
    size_t len = cx_chan_name_(_len_)(c);
    assert(mtx_unlock(&c->mut_) == thrd_success);
    return len;
}

cx_chan_api_ size_t cx_chan_name_(_cap)(cx_chan_name* c) {

    if (c->cap_ == 0) {
        return 0;
    }
    return c->cap_ - 1;
}

cx_chan_api_ void cx_chan_name_(_close)(cx_chan_name* c) {

    assert(mtx_lock(&c->mut_) == thrd_success);
    c->closed_ = true;
    cnd_broadcast(&c->rcnd_);
    cnd_broadcast(&c->wcnd_);
    assert(mtx_unlock(&c->mut_) == thrd_success);
}

cx_chan_api_ bool cx_chan_name_(_isclosed)(cx_chan_name* c) {

    assert(mtx_lock(&c->mut_) == thrd_success);
    bool closed = c->closed_;
    assert(mtx_unlock(&c->mut_) == thrd_success);
    return closed;
}

cx_chan_api_ bool cx_chan_name_(_send)(cx_chan_name* c, cx_chan_type v) {

    // Unbuffered channel
    if (c->cap_ == 0) {
        assert(mtx_lock(&c->wmut_) == thrd_success);
        assert(mtx_lock(&c->mut_) == thrd_success);

        // If channel is closed, returns false
        if (c->closed_) {
            assert(mtx_unlock(&c->mut_) == thrd_success);
            assert(mtx_unlock(&c->wmut_) == thrd_success);
            return false;
        }

        // Store data and signal to readers
        c->data_ = v;
        c->in_ = 1;
        cnd_signal(&c->rcnd_);

        // Wait for reader to read data or channel is closed
        while (c->in_ > 0 && !c->closed_) {
            assert(cnd_wait(&c->wcnd_, &c->mut_) == thrd_success);
        }
        bool res = c->in_ ==  0 ? true : false;
        assert(mtx_unlock(&c->mut_) == thrd_success);
        assert(mtx_unlock(&c->wmut_) == thrd_success);
        return res;
    }

    // Buffered channel
    // If channel is closed, returns false
    assert(mtx_lock(&c->mut_) == thrd_success); // block writers
    if (c->closed_) {
        assert(mtx_unlock(&c->mut_) == thrd_success);
        return false;
    }

    // Waits for available space
    while (1) {
        size_t len = cx_chan_name_(_len_)(c);
        if (len) {
            break;
        }
        // Unblock readers
        assert(cnd_wait(&c->rcnd_, &c->mut_) == thrd_success);
    }

    // Push data at the back of the queue
    c->queue_[c->in_] = v;
    c->in_++;
    c->in_ %= c->cap_;
    cnd_broadcast(&c->rcnd_); // broadcast to readers
    assert(mtx_unlock(&c->mut_) == thrd_success);
    return true;
}

cx_chan_api_ cx_chan_type cx_chan_name_(_recv)(cx_chan_name* c) {

    // Unbuffered channel
    if (c->cap_ == 0) {
        assert(mtx_lock(&c->rmut_) == thrd_success);
        assert(mtx_lock(&c->mut_) == thrd_success);

        // If no data available and channel is closed, returns channel type zero value
        if (c->in_ == 0 && c->closed_) {
            assert(mtx_unlock(&c->mut_) == thrd_success);
            assert(mtx_unlock(&c->rmut_) == thrd_success);
            return (cx_chan_type){0};
        }

        // Waits for data
        while (c->in_ == 0) {
            assert(cnd_wait(&c->rcnd_, &c->mut_) == thrd_success);
        }

        // Reads data and notify writer
        cx_chan_type data = c->data_;
        c->in_ = 0;
        cnd_signal(&c->wcnd_);
        assert(mtx_unlock(&c->mut_) == thrd_success);
        assert(mtx_unlock(&c->rmut_) == thrd_success);
        return data;
    }

    // Buffered channel
    assert(mtx_lock(&c->mut_) == thrd_success);
    // If queue is empty and channel closed, returns channel type zero value
    if ((c->in_ == c->out_) && c->closed_) {
        assert(mtx_unlock(&c->mut_) == thrd_success);
        return (cx_chan_type){0};
    }

    // Waits for available data
    while (1) {
        if (c->in_ != c->out_) {
            break;
        }
        assert(cnd_wait(&c->wcnd_, &c->mut_) == thrd_success);
    }
    // Removes data at the front of the queue
    cx_chan_type data = c->queue_[c->out_];
    c->out_++;
    c->out_ %= c->cap_;
    cnd_broadcast(&c->wcnd_);
    assert(mtx_unlock(&c->mut_) == thrd_success);
    return data;
}


#endif // cx_chan_implement

