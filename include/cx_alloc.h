#ifndef CX_ALLOC_H
#define CX_ALLOC_H
#include <stddef.h>

// Types for allocator methods
typedef struct CxAllocator CxAllocator;
typedef void* (*CxAllocatorAllocFn)(void* ctx, size_t);
typedef void  (*CxAllocatorFreeFn)(void* ctx, void* p, size_t size);
typedef void* (*CxAllocatorReallocFn)(void* ctx, void* old_ptr, size_t old_size, size_t new_size);

// Interface for allocators
typedef struct CxAllocator {
    void* ctx;                      // Allocator internal state
    CxAllocatorAllocFn alloc;       // Allocates new area and returns pointer
    CxAllocatorFreeFn free;         // Free previous allocated area (may be a NOOP)
    CxAllocatorReallocFn realloc;   // Reallocates previous allocated pointer
} CxAllocator;

// Type for allocator error function
typedef void (*CxAllocatorErrorFn)(const char* emsg, void *userdata);

// Returns the default allocator (malloc/free)
const CxAllocator* cxDefaultAllocator();

// Sets default allocator error function
void cx_def_allocator_set_error_fn(CxAllocatorErrorFn fn, void *userdata);


// Utility macros for calling allocator functions
#define cx_alloc_malloc(a,size)\
    a->alloc(a->ctx,size)

#define cx_alloc_free(a,ptr,size)\
    a->free(a->ctx,ptr,size)

#define cx_alloc_realloc(a,old_ptr,old_size,size)\
    a->realloc(a->ctx,old_ptr,old_size,size)

#endif



