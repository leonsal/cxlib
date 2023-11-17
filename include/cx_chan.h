#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
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
    pthread_mutex_t     mut_;
    pthread_mutex_t     rmut_;
    pthread_mutex_t     wmut_;
    pthread_cond_t      rcnd_;
    pthread_cond_t      wcnd_;
    bool                closed_;
    cx_chan_type        data_;
    cx_chan_cap_type_   cap_;   // current capacity
    cx_chan_cap_type_   in_;    // input index
    cx_chan_cap_type_   out_;   // output index
    cx_chan_type*       queue_;
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
        assert(pthread_mutex_init(&ch->mut_, NULL) == 0);
        assert(pthread_mutex_init(&ch->rmut_, NULL) == 0);
        assert(pthread_mutex_init(&ch->wmut_, NULL) == 0);
        assert(pthread_cond_init(&ch->rcnd_, NULL) == 0);
        assert(pthread_cond_init(&ch->wcnd_, NULL) == 0);
    }

    // Internal queue length
    cx_chan_api_ size_t cx_chan_name_(_len_)(cx_chan_name* c) {
        if (c->in_ >= c->out_) {
            return c->in_ - c->out_;
        }
        return c->in_ + (c->cap_ - c->out_);
    }

#ifdef cx_chan_allocator

    // Initialize using instance custom allocator
    cx_chan_api_ cx_chan_name cx_chan_name_(_init)(const CxAllocator* alloc, size_t size) {
        cx_chan_name ch = {.alloc = alloc};
        cx_chan_name_(_init_)(&ch, size);
        return ch;
    }

#else

    // Initialize using global type allocator
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
        cx_chan_free_(c, c->queue_, c->cap_ * sizeof(*c->queue_));
    }
    pthread_cond_destroy(&c->wcnd_);
    pthread_cond_destroy(&c->rcnd_);
    pthread_mutex_destroy(&c->wmut_);
    pthread_mutex_destroy(&c->rmut_);
    pthread_mutex_destroy(&c->mut_);
}

cx_chan_api_ size_t cx_chan_name_(_len)(cx_chan_name* c) {

    assert(pthread_mutex_lock(&c->mut_) == 0);
    size_t len = cx_chan_name_(_len_)(c);
    assert(pthread_mutex_unlock(&c->mut_) == 0);
    return len;
}

cx_chan_api_ size_t cx_chan_name_(_cap)(cx_chan_name* c) {

    if (c->cap_ == 0) {
        return 0;
    }
    return c->cap_ - 1;
}

cx_chan_api_ void cx_chan_name_(_close)(cx_chan_name* c) {

    assert(pthread_mutex_lock(&c->mut_) == 0);
    c->closed_ = true;
    pthread_cond_broadcast(&c->rcnd_);
    pthread_cond_broadcast(&c->wcnd_);
    assert(pthread_mutex_unlock(&c->mut_) == 0);
}

cx_chan_api_ bool cx_chan_name_(_isclosed)(cx_chan_name* c) {

    assert(pthread_mutex_lock(&c->mut_) == 0);
    bool closed = c->closed_;
    assert(pthread_mutex_unlock(&c->mut_) == 0);
    return closed;
}

cx_chan_api_ bool cx_chan_name_(_send)(cx_chan_name* c, cx_chan_type v) {

    // Unbuffered channel
    if (c->cap_ == 0) {
        assert(pthread_mutex_lock(&c->wmut_) == 0);
        assert(pthread_mutex_lock(&c->mut_) == 0);

        // If channel is closed, returns false
        if (c->closed_) {
            assert(pthread_mutex_unlock(&c->mut_) == 0);
            assert(pthread_mutex_unlock(&c->wmut_) == 0);
            return false;
        }

        // Store data and signal to readers
        c->data_ = v;
        c->in_ = 1;
        pthread_cond_signal(&c->rcnd_);

        // Wait for reader to read data or channel is closed
        while (c->in_ > 0 && !c->closed_) {
            assert(pthread_cond_wait(&c->wcnd_, &c->mut_) == 0);
        }
        bool res = c->in_ ==  0 ? true : false;
        assert(pthread_mutex_unlock(&c->mut_) == 0);
        assert(pthread_mutex_unlock(&c->wmut_) == 0);
        return res;
    }

    // Buffered channel
    assert(pthread_mutex_lock(&c->mut_) == 0);

    // Waits for available space or channel closed
    while (1) {
        size_t len = cx_chan_name_(_len_)(c);
        if (len < c->cap_ - 1 || c->closed_) {
            break;
        }
        assert(pthread_cond_wait(&c->wcnd_, &c->mut_) == 0);
    }

    // Push data at the back of the queue
    bool res = false;
    if (!c->closed_) {
        c->queue_[c->in_] = v;
        c->in_++;
        c->in_ %= c->cap_;
        pthread_cond_broadcast(&c->rcnd_); // broadcast to readers
        res = true;                        
    }
    assert(pthread_mutex_unlock(&c->mut_) == 0);
    return res;
}

cx_chan_api_ cx_chan_type cx_chan_name_(_recv)(cx_chan_name* c) {

    // Unbuffered channel
    if (c->cap_ == 0) {
        assert(pthread_mutex_lock(&c->rmut_) == 0);
        assert(pthread_mutex_lock(&c->mut_) == 0);

        // If no data available and channel is closed, returns channel type zero value
        if (c->in_ == 0 && c->closed_) {
            assert(pthread_mutex_unlock(&c->mut_) == 0);
            assert(pthread_mutex_unlock(&c->rmut_) == 0);
            return (cx_chan_type){0};
        }

        // Waits for data or channel closed
        while (c->in_ == 0 && !c->closed_) {
            assert(pthread_cond_wait(&c->rcnd_, &c->mut_) == 0);
        }
        cx_chan_type data;
        if (!c->closed_) {
            // Reads data and notify writer
            data = c->data_;
            c->in_ = 0;
            pthread_cond_signal(&c->wcnd_);
        } else {
            // Channel closed: returns 0 value
            data = (cx_chan_type){0};
        }
        assert(pthread_mutex_unlock(&c->mut_) == 0);
        assert(pthread_mutex_unlock(&c->rmut_) == 0);
        return data;
    }

    // Buffered channel
    // Waits for available data or channel closed
    assert(pthread_mutex_lock(&c->mut_) == 0);
    while ((c->in_ == c->out_) && !c->closed_) {
        assert(pthread_cond_wait(&c->rcnd_, &c->mut_) == 0);
    }

    cx_chan_type data;
    if (c->in_ != c->out_) {
        data = c->queue_[c->out_];
        c->out_++;
        c->out_ %= c->cap_;
        pthread_cond_broadcast(&c->wcnd_); // signal writers
    } else {
        data = (cx_chan_type){0};
    }
    assert(pthread_mutex_unlock(&c->mut_) == 0);
    return data;
}
#endif // cx_chan_implement

// Undefine config  macros
#undef cx_chan_name
#undef cx_chan_type
#undef cx_chan_cap
#undef cx_chan_error_handler
#undef cx_chan_allocator
#undef cx_chan_static
#undef cx_chan_inline
#undef cx_chan_implement

// Undefine internal macros
#undef cx_chan_concat2_
#undef cx_chan_concat1_
#undef cx_chan_name_
#undef cx_chan_cap8_
#undef cx_chan_cap16_
#undef cx_chan_cap32_
#undef cx_chan_cap64_
#undef cx_chan_cap_type_
#undef cx_chan_max_cap_
#undef cx_chan_api_
#undef cx_chan_alloc_field_
#undef cx_chan_alloc_global_
#undef cx_chan_alloc_
#undef cx_chan_free_

