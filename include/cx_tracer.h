#ifndef CX_TRACER_H
#define CX_TRACER_H

#include "cx_error.h"
#include "cx_alloc.h"
#include "cx_writer.h"

typedef enum {
    CxTracerScopeDefault = ' ',
    CxTracerScopeGlobal  = 'g',
    CxTracerScopeProcess = 'p',
    CxTracerScopeThread  = 't'
} CxTracerScope;

// Color names
#define CxTracerBlack   "black"
#define CxTracerGrey    "grey"
#define CxTracerWhite   "white"
#define CxTracerYellow  "yellow"
#define CXTracerOlive   "olive"

// Creates and returns new event tracer using specified custom allocator and capacity in number of events.
// Pass NULL as the allocator to use the default system allocator.
typedef struct CxTracer CxTracer;
CxTracer* cx_tracer_new(const CxAllocator* alloc, size_t cap);

// Deletes previously created instance of event tracer
void cx_tracer_del(CxTracer* evt);

// Returns the current number of events in the tracer buffer.
size_t cx_tracer_get_count(CxTracer* tr);

// Clear all events in the event tracer buffer
void cx_tracer_clear(CxTracer* evt);

// Appends a 'begin' event to the tracer buffer with the specified name and category
void cx_tracer_begin(CxTracer* tr, const char* name, const char* cat);

// Appends an 'end' event to the tracer buffer corresponding to a previous added 'begin' event
void cx_tracer_end(CxTracer* tr, const char* name, const char* cat);

// Appends an 'instant' event to the tracer buffer with the specified name, category and scope
void cx_tracer_instant(CxTracer* tr, const char* name, const char* cat, CxTracerScope scope);

// Writes all events in JSON format to specified writer.
// Returns positive error number.
CxError cx_tracer_json_write(CxTracer* tr, CxWriter* out);

// Writes all events in JSON format to specified file
// Returns positive error number.
CxError cx_tracer_json_write_file(CxTracer* tr, const char* path);

#endif


