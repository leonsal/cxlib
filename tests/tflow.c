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
    size_t inps = cx_tflow_task_inps(desc->task);
    size_t outs = cx_tflow_task_outs(desc->task);

    // For each input task, get its output value and sums to the output of this task.
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
    //printf("%s: name:%s out:%zu\n", __func__, desc->name, desc->out);
    usleep(desc->us);
}

// Builds TaskFlow from array of task descriptors
static CxTFlow* build_tflow(const CxAllocator* alloc, size_t nthreads, TaskDesc* desc) {

    // Creates Task Flow
    CxTracer* tr = cx_tracer_new(alloc, 128*1024);
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

// Runs Task Flow for the specifieed number of cycles and then generates event trace file
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
    if (tracer) {
        cx_tracer_del(tracer);
    }
    cx_tflow_del(tf);
}

// 1 source only
static void test_tflow1(const CxAllocator* alloc, size_t nthreads, size_t ncycles) {

    TaskDesc flow[] = {
        { .name = "t1", .us = 1000},
        {}, // terminator
    };
    CxTFlow* tf = build_tflow(alloc, nthreads, flow);
    run_tflow(tf, ncycles, __func__);
}

// 3 sources, 1 sink
// t0 ----> t3
// t1 -----/
// t2 -----/
static void test_tflow2(const CxAllocator* alloc, size_t nthreads, size_t ncycles) {

    TaskDesc flow[] = {
        { .name = "t0", .us = 100 },
        { .name = "t1", .us = 200 },
        { .name = "t2", .us = 250 },
        { .name = "t3", .us = 200, .deps = {"t0", "t1", "t2", NULL }},
        {}, // terminator
    };
    CxTFlow* tf = build_tflow(alloc, nthreads, flow);
    run_tflow(tf, ncycles, __func__);
}

// 1 source, 3 sinks
// t0 -----> t1
//  \------> t2
//  \------> t3
static void test_tflow3(const CxAllocator* alloc, size_t nthreads, size_t ncycles) {

    TaskDesc flow[] = {
        { .name = "t0", .us = 1000, .out= 1 },
        { .name = "t1", .us = 500, .deps = {"t0", NULL}},
        { .name = "t2", .us = 500, .deps = {"t0", NULL}},
        { .name = "t3", .us = 500, .deps = {"t0", NULL}},
        {}, // terminator
    };
    CxTFlow* tf = build_tflow(alloc, nthreads, flow);
    run_tflow(tf, ncycles, __func__);
}

// t0 ----> t1, t2, t3 ---> t4
//          t1, t2 ----> t5
static void test_tflow4(const CxAllocator* alloc, size_t nthreads, size_t ncycles) {

    TaskDesc flow[] = {
        { .name = "t0", .us = 100, .out = 1},
        { .name = "t1", .us = 200, .deps= {"t0", NULL}},               // out=1
        { .name = "t2", .us = 100, .deps= {"t0", NULL}},               // out=1
        { .name = "t3", .us = 300, .deps= {"t0", NULL}},               // out=1
        { .name = "t4", .us = 100, .deps= {"t1", "t2", "t3", NULL }},  // out=3
        { .name = "t5", .us = 100, .deps= {"t1", "t2", NULL }},        // out=2
        {}, // terminator
    };
    CxTFlow* tf = build_tflow(alloc, nthreads, flow);
    run_tflow(tf, ncycles, __func__);
    CXCHK(flow[4].out == 3 * ncycles);
    CXCHK(flow[5].out == 2 * ncycles);
}

// t0, t1, t2 ---> t3
// t3 ---> t4
// t5 ---> t3
// t6 ---> t3
// t5, t6 ---> t7
// t7 ---> t8
static void test_tflow5(const CxAllocator* alloc, size_t nthreads, size_t ncycles) {

    TaskDesc flow[] = {
        { .name = "t0", .us = 100, .out = 1},
        { .name = "t1", .us = 100, .out = 1},
        { .name = "t2", .us = 100, .out = 1},
        { .name = "t3", .us = 100, .deps = {"t0", "t1", "t2", NULL }},
        { .name = "t4", .us = 100, .deps = {"t3", NULL}},
        { .name = "t5", .us = 100, .deps = {"t3", NULL}},
        { .name = "t6", .us = 100, .deps = {"t3", NULL}},
        { .name = "t7", .us = 100, .deps = {"t5", "t6", NULL}},
        { .name = "t8", .us = 100, .deps = {"t7", NULL}},
        {}, // terminator
    };
    CxTFlow* tf = build_tflow(alloc, nthreads, flow);
    run_tflow(tf, ncycles, __func__);
    CXCHK(flow[4].out == 3 * ncycles);
    CXCHK(flow[8].out == 6 * ncycles);
}

static void test_tflow6(const CxAllocator* alloc, size_t nthreads, size_t ncycles) {

    TaskDesc flow[] = {
        { .name = "t0", .us = 100, .out = 1},
        { .name = "t1", .us = 100, .deps = {"t0", NULL }},
        { .name = "t2", .us = 100, .deps = {"t1", NULL }},
        { .name = "t3", .us = 100, .deps = {"t1", NULL }},
        { .name = "t4", .us = 100, .deps = {"t2", NULL }},
        { .name = "t5", .us = 100, .deps = {"t2", NULL }},
        { .name = "t6", .us = 100, .deps = {"t3", NULL }},
        { .name = "t7", .us = 100, .deps = {"t3", NULL }},
        { .name = "t8", .us = 100, .deps = {"t4", "t5", NULL }},
        { .name = "t9", .us = 100, .deps = {"t6", "t7", NULL }},
        { .name = "t10",.us = 100, .out = 1},
        { .name = "t11", .us = 100, .deps = {"t8", "t9", "t10", NULL }},
        { .name = "t12", .us = 100, .deps = {"t8", NULL }},
        {}, // terminator
    };
    CxTFlow* tf = build_tflow(alloc, nthreads, flow);
    run_tflow(tf, ncycles, __func__);
    // CXCHK(flow[4].out == 3 * ncycles);
    // CXCHK(flow[8].out == 6 * ncycles);
}

void test_tflow(void) {

    LOGI("%s: ", __func__);
    test_tflow1(NULL, 8, 20);
    test_tflow2(NULL, 8, 10);
    test_tflow3(NULL, 8, 20);
    test_tflow4(NULL, 8, 20);
    test_tflow5(NULL, 8, 20);
    test_tflow6(NULL, 8, 20);

}

__attribute__((constructor)) static void reg_task(void) {

    reg_add_test("tflow", test_tflow);
}

