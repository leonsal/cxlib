#ifndef CX_JSON_H
#define CX_JSON_H

#include <stdint.h>
#include "cx_alloc.h"
#include "cx_var.h"

#define cx_str_name cxstr
#ifdef CX_JSON_IMPLEMENT
#   define cx_str_implement
#endif
#include "cx_str.h"


int cx_json_parse(const CxAllocator* alloc, const char* data, size_t len, CxVar* json);

// Type for JSON build configuration
typedef struct CxJsonBuildCfg {
    int indent;     // Number of spaces of indentation
} CxJsonBuildCfg;

// Builds JSON string from the specified CxVar
// Return non-zero error if CxVa....
int cx_json_build_str(const CxVar* json, CxJsonBuildCfg* cfg, cxstr* dst); 



#endif


