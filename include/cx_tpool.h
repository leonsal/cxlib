#ifndef CX_TPOOL_H
#define CX_TPOOL_H

#include <stdlib.h>
#include "cx_alloc.h"

// Creates thread pool with 'tcount' threads and with worke queue of 'wsize' using optional allocator.
// The work queue size must be equal or greater the number of threads
// Pass NULL to use default 'malloc/free' allocator.
// Returns pointer to created thread pool or NULL if error.
typedef struct CxThreadPool CxThreadPool;
CxThreadPool* cx_tpool_new(const CxAllocator* alloc, size_t tcount, size_t wsize);

// Finishes all threads and destroys the previously created thread pool
// Returns non-zero system error code
int cx_tpool_del(CxThreadPool* tp);

// Returns current number of work items in the work queue
size_t cx_tpool_work_len(CxThreadPool* tp);

// Clear worker queue and waits for running threads to finish.
// The thread pool continue to be valid
void cx_tpool_work_clear(CxThreadPool* tp);

// Adds worker to the thread pool
typedef void (*CxThreadPoolWorker)(void*);
int cx_tpool_run(CxThreadPool* tp, CxThreadPoolWorker worker, void* param);

#endif

