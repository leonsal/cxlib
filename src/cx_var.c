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
#define cx_array_static
#define cx_array_implement
#include "cx_array.h"

// Define dynamic buffer used in CxVar
#define cx_array_name cxvar_buf
#define cx_array_type uint8_t
#define cx_array_instance_allocator
#define cx_array_static
#define cx_array_implement
#include "cx_array.h"

// Define array to store map keys in order
#define cx_array_name cxvar_keys
#define cx_array_type char*
#define cx_array_instance_allocator
#define cx_array_static
#define cx_array_implement
#include "cx_array.h"

// Define hash map used in CxVar
#define cx_hmap_name cxvar_map
#define cx_hmap_key char*
#define cx_hmap_val CxVar*
#define cx_hmap_cmp_key  cx_hmap_cmp_key_str_ptr
#define cx_hmap_hash_key cx_hmap_hash_key_str_ptr
#define cx_hmap_instance_allocator
#define cx_hmap_static
#define cx_hmap_implement
#include "cx_hmap.h"

#include "cx_alloc.h"
#include "cx_var.h"

// CxVar map contains an array to keep the order of the keys inserted
typedef struct cxvar_maparr {
    cxvar_keys keys;
    cxvar_map  map;     
} cxvar_maparr;

// Declare CxVar state
typedef struct CxVar {
    const CxAllocator*  alloc;
    CxVarType           type;    
    union {
        bool            boolean;
        int64_t         i64;
        double          f64;
        cxvar_str*      str;
        cxvar_arr*      arr;
        cxvar_buf*      buf;
        cxvar_maparr*   map;
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

const CxAllocator* cx_var_allocator(CxVar* var) {

    return var->alloc;
}

void cx_var_del(CxVar* var) {

    cx_var_free_cont(var);
    cx_alloc_free(var->alloc, var, sizeof(CxVar));
}

CxVar* cx_var_set_undef(CxVar* var) {

    cx_var_free_cont(var);
    var->type = CxVarUndef;
    return var;
}

CxVar* cx_var_set_null(CxVar* var) {

    cx_var_free_cont(var);
    var->type = CxVarNull;
    return var;
}

CxVar* cx_var_set_bool(CxVar* var, bool v) {

    cx_var_free_cont(var);
    var->type = CxVarBool;
    var->v.boolean = v;
    return var;
}

CxVar* cx_var_set_int(CxVar* var, int64_t v) {

    cx_var_free_cont(var);
    var->type = CxVarInt;
    var->v.i64 = v;
    return var;
}

CxVar* cx_var_set_float(CxVar* var, double v) {

    cx_var_free_cont(var);
    var->type = CxVarFloat;
    var->v.f64 = v;
    return var;
}

CxVar* cx_var_set_strn(CxVar* var, const char* str, size_t len) {

    if (var->type != CxVarStr) {
        cx_var_free_cont(var);
        var->type = CxVarStr;
        var->v.str = cx_alloc_malloc(var->alloc, sizeof(cxvar_str));
        *(var->v.str) = cxvar_str_init(var->alloc);
    }
    cxvar_str_cpyn(var->v.str, str, len);
    return var;
}

CxVar* cx_var_set_str(CxVar* var, const char* str) {

    return cx_var_set_strn(var, str, strlen(str));
}

CxVar* cx_var_set_arr(CxVar* var) {

    if (var->type != CxVarArr) {
        cx_var_free_cont(var);
        var->type = CxVarArr;
        var->v.arr = cx_alloc_malloc(var->alloc, sizeof(cxvar_arr));
        *(var->v.arr) = cxvar_arr_init(var->alloc);
    }
    cxvar_arr_clear(var->v.arr);
    return var;
}

CxVar* cx_var_set_map(CxVar* var) {

    if (var->type != CxVarMap) {
        cx_var_free_cont(var);
        var->type = CxVarMap;
        var->v.map = cx_alloc_malloc(var->alloc, sizeof(cxvar_maparr));
        var->v.map->keys = cxvar_keys_init(var->alloc);
        var->v.map->map = cxvar_map_init(var->alloc, 0);
    }
    cxvar_keys_clear(&var->v.map->keys);
    cxvar_map_clear(&var->v.map->map);
    return var;
}

CxVar* cx_var_set_buf(CxVar* var, void* data, size_t len) {

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
    return var;
}

CxVar* cx_var_push_arr_val(CxVar* arr, CxVar* val) {

    if (arr->type != CxVarArr) {
        return NULL;
    }
    cxvar_arr_push(arr->v.arr, val);
    return val;
}

CxVar* cx_var_push_arr_null(CxVar* arr) {

    return cx_var_push_arr_val(arr, cx_var_set_null(cx_var_new(arr->alloc)));
}

CxVar* cx_var_push_arr_bool(CxVar* arr, bool v) {

    return cx_var_push_arr_val(arr, cx_var_set_bool(cx_var_new(arr->alloc), v));
}

CxVar* cx_var_push_arr_int(CxVar* arr, int64_t v) {

    return cx_var_push_arr_val(arr, cx_var_set_int(cx_var_new(arr->alloc), v));
}

CxVar* cx_var_push_arr_float(CxVar* arr, double v) {

    return cx_var_push_arr_val(arr, cx_var_set_float(cx_var_new(arr->alloc), v));
}

CxVar* cx_var_push_arr_str(CxVar* arr, const char* str) {

    return cx_var_push_arr_val(arr, cx_var_set_str(cx_var_new(arr->alloc), str));
}

CxVar* cx_var_push_arr_strn(CxVar* arr, const char* str, size_t len) {

    return cx_var_push_arr_val(arr, cx_var_set_strn(cx_var_new(arr->alloc), str, len));
}

CxVar* cx_var_push_arr_buf(CxVar* arr, void* data, size_t len) {

    return cx_var_push_arr_val(arr, cx_var_set_buf(cx_var_new(arr->alloc), data, len));
}

CxVar* cx_var_push_arr_arr(CxVar* arr) {

    return cx_var_push_arr_val(arr, cx_var_set_arr(cx_var_new(arr->alloc)));
}

CxVar* cx_var_push_arr_map(CxVar* arr) {

    return cx_var_push_arr_val(arr, cx_var_set_map(cx_var_new(arr->alloc)));
}

CxVar* cx_var_set_map_val(CxVar* map, const char* key, CxVar* val) {

    return cx_var_set_map_valn(map, key, strlen(key), val);
}

CxVar* cx_var_set_map_valn(CxVar* map, const char* key, size_t key_len, CxVar* val) {

    if (map->type != CxVarMap) {
        return NULL;
    }

    // If no current value at this key, sets with the specified 'val'
    CxVar** curr = cxvar_map_get(&map->v.map->map, (char*)key);
    if (curr == NULL) {
        char* key_copy = cx_alloc_malloc(map->alloc, key_len + 1);
        strcpy(key_copy, key);
        cxvar_map_set(&map->v.map->map, key_copy, val);
        cxvar_keys_push(&map->v.map->keys, key_copy);
        return val;
    }
    cx_var_del(*curr);
    cxvar_map_set(&map->v.map->map, (char*)key, val);
    return val;
}

CxVar* cx_var_set_map_null(CxVar* map, const char* key) {

    return cx_var_set_map_val(map, key, cx_var_set_null(cx_var_new(map->alloc)));
}

CxVar* cx_var_set_map_bool(CxVar* map, const char* key, bool v) {

    return cx_var_set_map_val(map, key, cx_var_set_bool(cx_var_new(map->alloc), v));
}

CxVar* cx_var_set_map_int(CxVar* map, const char* key, int64_t v) {

    return cx_var_set_map_val(map, key, cx_var_set_int(cx_var_new(map->alloc), v));
}

CxVar* cx_var_set_map_float(CxVar* map, const char* key, double v) {

    return cx_var_set_map_val(map, key, cx_var_set_float(cx_var_new(map->alloc), v));
}

CxVar* cx_var_set_map_str(CxVar* map, const char* key, const char* v) {

    return cx_var_set_map_val(map, key, cx_var_set_str(cx_var_new(map->alloc), v));
}

CxVar* cx_var_set_map_arr(CxVar* map, const char* key) {

    return cx_var_set_map_val(map, key, cx_var_set_arr(cx_var_new(map->alloc)));
}

CxVar* cx_var_set_map_map(CxVar* map, const char* key) {

    return cx_var_set_map_val(map, key, cx_var_set_map(cx_var_new(map->alloc)));
}

CxVar* cx_var_set_map_buf(CxVar* map, const char* key, void* data, size_t len) {

    return cx_var_set_map_val(map, key, cx_var_set_buf(cx_var_new(map->alloc), data, len));
}

CxVarType cx_var_get_type(const CxVar* var) {

    return var->type;
}

bool cx_var_get_undef(const CxVar* var) {

    return var->type == CxVarUndef ? true : false;
}

bool cx_var_get_null(const CxVar* var) {

    return var->type == CxVarNull ? true : false;
}

bool cx_var_get_bool(const CxVar* var, bool *pval) {

    if (var->type != CxVarBool) {
        return false;
    }
    *pval = var->v.boolean;
    return true;
}

bool cx_var_get_int(const CxVar* var, int64_t* pval) {

    if (var->type != CxVarInt) {
        return false;
    }
    *pval = var->v.i64;
    return true;
}

bool cx_var_get_float(const CxVar* var, double* pval) {

    if (var->type != CxVarFloat) {
        return false;
    }
    *pval = var->v.f64;
    return true;
}

bool cx_var_get_str(const CxVar* var, const char** pval) {

    if (var->type != CxVarStr) {
        return false;
    }
    *pval = var->v.str->data;
    return true;
}

bool cx_var_get_buf(const CxVar* var, const void** data, size_t* len) {

    if (var->type != CxVarBuf) {
        return false;
    }
    *data = var->v.buf->data;
    *len = cxvar_buf_len(var->v.buf);
    return true;
}

CxVar* cx_var_get_arr_len(const CxVar* arr, size_t* len) {

    if (arr->type != CxVarArr) {
        return NULL;
    }
    *len = cxvar_arr_len(arr->v.arr);
    return (CxVar*)arr;
}

CxVar* cx_var_get_arr_val(const CxVar* arr, size_t index) {

    if (arr->type != CxVarArr) {
        return NULL;
    }
    if (index >= cxvar_arr_len(arr->v.arr)) {
        return NULL;
    }
    return arr->v.arr->data[index];
}

CxVar* cx_var_get_arr_null(const CxVar* arr, size_t index) {

    CxVar* val = cx_var_get_arr_val(arr, index);
    if (val && cx_var_get_null(val)) {
        return (CxVar*)val;
    } else {
        return NULL;
    }
}

CxVar* cx_var_get_arr_bool(const CxVar* arr, size_t index, bool* pbool) {

    CxVar* val = cx_var_get_arr_val(arr, index);
    if (val && cx_var_get_bool(val, pbool)) {
        return (CxVar*)val;
    } else {
        return NULL;
    }
}

CxVar* cx_var_get_arr_int(const CxVar* arr, size_t index, int64_t* pint) {

    CxVar* val = cx_var_get_arr_val(arr, index);
    if (val && cx_var_get_int(val, pint)) {
        return (CxVar*)val;
    } else {
        return NULL;
    }
}

CxVar* cx_var_get_arr_float(const CxVar* arr, size_t index, double* pfloat) {

    CxVar* val = cx_var_get_arr_val(arr, index);
    if (val && cx_var_get_float(val, pfloat)) {
        return (CxVar*)val;
    } else {
        return NULL;
    }
}

CxVar* cx_var_get_arr_str(const CxVar* arr, size_t index, const char** pstr) {

    CxVar* val = cx_var_get_arr_val(arr, index);
    if (val && cx_var_get_str(val, pstr)) {
        return (CxVar*)val;
    } else {
        return NULL;
    }
}

CxVar* cx_var_get_arr_arr(const CxVar* arr, size_t index) {

    CxVar* val = cx_var_get_arr_val(arr, index);
    if (val && val->type == CxVarArr) {
        return val;
    } else {
        return NULL;
    }
}

CxVar* cx_var_get_arr_map(const CxVar* arr, size_t index) {

    CxVar* val = cx_var_get_arr_val(arr, index);
    if (val && val->type == CxVarMap) {
        return val;
    } else {
        return NULL;
    }
}

CxVar* cx_var_get_arr_buf(const CxVar* arr, size_t index, const void** data, size_t* len) {

    CxVar* val = cx_var_get_arr_val(arr, index);
    if (val && cx_var_get_buf(val, data, len)) {
        return val;
    } else {
        return NULL;
    }
}

CxVar* cx_var_get_map_len(const CxVar* map, size_t* len) {

    if (map->type != CxVarMap) {
        return NULL;
    }
    *len = cxvar_map_count(&map->v.map->map);
    return (CxVar*)map;
}

const char* cx_var_get_map_key(const CxVar* map, size_t order) {

    if (map->type != CxVarMap) {
        return NULL;
    }
    if (order >= cxvar_keys_len(&map->v.map->keys)) {
        return NULL;
    }
    return map->v.map->keys.data[order];
}

CxVar* cx_var_get_map_val(const CxVar* map, const char* key) {

    if (map->type != CxVarMap) {
        return NULL;
    }
    CxVar** val = cxvar_map_get(&map->v.map->map, (char*)key);
    if (val == NULL) {
        return NULL;
    }
    return *val;
}

CxVar* cx_var_get_map_null(const CxVar* map, const char* key) {

    CxVar* val = cx_var_get_map_val(map, key);
    if (val && cx_var_get_null(val)) {
        return (CxVar*)val;
    } else {
        return NULL;
    }
}

CxVar* cx_var_get_map_bool(const CxVar* map, const char* key, bool* pbool) {

    CxVar* val = cx_var_get_map_val(map, key);
    if (val && cx_var_get_bool(val, pbool)) {
        return (CxVar*)val;
    } else {
        return NULL;
    }
}

CxVar* cx_var_get_map_int(const CxVar* map, const char* key, int64_t* pint) {

    CxVar* val = cx_var_get_map_val(map, key);
    if (val && cx_var_get_int(val, pint)) {
        return (CxVar*)val;
    } else {
        return NULL;
    }
}

CxVar* cx_var_get_map_float(const CxVar* map, const char* key, double* pfloat) {

    CxVar* val = cx_var_get_map_val(map, key);
    if (val && cx_var_get_float(val, pfloat)) {
        return (CxVar*)val;
    } else {
        return NULL;
    }
}

CxVar* cx_var_get_map_str(const CxVar* map, const char* key, const char** pstr) {

    CxVar* val = cx_var_get_map_val(map, key);
    if (val && cx_var_get_str(val, pstr)) {
        return (CxVar*)val;
    } else {
        return NULL;
    }
}

CxVar* cx_var_get_map_arr(const CxVar* map, const char* key) {

    CxVar* val = cx_var_get_map_val(map, key);
    if (val && val->type == CxVarArr) {
        return (CxVar*)val;
    } else {
        return NULL;
    }
}

CxVar* cx_var_get_map_map(const CxVar* map, const char* key) {

    CxVar* val = cx_var_get_map_val(map, key);
    if (val && val->type == CxVarMap) {
        return (CxVar*)val;
    } else {
        return NULL;
    }
}

CxVar* cx_var_get_map_buf(const CxVar* map, const char* key, const void** data, size_t* len) {

    CxVar* val = cx_var_get_map_val(map, key);
    if (val && cx_var_get_buf(val, data, len)) {
        return (CxVar*)val;
    } else {
        return NULL;
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
        case CxVarMap: {
            cxvar_keys_free(&var->v.map->keys);
            cxvar_map_iter iter = {0};
            while (true) {
                cxvar_map_entry* e = cxvar_map_next(&var->v.map->map, &iter);
                if (e == NULL) {
                    break;
                }
                cx_alloc_free(var->alloc, e->key, strlen(e->key)+1);
                cx_var_del(e->val);
            }
            cxvar_map_free(&var->v.map->map);
            cx_alloc_free(var->alloc, var->v.map, sizeof(cxvar_map));
            var->v.map = NULL;
            break;
        }
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

