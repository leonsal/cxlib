#ifndef CX_ALLOC_H
#define CX_ALLOC_H
#include <stddef.h>


typedef void* (*CxAllocatorAllocFn)(void*, size_t);
typedef void  (*CxAllocatorFreeFn)(void*, void*, size_t);

// Interface for allocators
typedef struct CxAllocator {
    void* ctx;                  // Allocator internal state
    CxAllocatorAllocFn alloc;   // Allocates and returns pointer
    CxAllocatorFreeFn free;     // Free previous allocated pointer (may be a NOOP)
} CxAllocator;

//
// Returns the default allocator (malloc/free)
//
const CxAllocator* cxDefaultAllocator();

// Macros for allocating and freeing from an Allocator
#define cx_alloc_malloc(a,n)    a->alloc(a->ctx,n)
#define cx_alloc_free(a,p,n)    a->free(a->ctx,p,n)

#endif



