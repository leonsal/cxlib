#ifndef CX_BQUEUE_H
#define CX_BQUEUE_H

#include "cx_alloc.h"

// Block queue opaque type
typedef struct CxBqueue CxBqueue;

// Creates and returns pointer to a new block queue using the specified allocator.
// Pass NULL to use the default allocator.
CxBqueue* cx_bqueue_new(const CxAllocator* alloc);

// Destroy previously created instace of block queue
void cx_bqueue_del(CxBqueue* q);

// Clears the block queue without deallocating its memory
void cx_bqueue_clear(CxBqueue* q);

// Returns the current number of blocks in the queue.
size_t cx_bqueue_count(const CxBqueue* q);

// Puts a new block of the specified size at the end of the queue.
// Returns pointer to the allocated block
// The pointer is valid until another block is put() or get() from the queue.
void* cx_bqueue_put(CxBqueue* q, size_t nbytes);

// Remove and get pointer and size of oldest block in the queue
// The pointer is valid until another block is put() or get() from the queue.
void* cx_bqueue_get(CxBqueue*, size_t* nbytes);

// Returns stats
typedef struct CxBqueueStats {
    size_t used_blocks;
    size_t free_blocks;
    size_t nallocs;
    size_t nreallocs;
    size_t allocmem;
} CxBqueueStats;
CxBqueueStats cx_bqueue_stats(const CxBqueue* q);

// Print stats
void cx_bqueue_pstats(const CxBqueue* q);


#endif

