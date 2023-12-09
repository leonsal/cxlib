#ifndef CX_ALLOC_POOL_H
#define CX_ALLOC_POOL_H
#include <stddef.h>
#include "cx_alloc.h"

// Pool allocator opaque type
typedef struct CxAllocPool CxAllocPool;

// Pool allocator stats
typedef struct CxAllocPoolStats {
    size_t nallocs;     // Number of individual allocations
    size_t nbytes;      // Total number of requested bytes
    size_t usedBlocks;  // Number of allocated used blocks
    size_t freeBlocks;  // Number of allocated blocks in free list
} CxAllocPoolStats;

// Creates an block allocator using the specified minimum blocksize and
// using the specified memory allocator. If NULL is passed as the allocator,
// the default global allocator (malloc/free) will be used.
// This allocator is used to allocate the block control state and new blocks
// as required.
CxAllocPool* cxAllocPoolCreate(size_t blockSize, const CxAllocator* ca);

// Destroy a previously create block allocator.
void cxAllocPoolDestroy(CxAllocPool* a);

//
// Allocates size bytes using standard alignment and return its pointer
//
void* cxAllocPoolAlloc(CxAllocPool* a, size_t size);

//
// Allocates size bytes using specified alignment and return its pointer
//
void* cxAllocPoolAlloc2(CxAllocPool* a, size_t size, size_t align);

//
// Clears the allocator keeping the memory allocated from parent allocator
//
void cxAllocPoolClear(CxAllocPool* a);

//
// Free all allocated memory
//
void cxAllocPoolFree(CxAllocPool* a);

//
// Returns allocator struct
//
const CxAllocator* cxAllocPoolGetAllocator(CxAllocPool* a);

//
// Returns allocator statistics
//
CxAllocPoolStats cxAllocPoolGetStats(const CxAllocPool* a);

#endif


