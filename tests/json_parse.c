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

    // Use pool allocator
    CxPoolAllocator* pa = cx_pool_allocator_create(4*1024, NULL);
    json_parse_test(cx_pool_allocator_iface(pa));
    cx_pool_allocator_destroy(pa);
}

void json_parse_test(const CxAllocator* alloc) {

    LOGI("json parse alloc:%p", alloc);
    size_t      len;
    bool        vbool;
    int64_t     vint;
    double      vfloat;
    const char* vstr;

    {
        const char* json = "null";
        CxVar* var = cx_var_new(alloc);
        CHK(cx_json_parse(json, strlen(json), &var, alloc) == 0);
        CHK(cx_var_get_type(var) == CxVarNull);
        cx_var_del(var);
    }

    {
        const char* json = "false";
        CxVar* var = cx_var_new(alloc);
        CHK(cx_json_parse(json, strlen(json), &var, alloc) == 0);
        bool v;
        CHK(cx_var_get_bool(var, &v) && !v);
        cx_var_del(var);
    }

    {
        const char* json = "true";
        CxVar* var = cx_var_new(alloc);
        CHK(cx_json_parse(json, strlen(json), &var, alloc) == 0);
        CHK(cx_var_get_bool(var, &vbool) && vbool);
        cx_var_del(var);
    }

    {
        const char* json = "-10";
        CxVar* var = cx_var_new(alloc);
        CHK(cx_json_parse(json, strlen(json), &var, alloc) == 0);
        CHK(cx_var_get_int(var, &vint) && vint == -10);
        cx_var_del(var);
    }

    {
        const char* json = "-0.45";
        CxVar* var = cx_var_new(alloc);
        CHK(cx_json_parse(json, strlen(json), &var, alloc) == 0);
        CHK(cx_var_get_float(var, &vfloat) && vfloat == -0.45);
        cx_var_del(var);
    }

    {
        const char* json = "\"string\"";
        CxVar* var = cx_var_new(alloc);
        CHK(cx_json_parse(json, strlen(json), &var, alloc) == 0);
        CHK(cx_var_get_str(var, &vstr) && (strcmp(vstr, "string") == 0));
        cx_var_del(var);
    }

    {
        const char* json = "\"_\\\"_\\\\_\\b_\\f_\\n_\\r_\\t_\"";
        CxVar* var = cx_var_new(alloc);
        CHK(cx_json_parse(json, strlen(json), &var, alloc) == 0);
        CHK(cx_var_get_str(var, &vstr) && (strcmp(vstr, "_\"_\\_\b_\f_\n_\r_\t_") == 0));
        cx_var_del(var);
    }

    {
        const char* json = "\"_\\u00A2_\\u0107_\"";
        CxVar* var = cx_var_new(alloc);
        CHK(cx_json_parse(json, strlen(json), &var, alloc) == 0);
        CHK(cx_var_get_str(var, &vstr) && (strcmp(vstr, "_\u00A2_\u0107_") == 0));
        cx_var_del(var);
    }

    {
        const char* json = "[null, false, true, 42000, -0.1, \"strel\"]";
        CxVar* arr = cx_var_new(alloc);
        CHK(cx_json_parse(json, strlen(json), &arr, alloc) == 0);

        CHK(cx_var_get_arr_len(arr, &len) && len == 6);
        CHK(cx_var_get_arr_null(arr, 0));
        CHK(cx_var_get_arr_bool(arr, 1, &vbool) && !vbool);
        CHK(cx_var_get_arr_bool(arr, 2, &vbool) && vbool);
        CHK(cx_var_get_arr_int(arr, 3, &vint) && vint == 42000);
        CHK(cx_var_get_arr_float(arr, 4, &vfloat) && vfloat == -0.1);
        CHK(cx_var_get_arr_str(arr, 5, &vstr) && strcmp(vstr, "strel") == 0);
        cx_var_del(arr);
    }

    {
        const char* json = "{\"k1\":null, \"k2\":false, \"k3\": -124, \"k4\": -0.8, \"k5\": \"strel\", \"k6\": {\"k61\": \"params\"}}";
        CxVar* map = cx_var_new(alloc);
        CHK(cx_json_parse(json, strlen(json), &map, alloc) == 0);

        CHK(cx_var_get_map_len(map, &len) && len == 6);
        CHK(cx_var_get_map_null(map, "k1"));
        CHK(cx_var_get_map_bool(map, "k2", &vbool) && !vbool);
        CHK(cx_var_get_map_int(map, "k3", &vint) && vint == -124);
        CHK(cx_var_get_map_float(map, "k4", &vfloat) && vfloat == -0.8);
        CHK(cx_var_get_map_str(map, "k5", &vstr) && strcmp(vstr, "strel") == 0);
        CxVar* map2 = cx_var_get_map_map(map, "k6");
        CHK(cx_var_get_map_str(map2, "k61", &vstr));
        cx_var_del(map);
    }

    {
        const char* json = "[\"1\", {\"k1\":1, \"k2\":[2], \"k3\": 3}, null]";
        CxVar* arr = cx_var_new(alloc);
        CHK(cx_json_parse(json, strlen(json), &arr, alloc) == 0);

        CHK(cx_var_get_arr_len(arr, &len) && len == 3);
        CHK(cx_var_get_arr_str(arr, 0, &vstr) && strcmp(vstr, "1") == 0);
        CxVar* map = cx_var_get_arr_map(arr, 1);
        {
            CHK(cx_var_get_map_len(map, &len) && len == 3);
            CHK(cx_var_get_map_int(map, "k1", &vint) && vint == 1);
            CxVar* arr2 = cx_var_get_map_arr(map, "k2");
            {
                CHK(cx_var_get_arr_len(arr2, &len) && len == 1);
                CHK(cx_var_get_arr_int(arr2, 0, &vint) && vint == 2);
            }
            CHK(cx_var_get_map_int(map, "k3", &vint) && vint == 3);
        }
        CHK(cx_var_get_arr_null(arr, 2));
        cx_var_del(arr);
    }
}


