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
        CxVar var;
        CHK(cx_json_parse(json, strlen(json), &var, alloc) == 0);
        CHK(cx_var_get_type(&var) == CxVarNull);
        cx_var_del(&var);
    }

    {
        const char* json = "false";
        CxVar var;
        CHK(cx_json_parse(json, strlen(json), &var, alloc) == 0);
        bool v;
        CHK(cx_var_get_bool(&var, &v) == 0 && !v);
        cx_var_del(&var);
    }

    {
        const char* json = "true";
        CxVar var;
        CHK(cx_json_parse(json, strlen(json), &var, alloc) == 0);
        CHK(cx_var_get_bool(&var, &vbool) == 0 && vbool);
        cx_var_del(&var);
    }

    {
        const char* json = "-10";
        CxVar var;
        CHK(cx_json_parse(json, strlen(json), &var, alloc) == 0);
        CHK(cx_var_get_int(&var, &vint) == 0 && vint == -10);
        cx_var_del(&var);
    }

    {
        const char* json = "-0.45";
        CxVar var;
        CHK(cx_json_parse(json, strlen(json), &var, alloc) == 0);
        CHK(cx_var_get_float(&var, &vfloat) == 0 && vfloat == -0.45);
        cx_var_del(&var);
    }

    {
        const char* json = "\"string\"";
        CxVar var;
        CHK(cx_json_parse(json, strlen(json), &var, alloc) == 0);
        CHK(cx_var_get_str(&var, &vstr) == 0 && (strcmp(vstr, "string") == 0));
        cx_var_del(&var);
    }

    {
        const char* json = "\"_\\\"_\\\\_\\b_\\f_\\n_\\r_\\t_\"";
        CxVar var;
        CHK(cx_json_parse(json, strlen(json), &var, alloc) == 0);
        CHK(cx_var_get_str(&var, &vstr) == 0 && (strcmp(vstr, "_\"_\\_\b_\f_\n_\r_\t_") == 0));
        cx_var_del(&var);
    }

    {
        const char* json = "\"_\\u00A2_\\u0107_\"";
        CxVar var;
        CHK(cx_json_parse(json, strlen(json), &var, alloc) == 0);
        CHK(cx_var_get_str(&var, &vstr) == 0 && (strcmp(vstr, "_\u00A2_\u0107_") == 0));
        cx_var_del(&var);
    }

    {
        const char* json = "[null, false, true, 42000, -0.1, \"strel\"]";
        CxVar arr;
        CHK(cx_json_parse(json, strlen(json), &arr, alloc) == 0);

        CHK(cx_var_get_arr_len(&arr, &len) == 0 && len == 6);
        CHK(cx_var_get_arr_null(&arr, 0) == 0);
        CHK(cx_var_get_arr_bool(&arr, 1, &vbool) == 0 && !vbool);
        CHK(cx_var_get_arr_bool(&arr, 2, &vbool) == 0 && vbool);
        CHK(cx_var_get_arr_int(&arr, 3, &vint) == 0 && vint == 42000);
        CHK(cx_var_get_arr_float(&arr, 4, &vfloat) == 0 && vfloat == -0.1);
        CHK(cx_var_get_arr_str(&arr, 5, &vstr) == 0 && strcmp(vstr, "strel") == 0);
        cx_var_del(&arr);
    }

    {
        const char* json = "{\"k1\":null, \"k2\":false, \"k3\": -124, \"k4\": -0.8, \"k5\": \"strel\" }";
        CxVar map;
        CHK(cx_json_parse(json, strlen(json), &map, alloc) == 0);

        CHK(cx_var_get_map_count(&map, &len) == 0 && len == 5);
        CHK(cx_var_get_map_null(&map, "k1") == 0);
        CHK(cx_var_get_map_bool(&map, "k2", &vbool) == 0 && !vbool);
        CHK(cx_var_get_map_int(&map, "k3", &vint) == 0 && vint == -124);
        CHK(cx_var_get_map_float(&map, "k4", &vfloat) == 0 && vfloat == -0.8);
        CHK(cx_var_get_map_str(&map, "k5", &vstr) == 0 && strcmp(vstr, "strel") == 0);
        cx_var_del(&map);
    }


    {
        const char* json = "[\"1\", {\"1\":1, \"2\":[2], \"3\": 3}, null]";
        CxVar arr;
        CHK(cx_json_parse(json, strlen(json), &arr, alloc) == 0);

        CHK(cx_var_get_arr_len(&arr, &len) == 0 && len == 3);
        CHK(cx_var_get_arr_str(&arr, 0, &vstr) == 0 && strcmp(vstr, "1") == 0);
        CxVar map;
        CHK(cx_var_get_arr_map(&arr, 1, &map) == 0);
        {
            CHK(cx_var_get_map_count(&map, &len) == 0 && len == 3);
            CHK(cx_var_get_map_int(&map, "1", &vint) == 0);
            CxVar arr2;
            CHK(cx_var_get_map_arr(&map, "2", &arr2) == 0);
            {
                CHK(cx_var_get_arr_len(&arr2, &len) == 0 && len == 1);
                CHK(cx_var_get_arr_int(&arr2, 0, &vint) == 0 && vint == 2);
            }
            CHK(cx_var_get_map_int(&map, "3", &vint) == 0 && vint == 3);
        }
        CHK(cx_var_get_arr_null(&arr, 2) == 0);
    }
}


