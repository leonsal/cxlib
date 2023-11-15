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

    cx_chan_api_ cx_chan_name cx_chan_name_(_init_)(const CxAllocator* alloc, size_t size) {

        cx_chan_name ch = {0};
        if (size > 0) {
            ch.queue_ = alloc->alloc(alloc->ctx, (size + 1) * sizeof(*ch.queue_));
            ch.cap_ = size + 1;
        }
        assert(mtx_init(&ch.mut_, mtx_plain) == thrd_success);
        assert(mtx_init(&ch.rmut_, mtx_plain) == thrd_success);
        assert(mtx_init(&ch.wmut_, mtx_plain) == thrd_success);
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

cx_chan_api_ size_t cx_chan_name_(_len)(cx_chan_name* c) {

    assert(mtx_lock(&c->mut_) == thrd_success);
    size_t len;
    if (c->in_ >= c->out_) {
        len = c->in_ - c->out_;
    }
    len = c->in_ + (c->cap_ - c->out_);
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
        //printf("writer write:%d\n", v);
        c->data_ = v;
        c->in_ = 1;
        cnd_signal(&c->rcnd_);

        // Wait for reader to read data
        //printf("writer: wait reader\n");
        while (c->in_ > 0) {
            printf("writer: wait %d\n", c->in_);
            assert(cnd_wait(&c->wcnd_, &c->mut_) == thrd_success);
        }

        //printf("writer exit1\n");
        //cnd_broadcast(&c->wcnd_);   // for other writers
        assert(mtx_unlock(&c->mut_) == thrd_success);
        assert(mtx_unlock(&c->wmut_) == thrd_success);
        //printf("writer exit2\n");
        return true;
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
        size_t len;
        if (c->in_ >= c->out_) {
            len = c->in_ - c->out_;
        }
        len = c->in_ + (c->cap_ - c->out_);
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
        printf("reader1\n");
        assert(mtx_lock(&c->rmut_) == thrd_success); // block other readers
        assert(mtx_lock(&c->mut_) == thrd_success); // block other readers
        // If channel is closed, returns channel type zero value
        if (c->closed_) {
            assert(mtx_unlock(&c->mut_) == thrd_success);
            assert(mtx_unlock(&c->rmut_) == thrd_success);
            return (cx_chan_type){0};
        }

        // Waits for data
        printf("reader2\n");
        while (c->in_ == 0) {
            printf("reader3\n");
            assert(cnd_wait(&c->rcnd_, &c->mut_) == thrd_success);
        }
        printf("reader4\n");

        // Reads data and notify writer
        cx_chan_type data = c->data_;
        c->in_ = 0;
        cnd_signal(&c->wcnd_);
        assert(mtx_unlock(&c->mut_) == thrd_success);
        assert(mtx_unlock(&c->rmut_) == thrd_success);
        //printf("reader end:%d\n", data);
        return data;
    }

    // Buffered channel
    assert(mtx_lock(&c->mut_) == thrd_success); // block writers

    // If channel is closed, returns channel type zero value
    if (c->closed_) {
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

