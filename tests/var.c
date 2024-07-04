#include <stdio.h>
#include <time.h>

#include "cx_alloc.h"
#include "cx_pool_allocator.h"
#include "cx_var.h"
#include "cx_writer.h"
#include "cx_json_build.h"

#include "util.h"
#include "logger.h"
#include "var.h"


void cx_var_tests(void) {

    // Use default 'malloc/free' allocator
    cx_var_test(cx_def_allocator());

    // Use pool allocator
    CxPoolAllocator* pa = cx_pool_allocator_create(4*1024, NULL);
    cx_var_test(cx_pool_allocator_iface(pa));
    cx_pool_allocator_destroy(pa);
}

void cx_var_test(const CxAllocator* alloc) {

    LOGI("var alloc=%p", alloc);
    // Primitives
    {
        CxVar* var = cx_var_new(alloc);

        cx_var_set_null(var);
        CHK(cx_var_get_null(var));

        bool vbool;
        cx_var_set_bool(var, true);
        CHK(cx_var_get_bool(var, &vbool) && vbool);

        cx_var_set_bool(var, false);
        CHK(cx_var_get_bool(var, &vbool) && !vbool);

        int64_t vint;
        cx_var_set_int(var, 42);
        CHK(cx_var_get_int(var, &vint) && vint == 42);

        double vfloat;
        cx_var_set_float(var, -0.1);
        CHK(cx_var_get_float(var, &vfloat) && vfloat == -0.1);

        const void* pbuf;
        size_t len;
        cx_var_set_buf(var, (uint8_t[]){0,1,2,3}, 4);
        CHK(cx_var_get_buf(var, &pbuf, &len) && len == 4);
        CHK(memcmp(pbuf, (uint8_t[]){0,1,2,3}, len) == 0);
        cx_var_del(var);
    }

    // String
    {
        CxVar* var = cx_var_new(alloc);

        const char* sets = NULL;
        const char* gets = NULL;

        sets = "";
        cx_var_set_str(var, sets);
        CHK(cx_var_get_str(var, &gets) && strcmp(gets, sets) == 0);

        sets = "123";
        cx_var_set_str(var, sets);
        CHK(cx_var_get_str(var, &gets) && strcmp(gets, sets) == 0);

        sets = "0123456789";
        cx_var_set_str(var, sets);
        CHK(cx_var_get_str(var, &gets) && strcmp(gets, sets) == 0);


        cx_var_del(var);
    }

    // Array
    {
        bool vbool;
        int64_t vint;
        double vfloat;
        const char* vstr;
        const void* vbuf;
        size_t len;
        CxVar* arr = cx_var_set_arr(cx_var_new(alloc));

        CHK(cx_var_get_type(arr) == CxVarArr);
        CHK(cx_var_get_arr_len(arr, &len) && len == 0);

        CHKN(cx_var_push_arr_null(arr));
        CHKN(cx_var_push_arr_bool(arr, true));
        CHKN(cx_var_push_arr_bool(arr, false));
        CHKN(cx_var_push_arr_int(arr, 42));
        CHKN(cx_var_push_arr_float(arr, -0.1));
        CHKN(cx_var_push_arr_str(arr, "strel"));
        // Array element
        {
            CxVar* arr_el = cx_var_push_arr_arr(arr);
            CHKN(arr_el);
            CHKN(cx_var_push_arr_null(arr_el));
            CHKN(cx_var_push_arr_bool(arr_el, false));
            CHKN(cx_var_push_arr_int(arr_el, 88));
            CHKN(cx_var_push_arr_float(arr_el, 0.4));
        }
        // Map element
        {
            CxVar* map_el = cx_var_push_arr_map(arr);
            CHKN(cx_var_set_map_null(map_el, "knull"));
            CHKN(cx_var_set_map_bool(map_el, "kbool", false));
            CHKN(cx_var_set_map_bool(map_el, "kbool", true));
            CHKN(cx_var_set_map_int(map_el, "kint", 10));
            CHKN(cx_var_set_map_int(map_el, "kint", 20));
            CHKN(cx_var_set_map_float(map_el, "kfloat", -0.1));
            CHKN(cx_var_set_map_float(map_el, "kfloat", -0.2));
            CHKN(cx_var_set_map_str(map_el, "kstr", "first"));
            CHKN(cx_var_set_map_str(map_el, "kstr", "second"));
        }
        CHKN(cx_var_push_arr_buf(arr, (uint8_t[]){0,1,2}, 3));

        // Checks

        CHK(cx_var_get_arr_len(arr, &len) && len == 9);
        CHK(cx_var_get_arr_null(arr, 0));
        CHK(cx_var_get_arr_bool(arr, 1, &vbool) && vbool == true);
        CHK(cx_var_get_arr_bool(arr, 2, &vbool) && vbool == false);
        CHK(cx_var_get_arr_int(arr, 3, &vint) && vint == 42);
        CHK(cx_var_get_arr_float(arr, 4, &vfloat) && vfloat == -0.1);
        CHK(cx_var_get_arr_str(arr, 5, &vstr) && strcmp(vstr, "strel") == 0);

        CxVar* arr_el = cx_var_get_arr_arr(arr, 6);
        CHK(cx_var_get_arr_len(arr_el, &len) && len == 4);
        CHK(cx_var_get_arr_null(arr_el, 0));
        CHK(cx_var_get_arr_bool(arr_el, 1, &vbool) && vbool == false);
        CHK(cx_var_get_arr_int(arr_el, 2, &vint) && vint == 88);
        CHK(cx_var_get_arr_float(arr_el, 3, &vfloat) && vfloat == 0.4);

        CxVar* map_el = cx_var_get_arr_map(arr, 7);
        CHK(cx_var_get_map_len(map_el, &len) && len == 5);
        CHK(cx_var_get_map_null(map_el, "knull"));
        CHK(cx_var_get_map_bool(map_el, "kbool", &vbool) && vbool == true);
        CHK(cx_var_get_map_int(map_el, "kint", &vint) && vint == 20);
        CHK(cx_var_get_map_float(map_el, "kfloat", &vfloat) && vfloat == -0.2);
        CHK(cx_var_get_map_str(map_el, "kstr", &vstr) && strcmp(vstr, "second") == 0);

        CHK(cx_var_get_arr_buf(arr, 8, &vbuf, &len) && len == 3 && memcmp(vbuf, (uint8_t[]){0,1,2}, 3) == 0);

        cx_var_del(arr);
    }

    // Copy map
    {
        // Builds source map
        CxVar* src = cx_var_new(alloc);
        cx_var_set_map(src);
        // Add primitive keys
        cx_var_set_map_null(src, "null");
        cx_var_set_map_bool(src, "bool", true);
        cx_var_set_map_int(src, "int", 1);
        cx_var_set_map_float(src, "float", 1.1);
        cx_var_set_map_str(src, "str", "str1");
        
        CxVar* m1 = cx_var_set_map_map(src, "map");
        cx_var_set_map_int(m1, "int", 2);
        cx_var_set_map_float(m1, "float", 2.1);
        CxVar* ma = cx_var_set_map_arr(m1, "arr");
        cx_var_push_arr_int(ma, 5);
        cx_var_push_arr_str(ma, "str5");

        CxVar* a1 = cx_var_set_map_arr(src, "arr");
        cx_var_push_arr_int(a1, 3);
        cx_var_push_arr_float(a1, 3.1);
        CxVar* a1m = cx_var_push_arr_map(a1);
        cx_var_set_map_int(a1m, "int", 4);
        cx_var_set_map_float(a1m, "float", 4.1);

        // Destination
        CxVar* dst = cx_var_new(alloc);
        cx_var_cpy_val(src, dst);
        // TODO val comparator function
        // CxWriter writer = cx_writer_file(stdout);
        // cx_json_build(dst, NULL, &writer);

        cx_var_del(src);
        cx_var_del(dst);
    }

}

