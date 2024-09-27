#ifndef CX_TFLOW_H
#define CX_TFLOW_H

#include <stdlib.h>
#include <stdbool.h>
#include "cx_error.h"
#include "cx_alloc.h"

// Creates task flow using specified custom allocator and maximum number of threads
typedef struct CxTFlow CxTFlow;
CxTFlow* cx_tflow_new(const CxAllocator* alloc, size_t nthreads);

// Finishes all tasks in the current cycle and then destroys the task flow
CxError cx_tflow_del(CxTFlow* tf);

// Run tasks for the specified number of cycles.
// If cycles is 0, runs till cx_task_runner_stop() is called
CxError cx_tflow_start(CxTFlow* tf, size_t cycles);

// Waits for the current cycle to run and then stops the task runner.
CxError cx_tflow_stop(CxTFlow* tf);

// Returns if the Task Flow is running
bool cx_tflow_running(CxTFlow* tf);

// Returns the number of cycles run
size_t cx_tflow_cycles(CxTFlow* tf);

// Adds task in the Task Flow
// The task runner must be stopped.
typedef void (*CxTFlowTaskFn)(void*);
typedef struct CxTFlowTask CxTFlowTask;
CxError cx_tflow_add_task(CxTFlow* tf, const char* name, CxTFlowTaskFn fn, void* arg, void* udata, CxTFlowTask** ptask);

// Sets the dependency of a task
CxError cx_tflow_add_output(CxTFlowTask* task, CxTFlowTask* dep);

// Returns the current number of added tasks
size_t cx_tflow_task_count(CxTFlow* tf);

// Returns task at the specified index
CxTFlowTask* cx_tflow_get_task(CxTFlow* tf, size_t idx);

// Returns task name
const char* cx_tflow_get_name(CxTFlowTask* task);

// Returns user data associated with task
void* cx_tflow_get_udata(CxTFlowTask* task);


#endif

