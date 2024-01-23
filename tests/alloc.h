#pragma once
#include <stdlib.h>

void cxAllocPoolTests();
void cxAllocPoolTest(size_t allocs, size_t blockSize, size_t ncycles);
void cxAllocPoolTestPrint(CxPoolAllocator* pa);
