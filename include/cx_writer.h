#ifndef CX_WRITER_H
#define CX_WRITER_H

#include <stddef.h>
#include <string.h>

// Type for write function
// Returns number of bytes written or negative error
typedef int (*CxWriterWrite)(void* ctx, const void* data, size_t len);

// Type for Writer interface
typedef struct CxWriter {
    void*           ctx;
    CxWriterWrite   write;
} CxWriter;

// Writer function helpers
static inline int cx_writer_write(CxWriter* w, const void* data, size_t len) {
    return w->write(w->ctx, data, len);
}
static inline int cx_writer_write_str(CxWriter* w, const char* data) {
    return w->write(w->ctx, data, strlen(data));
}
#endif

