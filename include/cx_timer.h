#ifndef CXTIMER_H
#define CXTIMER_H

#include <stddef.h>
#include <time.h>
#include "cx_alloc.h"

// Creates timer manager using optional allocator.
// Pass NULL to use default 'malloc/free' allocator.
typedef struct CxTimerMan CxTimer;
CxTimer* cx_timer_create(const CxAllocator* alloc);

// Destroys previously created timer manager
void cx_timer_destroy(CxTimer*);

// Sets a timer function to be executed at the specified relative time from now.
// Sets the 'task_id' with 
// Returns non-zero error code.
typedef void (*CxTimerFunc)(CxTimer* tm, void* arg);
int cx_timer_set(CxTimer* tm, struct timespec reltime, CxTimerFunc fn, void* arg, size_t* task_id); 

// Unsets
int cx_timer_unset(CxTimer* timer, size_t task_id); 

#endif

