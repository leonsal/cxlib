#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <pthread.h>

#include "cx_alloc.h"
#include "cx_pool_allocator.h"

// Block header
typedef struct Block Block;
typedef struct Block {
    Block*  next;       // Pointer to next block in the chain
    size_t  size;       // Size of this block (not including this header)
    char    data[];     // Block data
} Block;

// Block Allocator state
typedef struct CxPoolAllocator {
    pthread_mutex_t     lock;
    const CxAllocator*  alloc;          // Allocator for blocks
    CxAllocator         iface;          // Allocator interface
    CxAllocatorErrorFn  error_fn;       // Optional error function
    void*               error_udata;    // Optional error function userdata
    size_t              blockSize;      // Minimum block size
    Block*              nextFree;       // Next free block of the free chain
    Block*              firstBlock;     // First block of the allocated chain
    Block*              currBlock;      // Current block of the allocated chain
    size_t              used;           // Bytes allocated in current block (not including block header)
    size_t              nallocs;        // Number of individual allocations
    size_t              nbytes;         // Total bytes requested for allocation
} CxPoolAllocator;


// Local functions forward declarations
static void cxAllocPoolDummyFree(void* ctx, void* p, size_t n);
static inline int newBlock(CxPoolAllocator* p, size_t size);
static inline uintptr_t alignForward(uintptr_t ptr, size_t align);


CxPoolAllocator* cx_pool_allocator_create(size_t blockSize, const CxAllocator* alloc) {

    if (alloc == NULL) {
        alloc = cxDefaultAllocator();
    }
    CxPoolAllocator* a = cx_alloc_malloc(alloc, sizeof(CxPoolAllocator));
    a->alloc = alloc;
    a->iface = (CxAllocator){
        .ctx = a,
        .alloc = (CxAllocatorAllocFn)cx_pool_allocator_alloc,
        .free = cxAllocPoolDummyFree,
        .realloc = (CxAllocatorReallocFn)cx_pool_allocator_realloc,
    };
    assert(pthread_mutex_init(&a->lock, NULL) == 0);

    a->blockSize = blockSize;
    a->nextFree = NULL;
    a->firstBlock = NULL;
    a->currBlock = NULL;
    a->used = 0;
    a->nallocs = 0;
    a->nbytes = 0;
    return a;
}

void cx_pool_allocator_set_error_fn(CxPoolAllocator* a, CxAllocatorErrorFn fn, void* userdata) {

    a->error_fn = fn;
    a->error_udata = userdata;
}

void cx_pool_allocator_destroy(CxPoolAllocator* a) {

    cx_pool_allocator_free(a);
    assert(pthread_mutex_destroy(&a->lock) == 0);
    cx_alloc_free(a->alloc, a, sizeof(CxPoolAllocator));
}

void* cx_pool_allocator_alloc(CxPoolAllocator* a, size_t size) {

    return cx_pool_allocator_alloc2(a, size, _Alignof(long double));
}

void* cx_pool_allocator_alloc2(CxPoolAllocator* a, size_t size, size_t align) {

    assert(pthread_mutex_lock(&a->lock) == 0);
    void* pdata = NULL;
    uintptr_t padding = 0;
    if (a->currBlock) {
        padding = alignForward(a->used, align) - a->used;
    }
    int res = 0;
    if (a->currBlock == NULL || (a->used + padding + size > a->currBlock->size)) {
        res = newBlock(a, size);
        if (res) {
            goto exit;
        }
        padding = 0;
    }
    pdata = a->currBlock->data + a->used + padding;
    a->used += padding + size;
    a->nallocs++;
    a->nbytes += size;

exit:
    assert(pthread_mutex_unlock(&a->lock) == 0);
    if (res && a->error_fn) {
        a->error_fn("Error allocating new block", a->error_udata);
    }
    return pdata;
}

void* cx_pool_allocator_realloc(CxPoolAllocator* a, void* old_ptr, size_t old_size, size_t size) {

    if (size <= old_size) {
        return old_ptr;
    }

    void* pnew = cx_pool_allocator_alloc(a, size);
    if (pnew == NULL) {
        return pnew;
    }

    if (old_ptr != NULL) {
        assert(pthread_mutex_lock(&a->lock) == 0);
        memcpy(pnew, old_ptr, old_size);
        assert(pthread_mutex_unlock(&a->lock) == 0);
    }
    return pnew;
}

void cx_pool_allocator_clear(CxPoolAllocator* a) {

    assert(pthread_mutex_lock(&a->lock) == 0);
    if (a->firstBlock == NULL) {
        goto exit;
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

exit:
    assert(pthread_mutex_unlock(&a->lock) == 0);
}

void cx_pool_allocator_free(CxPoolAllocator* a) {

    assert(pthread_mutex_lock(&a->lock) == 0);
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
    assert(pthread_mutex_unlock(&a->lock) == 0);
}

static void cxAllocPoolDummyFree(void* ctx, void* p, size_t n) {}

const CxAllocator* cx_pool_allocator_iface(const CxPoolAllocator* a) {

    return &a->iface;
}

CxPoolAllocatorStats cx_pool_allocator_stats(CxPoolAllocator* a) {

    assert(pthread_mutex_lock(&a->lock) == 0);
    CxPoolAllocatorStats stats = {
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
    assert(pthread_mutex_unlock(&a->lock) == 0);
    return stats;
}

// Allocates a new block for with the specified size.
static inline int newBlock(CxPoolAllocator* a, size_t size) {

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
