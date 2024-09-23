/*
Dynamic Queue Implementation

Implements a dynamic sized queue (FIFO)
 
Example
-------

// Defines queue of 'ints'
#define cx_queue_name qint
#define cx_queue_type int
#define cx_queue_static
#define cx_queue_inline
#define cx_queue_implement
#include "cx_queue.h"

int main() {

    qint q1 = qint_init(10);
    int buf1[] = {0,1,2,3,4};
    qint_putn(&q1, buf, 5);
    qint_put(&q1, 6);
    qint_close(&q1);

    int buf2[10];
    int res = qint_getn(&q1, buf2, 6);
    assert(res == 0 && buf2[5] == 5);

    qint_free(&q1);
    return 0;
}
 
Queue configuration defines
---------------------------

Define the name of the queue type (mandatory):
    #define cx_queue_name <name>

Define the type of the queue elements (mandatory):
    #define cx_queue_type <name>

Define optional custom allocator pointer or function which return pointer to allocator.
Uses default allocator if not defined.
This allocator will be used for all instances of this queue type.
    #define cx_queue_allocator <allocator>

Sets if queue uses custom allocator per instance.
If set, it is necessary to initialize each queue with the desired allocator.
    #define cx_queue_instance_allocator

Sets if all queue functions are prefixed with 'static'
    #define cx_queue_static

Sets if all queue functions are prefixed with 'inline'
    #define cx_queue_inline

Sets to implement functions in this translation unit:
    #define cx_queue_implement


Queue API
---------

Assuming:
#define cx_queue_name cxqueue   // Queue type
#define cx_queue_type cxtype    // Type of elements of the queue

Initialize queue using default allocator and with specified initial capacity in number of elements.
    cxqueue cxqueue_init(size_t cap);

Initialize queue using custom instance allocator and with specified initial capacity in number of elements.
    cxqueue cxqueue_init(const CxAllocator* a, size_t cap);

Free memory allocated by the queue.
    void cxqueue_free(cxqueue* s);

Empty the queue but keeps allocated memory
    void cxqueue_clear(cxqueue* s);

Returns the queue capacity in number of elements
    size_t cxqueue_cap(cxqueue* q);

Returns the current queue length in number of elements
    size_t cxqueue_len(cxqueue* q);

Returns it the queue is empty (length == 0)
    bool cxqueue_empty(const cxqueue* q);

Puts 'n' elements from 'src' at the input (front) of the queue.
    void cxqueue_putn(cxqueue* q, const cxtype* src, size_t n);

Inserts one element at the input (front) of the queue.
    void cxqueue_put(cxqueue* q, cxtype v);

Get 'n' elements from the output (back) of the queue.
Returns number of elements read.
    size_t cxqueue_getn(cxqueue* q, cxtype* src, size_t n);

Get one element from output (back) of the queue.
Returns number of elements read.
    int cxqueue_get(cxqueue* q, cxtype* src);
*/ 
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
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
    #define cx_queue_allocator cx_def_allocator()
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
    cx_queue_alloc_field_           // Optional instance allocator
    size_t              cap_;       // current capacity in number of elements
    size_t              len_;       // current length in number of elements
    size_t              in_;        // input index
    size_t              out_;       // output index
    cx_queue_type*      data_;      // pointer to queue data
} cx_queue_name;

#ifdef cx_queue_instance_allocator
    cx_queue_api_ cx_queue_name cx_queue_name_(_init)(const CxAllocator*, size_t cap);
#else
    cx_queue_api_ cx_queue_name cx_queue_name_(_init)(size_t cap);
#endif
cx_queue_api_ void cx_queue_name_(_free)(cx_queue_name* q);
cx_queue_api_ void cx_queue_name_(_clear)(cx_queue_name* q);
cx_queue_api_ size_t cx_queue_name_(_cap)(const cx_queue_name* q);
cx_queue_api_ size_t cx_queue_name_(_len)(cx_queue_name* q);
cx_queue_api_ bool cx_queue_name_(_empty)(cx_queue_name* q);
cx_queue_api_ void cx_queue_name_(_putn)(cx_queue_name* q, const cx_queue_type* src, size_t n);
cx_queue_api_ void cx_queue_name_(_put)(cx_queue_name* q, const cx_queue_type v);
cx_queue_api_ int cx_queue_name_(_getn)(cx_queue_name* q, cx_queue_type* dst, size_t n);
cx_queue_api_ int cx_queue_name_(_get)(cx_queue_name* q, cx_queue_type* v);

//
// Implementation
//
#ifdef cx_queue_implement
    #include <assert.h>

    // Internal initialization
    cx_queue_api_ void cx_queue_name_(_init_)(cx_queue_name* q, size_t cap) {

        assert(cap > 0);
        q->data_ = cx_queue_alloc_(q, cap * sizeof(cx_queue_type));
        q->cap_ = cap;
    }

#ifdef cx_queue_instance_allocator

    cx_queue_api_ cx_queue_name cx_queue_name_(_init)(const CxAllocator* alloc, size_t cap) {

        cx_queue_name q = {.alloc = alloc};
        cx_queue_name_(_init_)(&q, cap);
        return q;
    }
#else

    cx_queue_api_ cx_queue_name cx_queue_name_(_init)(size_t cap) {

        cx_queue_name q = {0};
        cx_queue_name_(_init_)(&q, cap);
        return q;
    }
#endif

cx_queue_api_ void cx_queue_name_(_free)(cx_queue_name* q) {

    cx_queue_free_(q, q->data_, q->cap_ * sizeof(cx_queue_type));
    q->cap_ = 0;
    q->len_ = 0;
    q->in_ = 0;
    q->out_ = 0;
    q->data_ = NULL;
}

cx_queue_api_ void cx_queue_name_(_clear)(cx_queue_name* q) {

    q->len_ = 0;
    q->in_ = 0;
    q->out_ = 0;
}

cx_queue_api_ size_t cx_queue_name_(_cap)(const cx_queue_name* q) {

    return q->cap_;
}

cx_queue_api_ size_t cx_queue_name_(_len)(cx_queue_name* q) {

    return q->len_;
}

cx_queue_api_ bool cx_queue_name_(_empty)(cx_queue_name* q) {

    return q->len_ == 0;
}

cx_queue_api_ void cx_queue_name_(_putn)(cx_queue_name* q, const cx_queue_type* src, size_t n) {

    // Reallocates area if necessary
    const size_t available = q->cap_ - q->len_;
    if (available < n * sizeof(cx_queue_type)) {
        const size_t new_cap = q->cap_ * 2;
        const size_t alloc_size = new_cap * sizeof(cx_queue_type);
        void* new_data = cx_queue_alloc_(q, alloc_size);
        memcpy(new_data, q->data_, q->cap_);
        q->cap_ = new_cap;
        q->data_ = new_data;
    }

    // Copy data to queue
    const size_t space = q->cap_ - q->in_;
    if (n <= space) {
        memcpy(&q->data_[q->in_], src, n * sizeof(cx_queue_type));
    } else {
        memcpy(&q->data_[q->in_], src, space * sizeof(cx_queue_type));
        memcpy(q->data_, src+space, (n-space) * sizeof(cx_queue_type));
    }
    q->in_ = (q->in_ + n) % q->cap_;
    q->len_ += n;
}

cx_queue_api_ void cx_queue_name_(_put)(cx_queue_name* q, const cx_queue_type v) {

    cx_queue_name_(_putn)(q, &v, 1);
}

cx_queue_api_ int cx_queue_name_(_getn)(cx_queue_name* q, cx_queue_type* dst, size_t n) {

    if (n > q->len_) {
        n = q->len_;
    }
    if (n == 0) {
        return 0;
    }

    // Copy data from queue
    size_t space = q->cap_ - q->out_;
    if (n <= space) {
        memcpy(dst, &q->data_[q->out_], n* sizeof(cx_queue_type));
    } else {
        memcpy(dst, &q->data_[q->out_], space * sizeof(cx_queue_type));
        memcpy(dst + space, q->data_, (n-space) * sizeof(cx_queue_type));
    }
    q->out_ = (q->out_ + n) % q->cap_;
    q->len_ -= n;
    return n;
}



cx_queue_api_ int cx_queue_name_(_get)(cx_queue_name* q, cx_queue_type* v) {

    return cx_queue_name_(_getn)(q, v, 1);
}

#endif

// Undefine config  macros
#undef cx_queue_name
#undef cx_queue_type
#undef cx_queue_cap
#undef cx_queue_allocator
#undef cx_queue_instance_allocator
#undef cx_queue_static
#undef cx_queue_inline
#undef cx_queue_implement

// Undefine internal macros
#undef cx_queue_concat2_
#undef cx_queue_concat1_
#undef cx_queue_name_
#undef cx_queue_api_
#undef cx_queue_alloc_field_
#undef cx_queue_alloc_
#undef cx_queue_free_



