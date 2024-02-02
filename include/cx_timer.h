#ifndef CX_TIMER_H
#define CX_TIMER_H

#include <time.h>
#include "cx_alloc.h"

// Creates timer manager using optional allocator.
// Pass NULL to use default 'malloc/free' allocator.
// Returns pointer to created timer or NULL if error.
typedef struct CxTimer CxTimer;
CxTimer* cx_timer_create(const CxAllocator* alloc);

// Destroys previously created timer manager
// Returns non-zero system error code
int cx_timer_destroy(CxTimer*);

// Sets user data associated with this timer manager.
void cx_timer_set_userdata(CxTimer* tm, void* userdata);

// Get user data previously associated with this timer manager
void* cx_timer_get_userdata(CxTimer* tm);

// Sets a function to be called at the specified relative time from now 
// by the timer manager internal thread.
// Sets the optional 'task_id' pointer to get the function task id which
// can be used to clear this timer before it expires.
// Returns non-zero system error code
typedef void (*CxTimerFunc)(CxTimer* tm, void* arg);
int cx_timer_set(CxTimer* tm, struct timespec reltime, CxTimerFunc fn, void* arg, size_t* task_id); 

// Clears previously set timer with the specified task id
// Returns non-zero system error code
int cx_timer_clear(CxTimer* timer, size_t task_id); 

// Clears all pending timer tasks
// Returns non-zero system error code
int cx_timer_clear_all(CxTimer* timer); 

// Returns the current number of tasks waiting to be executed
// Returns non-zero system error code
size_t cx_timer_count(CxTimer* timer);

// Converts specified number of seconds to struct timespec
struct timespec cx_timer_ts_from_secs(double secs);

// Converts specified struct timespec to number of seconds
double cx_timer_secs_from_ts(struct timespec ts);

// Compares struct timespec
// Returns -1 if t1 < t2, 0 if t1 == t2 or 1 if t1 > t2;
int cx_timer_cmp_ts(struct timespec t1, struct timespec t2);

#endif

