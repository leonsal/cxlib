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

    LOGI("var alloc=%p", alloc);
    // Primitives
    {
        CxVar* var = cx_var_new(alloc);

        cx_var_set_undef(var);
        CHKZ(cx_var_get_undef(var));

        cx_var_set_null(var);
        CHKZ(cx_var_get_null(var));

        bool vbool;
        cx_var_set_bool(var, true);
        CHK(cx_var_get_bool(var, &vbool) == 0 && vbool);

        cx_var_set_bool(var, false);
        CHK(cx_var_get_bool(var, &vbool) == 0 && !vbool);

        int64_t vint;
        cx_var_set_int(var, 42);
        CHK(cx_var_get_int(var, &vint) == 0 && vint == 42);

        double vfloat;
        cx_var_set_float(var, -0.1);
        CHK(cx_var_get_float(var, &vfloat) == 0 && vfloat == -0.1);


        cx_var_set_map(var);
        CHK(cx_var_get_type(var) == CxVarMap);

        const void* pbuf;
        size_t len;
        cx_var_set_buf(var, (uint8_t[]){0,1,2,3}, 4);
        CHK(cx_var_get_buf(var, &pbuf, &len) == 0 && len == 4);
        CHK(memcmp(pbuf, (uint8_t[]){0,1,2,3}, len) == 0);
        cx_var_del(var);
    }

    // String
    {
        CxVar* var = cx_var_new(alloc);

        const char* sets = NULL;
        const char* gets = NULL;

        sets = "123";
        cx_var_set_str(var, sets);
        CHK(cx_var_get_str(var, &gets) == 0 && strcmp(gets, sets) == 0);

        sets = "0123456789";
        cx_var_set_str(var, sets);
        CHK(cx_var_get_str(var, &gets) == 0 && strcmp(gets, sets) == 0);

        sets = "";
        cx_var_set_str(var, sets);
        CHK(cx_var_get_str(var, &gets) == 0 && strcmp(gets, sets) == 0);

        cx_var_del(var);
    }

    // Array
    {
        CxVar* arr = cx_var_set_arr(cx_var_new(alloc));
        size_t len;

        CHK(cx_var_get_type(arr) == CxVarArr);
        CHK(cx_var_get_arr_len(arr, &len) == 0 && len == 0);

        CHKN(cx_var_push_arr_null(arr));
        CHKN(cx_var_push_arr_bool(arr, true));
        CHKN(cx_var_push_arr_bool(arr, false));
        CHKN(cx_var_push_arr_int(arr, 42));
        CHKN(cx_var_push_arr_float(arr, -0.1));
        {
            CxVar* arr_el = cx_var_push_arr_arr(arr);
            CHKN(arr_el);
            CHKN(cx_var_push_arr_null(arr_el));
            CHKN(cx_var_push_arr_bool(arr_el, false));
            CHKN(cx_var_push_arr_int(arr_el, 88));
            CHKN(cx_var_push_arr_float(arr_el, 0.4));
        }
        {
            CxVar* map_el = cx_var_push_arr_map(arr);
            CHKN(map_el);
            CHKN(cx_var_set_map_null(map_el, "k1"));
        }

        cx_var_del(arr);
    }

}

