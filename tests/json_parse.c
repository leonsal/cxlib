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

    {
        const char* json = "null";
        CxVar res;
        CHK(cx_json_parse(json, strlen(json), &res, alloc) == 0);
        CHK(cx_var_get_type(&res) == CxVarNull);
        cx_var_del(&res);
    }

    {
        const char* json = "false";
        CxVar res;
        CHK(cx_json_parse(json, strlen(json), &res, alloc) == 0);
        bool v;
        CHK(cx_var_get_bool(&res, &v) == 0 && !v);
        cx_var_del(&res);
    }

    {
        const char* json = "true";
        CxVar res;
        CHK(cx_json_parse(json, strlen(json), &res, alloc) == 0);
        bool v;
        CHK(cx_var_get_bool(&res, &v) == 0 && v);
        cx_var_del(&res);
    }

    {
        const char* json = "-10";
        CxVar res;
        CHK(cx_json_parse(json, strlen(json), &res, alloc) == 0);
        int64_t v;
        CHK(cx_var_get_int(&res, &v) == 0 && v == -10);
        cx_var_del(&res);
    }

    {
        const char* json = "-0.45";
        CxVar res;
        CHK(cx_json_parse(json, strlen(json), &res, alloc) == 0);
        double  v;
        CHK(cx_var_get_float(&res, &v) == 0 && v == -0.45);
        cx_var_del(&res);
    }

    {
        const char* json = "\"string\"";
        CxVar res;
        CHK(cx_json_parse(json, strlen(json), &res, alloc) == 0);
        const char* v;
        CHK(cx_var_get_str(&res, &v) == 0 && (strcmp(v, "string") == 0));
        cx_var_del(&res);
    }

    {
        const char* json = "\"_\\\"_\\\\_\\b_\\f_\\n_\\r_\\t_\"";
        CxVar res;
        CHK(cx_json_parse(json, strlen(json), &res, alloc) == 0);
        const char* v;
        CHK(cx_var_get_str(&res, &v) == 0 && (strcmp(v, "_\"_\\_\b_\f_\n_\r_\t_") == 0));
        cx_var_del(&res);
    }

    {
        const char* json = "\"_\\u00A2_\\u0107_\"";
        CxVar res;
        CHK(cx_json_parse(json, strlen(json), &res, alloc) == 0);
        const char* v;
        CHK(cx_var_get_str(&res, &v) == 0 && (strcmp(v, "_\u00A2_\u0107_") == 0));
        cx_var_del(&res);
    }

    {
        const char* json = "[null, false, true, 42000, -0.1, \"strel\"]";
        CxVar res;
        CHK(cx_json_parse(json, strlen(json), &res, alloc) == 0);

        size_t len;
        CHK(cx_var_get_arr_len(&res, &len) == 0);
        CHK(len == 6);

        CxVar el;
        CHK(cx_var_get_arr_val(&res, 0, &el) == 0);
        CHK(cx_var_get_type(&el) == CxVarNull);

        bool vbool;
        CHK(cx_var_get_arr_val(&res, 1, &el) == 0);
        CHK(cx_var_get_bool(&el, &vbool) == 0 && !vbool);

        CHK(cx_var_get_arr_val(&res, 2, &el) == 0);
        CHK(cx_var_get_bool(&el, &vbool) == 0 && vbool);

        int64_t vint;
        CHK(cx_var_get_arr_val(&res, 3, &el) == 0);
        CHK(cx_var_get_int(&el, &vint) == 0 && vint == 42000l);

        double vfloat;
        CHK(cx_var_get_arr_val(&res, 4, &el) == 0);
        CHK(cx_var_get_float(&el, &vfloat) == 0 && vfloat == -0.1);

        const char* vstr;
        CHK(cx_var_get_arr_val(&res, 5, &el) == 0);
        char buf[32] = "strel";
        CHK(cx_var_get_str(&el, &vstr) == 0 && strcmp(vstr, buf) == 0);

        cx_var_del(&res);
    }

    {
        const char* json = "{\"k1\":null, \"k2\":false, \"k3\": -124, \"k4\": -0.8, \"k5\": \"strel\" }";
        CxVar res;
        CHK(cx_json_parse(json, strlen(json), &res, alloc) == 0);

        size_t count;
        CHK(cx_var_get_map_count(&res, &count) == 0);
        CHK(count == 5);

        CxVar el;
        CHK(cx_var_get_map_val(&res, "k1", &el) == 0);
        CHK(cx_var_get_type(&el) == CxVarNull);

        bool vbool;
        CHK(cx_var_get_map_val(&res, "k2", &el) == 0);
        CHK(cx_var_get_bool(&el, &vbool) == 0 && !vbool);

        int64_t vint;
        CHK(cx_var_get_map_val(&res, "k3", &el) == 0);
        CHK(cx_var_get_int(&el, &vint) == 0 && vint == -124);

        double vfloat;
        CHK(cx_var_get_map_val(&res, "k4", &el) == 0);
        CHK(cx_var_get_float(&el, &vfloat) == 0 && vfloat == -0.8);

        const char* vstr;
        CHK(cx_var_get_map_val(&res, "k5", &el) == 0);
        CHK(cx_var_get_str(&el, &vstr) == 0 && strcmp(vstr, "strel") == 0);
    }
}


