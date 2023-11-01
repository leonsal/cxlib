#ifndef CX_ALLOC_BLOCK_H
#define CX_ALLOC_BLOCK_H
#include <stddef.h>
#include "cx_alloc.h"

// Block allocator opaque type
typedef struct CxAllocBlock CxAllocBlock;

// Block allocator info
typedef struct CxAllocBlockInfo {
    size_t allocs;      // Number of individual allocations
    size_t blocks;      // Number of blocks allocated
    size_t maxBlock;    // Maximum block size
    size_t nbytes;      // Total number of bytes allocated
} CxAllocBlockInfo;

// Creates an block allocator using the specified minimum blocksize and
// using the specified memory allocator. If NULL is passed as the allocator,
// the default global allocator (malloc/free) will be used.
// This allocator is used to allocate the block control state and new blocks
// as required.
CxAllocBlock* cxAllocBlockCreate(size_t blockSize, const CxAllocator* ca);

// Destroy a previously create block allocator.
void cxAllocBlockDestroy(CxAllocBlock* a);

//
// Allocates size bytes using standard alignment and return its pointer
//
void* cxAllocBlockAlloc(CxAllocBlock* a, size_t size);

//
// Allocates size bytes using specified alignment and return its pointer
//
void* cxAllocBlockAlloc2(CxAllocBlock* a, size_t size, size_t align);

//
// Free all allocated blocks of the allocator
//
void cxAllocBlockFree(CxAllocBlock* a);

//
// Returns allocator struct
//
const CxAllocator* cxAllocBlockGetAllocator(CxAllocBlock* a);

//
// Returns nfo
//
CxAllocBlockInfo cxAllocBlockGetInfo(const CxAllocBlock* a);

#endif


