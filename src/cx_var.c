#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>

#define CX_VAR_IMPLEMENT
#include "cx_var.h"

CxVar* cx_var_create(const CxAllocator* alloc, CxVarType vt) {

#define CHKNULL(V) if (V == NULL) { return NULL; } 

    CxVar* var = cx_alloc_malloc(alloc, sizeof(CxVar));
    CHKNULL(var);
    *var = (CxVar){.type = vt};

    if (vt == CxVarStr) {
        var->v.str = cx_alloc_malloc(alloc, sizeof(cxstr));
        CHKNULL(var->v.str);
        *(var->v.str) = cxstr_init(alloc);
        return var;
    }
    if (vt == CxVarArr) {
        var->v.arr = cx_alloc_malloc(alloc, sizeof(cxarr));
        CHKNULL(var->v.arr);
        *(var->v.arr) = cxarr_init(alloc);
        return var;
    }
    if (vt == CxVarMap) {
        var->v.map = cx_alloc_malloc(alloc, sizeof(cxmap));
        CHKNULL(var->v.map);
        *(var->v.map) = cxmap_init(alloc, 0);
        return var;
    }
    return var;

#undef CHKNULL
}

void cx_var_destroy(const CxAllocator* alloc, CxVar* var) {

    if (var->type == CxVarStr) {
        cxstr_free(var->v.str);
        cx_alloc_free(alloc, var->v.str, sizeof(cxstr));
    } else if (var->type == CxVarArr) {
        cxarr_free(var->v.arr);
        for (size_t i = 0; i < cxarr_len(var->v.arr); i++) {
            cx_var_destroy(alloc, &var->v.arr->data[i]);
        }
    } else if (var->type == CxVarMap) {
        // TODO iter allocator
        cxmap_free(var->v.map);
    }
    cx_alloc_free(alloc, var, sizeof(CxVar));
}


int cx_set_bool_var(CxVar* var, bool vb) {

    if (var->type == CxVarBool) {
        var->v.boolean = vb;
        return 0;
    }
    return 1;
}

int cx_set_int_var(CxVar* var, int64_t vi) {

    if (var->type == CxVarInt) {
        var->v.integer = vi;
        return 0;
    }
    return 1;
}

int cx_set_f32_var(CxVar* var, float vf) {

    if (var->type == CxVarF32) {
        var->v.f32 = vf;
        return 0;
    }
    return 1;
}

int cx_set_f64_var(CxVar* var, double vd) {

    if (var->type == CxVarF64) {
        var->v.f64 = vd;
        return 0;
    }
    return 1;
}

int cx_push_arr_var(CxVar* var, const CxVar* kvar) {

    return 0;
}

int cx_var_map_set(CxVar* var, const char* key, const CxVar* kvar) {

    return 0;
}

int cx_var_null_get(const CxVar* var) {

    if (var->type == CxVarNull) {
        return 0;
    }
    return 1;
}

CxVarType cx_var_get_type(const CxVar* var) {

    return var->type;
}

int cx_var_get_bool(const CxVar* var, bool *pb) {

    if (var->type == CxVarBool) {
        *pb = var->v.boolean;
        return 0;
    }
    return 1;
}

int cx_var_get_int(const CxVar* var, int64_t* pi) {

    if (var->type == CxVarInt) {
        *pi = var->v.integer;
        return 0;
    }
    return 1;
}

int cx_var_get_arr_val(const CxVar* var, size_t index, CxVar* pval) {

    if (var->type == CxVarArr) {
        if (index >= cxarr_len(var->v.arr)) {
            return 1;
        }
        *pval = var->v.arr->data[index];
        return 0;
    }
    return 1;
}

int cx_var_get_map_val(const CxVar* var, const char* key, CxVar* kv) {

    if (var->type == CxVarMap) {
        CxVar* x = cxmap_get(var->v.map, key);
        if (x == NULL) {
            return 1;
        }
        *kv = *x;
        return 0;
    }
    return 1;
}

/*
CxVar* map = cx_var_create(alloc, CxVarMap);
CxVar* vi = cx_var_create(alloc, CxVarInt);
cx_var_set_int(vi, 3);
cx_var_set_map_key(map, "key", vi);
*/
