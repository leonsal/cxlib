#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "cx_alloc.h"
#include "cx_alloc_pool.h"

// Header of arena blocks
typedef struct Block Block;
typedef struct Block {
    Block*  prev;       // Pointer to previous block in the chain
    size_t  size;       // Size of this block (not including this header)
    size_t  used;       // How many bytes used in this block (not including this header)
    char   data[];     // Block data
} Block;

// Block Allocator control state
typedef struct CxAllocPool {
    size_t              blockSize;  // Minimum block size
    Block*              curr;       // Pointer to current block
    const CxAllocator*  alloc;      // Allocator for blocks
    CxAllocator         userAlloc;  // Block allocator for users
    CxAllocPoolInfo    info;
} CxAllocPool;


// Local functions forward declarations
static void cxAllocPoolDummyFree(void* ctx, void* p, size_t n);
static inline void newBlock(CxAllocPool* p, size_t size);
static inline uintptr_t alignForward(uintptr_t ptr, size_t align);


CxAllocPool* cxAllocPoolCreate(size_t blockSize, const CxAllocator* alloc) {

    if (alloc == NULL) {
        alloc = cxDefaultAllocator();
    }
    CxAllocPool* a = alloc->alloc(alloc->ctx, sizeof(CxAllocPool));
    a->curr = NULL;
    a->blockSize = blockSize;
    a->alloc = alloc;
    a->userAlloc = (CxAllocator){
        .ctx = a,
        .alloc = (CxAllocatorAllocFn)cxAllocPoolAlloc,
        .free = cxAllocPoolDummyFree,
    };
    a->info = (CxAllocPoolInfo){};
    return a;
}

void cxAllocPoolDestroy(CxAllocPool* a) {

    cxAllocPoolFree(a);
    a->alloc->free(a->alloc->ctx, a, sizeof(CxAllocPool));
}

void* cxAllocPoolAlloc(CxAllocPool* a, size_t size) {

    return cxAllocPoolAlloc2(a, size, _Alignof(long double));
}

void* cxAllocPoolAlloc2(CxAllocPool* a, size_t size, size_t align) {

    uintptr_t padding = 0;
    if (a->curr) {
        padding = alignForward(a->curr->used, align) - a->curr->used;
    }
    if (a->curr == NULL || (a->curr->used + padding + size > a->blockSize)) {
        newBlock(a, size);
        padding = 0;
    }
    void *newp = a->curr->data + a->curr->used + padding;
    a->curr->used += padding + size;
    a->info.allocs++;
    return newp;
}

void cxAllocPoolClear(CxAllocPool* a) {



}

void cxAllocPoolFree(CxAllocPool* a) {

    Block *b = a->curr;
    while (b != NULL) {
        Block* prev = b->prev;
        a->alloc->free(a->alloc->ctx, b, sizeof(Block));
        b = prev;
    }
    a->curr = NULL;
}

static void cxAllocPoolDummyFree(void* ctx, void* p, size_t n) {}

const CxAllocator* cxAllocPoolGetAllocator(CxAllocPool* a) {

    return &a->userAlloc;
}

CxAllocPoolInfo cxAllocPoolGetInfo(const CxAllocPool* a) {

    return a->info;
}


// Allocates a new block for the pool with the specified size.
static inline void newBlock(CxAllocPool* a, size_t size) {

    if (size < a->blockSize) {
        size = a->blockSize;
    }
    const size_t allocSize = sizeof(Block) + size;
    Block* b;
    b = a->alloc->alloc(a->alloc->ctx, allocSize);
    if (b == NULL) {
        abort();
    }
    b->size = size;
    b->used = 0;
    b->prev = a->curr;
    a->curr = b;
    a->info.blocks++;
    a->info.nbytes += size;
    if (size > a->info.maxPool) {
        a->info.maxPool = size;
    }
}

// Returns the aligned pointer for the specified pointer and desired alignment
static uintptr_t alignForward(uintptr_t ptr, size_t align) {

    // Alignment must be power of 2
    assert((align & (align-1)) == 0);
	uintptr_t p, a, modulo;

	p = ptr;
	a = (uintptr_t)align;
	// Same as (p % a) but faster as 'a' is a power of two
	modulo = p & (a-1);

	if (modulo != 0) {
		// If 'p' address is not aligned, push the address to the
		// next value which is aligned
		p += a - modulo;
	}
	return p;
}