#ifndef CX_VAR_H
#define CX_VAR_H

#include <stdint.h>
#include <stdbool.h>
#include "cx_alloc.h"

typedef enum {
    CxVarNone,
    CxVarNull,
    CxVarBool,
    CxVarInt,
    CxVarFloat,
    CxVarStr,
    CxVarArr,
    CxVarMap,
} CxVarType;

// Creates CxVar using the specified allocator and with the specified type.
// Once created the type of the CxVar cannot be changed.
// Returns NULL if error.
typedef struct CxVar CxVar;
CxVar* cx_var_create(const CxAllocator* alloc, CxVarType vt);

// Destroy previously created CxVar.
void cx_var_destroy(const CxAllocator* alloc, CxVar* var);

// Sets value of boolean CxVar.
// Returns non-zero error if CxVar has not boolean type
int cx_var_set_bool(CxVar* var, bool vb);

// Set value of integer CxVar
// Returns non-zero error if CxVar has not integer type
int cx_var_set_int(CxVar* var, int64_t vi);

// Set value of floating point CxVar
// Returns non-zero error if CxVar has not floating point type
int cx_set_set_float(CxVar* var, double vd);

// Set value of string CxVar.
// The specified nul terminated string is copied into the CxVar.
// Returns non-zero error if CxVar has not floating point type
int cx_set_set_str(CxVar* var, const char* str);

// Set value of string CxVar.
// The specified string is copied into the CxVar.
// Returns non-zero error if CxVar has not floating point type
int cx_set_set_strn(CxVar* var, const char* str, size_t len);

// Push CxVar into an array CxVar.
// Returns non-zero error if 'var' has not array type
int cx_var_arr_push(CxVar* var, const CxVar* el);

// Sets value associated with key of CxVar map
// Returns non-zero error if 'var' has not map type
int cx_var_map_set(CxVar* var, const char* key, CxVar* v);

// Returns the type of CxVar.
CxVarType cx_var_get_type(const CxVar* var);

// Get value of boolean CxVar
// Returns non-zero error if 'var' has not boolean type
int cx_var_get_bool(const CxVar* var, bool *pval);

// Get value of integer CxVar
// Returns non-zero error if 'var' is not of integer type
int cx_var_get_int(const CxVar* var, int64_t* pval);

// Get value of string CxVar 
// Returns non-zero error if 'var' is not of string type
int cx_var_get_str(const CxVar* var, const char** pval);

// Returns the number of elements of the CxVar array.
// Returns non-zero error if CxVar is not of array type.
int cx_var_get_arr_len(CxVar* var, size_t* len);

// Get value at the specified index from CxVar array
// Returns non-zero error if 'var' is not of array type
// or the index is invalid.
int cx_var_get_arr_val(const CxVar* var, size_t index, CxVar** pval);

// Returns the number of entries from the CxVar map.
// Returns non-zero error if CxVar is not of map type.
int cx_var_get_map_count(CxVar* var, size_t* len);

// Get value with the specified key from CxVar map
// Returns non-zero error if 'var' is not of map type
// or the key was not found.
int cx_var_get_map_val(const CxVar* var, const char* key, CxVar* pval);

#endif

