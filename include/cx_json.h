#ifndef CX_JSON_H
#define CX_JSON_H

#include <stdint.h>
#include "cx_alloc.h"
#include "cx_var.h"
#include "cx_writer.h"

// Type for JSON build configuration
typedef struct CxJsonBuildCfg {
    int indent;     // Number of spaces of indentation
} CxJsonBuildCfg;

// Builds JSON string from the specified CxVar
// Return non-zero error if CxVa....
int cx_json_build(const CxVar* json, CxJsonBuildCfg* cfg, CxWriter* out); 


//int cx_json_parse(const CxAllocator* alloc, const char* data, size_t len, CxVar* json);

#endif


