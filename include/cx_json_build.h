#ifndef CX_JSON_BUILD_H
#define CX_JSON_BUILD_H

#include "cx_var.h"
#include "cx_writer.h"

// Type for replacer function
typedef void (*CxJsonBuildReplacer)(CxVar* val, void* userdata);

// Type for JSON build configuration
typedef struct CxJsonBuildCfg {
    CxJsonBuildReplacer replacer_fn;
    void*               replacer_data;
} CxJsonBuildCfg;

// Builds JSON string from the supplied CxVar using the specified configuration.
// If a replacer function was defined, the CxVar may be modified.
// The generated JSON is written using the supplied CxWriter interface.
// Return non-zero error if CxVar cannot be encoded in JSON format.
int cx_json_build(CxVar* var, CxJsonBuildCfg* cfg, const CxWriter* out); 

#endif


