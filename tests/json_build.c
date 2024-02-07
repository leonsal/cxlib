#include <stdio.h>
#include <time.h>

#include "cx_alloc.h"
#include "cx_pool_allocator.h"
#include "util.h"
#include "logger.h"
#include "cx_json_build.h"
#include "json_build.h"

#define cx_str_name cxstr
#define cx_str_instance_allocator
#define cx_str_static
#define cx_str_implement
#include "cx_str.h"


static int cxstring_write(cxstr* str, void* data, size_t len);

void json_build_tests(void) {

    // Use default 'malloc/free' allocator
    json_build_test(cxDefaultAllocator());

    // Use pool allocator
    CxPoolAllocator* pa = cx_pool_allocator_create(4*1024, NULL);
    json_build_test(cx_pool_allocator_iface(pa));
    cx_pool_allocator_destroy(pa);
}

void json_build_test(const CxAllocator* alloc) {

    LOGI("json_build alloc:%p", alloc);
    CxVar vmap = cx_var_new_map(alloc);
    CHKZ(cx_var_set_map_null(&vmap, "null"));
    CHKZ(cx_var_set_map_bool(&vmap, "bool", true));
    CHKZ(cx_var_set_map_bool(&vmap, "bool", false));
    CHKZ(cx_var_set_map_int(&vmap, "int", 10));
    CHKZ(cx_var_set_map_float(&vmap, "float", 3.1415));
    CHKZ(cx_var_set_map_str(&vmap, "str", "string text"));
        CxVar varr = cx_var_new_arr(alloc);
        cx_var_push_arr_val(&varr, cx_var_new_null());
        cx_var_push_arr_val(&varr, cx_var_new_bool(false));
        cx_var_push_arr_val(&varr, cx_var_new_int(1));
        cx_var_push_arr_val(&varr, cx_var_new_float(-34.33));
        cx_var_push_arr_val(&varr, cx_var_new_str("string \"el\"", alloc));

            CxVar varr2 = cx_var_new_arr(alloc);
            cx_var_push_arr_val(&varr2, cx_var_new_null());
            cx_var_push_arr_val(&varr2, cx_var_new_int(123));
            cx_var_push_arr_val(&varr, varr2);

            CxVar vmap2 = cx_var_new_map(alloc);
            cx_var_set_map_val(&vmap2, "b1", cx_var_new_bool(true));
            cx_var_push_arr_val(&varr, vmap2);

    cx_var_set_map_val(&vmap, "arr", varr);

    // Creates CxWriter using a string to store the json build output 
    cxstr out = cxstr_init(alloc);
    CxWriter writer = {.ctx = &out, .write = (CxWriterWrite)cxstring_write};
    CHK(cx_json_build(&vmap, NULL, &writer) == 0);
    printf("json:%s\n", out.data);
    cxstr_free(&out);
    cx_var_del(&vmap);
}

static int cxstring_write(cxstr* str, void* data, size_t len) {

    cxstr_catn(str, data, len);
    return len;
}
