#ifndef CXTIMER_H
#define CXTIMER_H

#include <stddef.h>
#include <time.h>
#include "cx_alloc.h"

// Creates timer manager using optional allocator.
// Pass NULL to use default 'malloc/free' allocator.
// Returns pointer to created timer or NULL if error.
typedef struct CxTimer CxTimer;
CxTimer* cx_timer_create(const CxAllocator* alloc);

// Destroys previously created timer manager
int cx_timer_destroy(CxTimer*);

// Sets user data associated with this timer.
void cx_timer_set_userdata(CxTimer* tm, void* userdata);

// Get user data previously associated with this timer.
void* cx_timer_get_userdata(CxTimer* tm);

// Sets a timer function to be executed at the specified relative time from now.
// Sets the 'task_id' with 
// Returns non-zero error code.
typedef void (*CxTimerFunc)(CxTimer* tm, void* arg);
int cx_timer_set(CxTimer* tm, struct timespec reltime, CxTimerFunc fn, void* arg, size_t* task_id); 

// Unsets previously set timer with the specified task_id
int cx_timer_unset(CxTimer* timer, size_t task_id); 

// Returns the current number of tasks waiting to be executed
size_t cx_timer_count(CxTimer* timer);

// Returns timespec from the specified number of seconds
struct timespec cx_timer_ts_from_secs(double secs);

// Returns number of seconds from the specified struct timespec
double cx_timer_secs_from_ts(struct timespec ts);

// Compares struct timespec
// Returns -1 if t1 < t2, 0 if t1 == t2 or 1 if t1 > t2;
int cx_timer_cmp_ts(struct timespec t1, struct timespec t2);

#endif

