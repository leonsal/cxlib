
#include "cx_task.h"



void task_test(const CxAllocator* alloc, size_t nthreads) {

    CxTaskRunner* tr = cx_task_runner_new(alloc, nthreads);

    cx_task_runner_del(tr);
}

void task_tests(void) {

    task_test(NULL, 2);

}

