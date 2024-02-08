#include <stdlib.h>
#include <stdio.h>
#include "cx_alloc.h"

// Default allocator default error function
static void cx_def_allocator_def_error_fn(const char* emsg, void* userdata) {
    fprintf(stderr, "CX DEFAULT ALLOCATOR ERROR:%s\n", emsg);
    abort();
}

// Saves pointer to default allocator error function and user data
static CxAllocatorErrorFn cx_def_allocator_error = cx_def_allocator_def_error_fn;
static void* cx_def_allocator_error_userdata = NULL;

// Default allocator allocation function
static void* cx_def_allocator_alloc(void* ctx, size_t n) {

    void* p = malloc(n);
    if (p == NULL) {
        if (cx_def_allocator_error) {
            cx_def_allocator_error("malloc() returned NULL", NULL);
        }
    }
    return p;
}

// Default allocator free function
static void cx_def_allocator_free(void* ctx, void* p, size_t n) {

    free(p);
}

// Default allocator realloc function
static void* cx_def_allocator_realloc(void* ctx, void* old_ptr, size_t old_size, size_t new_size) {

    void* p = realloc(old_ptr, new_size);
    if (p == NULL) {
        if (cx_def_allocator_error) {
            cx_def_allocator_error("malloc() returned NULL", NULL);
        }
    }
    return p;
}

// Default allocator interface
static const CxAllocator CxDefaultAllocator = {
    .alloc      = cx_def_allocator_alloc,
    .free       = cx_def_allocator_free,
    .realloc    = cx_def_allocator_realloc,
};

// Returns default allocator interface
const CxAllocator* cxDefaultAllocator() {
    return &CxDefaultAllocator;
}

// Sets default allocator error function
void cx_def_allocator_set_error_fn(CxAllocatorErrorFn fn, void *userdata) {

    cx_def_allocator_error = fn;
    cx_def_allocator_error_userdata = userdata;
}



