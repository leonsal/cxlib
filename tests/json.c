#include <stdio.h>
#include <time.h>

#include "cx_alloc.h"
#include "cx_pool_allocator.h"
#include "util.h"
#include "cx_json.h"
#include "logger.h"
#include "json.h"

#define cx_str_name cxstr
#define cx_str_instance_allocator
#define cx_str_static
#define cx_str_implement
#include "cx_str.h"


static int cxstring_write(cxstr* str, void* data, size_t len);

void json_tests(void) {

    // Use default 'malloc/free' allocator
    json_test(cxDefaultAllocator());

    // Use pool allocator
    CxPoolAllocator* pa = cx_pool_allocator_create(4*1024, NULL);
    json_test(cx_pool_allocator_iface(pa));
    cx_pool_allocator_destroy(pa);
}

void json_test(const CxAllocator* alloc) {

    CxVar vmap = cx_var_new_map(alloc);
    cx_var_map_set(&vmap, "null", cx_var_new_null());
    cx_var_map_set(&vmap, "bool", cx_var_new_bool(true));
    cx_var_map_set(&vmap, "int", cx_var_new_int(10));
    cx_var_map_set(&vmap, "float", cx_var_new_float(3.1415));
    cx_var_map_set(&vmap, "str", cx_var_new_str("string text", alloc));

    // Creates CxWriter using a string to store the json build output 
    cxstr out = cxstr_init(alloc);
    CxWriter writer = {.ctx = &out, .write = (CxWriterWrite)cxstring_write};
    CHK(cx_json_build(&vmap, NULL, &writer) == 0);
}

static int cxstring_write(cxstr* str, void* data, size_t len) {

    cxstr_catn(str, data, len);
    return len;
}
