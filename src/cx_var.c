#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#define CX_VAR_IMPLEMENT
#include "cx_var.h"
#define CHKNULL(V) if (V == NULL) { return (CxVar){0}; } 

CxVar cx_var_new_null(void) {

    return (CxVar){.type = CxVarNull};
}

CxVar cx_var_new_bool(bool v) {

    return (CxVar){.type = CxVarBool, .v.boolean = v};
}

CxVar cx_var_new_int(int64_t v) {

    return (CxVar){.type = CxVarInt, .v.i64 = v};
}

CxVar cx_var_new_float(double v) {

    return (CxVar){.type = CxVarFloat, .v.f64 = v};
}

CxVar cx_var_new_str(const char* str, const CxAllocator* alloc) {

    CxVar var = {.type = CxVarStr };
    var.v.str = cx_alloc_malloc(alloc, sizeof(cxvar_str));
    CHKNULL(var.v.str);
    *(var.v.str) = cxvar_str_init(alloc);
    return var;
}

CxVar cx_var_new_arr(const CxAllocator* alloc) {

    CxVar var = {.type = CxVarArr };
    var.v.arr = cx_alloc_malloc(alloc, sizeof(cxvar_arr));
    CHKNULL(var.v.arr);
    *(var.v.arr) = cxvar_arr_init(alloc);
    return var;
}

CxVar cx_var_new_map(const CxAllocator* alloc) {

    CxVar var = {.type = CxVarMap };
    var.v.map = cx_alloc_malloc(alloc, sizeof(cxvar_map));
    CHKNULL(var.v.map);
    *(var.v.map) = cxvar_map_init(alloc, 0);
    return var;
}

// void cx_var_destroy(const CxAllocator* alloc, CxVar* var) {
//
//     if (var->type == CxVarStr) {
//         cxstr_free(var->v.str);
//         cx_alloc_free(alloc, var->v.str, sizeof(cxstr));
//     } else if (var->type == CxVarArr) {
//         cxarr_free(var->v.arr);
//         for (size_t i = 0; i < cxarr_len(var->v.arr); i++) {
//             cx_var_destroy(alloc, &var->v.arr->data[i]);
//         }
//     } else if (var->type == CxVarMap) {
//         cxmap_iter iter = {0};
//         while (true) {
//             cxmap_entry* e = cxmap_next(var->v.map, &iter);
//             if (e == NULL) {
//                 break;
//             }
//             cx_alloc_free(alloc, e->key, strlen(e->key)+1);
//             cx_var_destroy(alloc, &e->val);
//         }
//         cxmap_free(var->v.map);
//     }
//     cx_alloc_free(alloc, var, sizeof(CxVar));
// }
//

int cx_var_set_bool(CxVar* var, bool vb) {

    if (var->type == CxVarBool) {
        var->v.boolean = vb;
        return 0;
    }
    return 1;
}

int cx_var_set_int(CxVar* var, int64_t vi) {

    if (var->type == CxVarInt) {
        var->v.i64 = vi;
        return 0;
    }
    return 1;
}

int cx_var_set_float(CxVar* var, double vd) {

    if (var->type == CxVarFloat) {
        var->v.f64 = vd;
        return 0;
    }
    return 1;
}

int cx_var_set_str(CxVar* var, const char* str) {

    if (var->type == CxVarStr) {
        cxvar_str_cpy(var->v.str, str);
        return 0;
    }
    return 1;
}

int cx_var_set_strn(CxVar* var, const char* str, size_t len) {

    if (var->type == CxVarStr) {
        cxvar_str_cpyn(var->v.str, str, len);
        return 0;
    }
    return 1;
}

int cx_var_arr_push(CxVar* var, const CxVar el) {

    if (var->type == CxVarStr) {
        cxvar_arr_push(var->v.arr, el);
        return 0;
    }
    return 1;
}

int cx_var_map_set(CxVar* var, const char* key, CxVar v) {

    if (var->type == CxVarMap) {
        const CxAllocator* alloc = var->v.map->alloc_;
        char* kcopy = cx_alloc_malloc(alloc, strlen(key)+1);
        strcpy(kcopy, key);
        cxvar_map_set(var->v.map, kcopy, v);
        return 0;
    }
    return 1;
}

CxVarType cx_var_get_type(const CxVar* var) {

    return var->type;
}

int cx_var_get_bool(const CxVar* var, bool *pval) {

    if (var->type == CxVarBool) {
        *pval = var->v.boolean;
        return 0;
    }
    return 1;
}

int cx_var_get_int(const CxVar* var, int64_t* pval) {

    if (var->type == CxVarInt) {
        *pval = var->v.i64;
        return 0;
    }
    return 1;
}

int cx_var_get_float(const CxVar* var, double* pval) {

    if (var->type == CxVarFloat) {
        *pval = var->v.f64;
        return 0;
    }
    return 1;
}

int cx_var_get_str(const CxVar* var, const char** pval) {

    if (var->type == CxVarStr) {
        *pval = var->v.str->data;
        return 0;
    }
    return 1;
}

int cx_var_get_arr_len(const CxVar* var, size_t* len) {

    if (var->type == CxVarArr) {
        *len = cxvar_arr_len(var->v.arr);
        return 0;
    }
    return 1;
}

int cx_var_get_arr_val(const CxVar* var, size_t index, CxVar* pval) {

    if (var->type == CxVarArr) {
        if (index >= cxvar_arr_len(var->v.arr)) {
            return 1;
        }
        *pval = var->v.arr->data[index];
        return 0;
    }
    return 1;
}

int cx_var_get_map_count(const CxVar* var, size_t* len) {

    if (var->type == CxVarMap) {
        *len = cxvar_map_count(var->v.map);
        return 0;
    }
    return 1;
}

int cx_var_get_map_val(const CxVar* var, const char* key, CxVar* pval) {

    if (var->type == CxVarMap) {
        CxVar* val = cxvar_map_get(var->v.map, (char*)key);
        if (val == NULL) {
            return 1;
        }
        *pval = *val;
        return 0;
    }
    return 1;
}

// /*
// CxVar* map = cx_var_create(alloc, CxVarMap);
// CxVar* vi = cx_var_create(alloc, CxVarInt);
// cx_var_set_int(vi, 3);
// cx_var_set_map_key(map, "key", vi);
// */
