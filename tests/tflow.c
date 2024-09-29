#include <unistd.h>
#include "cx_tflow.h"
#include "logger.h"
#include "registry.h"

// Task arguments
typedef struct TaskArgs {
    const char* name;              // Task name
    size_t      us;                // Microseconds to sleep
} TaskArgs;

// Task descriptor used to build the Task Flows
typedef struct TaskDesc {
    const char*     name;          // Task name
    TaskArgs        args;          // Task arguments
    const char*     deps[32];      // Array of names of dependant tasks (NULL terminated)
    CxTFlowTask*    task;          // Pointer to created task
} TaskDesc;

static void task_func(void* arg) {

    TaskArgs* args = arg;
    //printf("%s: %s\n", __func__, args->name);
    usleep(args->us);
}

// Builds TaskFlow from array of task descriptors
static CxTFlow* build_tflow(const CxAllocator* alloc, size_t nthreads, TaskDesc* desc) {

    // Creates Task Flow
    CxTracer* tr = cx_tracer_new(alloc, 16*1024);
    CxTFlow* tf = cx_tflow_new(alloc, nthreads, tr);

    // Adds all tasks from descriptor array
    for (size_t i = 0; desc[i].name != NULL; i++) {
        CxTFlowTask* ptask;
        desc[i].args.name = desc[i].name;
        CXERR_CHK(cx_tflow_add_task(tf, desc[i].name, task_func, &desc[i].args, &desc[i].task));
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
    struct timespec timeout = {.tv_sec = 100};

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
        { .name = "t1", .args = {.us = 1000}},
        {}, // terminator
    };
    CxTFlow* tf = build_tflow(alloc, nthreads, flow);
    run_tflow(tf, 5, __func__);
    cx_tflow_del(tf);
}

// Some source/sink tasks
void test_tflow2(const CxAllocator* alloc, size_t nthreads, size_t ncycles) {

    TaskDesc flow[] = {
        { .name = "t1", .args = {.us = 1000}, },
        { .name = "t2", .args = {.us = 500}, },
        { .name = "t3", .args = {.us = 200}, },
        {}, // terminator
    };
    CxTFlow* tf = build_tflow(alloc, nthreads, flow);
    run_tflow(tf, ncycles, __func__);
    cx_tflow_del(tf);
}

void test_tflow3(const CxAllocator* alloc, size_t nthreads, size_t ncycles) {

    TaskDesc flow[] = {
        { .name = "t1", .args = {.us = 1000}, },
        { .name = "t2", .args = {.us = 500}, .deps= {"t1", NULL}},
        { .name = "t3", .args = {.us = 800}, .deps= {"t1", NULL}},
        {}, // terminator
    };
    CxTFlow* tf = build_tflow(alloc, nthreads, flow);
    run_tflow(tf, ncycles, __func__);
    cx_tflow_del(tf);
}

void test_tflow(void) {

    //test_tflow1(NULL, 2, 3);
    //test_tflow2(NULL, 2, 3);
    test_tflow3(NULL, 4, 2);

}

__attribute__((constructor)) static void reg_task(void) {

    reg_add_test("tflow", test_tflow);
}

