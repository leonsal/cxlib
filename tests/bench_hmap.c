#include "cx_alloc.h"

#define cx_hmap_name hmap1
#define cx_hmap_key  uint64_t
#define cx_hmap_val  uint64_t
#define cx_hmap_instance_allocator
#define cx_hmap_static
#define cx_hmap_stats
#define cx_hmap_implement
#include "cx_hmap.h"

#define cx_hmap_name hmap2
#define cx_hmap_key  uint64_t
#define cx_hmap_val  uint64_t
#define cx_hmap_instance_allocator
#define cx_hmap_static
#define cx_hmap_stats
#define cx_hmap_implement
#include "cx_hmap2.h"

// Auxiliary macros
#define concat1_(a,b) a ## b
#define concat2_(a,b) concat1_(a,b)

// Define bench_hmap1
#define HMAP hmap1
#define HMAP_(name) concat2_(HMAP,name)
#define BENCH_(name) concat2_(bench_,name)
#define ARR arr1
#define ARR_(name) concat2_(ARR,name)
#include "bench_hmap_inc.c"
#undef ARR
#undef ARR_

// Define bench_hmap2
#undef HMAP
#define HMAP hmap2
#define ARR arr2
#define ARR_(name) concat2_(ARR,name)
#include "bench_hmap_inc.c"

void bench_hmap() {

    const size_t elcount = 10000;
    const size_t lookups = elcount * 10;
    bench_hmap1(cxDefaultAllocator(), elcount, lookups);
    bench_hmap2(cxDefaultAllocator(), elcount, lookups);
}

