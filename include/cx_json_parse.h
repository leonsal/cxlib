#ifndef CX_JSON_PARSE_H
#define CX_JSON_PARSE_H

#include "cx_alloc.h"
#include "cx_var.h"

// Type for replacer function
typedef void (*CxJsonParseReplacer)(CxVar* val, void* userdata);

// Type for JSON parse configuration
typedef struct CxJsonParseCfg {
    const CxAllocator*  alloc;              // Optional allocator
    CxJsonParseReplacer replacer_fn;        // Optional replacer function
    void*               replacer_data;      // Optional replacer data
} CxJsonParseCfg;

// Parses JSON string and builds CxVar.
// Return non-zero error code
int cx_json_parse(const char* data, size_t len, CxVar* var, const CxJsonParseCfg* cfg); 

#endif

