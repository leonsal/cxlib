#pragma once
#include <stdlib.h>

void cxAllocBlockTests(void);
void cxAllocBlockTest(size_t allocs, size_t blockSize);
void cxAllocPoolTest(size_t allocs, size_t blockSize, size_t ncycles);

