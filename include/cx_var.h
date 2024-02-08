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

// Creates and returns pointer to new CxVar of the specified type
typedef struct CxVar CxVar;
CxVar* cx_var_new(const CxAllocator* alloc);
void cx_var_del(CxVar* var);

// Sets the new type and/or value of a previously created CxVar.
// If memory was allocated by the previous value, it is freed.
// Returns pointer to the CxVar.
CxVar* cx_var_set_undef(CxVar* var);
CxVar* cx_var_set_null(CxVar* var);
CxVar* cx_var_set_bool(CxVar* var, bool v);
CxVar* cx_var_set_int(CxVar* var, int64_t v);
CxVar* cx_var_set_float(CxVar* var, double v);
CxVar* cx_var_set_strn(CxVar* var, const char* str, size_t len);
CxVar* cx_var_set_str(CxVar* var, const char* str);
CxVar* cx_var_set_arr(CxVar* var);
CxVar* cx_var_set_map(CxVar* var);
CxVar* cx_var_set_buf(CxVar* var, void* data, size_t len);

// Push CxVar into the array
CxVar* cx_var_push_arr_val(CxVar* arr, CxVar* val);

// Creates and push new CxVar into an array
// Returns pointer to the new CxVar or NULL if 'arr' if not of CxVarArr type
CxVar* cx_var_push_arr_null(CxVar* arr);
CxVar* cx_var_push_arr_bool(CxVar* arr, bool v);
CxVar* cx_var_push_arr_int(CxVar* arr, int64_t v);
CxVar* cx_var_push_arr_float(CxVar* arr, double val);
CxVar* cx_var_push_arr_str(CxVar* arr, const char* str);
CxVar* cx_var_push_arr_strn(CxVar* arr, const char* str, size_t len);
CxVar* cx_var_push_arr_arr(CxVar* arr);
CxVar* cx_var_push_arr_map(CxVar* arr);
CxVar* cx_var_push_arr_buf(CxVar* arr, void* data, size_t len);

// Sets map entry
// Returns pointer to the new entry or NULL if 'map' is not a map
CxVar* cx_var_set_map_val(CxVar* arr, const char* key, CxVar* val);
CxVar* cx_var_set_map_null(CxVar* map, const char* key);
CxVar* cx_var_set_map_bool(CxVar* map, const char* key, bool v);
CxVar* cx_var_set_map_int(CxVar* map, const char* key, int64_t v);
CxVar* cx_var_set_map_float(CxVar* map, const char* key, double v);
CxVar* cx_var_set_map_str(CxVar* map, const char* key, const char* v);
CxVar* cx_var_set_map_arr(CxVar* map, const char* key);
CxVar* cx_var_set_map_map(CxVar* map, const char* key);
CxVar* cx_var_set_map_buf(CxVar* map, const char* key, void* data, size_t len);
//
// // Pushes additional data into the CxVar buf.
// // Returns non-zero error if 'var' is of CxVarBuf type.
// int cx_var_buf_push(CxVar* var, void* data, size_t len);
//
// //-----------------------------------------------------------------------------
// // Getters
// //-----------------------------------------------------------------------------
//
// Returns the type of CxVar.
CxVarType cx_var_get_type(const CxVar* var);

// Get value of CxVar
// Returns non-zero error if 'var' has not of the requested type
int cx_var_get_undef(const CxVar* var);
int cx_var_get_null(const CxVar* var);
int cx_var_get_bool(const CxVar* var, bool *pval);
int cx_var_get_int(const CxVar* var, int64_t* pval);
int cx_var_get_float(const CxVar* var, double* pval);
int cx_var_get_str(const CxVar* var, const char** pval);
int cx_var_get_buf(const CxVar* var, const void** data, size_t* len);

int cx_var_get_arr_len(const CxVar* arr, size_t* len);
int cx_var_get_arr_val(const CxVar* arr, size_t index, CxVar* pval);

// // Returns the number of elements of the CxVar array.
// // Returns non-zero error if CxVar is not of array type.
// int cx_var_get_arr_len(const CxVar* var, size_t* len);
//
// // Get value at the specified index from CxVar array.
// // Returns non-zero error if 'arr' is not of array type
// // or the index is invalid or the element at the specified index
// // is not of the requested type.
// int cx_var_get_arr_val(const CxVar* arr, size_t index, CxVar* pval);
// int cx_var_get_arr_null(const CxVar* arr, size_t index);
// int cx_var_get_arr_bool(const CxVar* arr, size_t index, bool* pbool);
// int cx_var_get_arr_int(const CxVar* arr, size_t index, int64_t* pint);
// int cx_var_get_arr_float(const CxVar* arr, size_t index, double* pfloat);
// int cx_var_get_arr_str(const CxVar* arr, size_t index, const char** pstr);
// int cx_var_get_arr_arr(const CxVar* arr, size_t index, CxVar* arr_el);
// int cx_var_get_arr_map(const CxVar* arr, size_t index, CxVar* map_el);
// int cx_var_get_arr_buf(const CxVar* arr, size_t index, const void** data, size_t* len);
//
// // Returns the number of entries from the CxVar map.
// // Returns non-zero error if CxVar is not of map type.
// int cx_var_get_map_count(const CxVar* var, size_t* len);
//
// // Get value with the specified key from CxVar map
// // Returns non-zero error if 'var' is not of map type
// // or the key was not found or the element associated with the key
// // is not of the requested type.
// int cx_var_get_map_val(const CxVar* map, const char* key, CxVar* pval);
// int cx_var_get_map_null(const CxVar* map, const char* key);
// int cx_var_get_map_bool(const CxVar* map, const char* key, bool* pbool);
// int cx_var_get_map_int(const CxVar* map, const char* key, int64_t* pint);
// int cx_var_get_map_float(const CxVar* map, const char* key, double* pfloat);
// int cx_var_get_map_str(const CxVar* map, const char* key, const char** pstr);
// int cx_var_get_map_arr(const CxVar* map, const char* key, CxVar* arr_el);
// int cx_var_get_map_map(const CxVar* map, const char* key, CxVar* map_el);
// int cx_var_get_map_buf(const CxVar* map, const char* key, const void** data, size_t* len);
//

#endif

