#ifndef CX_VAR_H
#define CX_VAR_H

#include <stdint.h>
#include <stdbool.h>
#include "cx_alloc.h"

typedef enum {
    CxVarUndef,
    CxVarNull,
    CxVarBool,
    CxVarInt,
    CxVarFloat,
    CxVarStr,
    CxVarArr,
    CxVarMap,
    CxVarBuf,
} CxVarType;

// Declare/define dynamic string used in CxVar
#define cx_str_name cxvar_str
#define cx_str_instance_allocator
#ifdef CX_VAR_IMPLEMENT
#define cx_str_implement
#endif
#include "cx_str.h"

// Declare CxVar state
typedef struct cxvar_arr cxvar_arr;
typedef struct cxvar_map cxvar_map;
typedef struct cxvar_buf cxvar_buf;
typedef struct CxVar {
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

// Declare/define dynamic array used in CxVar
#define cx_array_name cxvar_arr
#define cx_array_type CxVar
#define cx_array_instance_allocator
#ifdef CX_VAR_IMPLEMENT
#define cx_array_implement
#endif
#include "cx_array.h"

// Declare/define dynamic buffer used in CxVar
#define cx_array_name cxvar_buf
#define cx_array_type uint8_t
#define cx_array_instance_allocator
#ifdef CX_VAR_IMPLEMENT
#define cx_array_implement
#endif
#include "cx_array.h"

// Declare/define hash map used in CxVar
#define cx_hmap_name cxvar_map
#define cx_hmap_key char*
#define cx_hmap_val CxVar
#define cx_hmap_cmp_key  cx_hmap_cmp_key_str_ptr
#define cx_hmap_hash_key cx_hmap_hash_key_str_ptr
#define cx_hmap_instance_allocator
#ifdef CX_VAR_IMPLEMENT
#define cx_hmap_implement
#endif
#include "cx_hmap.h"

// Returns CxVar of the specified type
// For CxVar of types string, array, map and buf is necessary to supply an allocator.
CxVar cx_var_new_null(void);
CxVar cx_var_new_bool(bool val);
CxVar cx_var_new_int(int64_t val);
CxVar cx_var_new_float(double val);
CxVar cx_var_new_str(const char* str, const CxAllocator* alloc);
CxVar cx_var_new_strn(const char* str, size_t slen, const CxAllocator* alloc);
CxVar cx_var_new_arr(const CxAllocator* alloc);
CxVar cx_var_new_map(const CxAllocator* alloc);
CxVar cx_var_new_buf(void* data, size_t len, const CxAllocator* alloc);

// Deletes allocated memory from previously created CxVar.
void cx_var_del(CxVar* var);

// Sets value of CxVar.
// Returns non-zero error if CxVar is not of the specified type
int cx_var_set_bool(CxVar* var, bool vb);
int cx_var_set_int(CxVar* var, int64_t vi);
int cx_var_set_float(CxVar* var, double vd);
int cx_var_set_str(CxVar* var, const char* str);
int cx_var_set_strn(CxVar* var, const char* str, size_t slen);
int cx_var_set_buf(CxVar* var, void* data, size_t len);

// Push value into an array
// Returns non-zero error if 'var' has not array type
int cx_var_push_arr_val(CxVar* arr, const CxVar el);
int cx_var_push_arr_null(CxVar* arr);
int cx_var_push_arr_bool(CxVar* arr, bool val);
int cx_var_push_arr_int(CxVar* arr, int64_t val);
int cx_var_push_arr_float(CxVar* arr, double val);
int cx_var_push_arr_str(CxVar* arr, const char* str);
int cx_var_push_arr_strn(CxVar* arr, const char* str, size_t len);
int cx_var_push_arr_arr(CxVar* arr, CxVar val);
int cx_var_push_arr_map(CxVar* arr, CxVar val);
int cx_var_push_arr_buf(CxVar* arr, void* data, size_t len);

// Sets value associated with string key of CxVar
// Returns non-zero error if 'var' has not map type
int cx_var_set_map_val(CxVar* map, const char* key, CxVar v);
int cx_var_set_map_val2(CxVar* var, const char* key, size_t key_len, CxVar v);
int cx_var_set_map_null(CxVar* map, const char* key);
int cx_var_set_map_bool(CxVar* map, const char* key, bool v);
int cx_var_set_map_int(CxVar* map, const char* key, int64_t v);
int cx_var_set_map_float(CxVar* map, const char* key, double v);
int cx_var_set_map_str(CxVar* map, const char* key, const char* v);
int cx_var_set_map_arr(CxVar* map, const char* key, CxVar v);
int cx_var_set_map_map(CxVar* map, const char* key, CxVar v);
int cx_var_set_map_buf(CxVar* map, const char* key, void* data, size_t len);

// Pushes additional data into the CxVar buf.
// Returns non-zero error if 'var' is of CxVarBuf type.
int cx_var_buf_push(CxVar* var, void* data, size_t len);

//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

// Returns the type of CxVar.
CxVarType cx_var_get_type(const CxVar* var);

// Get value of CxVar
// Returns non-zero error if 'var' has not of the requested type
int cx_var_get_null(const CxVar* var);
int cx_var_get_bool(const CxVar* var, bool *pval);
int cx_var_get_int(const CxVar* var, int64_t* pval);
int cx_var_get_float(const CxVar* var, double* pval);
int cx_var_get_str(const CxVar* var, const char** pval);
int cx_var_get_buf(const CxVar* var, const void** data, size_t* len);

// Returns the number of elements of the CxVar array.
// Returns non-zero error if CxVar is not of array type.
int cx_var_get_arr_len(const CxVar* var, size_t* len);

// Get value at the specified index from CxVar array.
// Returns non-zero error if 'arr' is not of array type
// or the index is invalid or the element at the specified index
// is not of the requested type.
int cx_var_get_arr_val(const CxVar* arr, size_t index, CxVar* pval);
int cx_var_get_arr_null(const CxVar* arr, size_t index);
int cx_var_get_arr_bool(const CxVar* arr, size_t index, bool* pbool);
int cx_var_get_arr_int(const CxVar* arr, size_t index, int64_t* pint);
int cx_var_get_arr_float(const CxVar* arr, size_t index, double* pfloat);
int cx_var_get_arr_str(const CxVar* arr, size_t index, const char** pstr);
int cx_var_get_arr_arr(const CxVar* arr, size_t index, CxVar* arr_el);
int cx_var_get_arr_map(const CxVar* arr, size_t index, CxVar* map_el);
int cx_var_get_arr_buf(const CxVar* arr, size_t index, const void** data, size_t* len);

// Returns the number of entries from the CxVar map.
// Returns non-zero error if CxVar is not of map type.
int cx_var_get_map_count(const CxVar* var, size_t* len);

// Get value with the specified key from CxVar map
// Returns non-zero error if 'var' is not of map type
// or the key was not found or the element associated with the key
// is not of the requested type.
int cx_var_get_map_val(const CxVar* map, const char* key, CxVar* pval);
int cx_var_get_map_null(const CxVar* map, const char* key);
int cx_var_get_map_bool(const CxVar* map, const char* key, bool* pbool);
int cx_var_get_map_int(const CxVar* map, const char* key, int64_t* pint);
int cx_var_get_map_float(const CxVar* map, const char* key, double* pfloat);
int cx_var_get_map_str(const CxVar* map, const char* key, const char** pstr);
int cx_var_get_map_arr(const CxVar* map, const char* key, CxVar* arr_el);
int cx_var_get_map_map(const CxVar* map, const char* key, CxVar* map_el);
int cx_var_get_map_buf(const CxVar* map, const char* key, const void** data, size_t* len);


#endif

