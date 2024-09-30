#include <unistd.h>
#include "cx_tflow.h"
#include "logger.h"
#include "registry.h"


// Task descriptor used to build the Task Flows
// and passed as parameter to task function
typedef struct TaskDesc {
    const char*     name;          // Task name
    const char*     deps[32];      // Array of names of dependant tasks (NULL terminated)
    size_t          us;            // Microseconds to sleep
    size_t          out;           // Output value
    CxTFlowTask*    task;          // Pointer to created task
} TaskDesc;

static void task_func(void* arg) {

    TaskDesc* desc = arg;
    // For each input task, get its output value and sums to the output of this task.
    size_t inps = cx_tflow_task_inps(desc->task);
    size_t outs = cx_tflow_task_outs(desc->task);
    // For intermediate tasks only sum the inputs, do not accumulate.
    // This is done to be easier to calculate the final value for sink tasks.
    if (inps && outs) {
        desc->out = 0;
    }
    for (size_t i = 0; i < inps; i ++) {
        CxTFlowTask* task = cx_tflow_task_inp_at(desc->task, i);
        TaskDesc* inp_desc = cx_tflow_task_udata(task);
        desc->out += inp_desc->out;
    }
    printf("%s: name:%s out:%zu\n", __func__, desc->name, desc->out);
    usleep(desc->us);
}

// Builds TaskFlow from array of task descriptors
static CxTFlow* build_tflow(const CxAllocator* alloc, size_t nthreads, TaskDesc* desc) {

    // Creates Task Flow
    CxTracer* tr = cx_tracer_new(alloc, 16*1024);
    CxTFlow* tf = cx_tflow_new(alloc, nthreads, tr);

    // Adds all tasks from descriptor array
    for (size_t i = 0; desc[i].name != NULL; i++) {
        CxTFlowTask* ptask;
        CXERR_CHK(cx_tflow_add_task(tf, desc[i].name, task_func, &desc[i], &desc[i].task));
        cx_tflow_set_task_udata(desc[i].task, &desc[i]);
    }

    // Sets tasks dependencies
    for (size_t i = 0; desc[i].name != NULL; i++) {
        for (size_t j = 0; desc[i].deps[j] != NULL; j++) {
            const char* depname = desc[i].deps[j];
            CxTFlowTask* task_dep = cx_tflow_find_task(tf, depname);
            CXCHK(task_dep != NULL);
            CXERR_CHK(cx_tflow_set_task_dep(desc[i].task, task_dep));
        }
    }

    return tf;
}

static void run_tflow(CxTFlow* tf, size_t ncycles, const char* test_name) {

    CXERR_CHK(cx_tflow_start(tf, ncycles));
    struct timespec timeout = {};

    CXERR_CHK(cx_tflow_wait(tf, timeout));
    CxTFlowStatus status = cx_tflow_status(tf);

    CXCHK(status.running == false);
    CXCHK(status.cycles == ncycles);
    CXCHK(status.run_cycles == ncycles);

    // Generates optional tracer file
    CxTracer* tracer = cx_tflow_tracer(tf);
    if (tracer && test_name) {
        char test_path[256];
        strcpy(test_path, test_name);
        strcat(test_path, ".json");
        CxError error = cx_tracer_json_write_file(tracer, test_path);
        if (error.msg) {
            LOGE("Error generating tracer file:%s -> %s", test_path, error.msg);
        }
    }
}

// Single source/sink task
void test_tflow1(const CxAllocator* alloc, size_t nthreads) {

    TaskDesc flow[] = {
        { .name = "t1", .us = 1000},
        {}, // terminator
    };
    CxTFlow* tf = build_tflow(alloc, nthreads, flow);
    run_tflow(tf, 5, __func__);
    cx_tflow_del(tf);
}

// Some source/sink tasks
void test_tflow2(const CxAllocator* alloc, size_t nthreads, size_t ncycles) {

    TaskDesc flow[] = {
        { .name = "t1", .us = 1000 },
        { .name = "t2", .us = 500 },
        { .name = "t3", .us = 200 },
        {}, // terminator
    };
    CxTFlow* tf = build_tflow(alloc, nthreads, flow);
    run_tflow(tf, ncycles, __func__);
    cx_tflow_del(tf);
}

void test_tflow3(const CxAllocator* alloc, size_t nthreads, size_t ncycles) {

    TaskDesc flow[] = {
        { .name = "t0", .us = 1000, .out = 1},
        { .name = "t1", .us = 500, .deps= {"t0", NULL}},               // out=1
        { .name = "t2", .us = 800, .deps= {"t0", NULL}},               // out=1
        { .name = "t3", .us = 600, .deps= {"t0", NULL}},               // out=1
        { .name = "t4", .us = 100, .deps= {"t1", "t2", "t3", NULL }},  // out=3
        { .name = "t5", .us = 100, .deps= {"t1", "t2", NULL }},        // out=2
        {}, // terminator
    };
    CxTFlow* tf = build_tflow(alloc, nthreads, flow);
    run_tflow(tf, ncycles, __func__);
    CXCHK(flow[4].out == 3 * ncycles);
    CXCHK(flow[5].out == 2 * ncycles);
    cx_tflow_del(tf);
}

void test_tflow(void) {

    //test_tflow1(NULL, 2, 3);
    //test_tflow2(NULL, 2, 3);
    test_tflow3(NULL, 4, 5);

}

__attribute__((constructor)) static void reg_task(void) {

    reg_add_test("tflow", test_tflow);
}

