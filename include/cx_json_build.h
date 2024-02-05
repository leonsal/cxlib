#ifndef CX_JSON_BUILD_H
#define CX_JSON_BUILD_H

#include "cx_var.h"
#include "cx_writer.h"

// Type for JSON build configuration
typedef struct CxJsonBuildCfg {
    int indent;     // TODO: Number of spaces of indentation
} CxJsonBuildCfg;

// Builds JSON string from the specified CxVar using the specified configuration.
// The generated JSON is written using the supplied CxWriter interface.
// Return non-zero error if CxVar cannot be encoded in JSON format.
int cx_json_build(const CxVar* var, CxJsonBuildCfg* cfg, CxWriter* out); 

#endif


