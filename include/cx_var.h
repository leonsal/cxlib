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
typedef struct CxVar {
    CxVarType type;    
    union {
        bool        boolean;
        int64_t     i64;
        double      f64;
        cxvar_str*  str;
        cxvar_arr*  arr;
        cxvar_map*  map;
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

// Declare/define hash map used in CxVar
#define cx_hmap_name cxvar_map
#define cx_hmap_key char*
#define cx_hmap_val CxVar
#define cx_hmap_instance_allocator
#ifdef CX_VAR_IMPLEMENT
#define cx_hmap_implement
#endif
#include "cx_hmap.h"


// Returns CxVar of null type
CxVar cx_var_new_null(void);

// Returns CxVar of bool type
CxVar cx_var_new_bool(bool val);

// Returns CxVar of integer type
CxVar cx_var_new_int(int64_t val);

// Returns CxVar of floating point type
CxVar cx_var_new_float(double val);

// Returns CxVar of string type
CxVar cx_var_new_str(const char* str, const CxAllocator* alloc);

// Returns CxVar of string type
CxVar cx_var_new_strn(const char* str, size_t slen, const CxAllocator* alloc);

// Returns CxVar of array type
CxVar cx_var_new_arr(const CxAllocator* alloc);

// Returns CxVar of map type
CxVar cx_var_new_map(const CxAllocator* alloc);

// Deletes allocated memory from previously created CxVar.
void cx_var_del(CxVar* var);

// Sets value of boolean CxVar.
// Returns non-zero error if CxVar has not boolean type
int cx_var_set_bool(CxVar* var, bool vb);

// Set value of integer CxVar
// Returns non-zero error if CxVar has not integer type
int cx_var_set_int(CxVar* var, int64_t vi);

// Set value of floating point CxVar
// Returns non-zero error if CxVar has not floating point type
int cx_var_set_float(CxVar* var, double vd);

// Set value of string CxVar.
// The specified nul terminated string is copied into the CxVar.
// Returns non-zero error if CxVar has not floating point type
int cx_var_set_str(CxVar* var, const char* str);

// Set value of string CxVar.
// The specified nul terminated string is copied into the CxVar.
// Returns non-zero error if CxVar has not floating point type
int cx_var_set_strn(CxVar* var, const char* str, size_t slen);

// Set value of string CxVar.
// The specified string is copied into the CxVar.
// Returns non-zero error if CxVar has not floating point type
int cx_var_set_strn(CxVar* var, const char* str, size_t len);

// Push CxVar into an array CxVar.
// Returns non-zero error if 'var' has not array type
int cx_var_arr_push(CxVar* var, const CxVar el);

// Sets value associated with string key of CxVar map
// Returns non-zero error if 'var' has not map type
int cx_var_map_set(CxVar* var, const char* key, CxVar v);

// Sets value associated with string key of specified size of CxVar map
// Returns non-zero error if 'var' has not map type
int cx_var_map_setn(CxVar* var, const char* key, size_t klen, CxVar v);

// Returns the type of CxVar.
CxVarType cx_var_get_type(const CxVar* var);

// Get value of boolean CxVar
// Returns non-zero error if 'var' has not boolean type
int cx_var_get_bool(const CxVar* var, bool *pval);

// Get value of integer CxVar
// Returns non-zero error if 'var' is not of integer type
int cx_var_get_int(const CxVar* var, int64_t* pval);

// Get value of float CxVar
// Returns non-zero error if 'var' is not of integer type
int cx_var_get_float(const CxVar* var, double* pval);

// Get value of string CxVar 
// Returns non-zero error if 'var' is not of string type
int cx_var_get_str(const CxVar* var, const char** pval);

// Returns the number of elements of the CxVar array.
// Returns non-zero error if CxVar is not of array type.
int cx_var_get_arr_len(const CxVar* var, size_t* len);

// Get value at the specified index from CxVar array
// Returns non-zero error if 'var' is not of array type
// or the index is invalid.
int cx_var_get_arr_val(const CxVar* var, size_t index, CxVar* pval);

// Returns the number of entries from the CxVar map.
// Returns non-zero error if CxVar is not of map type.
int cx_var_get_map_count(const CxVar* var, size_t* len);

// Get value with the specified key from CxVar map
// Returns non-zero error if 'var' is not of map type
// or the key was not found.
int cx_var_get_map_val(const CxVar* var, const char* key, CxVar* pval);

#endif

