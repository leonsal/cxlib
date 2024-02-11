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


static void json_replacer(CxVar* var, void* userdata);
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

    // Enclosing map
    CxVar* vmap = cx_var_set_map(cx_var_new(alloc));
    CHK(cx_var_set_map_null(vmap, "null"));
    CHK(cx_var_set_map_bool(vmap, "bool", true));
    CHK(cx_var_set_map_bool(vmap, "bool", false));
    CHK(cx_var_set_map_int(vmap, "int", 10));
    CHK(cx_var_set_map_float(vmap, "float", 3.1415));
    CHK(cx_var_set_map_str(vmap, "str", "string text"));

    // Array element
    CxVar* varr = cx_var_set_map_arr(vmap, "arr");
    cx_var_push_arr_null(varr);
    cx_var_push_arr_bool(varr, false);
    cx_var_push_arr_int(varr, 1);
    cx_var_push_arr_float(varr, -34.33);
    cx_var_push_arr_str(varr, "string \"el\"");

    // Map element
    CxVar* vmap2 = cx_var_set_map_map(vmap, "map");
    cx_var_set_map_null(vmap2, "null");
    cx_var_set_map_bool(vmap2, "bool", false);
    cx_var_set_map_int(vmap2, "int", -42);
    cx_var_set_map_float(vmap2, "float", 0.1);
    cx_var_set_map_str(vmap2, "str", "mapel");

    // Creates CxWriter using a string to store the json build output 
    cxstr out = cxstr_init(alloc);
    CxWriter writer = {.ctx = &out, .write = (CxWriterWrite)cxstring_write};

    // Builds JSON 
    CxJsonBuildCfg cfg = {.replacer_fn = json_replacer};
    CHK(cx_json_build(vmap, &cfg, &writer) == 0);
    printf("json:%s\n", out.data);
    cxstr_free(&out);
    cx_var_del(vmap);
}

static int cxstring_write(cxstr* str, void* data, size_t len) {

    cxstr_catn(str, data, len);
    return len;
}

static void json_replacer(CxVar* var, void* userdata) {
    // size_t type = cx_var_get_type(var);
    // printf("type:%zu\n", type);
    // if (type == CxVarStr) {
    //     cx_var_set_str(var, "LIXO");
    // }
}

