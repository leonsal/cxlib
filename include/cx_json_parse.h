#ifndef CX_JSON_PARSE_H
#define CX_JSON_PARSE_H

#include "cx_alloc.h"
#include "cx_error.h"
#include "cx_var.h"

// Type for replacer function
typedef void (*CxJsonParseReplacer)(CxVar* val, void* userdata);

// Type for JSON parse configuration
typedef struct CxJsonParseCfg {
    bool                comments;           // Allow comments in JSON source (// & /* */)
    const CxAllocator*  alloc;              // Optional allocator
    CxJsonParseReplacer replacer_fn;        // Optional replacer function
    void*               replacer_data;      // Optional replacer data
} CxJsonParseCfg;

// Parses JSON string setting the specified CxVar which needs to be previously allocated.
// The returned error, if present, must be freed using CXERROR_FREE()
CxError cx_json_parse(const char* data, size_t len, CxVar* var, const CxJsonParseCfg* cfg); 

#endif

