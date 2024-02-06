#ifndef CX_JSON_PARSE_H
#define CX_JSON_PARSE_H

#include "cx_alloc.h"
#include "cx_var.h"

// Type for JSON parse configuration
typedef struct CxJsonBuildCfg {
} CxJsonParseCfg;

// Parses JSON string and builds CxVar.
// Return non-zero error code
int cx_json_parse(const char* data, size_t len, CxVar* var, const CxAllocator* alloc); 

#endif

