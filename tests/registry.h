#ifndef REGISTRY_H
#define REGISTRY_H
#include <stddef.h>

// Adds test function to registry
typedef void (*TestFunc)(void);
void reg_add_test(const char* name, TestFunc fn);

// Get test function with specified name
// Return NULL if not found
TestFunc reg_get_test(const char* name);

// Returns number of tests
size_t reg_get_count(void);

// Returns name of tests at specified index
// Returns NULL if index is invalid;
const char* reg_get_name(size_t idx);

#endif


