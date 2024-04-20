#ifndef CX_ALLOC_H
#define CX_ALLOC_H

#include <stddef.h>
#include <string.h>

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
const CxAllocator* cx_def_allocator();

// Sets default allocator error function
void cx_def_allocator_set_error_fn(CxAllocatorErrorFn fn, void *userdata);

// Allocates memory using the specified allocator or NULL for default allocator
static inline void* cx_alloc_malloc(const CxAllocator* alloc, size_t size) {
    const CxAllocator* al = alloc ? alloc : cx_def_allocator();
    return al->alloc(al->ctx, size);
}

// Allocates memory using the specified allocator or NULL for default allocator.
// Fills allocated area with zeros.
static inline void* cx_alloc_mallocz(const CxAllocator* alloc, size_t size) {
    const CxAllocator* al = alloc ? alloc : cx_def_allocator();
    void* p = al->alloc(al->ctx, size);
    if (p) {
        memset(p, 0, size);
    }
    return p;
}

// Free memory using the specified allocator or NULL for default allocator
static inline void cx_alloc_free(const CxAllocator* alloc, void* ptr, size_t size) {
    const CxAllocator* al = alloc ? alloc : cx_def_allocator();
    al->free(al->ctx, ptr, size);
}

// Reallocates memory using the specified allocator or NULL for default allocator
static inline void* cx_alloc_realloc(const CxAllocator* alloc, void* old_ptr, size_t old_size, size_t size) {
    const CxAllocator* al = alloc ? alloc : cx_def_allocator();
    return al->realloc(al->ctx, old_ptr, old_size, size);
}

#endif



