#include <stdio.h>
#include <string.h>
#include "registry.h"

typedef struct RegInfo {
    const char* name;
    TestFunc    fn;
} RegInfo;


#define REG_MAX_SIZE (64)
static RegInfo registry[REG_MAX_SIZE];
static size_t reg_count = 0;

void reg_add_test(const char* name, TestFunc fn) {

    if (reg_count >= REG_MAX_SIZE) {
        printf("TEST REGISTRY SIZE EXCEEDED\n");
        return;
    }
    RegInfo info = {.name = name, .fn = fn};
    registry[reg_count] = info;
    reg_count++;
}

TestFunc reg_get_test(const char* name) {

    for (size_t i = 0; i < reg_count; i++) {
        RegInfo* pinfo = &registry[i];
        if (strcmp(pinfo->name, name) == 0) {
            return pinfo->fn;
        }
    }
    return NULL;
}


size_t reg_get_count(void) {

    return reg_count;
}

const char* reg_get_name(size_t idx) {

    if (idx >= reg_count) {
        return NULL;
    }
    return registry[idx].name;
}


