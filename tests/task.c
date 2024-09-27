#include "cx_task.h"
#include "logger.h"
#include "registry.h"


static void task1(void* arg) {


}

void test_task1(const CxAllocator* alloc, size_t nthreads) {

    LOGI("%s: alloc:%p nthreads=%zu", __func__, alloc, nthreads);
    CxTFlow* tr = cx_tflow_new(alloc, nthreads);

    cx_tflow_del(tr);
}

void test_task(void) {

    test_task1(NULL, 2);

}

__attribute__((constructor))
static void reg_task(void) {

    reg_add_test("task", test_task);
}

