#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

#include "cx_alloc.h"
#include "cx_alloc_pool.h"

// Block header
typedef struct Block Block;
typedef struct Block {
    Block*  next;       // Pointer to next block in the chain
    size_t  size;       // Size of this block (not including this header)
    char    data[];     // Block data
} Block;

// Block Allocator state
typedef struct CxAllocPool {
    size_t              blockSize;  // Minimum block size
    Block*              nextFree;   // Next free block of the free chain
    Block*              firstBlock; // First block of the allocated chain
    Block*              currBlock;  // Current block of the allocated chain
    size_t              used;       // Bytes allocated in current block (not including block header)
    const CxAllocator*  alloc;      // Allocator for blocks
    CxAllocator         userAlloc;  // Block allocator for users
    CxAllocPoolInfo     info;
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
    a->blockSize = blockSize;
    a->nextFree = NULL;
    a->firstBlock = NULL;
    a->currBlock = NULL;
    a->used = 0;
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
    if (a->currBlock) {
        padding = alignForward(a->used, align) - a->used;
    }
    if (a->currBlock == NULL || (a->used + padding + size > a->currBlock->size)) {
        newBlock(a, size);
        padding = 0;
    }
    void *newp = a->currBlock->data + a->used + padding;
    a->used += padding + size;
    a->info.allocs++;
    return newp;
}

void cxAllocPoolClear(CxAllocPool* a) {

    if (a->firstBlock == NULL) {
        return;
    }
    // Join the used blocks chain with the eventual free blocks chain
    if (a->nextFree != NULL) {
        Block *curr = a->nextFree;
        Block *next = a->nextFree->next;
        while (next != NULL) {
            curr = next;
            next = curr->next;
        }
        curr->next = a->firstBlock;
    } else {
        a->nextFree = a->firstBlock;
    }
    a->firstBlock = NULL;
    a->currBlock = NULL;
    a->used = 0;
    a->info = (CxAllocPoolInfo){0};
}

void cxAllocPoolFree(CxAllocPool* a) {

    Block *b = a->firstBlock;
    while (b != NULL) {
        Block* next = b->next;
        a->alloc->free(a->alloc->ctx, b, sizeof(Block));
        b = next;
    }
    a->currBlock = NULL;
    a->firstBlock = NULL;
    a->used = 0;
    a->info = (CxAllocPoolInfo){0};
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

    // Adjusts block size
    size = size > a->blockSize ? size : a->blockSize;

    // Checks if it is possible to use the next free block
    Block* new = NULL;
    if (a->nextFree != NULL && a->nextFree->size >= size) {
        new = a->nextFree;
        a->nextFree = new->next;
    } else {
        // No next free block or next free block is small:
        // Allocates a new block
        const size_t allocSize = sizeof(Block) + size;
        new = a->alloc->alloc(a->alloc->ctx, allocSize);
        if (new == NULL) {
            abort();
        }
        new->size = size;
    }

    new->next = NULL;
    if (a->currBlock != NULL) {
        a->currBlock->next = new;
    }
    a->currBlock = new;
    if (a->firstBlock == NULL) {
        a->firstBlock = new;
    }
    a->used = 0;

    // Update stats
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
