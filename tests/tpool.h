#pragma once
#include <stdlib.h>
#include "cx_alloc.h"

void tpool_tests(void);
void tpool_test(const CxAllocator* alloc, size_t nthreads, size_t nworks);

