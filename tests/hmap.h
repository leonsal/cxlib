#pragma once
#include <stdlib.h>
#include "cx_alloc.h"

void cxHmapTests(void);
void cxHmapTest1(size_t size, size_t nbuckets, const CxAllocator* alloc);
void cxHmapTest2(size_t size, size_t nbuckets, const CxAllocator* alloc);
void cxHmapTest3(size_t size, size_t nbuckets, const CxAllocator* alloc);
void cxHmapTest4(size_t size, size_t nbuckets, const CxAllocator* alloc);
void cxHmapTest5(size_t size, size_t nbuckets, const CxAllocator* alloc);


