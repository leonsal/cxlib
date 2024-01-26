/*
Concurrent Queue Implementation

Implements a concurrent fixed size queue (FIFO) which supports
multiple producers and multiple consumers.
 
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

Define optional queue maximum capacity (default = 32):
    #define cx_queue_cap <8|16|32>

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

Initialize queue using default allocator and with specified maximum capacity in number of elements
    cxqueue cxqueue_init(size_t cap);

Initialize queue using custom instance allocator and with specified maximum capacity in number of elements
    cxqueue cxqueue_init(const CxAllocator* a, size_t cap);

Free memory allocated by the queue.
    void cxqueuey_free(cxqueue* s);

Returns the queue capacity in number of elements
    size_t cxqueue_cap(cxqueue* q);

Returns the current queue length in number of elements
    size_t cxqueue_len(cxqueue* q);

Returns it the queue is empty (length == 0)
    bool cxqueue_empty(const cxqueue* q);

Puts 'n' elements from 'src' at the input (front) of the queue.
Blocks till there is space in the queue to insert all elements.
Returns ECANCELED if the queue is closed.
    int cxqueue_putn(cxqueue* q, const cxtype* src, size_t n);

Puts 'n' elements from 'src' at the input (front) of the queue.
Blocks till there is space in the queue to insert all elements or
the specified relative timeout expires.
Returns ECANCELED if the queue is closed.
Returns ETIMEDOUT if timeout expires.
    int cx_queue_putnw)(cxqueue* q, const cxtype* src, size_t n, struct timespec reltime);

Inserts one element at the input (front) of the queue.
Blocks till there is space in the queue to insert the element.
Returns ECANCELED if the queue is closed.
    int cxqueue_put(cxqueue* q, cxtype v);

Inserts one element at the input (front) of the queue.
Blocks till there is space in the queue to insert the element or
the specified relative timeout expires.
Returns ECANCELED if the queue is closed.
Returns ETIMEDOUT if timeout expires.
    int cxqueue_putw(cxqueue* q, cxtype v, struct timespec reltime);

Get 'n' elements from the output (back) of the queue.
Blocks till there is the specified number of elements to remove.
Returns ECANCELED if the queue is closed.
    int cxqueue_getn(cxqueue* q, cxtype* src, size_t n);

Get 'n' elements from the output (back) of the queue.
Blocks till there is the specified number of elements to remove or
the specified relative timeout expires.
Returns ECANCELED if the queue is closed.
Returns ETIMEDOUT if timeout expires.
    int cxqueue_getnw(cxqueue* q, cxtype* src, size_t n, struct timespec reltime);

Get one element from output (back) of the queue.
Blocks till there is element to remove.
Returns ECANCELED if the queue is closed.
    int cxqueue_get(cxqueue* q, cxtype* src);

Get one element from output (back) of the queue.
Blocks till there is element to remove or the specified timeout expires.
Returns ECANCELED if the queue is closed.
Returns ETIMEDOUT if timeout expires.
    int cxqueue_getw(cxqueue* q, cxtype* src);

Closes the queue, unblocking other threads which are trying to put or get data.
After the queue is closed any operation of the queue returns error.
    int cxqueue_close(cxqueue* q);

Returns if the queue is closed.
    bool cxqueue_closed(cxqueue* q);

Resets the queue, enabling the reuse of a previously closed queue.
    int cxqueue_reset(cxqueue* q);

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
    cx_queue_alloc_field_           // Optional instance allocator
    pthread_mutex_t     lock_;      // For exclusive access to this struct
    pthread_cond_t      hasData_;   // Cond var signaled when data is available
    pthread_cond_t      hasSpace_;  // Cond var signaled when space is available
    bool                closed;     // Queue closed flag
    cx_queue_cap_type_  cap_;       // capacity in number of elements
    cx_queue_cap_type_  len_;       // current length in number of elements
    cx_queue_cap_type_  in_;        // input index
    cx_queue_cap_type_  out_;       // output index
    cx_queue_type*      data_;      // pointer to queue data
} cx_queue_name;

#ifdef cx_queue_instance_allocator
    cx_queue_api_ cx_queue_name cx_queue_name_(_init)(const CxAllocator*, size_t cap);
#else
    cx_queue_api_ cx_queue_name cx_queue_name_(_init)(size_t cap);
#endif
cx_queue_api_ void cx_queue_name_(_free)(cx_queue_name* q);
cx_queue_api_ size_t cx_queue_name_(_cap)(cx_queue_name* q);
cx_queue_api_ size_t cx_queue_name_(_len)(cx_queue_name* q);
cx_queue_api_ bool cx_queue_name_(_empty)(cx_queue_name* q);
cx_queue_api_ int cx_queue_name_(_putn)(cx_queue_name* q, const cx_queue_type* src, size_t n);
cx_queue_api_ int cx_queue_name_(_putnw)(cx_queue_name* q, const cx_queue_type* src, size_t n, struct timespec reltime);
cx_queue_api_ int cx_queue_name_(_put)(cx_queue_name* q, const cx_queue_type v);
cx_queue_api_ int cx_queue_name_(_putw)(cx_queue_name* q, const cx_queue_type v, struct timespec reltime);
cx_queue_api_ int cx_queue_name_(_getn)(cx_queue_name* q, cx_queue_type* dst, size_t n);
cx_queue_api_ int cx_queue_name_(_getnw)(cx_queue_name* q, cx_queue_type* dst, size_t n, struct timespec reltime);
cx_queue_api_ int cx_queue_name_(_get)(cx_queue_name* q, cx_queue_type* v);
cx_queue_api_ int cx_queue_name_(_getw)(cx_queue_name* q, cx_queue_type* v, struct timespec reltime);
cx_queue_api_ int cx_queue_name_(_close)(cx_queue_name* q);
cx_queue_api_ int cx_queue_name_(_is_closed)(cx_queue_name* q, bool* closed);
cx_queue_api_ int cx_queue_name_(_reset)(cx_queue_name* q);

//
// Implementation
//
#ifdef cx_queue_implement
    #include <assert.h>
    #include <pthread.h>

    // Internal initialization
    cx_queue_api_ void cx_queue_name_(_init_)(cx_queue_name* q, size_t cap) {

        assert(cap > 0);
        q->data_ = cx_queue_alloc_(q, cap * sizeof(*q->data_));
        q->cap_ = cap;
        assert(pthread_mutex_init(&q->lock_, NULL) == 0);
        assert(pthread_cond_init(&q->hasData_, NULL) == 0);
        assert(pthread_cond_init(&q->hasSpace_, NULL) == 0);
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

    assert(pthread_cond_destroy(&q->hasSpace_) == 0);
    assert(pthread_cond_destroy(&q->hasData_) == 0);
    assert(pthread_mutex_destroy(&q->lock_) == 0);
    cx_queue_free_(q, q->data_, q->cap_ * sizeof(*(q->data_)));
    q->closed = false;
    q->cap_ = 0;
    q->len_ = 0;
    q->in_ = 0;
    q->out_ = 0;
    q->data_ = NULL;
}

cx_queue_api_ size_t cx_queue_name_(_cap)(cx_queue_name* q) {

    return q->cap_;
}

cx_queue_api_ size_t cx_queue_name_(_len)(cx_queue_name* q) {

    assert(pthread_mutex_lock(&q->lock_) == 0);
    size_t len = q->len_;
    assert(pthread_mutex_unlock(&q->lock_) == 0);
    return len;
}

cx_queue_api_ bool cx_queue_name_(_empty)(cx_queue_name* q) {

    assert(pthread_mutex_lock(&q->lock_) == 0);
    bool empty = q->len_ == 0;
    assert(pthread_mutex_unlock(&q->lock_) == 0);
    return empty;
}

cx_queue_api_ int cx_queue_name_(_putn)(cx_queue_name* q, const cx_queue_type* src, size_t n) {

    assert(n <= q->cap_);
    // Waits for space in the queue
    int error = pthread_mutex_lock(&q->lock_);
    if (error) {
        return error;
    }
    while (n > q->cap_ - q->len_ && !error && !q->closed) {
        error = pthread_cond_wait(&q->hasSpace_, &q->lock_);
    }
    if (error) {
        pthread_mutex_unlock(&q->lock_);
        return error;
    }
    if (q->closed) {
        pthread_mutex_unlock(&q->lock_);
        return ECANCELED;
    }

    // Copy data to queue
    size_t space = q->cap_ - q->in_;
    if (n <= space) {
        memcpy(&q->data_[q->in_], src, n* sizeof(*q->data_));
    } else {
        memcpy(&q->data_[q->in_], src, space * sizeof(*q->data_));
        memcpy(q->data_, src+space, (n-space) * sizeof(*q->data_));
    }
    q->in_ = (q->in_ + n) % q->cap_;
    q->len_ += n;

    // Signal that there is data in the queue
    if ((error = pthread_cond_signal(&q->hasData_))) {
        return error;
    }
    return pthread_mutex_unlock(&q->lock_);
}

cx_queue_api_ int cx_queue_name_(_putnw)(cx_queue_name* q, const cx_queue_type* src, size_t n, struct timespec reltime) {

    assert(n <= q->cap_);

    // Calculates absolute time from specified relative time
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);
    abstime.tv_sec += reltime.tv_nsec;
    abstime.tv_nsec += reltime.tv_nsec;

    // Waits for space in the queue or timeout
    int error = pthread_mutex_lock(&q->lock_);
    if (error) {
        return error;
    }
    while (n > q->cap_ - q->len_ && !error && !q->closed) {
        error = pthread_cond_timedwait(&q->hasSpace_, &q->lock_, &abstime);
    }
    if (error) {
        pthread_mutex_unlock(&q->lock_);
        return error;
    }
    if (q->closed) {
        pthread_mutex_unlock(&q->lock_);
        return ECANCELED;
    }

    // Copy data to queue
    size_t space = q->cap_ - q->in_;
    if (n <= space) {
        memcpy(&q->data_[q->in_], src, n* sizeof(*q->data_));
    } else {
        memcpy(&q->data_[q->in_], src, space * sizeof(*q->data_));
        memcpy(q->data_, src+space, (n-space) * sizeof(*q->data_));
    }
    q->in_ = (q->in_ + n) % q->cap_;
    q->len_ += n;

    // Signal that there is data in the queue
    if ((error = pthread_cond_signal(&q->hasData_))) {
        return error;
    }
    return pthread_mutex_unlock(&q->lock_);
}


cx_queue_api_ int cx_queue_name_(_put)(cx_queue_name* q, const cx_queue_type v) {

    return cx_queue_name_(_putn)(q, &v, 1);
}

cx_queue_api_ int cx_queue_name_(_putw)(cx_queue_name* q, const cx_queue_type v, struct timespec reltime) {

    return cx_queue_name_(_putnw)(q, &v, 1, reltime);
}

cx_queue_api_ int cx_queue_name_(_getn)(cx_queue_name* q, cx_queue_type* dst, size_t n) {

    assert(n <= q->cap_);
    // Waits for data in the queue
    int error = pthread_mutex_lock(&q->lock_);
    if (error) {
        return error;
    }
    while (n > q->len_ && !error && !q->closed) {
        error = pthread_cond_wait(&q->hasData_, &q->lock_);
    }
    if (error) {
        pthread_mutex_unlock(&q->lock_);
        return error;
    }
    if (n > q->len_ && q->closed) {
        pthread_mutex_unlock(&q->lock_);
        return ECANCELED;
    }

    // Copy data from queue
    size_t space = q->cap_ - q->out_;
    if (n <= space) {
        memcpy(dst, &q->data_[q->out_], n* sizeof(*q->data_));
    } else {
        memcpy(dst, &q->data_[q->out_], space * sizeof(*q->data_));
        memcpy(dst + space, q->data_, (n-space) * sizeof(*q->data_));
    }
    q->out_ = (q->out_ + n) % q->cap_;
    q->len_ -= n;

    // Signal that there is free space in the queue
    if ((error = pthread_cond_signal(&q->hasSpace_))) {
        return error;
    }
    return pthread_mutex_unlock(&q->lock_);
}

cx_queue_api_ int cx_queue_name_(_getnw)(cx_queue_name* q, cx_queue_type* dst, size_t n, struct timespec reltime) {

    assert(n <= q->cap_);

    // Calculates absolute time from specified relative time
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);
    abstime.tv_sec += reltime.tv_nsec;
    abstime.tv_nsec += reltime.tv_nsec;

    int error = pthread_mutex_lock(&q->lock_);
    if (error) {
        return error;
    }
    while (n > q->len_ && !error && !q->closed) {
        error = pthread_cond_timedwait(&q->hasData_, &q->lock_, &abstime);
    }
    if (error) {
        pthread_mutex_unlock(&q->lock_);
        return error;
    }
    if (n > q->len_ && q->closed) {
        pthread_mutex_unlock(&q->lock_);
        return ECANCELED;
    }

    // Copy data from queue
    size_t space = q->cap_ - q->out_;
    if (n <= space) {
        memcpy(dst, &q->data_[q->out_], n* sizeof(*q->data_));
    } else {
        memcpy(dst, &q->data_[q->out_], space * sizeof(*q->data_));
        memcpy(dst + space, q->data_, (n-space) * sizeof(*q->data_));
    }
    q->out_ = (q->out_ + n) % q->cap_;
    q->len_ -= n;

    // Signal that there is free space in the queue
    if ((error = pthread_cond_signal(&q->hasSpace_))) {
        return error;
    }
    return pthread_mutex_unlock(&q->lock_);
}

cx_queue_api_ int cx_queue_name_(_get)(cx_queue_name* q, cx_queue_type* v) {

    return cx_queue_name_(_getn)(q, v, 1);
}

cx_queue_api_ int cx_queue_name_(_getw)(cx_queue_name* q, cx_queue_type* v, struct timespec reltime) {

    return cx_queue_name_(_getnw)(q, v, 1, reltime);
}

cx_queue_api_ int cx_queue_name_(_close)(cx_queue_name* q) {

    int error = pthread_mutex_lock(&q->lock_);
    if (error) {
        return error;
    }

    q->closed = true;
    int error1 = pthread_cond_broadcast(&q->hasData_);
    int error2 = pthread_cond_broadcast(&q->hasSpace_);
    int error3 = pthread_mutex_unlock(&q->lock_);
    if (error1) { return error1; }
    if (error2) { return error2; }
    if (error3) { return error3; }
    return 0;
}

cx_queue_api_ int cx_queue_name_(_is_closed)(cx_queue_name* q, bool* closed) {

    int error = pthread_mutex_lock(&q->lock_);
    if (error) { return error; }
    *closed = q->closed;
    return pthread_mutex_unlock(&q->lock_);
}

cx_queue_api_ int cx_queue_name_(_reset)(cx_queue_name* q) {

    int error = pthread_mutex_lock(&q->lock_);
    if (error) {
        return error;
    }
    q->closed = false;
    q->len_ = 0;
    q->in_ = 0;
    q->out_ = 0;
    return pthread_mutex_unlock(&q->lock_);
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
#undef cx_queue_cap8_
#undef cx_queue_cap16_
#undef cx_queue_cap32_
#undef cx_queue_cap64_
#undef cx_queue_cap_type_
#undef cx_queue_max_cap_
#undef cx_queue_api_
#undef cx_queue_alloc_field_
#undef cx_queue_alloc_
#undef cx_queue_free_



