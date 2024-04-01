#include <stdio.h>
#include "cx_writer.h"

static int cx_writer_file_write(void* ctx, const void* data, size_t len) {

    size_t res = fwrite(data, len, 1, ctx);
    if (res == len) {
        return (int)res;
    }
    return -1;
}

CxWriter cx_writer_file(FILE* f) {

    return (CxWriter) {
        .ctx = f,
        .write = cx_writer_file_write,
    };
}



