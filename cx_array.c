#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cx_alloc.h"

typedef struct Array {\
    CxAllocator*    alloc;
    ptrdiff_t       len;
    ptrdiff_t       cap;
    void*           data;
} Array;


void cxArrayGrowFn(void* ag, size_t elemsize, size_t addlen, size_t min_cap) {

    Array* a = (Array*)ag;
    size_t min_len = a->len + addlen;

    // Compute the minimum capacity needed
    if (min_len > min_cap) {
        min_cap = min_len;
    }
    if (min_cap <= a->cap) {
        return;
    }

    // Increase needed capacity to guarantee O(1) amortized
    if (min_cap < 2 * a->cap) {
        min_cap = 2 * a->cap;
    }
    else if (min_cap < 4) {
        min_cap = 4;
    }

    // Allocates new capacity
    size_t allocSize = elemsize * min_cap;
    void* new = a->alloc->alloc(a->alloc->ctx, allocSize);
    if (new == NULL) {
        return;
    }

    // Copy current data to new area and free previous
    memcpy(new, a->data, a->len * elemsize);
    a->alloc->free(a->alloc->ctx, a->data, a->len * elemsize);
    a->data = new;
    a->cap = min_cap;
}

