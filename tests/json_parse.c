#include <stdio.h>
#include <time.h>

#include "cx_alloc.h"
#include "cx_pool_allocator.h"
#include "util.h"
#include "logger.h"
#include "cx_json_parse.h"
#include "json_parse.h"



void json_parse_tests(void) {

    // Use default 'malloc/free' allocator
    json_parse_test(cxDefaultAllocator());

    // // Use pool allocator
    // CxPoolAllocator* pa = cx_pool_allocator_create(4*1024, NULL);
    // json_build_test(cx_pool_allocator_iface(pa));
    // cx_pool_allocator_destroy(pa);
}

void json_parse_test(const CxAllocator* alloc) {

    //const char* test1 = "[null,true]";
    const char* test1 = "{\"k\":null}";
    CxVar json;
    cx_json_parse(test1, strlen(test1), &json, alloc);
}


