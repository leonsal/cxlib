#ifndef CX_VAR_H
#define CX_VAR_H

#include <stdint.h>
#include <stdlib.h>
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

// Creates and returns pointer to new CxVar using the specified allocator.
typedef struct CxVar CxVar;
CxVar* cx_var_new(const CxAllocator* alloc);

// Returns the allocator used by the CxVar
const CxAllocator* cx_var_allocator(CxVar* var);

// Deletes CxVar
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

// Creates and push new CxVar of the specified type into an array
// Returns pointer to the new CxVar or NULL on errors.
CxVar* cx_var_push_arr_null(CxVar* arr);
CxVar* cx_var_push_arr_bool(CxVar* arr, bool v);
CxVar* cx_var_push_arr_int(CxVar* arr, int64_t v);
CxVar* cx_var_push_arr_float(CxVar* arr, double val);
CxVar* cx_var_push_arr_str(CxVar* arr, const char* str);
CxVar* cx_var_push_arr_strn(CxVar* arr, const char* str, size_t len);
CxVar* cx_var_push_arr_arr(CxVar* arr);
CxVar* cx_var_push_arr_map(CxVar* arr);
CxVar* cx_var_push_arr_buf(CxVar* arr, void* data, size_t len);

// Creates and sets new map entry
// Returns pointer to the new entry or NULL on errors.
CxVar* cx_var_set_map_val(CxVar* map, const char* key, CxVar* val);
CxVar* cx_var_set_map_valn(CxVar* map, const char* key, size_t key_len, CxVar* val);
CxVar* cx_var_set_map_null(CxVar* map, const char* key);
CxVar* cx_var_set_map_bool(CxVar* map, const char* key, bool v);
CxVar* cx_var_set_map_int(CxVar* map, const char* key, int64_t v);
CxVar* cx_var_set_map_float(CxVar* map, const char* key, double v);
CxVar* cx_var_set_map_str(CxVar* map, const char* key, const char* v);
CxVar* cx_var_set_map_arr(CxVar* map, const char* key);
CxVar* cx_var_set_map_map(CxVar* map, const char* key);
CxVar* cx_var_set_map_buf(CxVar* map, const char* key, void* data, size_t len);

//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

// Returns the type of CxVar.
CxVarType cx_var_get_type(const CxVar* var);

// Get value of CxVar
// Returns false if CxVar is not of the requested type
bool cx_var_get_undef(const CxVar* var);
bool cx_var_get_null(const CxVar* var);
bool cx_var_get_bool(const CxVar* var, bool *pval);
bool cx_var_get_int(const CxVar* var, int64_t* pval);
bool cx_var_get_float(const CxVar* var, double* pval);
bool cx_var_get_str(const CxVar* var, const char** pval);
bool cx_var_get_buf(const CxVar* var, const void** data, size_t* len);

// Get length of array
// Returns the supplied array pointer or NULL on error.
CxVar* cx_var_get_arr_len(const CxVar* arr, size_t* len);

// Get value at the specified array index
// Returns the supplied array pointer or NULL on error.
CxVar* cx_var_get_arr_val(const CxVar* arr, size_t index);

// Utility array getters
CxVar* cx_var_get_arr_null(const CxVar* arr, size_t index);
CxVar* cx_var_get_arr_bool(const CxVar* arr, size_t index, bool* pbool);
CxVar* cx_var_get_arr_int(const CxVar* arr, size_t index, int64_t* pint);
CxVar* cx_var_get_arr_float(const CxVar* arr, size_t index, double* pfloat);
CxVar* cx_var_get_arr_str(const CxVar* arr, size_t index, const char** pstr);
CxVar* cx_var_get_arr_arr(const CxVar* arr, size_t index);
CxVar* cx_var_get_arr_map(const CxVar* arr, size_t index);
CxVar* cx_var_get_arr_buf(const CxVar* arr, size_t index, const void** data, size_t* len);

// Get length of map
// Returns the supplied map pointer or NULL on error.
CxVar* cx_var_get_map_len(const CxVar* map, size_t* len);

// Get value of map element at the specified key
// Returns NULL on errors.
CxVar* cx_var_get_map_val(const CxVar* map, const char* key);

// Utility map element getters
CxVar* cx_var_get_map_null(const CxVar* map, const char* key);
CxVar* cx_var_get_map_bool(const CxVar* map, const char* key, bool* pbool);
CxVar* cx_var_get_map_int(const CxVar* map, const char* key, int64_t* pint);
CxVar* cx_var_get_map_float(const CxVar* map, const char* key, double* pfloat);
CxVar* cx_var_get_map_str(const CxVar* map, const char* key, const char** pstr);
CxVar* cx_var_get_map_arr(const CxVar* map, const char* key);
CxVar* cx_var_get_map_map(const CxVar* map, const char* key);
CxVar* cx_var_get_map_buf(const CxVar* map, const char* key, const void** data, size_t* len);

// Map iteration
typedef struct CxVarMapIter CxVarMapIter;
CxVarMapIter* cx_var_get_map_iter(const CxVar* map);
CxVar* cx_var_get_map_next(const CxVar* map, CxVarMapIter* iter, const char** key);
CxVar* cx_var_map_del_iter(const CxVar* map, CxVarMapIter* iter);

#endif

