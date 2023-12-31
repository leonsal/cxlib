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
    const CxAllocator*  alloc;      // Allocator for blocks
    CxAllocator         userAlloc;  // Allocator interface
    size_t              blockSize;  // Minimum block size
    Block*              nextFree;   // Next free block of the free chain
    Block*              firstBlock; // First block of the allocated chain
    Block*              currBlock;  // Current block of the allocated chain
    size_t              used;       // Bytes allocated in current block (not including block header)
    size_t              nallocs;    // Number of individual allocations
    size_t              nbytes;     // Total bytes requested for allocation
} CxAllocPool;


// Local functions forward declarations
static void cxAllocPoolDummyFree(void* ctx, void* p, size_t n);
static inline int newBlock(CxAllocPool* p, size_t size);
static inline uintptr_t alignForward(uintptr_t ptr, size_t align);


CxAllocPool* cxAllocPoolCreate(size_t blockSize, const CxAllocator* alloc) {

    if (alloc == NULL) {
        alloc = cxDefaultAllocator();
    }
    CxAllocPool* a = cx_alloc_malloc(alloc, sizeof(CxAllocPool));
    a->alloc = alloc;
    a->userAlloc = (CxAllocator){
        .ctx = a,
        .alloc = (CxAllocatorAllocFn)cxAllocPoolAlloc,
        .free = cxAllocPoolDummyFree,
    };

    a->blockSize = blockSize;
    a->nextFree = NULL;
    a->firstBlock = NULL;
    a->currBlock = NULL;
    a->used = 0;
    a->nallocs = 0;
    a->nbytes = 0;
    return a;
}

void cxAllocPoolDestroy(CxAllocPool* a) {

    cxAllocPoolFree(a);
    cx_alloc_free(a->alloc, a, sizeof(CxAllocPool));
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
        if (newBlock(a, size)) {
            return NULL;
        }
        padding = 0;
    }
    void *p = a->currBlock->data + a->used + padding;
    a->used += padding + size;
    a->nallocs++;
    a->nbytes += size;
    return p;
}

void cxAllocPoolClear(CxAllocPool* a) {

    if (a->firstBlock == NULL) {
        return;
    }
    // Join free blocks chain with the used blocks
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
    a->nallocs = 0;
    a->nbytes = 0;
}

void cxAllocPoolFree(CxAllocPool* a) {

    // Free used blocks
    Block *b = a->firstBlock;
    while (b != NULL) {
        Block* next = b->next;
        cx_alloc_free(a->alloc, b, sizeof(Block));
        b = next;
    }
    // Free unused blocks
    b = a->nextFree;
    while (b != NULL) {
        Block* next = b->next;
        cx_alloc_free(a->alloc, b, sizeof(Block));
        b = next;
    }
    a->nextFree = NULL;
    a->currBlock = NULL;
    a->firstBlock = NULL;
    a->used = 0;
    a->nallocs = 0;
    a->nbytes = 0;
}

static void cxAllocPoolDummyFree(void* ctx, void* p, size_t n) {}

const CxAllocator* cxAllocPoolGetAllocator(CxAllocPool* a) {

    return &a->userAlloc;
}

CxAllocPoolStats cxAllocPoolGetStats(const CxAllocPool* a) {

    CxAllocPoolStats stats = {
        .nallocs = a->nallocs,
        .nbytes = a->nbytes,
    };

    // Counts number of used blocks
    Block* curr = a->firstBlock;
    while (curr != NULL) {
        stats.usedBlocks++;
        curr = curr->next;
    }

    // Counts number of unused blocks
    curr = a->nextFree;
    while (curr != NULL) {
        stats.freeBlocks++;
        curr = curr->next;
    }
    return stats;
}

// Allocates a new block for with the specified size.
static inline int newBlock(CxAllocPool* a, size_t size) {

    // Adjusts block size
    size = size > a->blockSize ? size : a->blockSize;

    Block* new = NULL;
    // Looks for free block which fits requested size
    if (a->nextFree != NULL) {
        Block* prev = NULL;
        Block* curr = a->nextFree;
        while (curr != NULL && curr->size < size) {
            prev = curr;
            curr = curr->next;
        }
        if (curr != NULL) {
            new = curr;
            if (prev != NULL) {
                prev->next = curr->next;
            } else {
                a->nextFree = curr->next;
            }
        }
    }

    // If no free block found with requested size, allocates a new block
    if (new == NULL) {
        const size_t allocSize = sizeof(Block) + size;
        new = cx_alloc_malloc(a->alloc, allocSize);
        if (new == NULL) {
            return -1;
        }
        new->size = size;
    }
    new->next = NULL;

    // Add new block to used block chain
    if (a->currBlock != NULL) {
        a->currBlock->next = new;
    }
    a->currBlock = new;

    // Saves the pointer of first block of the used block chain
    if (a->firstBlock == NULL) {
        a->firstBlock = new;
    }
    a->used = 0;
    return 0;
}

// Returns the aligned pointer for the specified pointer and desired alignment
static inline uintptr_t alignForward(uintptr_t ptr, size_t align) {

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
