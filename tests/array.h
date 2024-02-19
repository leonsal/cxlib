#pragma once
#include <stdlib.h>
#include "cx_alloc.h"

void test_array(void);
void test_array_int(size_t size, const CxAllocator* alloc);
void test_array_str(size_t size, const CxAllocator* alloc);
void test_array_cxstr(size_t size, const CxAllocator* alloc);

