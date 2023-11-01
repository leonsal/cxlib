#ifndef CX_TESTS_H
#define CX_TESTS_H
#include "cx_alloc.h"

void cxArrayTests(void);
void cxArrayTest(size_t size, const CxAllocator* alloc);
void cxHmapTests(void);
void cxHmapTest(size_t size, size_t nbuckets, const CxAllocator* alloc);

#endif

