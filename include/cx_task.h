#ifndef CX_TASK_FLOW_H
#define CX_TASK_FLOW_H

#include <stdlib.h>
#include <stdbool.h>
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

// Returns if the Task Flow is running
bool cx_task_flow_running(CxTaskFlow* tf);

// Returns the number of cycles run
size_t cx_task_flow_cycles(CxTaskFlow* tf);

// Adds task in the Task Flow
// The task runner must be stopped.
typedef void (*CxTaskFlowTaskFn)(void*);
typedef struct CxTaskFlowTask CxTaskFlowTask;
CxError cx_task_flow_add_task(CxTaskFlow* tf, const char* name, CxTaskFlowTaskFn fn, void* arg, void* udata, CxTaskFlowTask** ptask);

// Sets the dependency of a task
CxError cx_task_flow_set_task_dep(CxTaskFlowTask* task, CxTaskFlowTask* dep);

// Returns the current number of added tasks
size_t cx_task_flow_task_count(CxTaskFlow* tf);

// Returns task at the specified index
CxTaskFlowTask* cx_task_flow_get_task(CxTaskFlow* tf, size_t idx);

// Returns task name
const char* cx_task_flow_get_name(CxTaskFlowTask* task);

// Returns user data associated with task
void* cx_task_flow_get_udata(CxTaskFlowTask* task);


#endif

