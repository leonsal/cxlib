#ifndef CX_TESTS_H
#define CX_TESTS_H
#include "cx_alloc.h"

void cxTests(void);

void cxAllocBlockTests(void);
void cxAllocBlockTest(size_t allocs, size_t blockSize);

void cxArrayTests(void);
void cxArrayTest(size_t size, const CxAllocator* alloc);

void cxHmapTests(void);
void cxHmapTest(size_t size, size_t nbuckets, const CxAllocator* alloc);

void cxStrTests(void);
void cxStrTest(const CxAllocator* alloc);

#endif

