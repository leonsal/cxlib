/*
    Implementation notes
    - CxVar consists of a type and value (union)
    - For primitive types the value is the primitive.
    - For container types: cxvar_str, cxvar_arr, cxvar_map
      the value contains pointer to allocated container
*/
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Define dynamic string used in CxVar
#define cx_str_name cxvar_str
#define cx_str_instance_allocator
#define cx_str_static
#define cx_str_implement
#include "cx_str.h"

// Define dynamic array used in CxVar
typedef struct CxVar CxVar;
#define cx_array_name cxvar_arr
#define cx_array_type CxVar*
#define cx_array_instance_allocator
#define cx_array_implement
#include "cx_array.h"

// Define dynamic buffer used in CxVar
#define cx_array_name cxvar_buf
#define cx_array_type uint8_t
#define cx_array_instance_allocator
#define cx_array_implement
#include "cx_array.h"

// Declare/define hash map used in CxVar
#define cx_hmap_name cxvar_map
#define cx_hmap_key char*
#define cx_hmap_val CxVar*
#define cx_hmap_cmp_key  cx_hmap_cmp_key_str_ptr
#define cx_hmap_hash_key cx_hmap_hash_key_str_ptr
#define cx_hmap_instance_allocator
#define cx_hmap_implement
#include "cx_hmap.h"

#include "cx_alloc.h"
#include "cx_var.h"

// Declare CxVar state
typedef struct CxVar {
    const CxAllocator* alloc;
    CxVarType type;    
    union {
        bool        boolean;
        int64_t     i64;
        double      f64;
        cxvar_str*  str;
        cxvar_arr*  arr;
        cxvar_map*  map;
        cxvar_buf*  buf;
    } v;
} CxVar;

static void cx_var_free_cont(CxVar* var);

CxVar* cx_var_new(const CxAllocator* alloc) {

    CxVar* var = cx_alloc_malloc(alloc, sizeof(CxVar));
    *var = (CxVar) {
        .alloc = alloc,
        .type = CxVarUndef
    };
    return var;
}

void cx_var_del(CxVar* var) {

    cx_var_free_cont(var);
    cx_alloc_free(var->alloc, var, sizeof(CxVar));
}

void cx_var_set_undef(CxVar* var) {

    cx_var_free_cont(var);
    var->type = CxVarUndef;
}

void cx_var_set_null(CxVar* var) {

    cx_var_free_cont(var);
    var->type = CxVarNull;
}

void cx_var_set_bool(CxVar* var, bool v) {

    cx_var_free_cont(var);
    var->type = CxVarBool;
    var->v.boolean = v;
}

void cx_var_set_int(CxVar* var, int64_t v) {

    cx_var_free_cont(var);
    var->type = CxVarInt;
    var->v.i64 = v;
}

void cx_var_set_float(CxVar* var, double v) {

    cx_var_free_cont(var);
    var->type = CxVarFloat;
    var->v.f64 = v;
}

void cx_var_set_strn(CxVar* var, const char* str, size_t len) {

    if (var->type != CxVarStr) {
        cx_var_free_cont(var);
        var->type = CxVarStr;
        var->v.str = cx_alloc_malloc(var->alloc, sizeof(cxvar_str));
        *(var->v.str) = cxvar_str_init(var->alloc);
    }
    cxvar_str_cpyn(var->v.str, str, len);
}

void cx_var_set_str(CxVar* var, const char* str) {

    cx_var_set_strn(var, str, strlen(str));
}

void cx_var_set_arr(CxVar* var) {

    if (var->type != CxVarArr) {
        cx_var_free_cont(var);
        var->type = CxVarArr;
        var->v.arr = cx_alloc_malloc(var->alloc, sizeof(cxvar_arr));
        *(var->v.arr) = cxvar_arr_init(var->alloc);
    }
    cxvar_arr_clear(var->v.arr);
}


void cx_var_set_map(CxVar* var) {

    if (var->type != CxVarMap) {
        cx_var_free_cont(var);
        var->type = CxVarMap;
        var->v.map = cx_alloc_malloc(var->alloc, sizeof(cxvar_map));
        *(var->v.map) = cxvar_map_init(var->alloc, 0);
    }
    cxvar_map_clear(var->v.map);
}

void cx_var_set_buf(CxVar* var, void* data, size_t len) {

    if (var->type != CxVarBuf) {
        cx_var_free_cont(var);
        var->type = CxVarBuf;
        var->v.buf = cx_alloc_malloc(var->alloc, sizeof(cxvar_buf));
        *(var->v.buf) = cxvar_buf_init(var->alloc);
    }
    cxvar_buf_clear(var->v.buf);
    if (data != NULL) {
        cxvar_buf_pushn(var->v.buf, data, len);
    }
}


static void cx_var_free_cont(CxVar* var) {

    switch (var->type) {
        case CxVarUndef:
        case CxVarNull:
        case CxVarBool:
        case CxVarInt:
        case CxVarFloat:
            return;
        case CxVarStr:
            cxvar_str_free(var->v.str);
            cx_alloc_free(var->alloc, var->v.str, sizeof(cxvar_str));
            var->v.str = NULL;
            break;
        case CxVarArr:
            for (size_t i = 0; i < cxvar_arr_len(var->v.arr); i++) {
                cx_var_del(var->v.arr->data[i]);
            }
            cxvar_arr_free(var->v.arr);
            cx_alloc_free(var->alloc, var->v.str, sizeof(cxvar_arr));
            var->v.arr = NULL;
            break;
        case CxVarMap:
            cxvar_map_iter iter = {0};
            while (true) {
                cxvar_map_entry* e = cxvar_map_next(var->v.map, &iter);
                if (e == NULL) {
                    break;
                }
                cx_alloc_free(var->alloc, e->key, strlen(e->key)+1);
                cx_var_del(e->val);
            }
            cxvar_map_free(var->v.map);
            cx_alloc_free(var->alloc, var->v.map, sizeof(cxvar_map));
            var->v.map = NULL;
            break;
        case CxVarBuf:
            cxvar_buf_free(var->v.buf);
            cx_alloc_free(var->alloc, var->v.buf, sizeof(cxvar_buf));
            var->v.buf = NULL;
            break;
        default:
            assert(0);
            break;
    }
}


//         cx_alloc_free(alloc, var->v.str, sizeof(cxvar_str));
//     } else if (var->type == CxVarArr) {
//         for (size_t i = 0; i < cxvar_arr_len(var->v.arr); i++) {
//             cx_var_del(&var->v.arr->data[i]);
//         }
//         const CxAllocator* alloc = var->v.arr->alloc_;
//         cxvar_arr_free(var->v.arr);
//         cx_alloc_free(alloc, var->v.arr, sizeof(cxvar_arr));
//     } else if (var->type == CxVarMap) {
//         const CxAllocator* alloc = var->v.map->alloc_;
//         cxvar_map_iter iter = {0};
//         while (true) {
//             cxvar_map_entry* e = cxvar_map_next(var->v.map, &iter);
//             if (e == NULL) {
//                 break;
//             }
//             cx_alloc_free(alloc, e->key, strlen(e->key)+1);
//             cx_var_del(&e->val);
//         }
//         cxvar_map_free(var->v.map);
//         cx_alloc_free(alloc, var->v.map, sizeof(cxvar_map));
//     } else if (var->type == CxVarBuf) {
//         const CxAllocator* alloc = var->v.buf->alloc_;
//         cxvar_buf_free(var->v.buf);
//         cx_alloc_free(alloc, var->v.buf, sizeof(cxvar_buf));
//     }
//     var->type = CxVarUndef;
//     var->v.str = NULL;
// }

void cx_var_set_null(CxVar* var);

// #define CX_VAR_IMPLEMENT
// #include "cx_var.h"
//
// #define CHKNULL(V) if (V == NULL) { return (CxVar){0}; } 
// #define CHKR(V)    {int res = V; if (res) {return res;}}
//
// CxVar cx_var_new_undef(void) {
//
//     return (CxVar){.type = CxVarUndef};
// }
//
// CxVar cx_var_new_null(void) {
//
//     return (CxVar){.type = CxVarNull};
// }
//
// CxVar cx_var_new_bool(bool v) {
//
//     return (CxVar){.type = CxVarBool, .v.boolean = v};
// }
//
// CxVar cx_var_new_int(int64_t v) {
//
//     return (CxVar){.type = CxVarInt, .v.i64 = v};
// }
//
// CxVar cx_var_new_float(double v) {
//
//     return (CxVar){.type = CxVarFloat, .v.f64 = v};
// }
//
// CxVar cx_var_new_str(const char* str, const CxAllocator* alloc) {
//
//     return cx_var_new_strn(str, strlen(str), alloc);
// }
//
// CxVar cx_var_new_strn(const char* str, size_t slen, const CxAllocator* alloc) {
//
//     CxVar var = {.type = CxVarStr };
//     var.v.str = cx_alloc_malloc(alloc, sizeof(cxvar_str));
//     CHKNULL(var.v.str);
//     *(var.v.str) = cxvar_str_init(alloc);
//     cxvar_str_cpyn(var.v.str, str, slen);
//     return var;
// }
//
// CxVar cx_var_new_arr(const CxAllocator* alloc) {
//
//     CxVar var = {.type = CxVarArr };
//     var.v.arr = cx_alloc_malloc(alloc, sizeof(cxvar_arr));
//     CHKNULL(var.v.arr);
//     *(var.v.arr) = cxvar_arr_init(alloc);
//     return var;
// }
//
// CxVar cx_var_new_map(const CxAllocator* alloc) {
//
//     CxVar var = {.type = CxVarMap };
//     var.v.map = cx_alloc_malloc(alloc, sizeof(cxvar_map));
//     CHKNULL(var.v.map);
//     *(var.v.map) = cxvar_map_init(alloc, 0);
//     return var;
// }
//
// CxVar cx_var_new_buf(void* data, size_t len, const CxAllocator* alloc) {
//
//     CxVar var = {.type = CxVarBuf };
//     var.v.buf = cx_alloc_malloc(alloc, len);
//     CHKNULL(var.v.buf);
//     *(var.v.buf) = cxvar_buf_init(alloc);
//     cxvar_buf_pushn(var.v.buf, data, len);
//     return var;
// }
//
// void cx_var_del(CxVar* var) {
//
//     if (var->type == CxVarStr) {
//         const CxAllocator* alloc = var->v.str->alloc_;
//         cxvar_str_free(var->v.str);
//         cx_alloc_free(alloc, var->v.str, sizeof(cxvar_str));
//     } else if (var->type == CxVarArr) {
//         for (size_t i = 0; i < cxvar_arr_len(var->v.arr); i++) {
//             cx_var_del(&var->v.arr->data[i]);
//         }
//         const CxAllocator* alloc = var->v.arr->alloc_;
//         cxvar_arr_free(var->v.arr);
//         cx_alloc_free(alloc, var->v.arr, sizeof(cxvar_arr));
//     } else if (var->type == CxVarMap) {
//         const CxAllocator* alloc = var->v.map->alloc_;
//         cxvar_map_iter iter = {0};
//         while (true) {
//             cxvar_map_entry* e = cxvar_map_next(var->v.map, &iter);
//             if (e == NULL) {
//                 break;
//             }
//             cx_alloc_free(alloc, e->key, strlen(e->key)+1);
//             cx_var_del(&e->val);
//         }
//         cxvar_map_free(var->v.map);
//         cx_alloc_free(alloc, var->v.map, sizeof(cxvar_map));
//     } else if (var->type == CxVarBuf) {
//         const CxAllocator* alloc = var->v.buf->alloc_;
//         cxvar_buf_free(var->v.buf);
//         cx_alloc_free(alloc, var->v.buf, sizeof(cxvar_buf));
//     }
//     var->type = CxVarUndef;
//     var->v.str = NULL;
// }
//
// int cx_var_set_bool(CxVar* var, bool vb) {
//
//     if (var->type == CxVarBool) {
//         var->v.boolean = vb;
//         return 0;
//     }
//     return 1;
// }
//
// int cx_var_set_int(CxVar* var, int64_t vi) {
//
//     if (var->type == CxVarInt) {
//         var->v.i64 = vi;
//         return 0;
//     }
//     return 1;
// }
//
// int cx_var_set_float(CxVar* var, double vd) {
//
//     if (var->type == CxVarFloat) {
//         var->v.f64 = vd;
//         return 0;
//     }
//     return 1;
// }
//
// int cx_var_set_str(CxVar* var, const char* str) {
//
//     if (var->type == CxVarStr) {
//         cxvar_str_cpy(var->v.str, str);
//         return 0;
//     }
//     return 1;
// }
//
// int cx_var_set_strn(CxVar* var, const char* str, size_t len) {
//
//     if (var->type == CxVarStr) {
//         cxvar_str_cpyn(var->v.str, str, len);
//         return 0;
//     }
//     return 1;
// }
//
// int cx_var_push_arr_val(CxVar* var, const CxVar el) {
//
//     if (var->type == CxVarArr) {
//         cxvar_arr_push(var->v.arr, el);
//         return 0;
//     }
//     return 1;
// }
//
// int cx_var_push_arr_null(CxVar* arr) {
//
//     const CxVar val = cx_var_new_null();
//     return cx_var_push_arr_val(arr, val);
// }
//
// int cx_var_push_arr_bool(CxVar* arr, bool val) {
//
//     const CxVar var = cx_var_new_bool(val);
//     return cx_var_push_arr_val(arr, var);
// }
//
// int cx_var_push_arr_int(CxVar* arr, int64_t val) {
//
//     const CxVar var = cx_var_new_int(val);
//     return cx_var_push_arr_val(arr, var);
// }
//
// int cx_var_push_arr_float(CxVar* arr, double val) {
//
//     const CxVar var = cx_var_new_float(val);
//     return cx_var_push_arr_val(arr, var);
// }
//
// int cx_var_push_arr_str(CxVar* arr, const char* val) {
//
//     if (arr->type != CxVarArr) {
//         return 1;
//     }
//     const CxAllocator* alloc = arr->v.arr->alloc_;
//     const CxVar var = cx_var_new_str(val, alloc);
//     return cx_var_push_arr_val(arr, var);
// }
//
// int cx_var_push_arr_strn(CxVar* arr, const char* val, size_t len) {
//
//     if (arr->type != CxVarArr) {
//         return 1;
//     }
//     const CxAllocator* alloc = arr->v.arr->alloc_;
//     const CxVar var = cx_var_new_strn(val, len, alloc);
//     return cx_var_push_arr_val(arr, var);
// }
//
// int cx_var_push_arr_arr(CxVar* arr, CxVar val) {
//
//     if (arr->type != CxVarArr || val.type != CxVarArr) {
//         return 1;
//     }
//     return cx_var_push_arr_val(arr, val);
// }
//
// int cx_var_push_arr_map(CxVar* arr, CxVar val) {
//
//     if (arr->type != CxVarArr || val.type != CxVarMap) {
//         return 1;
//     }
//     return cx_var_push_arr_val(arr, val);
// }
//
// int cx_var_push_arr_buf(CxVar* arr, void* data, size_t len) {
//
//     if (arr->type != CxVarArr) {
//         return 1;
//     }
//     const CxAllocator* alloc = arr->v.arr->alloc_;
//     const CxVar var = cx_var_new_buf(data, len, alloc);
//     return cx_var_push_arr_val(arr, var);
// }
//
// int cx_var_set_map_val(CxVar* var, const char* key, CxVar v) {
//
//     return cx_var_set_map_val2(var, key, strlen(key), v);
// }
//
// int cx_var_set_map_val2(CxVar* map, const char* key, size_t klen, CxVar v) {
//
//     if (map->type != CxVarMap) {
//         return 1;
//     }
//     CxVar* curr = cxvar_map_get(map->v.map, (char*)key);
//     if (curr != NULL) {
//         if (curr->type != v.type) {
//             return 1;
//         } else {
//             *curr = v;
//             return 0;
//         }
//     }
//     const CxAllocator* alloc = map->v.map->alloc_;
//     char* kcopy = cx_alloc_malloc(alloc, klen + 1);
//     strcpy(kcopy, key);
//     cxvar_map_set(map->v.map, kcopy, v);
//     return 0;
// }
// int cx_var_set_map_null(CxVar* map, const char* key) {
//
//     const CxVar val = cx_var_new_null();
//     return cx_var_set_map_val(map, key, val);
// }
//
// int cx_var_set_map_bool(CxVar* map, const char* key, bool v) {
//
//     const CxVar val = cx_var_new_bool(v);
//     return cx_var_set_map_val(map, key, val);
// }
//
// int cx_var_set_map_int(CxVar* map, const char* key, int64_t v) {
//
//     const CxVar val = cx_var_new_int(v);
//     return cx_var_set_map_val(map, key, val);
// }
//
// int cx_var_set_map_float(CxVar* map, const char* key, double v) {
//
//     const CxVar val = cx_var_new_float(v);
//     return cx_var_set_map_val(map, key, val);
// }
//
// int cx_var_set_map_str(CxVar* map, const char* key, const char* v) {
//
//     if (map->type != CxVarMap) {
//         return 1;
//     }
//     const CxAllocator* alloc = map->v.map->alloc_;
//     const CxVar val = cx_var_new_str(v, alloc);
//     return cx_var_set_map_val(map, key, val);
// }
//
// int cx_var_set_map_arr(CxVar* map, const char* key, CxVar v) {
//
//     if (map->type != CxVarMap) {
//         return 1;
//     }
//     const CxAllocator* alloc = map->v.map->alloc_;
//     const CxVar arr = cx_var_new_arr(alloc);
//     return cx_var_set_map_val(map, key, arr);
// }
//
// int cx_var_set_map_map(CxVar* map, const char* key, CxVar v) {
//
//     if (map->type != CxVarMap) {
//         return 1;
//     }
//     const CxAllocator* alloc = map->v.map->alloc_;
//     const CxVar map_val = cx_var_new_map(alloc);
//     return cx_var_set_map_val(map, key, map_val);
// }
//
// int cx_var_set_map_buf(CxVar* map, const char* key, void* data, size_t len) {
//
//     if (map->type != CxVarMap) {
//         return 1;
//     }
//     const CxAllocator* alloc = map->v.map->alloc_;
//     const CxVar buf_val = cx_var_new_buf(data, len, alloc);
//     return cx_var_set_map_val(map, key, buf_val);
// }
//
// int cx_var_buf_push(CxVar* var, void* data, size_t len) {
//
//     if (var->type == CxVarBuf) {
//         cxvar_buf_pushn(var->v.buf, data, len);
//         return 0;
//     }
//     return 1;
// }
//
// CxVarType cx_var_get_type(const CxVar* var) {
//
//     return var->type;
// }
//
// int cx_var_get_bool(const CxVar* var, bool *pval) {
//
//     if (var->type == CxVarBool) {
//         *pval = var->v.boolean;
//         return 0;
//     }
//     return 1;
// }
//
// int cx_var_get_int(const CxVar* var, int64_t* pval) {
//
//     if (var->type == CxVarInt) {
//         *pval = var->v.i64;
//         return 0;
//     }
//     return 1;
// }
//
// int cx_var_get_float(const CxVar* var, double* pval) {
//
//     if (var->type == CxVarFloat) {
//         *pval = var->v.f64;
//         return 0;
//     }
//     return 1;
// }
//
// int cx_var_get_str(const CxVar* var, const char** pval) {
//
//     if (var->type == CxVarStr) {
//         *pval = var->v.str->data;
//         return 0;
//     }
//     return 1;
// }
//
// int cx_var_get_buf(const CxVar* var, const void** data, size_t* len) {
//
//     if (var->type == CxVarBuf) {
//         *data = var->v.buf->data;
//         *len = cxvar_buf_len(var->v.buf);
//         return 0;
//     }
//     return 1;
// }
//
// int cx_var_get_arr_len(const CxVar* var, size_t* len) {
//
//     if (var->type == CxVarArr) {
//         *len = cxvar_arr_len(var->v.arr);
//         return 0;
//     }
//     return 1;
// }
//
// int cx_var_get_arr_val(const CxVar* var, size_t index, CxVar* pval) {
//
//     if (var->type == CxVarArr) {
//         if (index >= cxvar_arr_len(var->v.arr)) {
//             return 1;
//         }
//         *pval = var->v.arr->data[index];
//         return 0;
//     }
//     return 1;
// }
//
// int cx_var_get_arr_null(const CxVar* arr, size_t index) {
//
//     CxVar el;
//     CHKR(cx_var_get_arr_val(arr, index, &el));
//     return cx_var_get_type(&el) == CxVarNull ? 0 : 1;
// }
//
// int cx_var_get_arr_bool(const CxVar* arr, size_t index, bool* pbool) {
//
//     CxVar el;
//     CHKR(cx_var_get_arr_val(arr, index, &el));
//     return cx_var_get_bool(&el, pbool);
// }
//
// int cx_var_get_arr_int(const CxVar* arr, size_t index, int64_t* pint) {
//
//     CxVar el;
//     CHKR(cx_var_get_arr_val(arr, index, &el));
//     return cx_var_get_int(&el, pint);
// }
//
// int cx_var_get_arr_float(const CxVar* arr, size_t index, double* pfloat) {
//
//     CxVar el;
//     CHKR(cx_var_get_arr_val(arr, index, &el));
//     return cx_var_get_float(&el, pfloat);
// }
//
// int cx_var_get_arr_str(const CxVar* arr, size_t index, const char** pstr) {
//
//     CxVar el;
//     CHKR(cx_var_get_arr_val(arr, index, &el));
//     return cx_var_get_str(&el, pstr);
// }
//
// int cx_var_get_arr_arr(const CxVar* arr, size_t index, CxVar* arr_el) {
//
//     CHKR(cx_var_get_arr_val(arr, index, arr_el));
//     return cx_var_get_type(arr_el) == CxVarArr ? 0 : 1;
// }
//
// int cx_var_get_arr_map(const CxVar* arr, size_t index, CxVar* map_el) {
//
//     CHKR(cx_var_get_arr_val(arr, index, map_el));
//     return cx_var_get_type(map_el) == CxVarMap ? 0 : 1;
// }
//
// int cx_var_get_arr_buf(const CxVar* arr, size_t index, const void** data, size_t* len) {
//
//     CxVar el;
//     CHKR(cx_var_get_arr_val(arr, index, &el));
//     return cx_var_get_buf(&el, data, len);
// }
//
// int cx_var_get_map_count(const CxVar* var, size_t* len) {
//
//     if (var->type == CxVarMap) {
//         *len = cxvar_map_count(var->v.map);
//         return 0;
//     }
//     return 1;
// }
//
// int cx_var_get_map_val(const CxVar* var, const char* key, CxVar* pval) {
//
//     if (var->type == CxVarMap) {
//         CxVar* val = cxvar_map_get(var->v.map, (char*)key);
//         if (val == NULL) {
//             return 1;
//         }
//         *pval = *val;
//         return 0;
//     }
//     return 1;
// }
//
// int cx_var_get_map_null(const CxVar* map, const char* key) {
//
//     CxVar var;
//     CHKR(cx_var_get_map_val(map, key, &var));
//     return cx_var_get_type(&var) == CxVarNull ? 0 : 1;
// }
//
// int cx_var_get_map_bool(const CxVar* map, const char* key, bool* pbool) {
//
//     CxVar var;
//     CHKR(cx_var_get_map_val(map, key, &var));
//     return cx_var_get_bool(&var, pbool);
// }
//
// int cx_var_get_map_int(const CxVar* map, const char* key, int64_t* pint) {
//
//     CxVar var;
//     CHKR(cx_var_get_map_val(map, key, &var));
//     return cx_var_get_int(&var, pint);
// }
//
// int cx_var_get_map_float(const CxVar* map, const char* key, double* pfloat) {
//
//     CxVar var;
//     CHKR(cx_var_get_map_val(map, key, &var));
//     return cx_var_get_float(&var, pfloat);
// }
//
// int cx_var_get_map_str(const CxVar* map, const char* key, const char** pstr) {
//
//     CxVar var;
//     CHKR(cx_var_get_map_val(map, key, &var));
//     return cx_var_get_str(&var, pstr);
// }
//
// int cx_var_get_map_arr(const CxVar* map, const char* key, CxVar* var) {
//
//     CHKR(cx_var_get_map_val(map, key, var));
//     return cx_var_get_type(var) == CxVarArr ? 0 : 1;
// }
//
// int cx_var_get_map_map(const CxVar* map, const char* key, CxVar* var) {
//
//     CHKR(cx_var_get_map_val(map, key, var));
//     return cx_var_get_type(var) == CxVarMap ? 0 : 1;
// }
//
// int cx_var_get_map_buf(const CxVar* map, const char* key, const void** data, size_t* len) {
//
//     CxVar var;
//     CHKR(cx_var_get_map_val(map, key, &var));
//     return cx_var_get_buf(&var, data, len); 
// }


