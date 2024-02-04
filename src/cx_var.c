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

// CxVar* cx_var_create_null(const CxAllocator* alloc) {
//
//     return cx_var_create(alloc, CxVarNull);
// }
//
// CxVar* cx_var_create_bool(const CxAllocator* alloc, bool vb) {
//
//     CxVar* var = cx_var_create(alloc, CxVarBool);
//     var->v.boolean = vb;
//     return var;
// }

void cx_var_destroy(const CxAllocator* alloc, CxVar* var) {

    if (var->type == CxVarStr) {
        cxstr_free(var->v.str);
        cx_alloc_free(alloc, var->v.str, sizeof(cxstr));
    } else if (var->type == CxVarArr) {
        cxarr_free(var->v.arr);
        for (size_t i = 0; i < cxarr_len(var->v.arr); i++) {
            cx_var_destroy(alloc, var->v.arr->data[i]);
        }
    } else if (var->type == CxVarMap) {
        // TODO iter allocator
        cxmap_free(var->v.map);
    }
    cx_alloc_free(alloc, var, sizeof(CxVar));
}


int cx_var_set_bool(CxVar* var, bool vb) {

    if (var->type == CxVarBool) {
        var->v.boolean = vb;
        return 0;
    }
    return 1;
}

int cx_var_set_int(CxVar* var, int64_t vi) {

    if (var->type == CxVarInt) {
        var->v.integer = vi;
        return 0;
    }
    return 1;
}

int cx_var_arr_push(CxVar* var, const CxVar* kvar) {

    return 0;
}

int cx_var_map_set(CxVar* var, const char* key, const CxVar* kvar) {

    return 0;
}

int cx_var_get_null(const CxVar* var) {

    if (var->type == CxVarNull) {
        return 0;
    }
    return 1;
}

int cx_var_get_bool(const CxVar* var, bool *pb) {

    if (var->type == CxVarBool) {
        if (pb) {
            *pb = var->v.boolean;
        }
        return 0;
    }
    return 1;
}

int cx_var_get_int(const CxVar* var, int64_t* pi) {

    if (var->type == CxVarInt) {
        if (pi) {
            *pi = var->v.integer;
        }
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
