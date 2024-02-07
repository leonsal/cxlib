#ifndef CX_ALLOC_POOL_H
#define CX_ALLOC_POOL_H
#include <stddef.h>
#include "cx_alloc.h"

// Pool allocator opaque type
typedef struct CxPoolAllocator CxPoolAllocator;

// Pool allocator stats
typedef struct CxPoolAllocatorStats {
    size_t nallocs;     // Number of individual allocations
    size_t nbytes;      // Total number of requested bytes
    size_t usedBlocks;  // Number of allocated used blocks
    size_t freeBlocks;  // Number of allocated blocks in free list
} CxPoolAllocatorStats;

// Creates an block allocator using the specified minimum blocksize and
// using the specified memory allocator. If NULL is passed as the allocator,
// the default global allocator (malloc/free) will be used.
// This allocator is used to allocate the block control state and new blocks
// as required.
CxPoolAllocator* cx_pool_allocator_create(size_t blockSize, const CxAllocator* ca);

// Destroy a previously created block allocator freeing all allocated memory.
void cx_pool_allocator_destroy(CxPoolAllocator* a);

//
// Allocates size bytes using standard alignment and return its pointer
//
void* cx_pool_allocator_alloc(CxPoolAllocator* a, size_t size);

//
// Allocates size bytes using specified alignment and return its pointer
//
void* cx_pool_allocator_alloc2(CxPoolAllocator* a, size_t size, size_t align);

//
// Clears the allocator keeping the memory allocated from parent allocator
//
void cx_pool_allocator_clear(CxPoolAllocator* a);

// Free all allocated memory from upstream allocator.
// This allocator can continue to be used
void cx_pool_allocator_free(CxPoolAllocator* a);

//
// Returns allocator interface 
//
const CxAllocator* cx_pool_allocator_iface(CxPoolAllocator* a);

//
// Returns allocator statistics
//
CxPoolAllocatorStats cx_pool_allocator_stats(CxPoolAllocator* a);

#endif


