#ifndef CX_TFLOW_H
#define CX_TFLOW_H

#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "cx_error.h"
#include "cx_alloc.h"
#include "cx_tracer.h"

typedef struct CxTFlow CxTFlow;
// Creates task flow using specified custom allocator and maximum number of threads
// The tracer is optional
CxTFlow* cx_tflow_new(const CxAllocator* alloc, size_t nthreads, CxTracer* tracer);

// Finishes all tasks in the current cycle and then destroys the task flow
CxError cx_tflow_del(CxTFlow* tf);

typedef void (*CxTFlowStopCb)(CxTFlow*, void*);
// Sets optional callback which will be called when the flow stops
// after running for specified number of cycles.
// This callback is not called when the flow is stopped using: cx_tflow_stop()
void cx_tflow_set_stop_cb(CxTFlow* tf, CxTFlowStopCb cb, void* udata);

// Run tasks for the specified number of cycles.
// If cycles is 0, runs till cx_task_runner_stop() is called
CxError cx_tflow_start(CxTFlow* tf, size_t cycles);

// Waits for the current cycle to run and then stops the task runner.
CxError cx_tflow_stop(CxTFlow* tf, struct timespec timeout);

// Returns Task Flow status
typedef struct CxTFlowStatus {
    bool    running;        // Running flag
    size_t  cycles;         // Number of cycles to run (0=unlimited)
    size_t  run_cycles;     // Number of cycles run
} CxTFlowStatus;
CxTFlowStatus cx_tflow_status(CxTFlow* tf);

// Returns the tracer specified when creating the TaskFlow
CxTracer* cx_tflow_tracer(const CxTFlow* tf);

// Waits for Task Flow to finish
CxError cx_tflow_wait(CxTFlow* tf, struct timespec reltime);

typedef CxError (*CxTFlowTaskFn)(void*);
typedef struct CxTFlowTask CxTFlowTask;
// Adds task in the Task Flow
// The task flow must be stopped.
CxError cx_tflow_add_task(CxTFlow* tf, const char* name, CxTFlowTaskFn fn, void* arg, CxTFlowTask** ptask);

// Sets the dependency (input) of a task
// The task flow must be stopped.
CxError cx_tflow_set_task_dep(CxTFlowTask* task, CxTFlowTask* dep);

// Sets task associated user data
void cx_tflow_set_task_udata(CxTFlowTask* task, void* udata);

// Returns user data associated with task
void* cx_tflow_task_udata(const CxTFlowTask* task);

// Returns the current number of added tasks
size_t cx_tflow_task_count(const CxTFlow* tf);

// Returns task at the specified index (for iteration)
// Return NULL if index is invalid
CxTFlowTask* cx_tflow_task_at(const CxTFlow* tf, size_t idx);

// Returns task with specified name
// Return NULL if task name not found
CxTFlowTask* cx_tflow_find_task(const CxTFlow* tf, const char* name);

// Returns task name
const char* cx_tflow_task_name(const CxTFlowTask* task);

// Returns number of task inputs (dependencies)
size_t cx_tflow_task_inps(const CxTFlowTask* task);

// Returns input task for the specified index.
// Returns NULL if the index is invalid
CxTFlowTask* cx_tflow_task_inp_at(const CxTFlowTask* task, size_t idx);

// Returns number of task outputs (dependants)
size_t cx_tflow_task_outs(const CxTFlowTask* task);

// Returns ouput task for the specified index.
// Returns NULL if the index is invalid
CxTFlowTask* cx_tflow_task_out_at(const CxTFlowTask* task, size_t idx);


#endif

