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
#include "bench_hmap_inc.c"

// Define bench_hmap2
#undef HMAP
#define HMAP hmap2
#include "bench_hmap_inc.c"

void bench_hmap() {

    bench_hmap1(cxDefaultAllocator(), 100000, 1000000);
    bench_hmap2(cxDefaultAllocator(), 100000, 1000000);
}

