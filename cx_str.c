#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cx_str.h"

typedef struct Array {\
    CxAllocator*    alloc;
    ptrdiff_t       len;
    ptrdiff_t       cap;
    void*           data;
} Array;


char* cxStrGrowFn(char* data, cons CxAllocator* alloc, size_t len, size_t cap, size_t maxcap, size_t addlen, size_t min_cap) {

    size_t min_len = len + addlen;
    if (min_len > min_cap) {
        min_cap = min_len;
    }
    if (min_cap <= cap) {
        return data;
    }

    // Increase needed capacity to guarantee O(1) amortized
    if (min_cap < 2 * cap) {
        min_cap = 2 * cap;
    }
    else if (min_cap < 4) {
        min_cap = 4;
    }
    if (min_cap > maxcap) {
        abort();
    }

    // Allocates new capacity
    size_t allocSize = sizeof(char) * min_cap;
    void* new = alloc->alloc(alloc->ctx, allocSize);
    if (new == NULL) {
        return NULL;
    }

    // Copy current data to new area and free previous
    memcpy(new, data, len * sizeof(char));
    alloc->free(alloc->ctx, data, len * sizeof(char));
    a->data = new;
    a->cap = min_cap;

    return NULL;
}

