#include <stdlib.h>
#include "cx_alloc.h"

// Default global memory manager uses stdlib malloc()/free()
static void* defAlloc(void* ctx, size_t n) {
    return malloc(n);
}

static void defFree(void* ctx, void* p, size_t n) {
    free(p);
}

// Initializes default global allocator
static const CxAllocator defAllocator = {
    .alloc = defAlloc,
    .free = defFree,
};

const CxAllocator* cxDefaultAllocator() {
    return &defAllocator;
}

