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

    const char* pstr;
    cx_var_set_str(var, "string");
    CHK(cx_var_get_str(var, &pstr) == 0 && strcmp(pstr, "string") == 0);

    cx_var_set_arr(var);
    CHK(cx_var_get_type(var) == CxVarArr);

    cx_var_set_map(var);
    CHK(cx_var_get_type(var) == CxVarMap);

    const void* pbuf;
    size_t len;
    cx_var_set_buf(var, (uint8_t[]){0,1,2,3}, 4);
    CHK(cx_var_get_buf(var, &pbuf, &len) == 0 && len == 4);
    CHK(memcmp(pbuf, (uint8_t[]){0,1,2,3}, len) == 0);
    cx_var_del(var);



}

