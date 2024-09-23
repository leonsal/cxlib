#pragma once
#include "cx_alloc.h"

void test_hmapii(size_t size, size_t nbuckets, const CxAllocator* alloc);
void test_hmapss(size_t size, size_t nbuckets, const CxAllocator* alloc);
void test_hmapcc(size_t size, size_t nbuckets, const CxAllocator* alloc);


