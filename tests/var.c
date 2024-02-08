#include <stdio.h>
#include <time.h>

#include "cx_alloc.h"
#include "cx_pool_allocator.h"
#include "cx_var.h"

#include "util.h"
#include "logger.h"
#include "var.h"


void cx_var_tests(void) {

    // Use default 'malloc/free' allocator
    cx_var_test(cxDefaultAllocator());

    // // Use pool allocator
    // CxPoolAllocator* pa = cx_pool_allocator_create(4*1024, NULL);
    // cx_var_test(cx_pool_allocator_iface(pa));
    // cx_pool_allocator_destroy(pa);
}

void cx_var_test(const CxAllocator* alloc) {

    LOGI("var alloc=%p", alloc);
    CxVar* var = cx_var_new(alloc);
    cx_var_set_null(var);
    cx_var_set_bool(var, true);
    cx_var_set_bool(var, false);
    cx_var_set_int(var, 42);
    cx_var_set_float(var, -0.1);
    cx_var_set_str(var, "string");
    cx_var_set_arr(var);
    cx_var_set_map(var);
    cx_var_set_buf(var, (uint8_t[]){0,1,2,3}, 4);
    cx_var_del(var);



}

