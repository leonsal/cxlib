#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
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
    cnd_t              cnd_;
    cx_chan_cap_type_  cap_;   // current capacity
    cx_chan_cap_type_  in_;    // input index
    cx_chan_cap_type_  out_;   // output index
    cx_chan_type*      data_;
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
cx_chan_api_ void cx_chan_name_(_send)(cx_chan_name* q, cx_chan_type v);
cx_chan_api_ cx_chan_type cx_chan_name_(_recv)(cx_chan_name* q);

//
// Implementation
//
#ifdef cx_chan_implement
    cx_chan_alloc_global_;

    cx_chan_api_ cx_chan_name cx_chan_name_(_init_)(const CxAllocator* alloc, size_t size) {
        cx_chan_name ch = {0};
        if (size > 0) {
            ch.data_ = alloc->alloc(alloc->ctx, size * sizeof(*ch.data_));
            ch.cap_ = size;
        }
        assert(mtx_init(&ch.mut_, mtx_plain) == thrd_success);
        assert(cnd_init(&ch.cnd_) == thrd_success);
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



#endif

