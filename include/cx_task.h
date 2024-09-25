/* WORK IN PROGRESS */

#ifndef CX_TASK_H
#define CX_TASK_H

#include <stdlib.h>
#include "cx_error.h"
#include "cx_alloc.h"

// Creates task runner using specified custom allocator and maximum number of threads
typedef struct CxTaskRunner CxTaskRunner;
CxTaskRunner* cx_task_runner_new(const CxAllocator* alloc, size_t nthreads);

// Finishes all threads and destroys the previously created task runneruler
// Returns non-zero system error code
CxError cx_task_runner_del(CxTaskRunner* ts);

// Run tasks for the specified number of cycles.
// If cycles is 0, runs till cx_task_runner_stop() is calledd
CxError cx_task_runner_start(CxTaskRunner* ts, size_t cycles);

// Waits for the current cycle to run and then stops the task runner.
CxError cx_task_runner_stop(CxTaskRunner* ts);

// Returns the number of cycles run
size_t cx_task_runner_cycles(CxTaskRunner* ts);

// Creates task which will be run in the specified task runner.
// The task runner must be stopped.
typedef void (*CxTaskRunnerTask)(void*);
typedef struct CxTask CxTask;
CxError cx_task_new(CxTaskRunner* ts, CxTaskRunnerTask t, void* arg, CxTask* task);

// Sets the dependencies of a task
CxError cx_task_depends(CxTask* t, CxTask* other);


#endif

