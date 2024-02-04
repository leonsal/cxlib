#ifndef CX_VAR_H
#define CX_VAR_H

#include <stdint.h>

#include "cx_alloc.h"

#define cx_str_name cxstr
#define cx_str_static
#define cx_str_error_handler(msg,func)\
    printf("CXSTR ERROR:%s at %s\n", msg, func);abort()
#define cx_str_instance_allocator
#ifdef CX_VAR_IMPLEMENT
#   define cx_str_implement
#endif
#include "cx_str.h"

typedef struct CxVar CxVar;
#define cx_array_name cxarr
#define cx_array_type CxVar*
#define cx_array_instance_allocator
#ifdef CX_VAR_IMPLEMENT
#   define cx_array_implement
#endif
#include "cx_array.h"

#define cx_hmap_name cxmap
#define cx_hmap_key char*
#define cx_hmap_val CxVar*
#define cx_hmap_instance_allocator
#ifdef CX_VAR_IMPLEMENT
#   define cx_hmap_implement
#endif
#include "cx_hmap.h"

typedef enum {
    CxVarNone,
    CxVarNull,
    CxVarBool,
    CxVarInt,
    CxVarF32,
    CxVarF64,
    CxVarStr,
    CxVarArr,
    CxVarMap,
} CxVarType;

typedef struct CxVar {
    CxVarType type;    
    union {
        bool        boolean;
        int64_t     integer;
        float       f32;
        double      f64;
        cxstr*      str;
        cxarr*      arr;
        cxmap*      map;
    } v;
} CxVar;

#endif

