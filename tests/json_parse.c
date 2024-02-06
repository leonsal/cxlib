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
        const char* json = "\"str\"";
        CxVar res;
        CHK(cx_json_parse(json, strlen(json), &res, alloc) == 0);
        const char* v;
        CHK(cx_var_get_str(&res, &v) == 0 && (strcmp(v, "str") == 0));
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
        const char* json = "\"_\\u1234__\"";
        CxVar res;
        CHK(cx_json_parse(json, strlen(json), &res, alloc) == 0);
        const char* v;
        CHK(cx_var_get_str(&res, &v) == 0 && (strcmp(v, "__") == 0));
        cx_var_del(&res);
    }
}


