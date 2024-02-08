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

    CxVar* var = cx_var_new(alloc);

    cx_var_del(var);



}

