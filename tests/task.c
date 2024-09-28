#include <unistd.h>
#include "cx_task.h"
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
    printf("%s: %s\n", __func__, args->name);
    usleep(args->us);
}

// Builds TaskFlow from array of task descriptors
static CxTFlow* build_tflow(const CxAllocator* alloc, size_t nthreads, TaskDesc* desc) {

    // Creates Task Flow
    CxTFlow* tf = cx_tflow_new(alloc, nthreads);

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

void test_task1(const CxAllocator* alloc, size_t nthreads) {

    TaskDesc flow[] = {
        {
            .name = "t1",
            .args = {.us = 1000},
        },
        {}, // terminator
    };
    CxTFlow* tf = build_tflow(alloc, nthreads, flow);

    const size_t ncycles = 10;
    CXERR_CHK(cx_tflow_start(tf, ncycles));
    struct timespec timeout = {.tv_sec = 10};

    CXERR_CHK(cx_tflow_wait(tf, timeout));
    CxTFlowStatus status = cx_tflow_status(tf);

    cx_tflow_del(tf);
}

void test_task2(const CxAllocator* alloc, size_t nthreads) {

    TaskDesc flow[] = {
        {
            .name = "t1",
            .args = {.us = 1000},
        },
        {
            .name = "t2",
            .args = {.us = 1000},
        },
        {}, // terminator
    };
    CxTFlow* tf = build_tflow(alloc, nthreads, flow);

    const size_t ncycles = 5;
    CXERR_CHK(cx_tflow_start(tf, ncycles));

    struct timespec timeout = {.tv_sec = 10};
    CXERR_CHK(cx_tflow_wait(tf, timeout));
    CxTFlowStatus status = cx_tflow_status(tf);

    cx_tflow_del(tf);
}

void test_task(void) {

    //test_task1(NULL, 2);
    test_task2(NULL, 2);

}

__attribute__((constructor)) static void reg_task(void) {

    reg_add_test("task", test_task);
}

