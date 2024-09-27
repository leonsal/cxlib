/* WORK IN PROGRESS */

#ifndef CX_TASK_H
#define CX_TASK_H

#include <stdlib.h>
#include "cx_error.h"
#include "cx_alloc.h"

// Creates task flow using specified custom allocator and maximum number of threads
typedef struct CxTaskFlow CxTaskFlow;
CxTaskFlow* cx_task_flow_new(const CxAllocator* alloc, size_t nthreads);

// Finishes all tasks in the current cycle and then destroys the task flow
CxError cx_task_flow_del(CxTaskFlow* tf);

// Run tasks for the specified number of cycles.
// If cycles is 0, runs till cx_task_runner_stop() is called
CxError cx_task_flow_start(CxTaskFlow* tf, size_t cycles);

// Waits for the current cycle to run and then stops the task runner.
CxError cx_task_flow_stop(CxTaskFlow* tf);

// Returns the number of cycles run
size_t cx_task_flow_cycles(CxTaskFlow* tf);

// Creates task which will be run in the specified task runner.
// The task runner must be stopped.
typedef void (*CxTaskFlowTask)(void*);
CxError cx_task_flow_add_task(CxTaskFlow* tf, const char* name, CxTaskFlowTask fn, void* arg, size_t* tid);

// Sets the dependency of a task
CxError cx_task_flow_task_dep(CxTaskFlow* tf, size_t tid, size_t other);


#endif

