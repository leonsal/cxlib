#include "cx_pool_allocator.h"

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

int main() {

    const CxPoolAllocator* pa = cx_pool_allocator_create(1*4096, NULL);
    const CxAllocator* alloc = cx_pool_allocator_iface(pa);

    bench_hmap1(alloc, 100000, 1000000);
    return 0;
}
// #include "cx_pool_allocator.h"
//
// #include "logger.h"
// #include "alloc.h"
// #include "array.h"
// #include "hmap.h"
// #include "string.h"
// #include "queue.h"
// #include "list.h"
// #include "timer.h"
// #include "var.h"
// #include "json_build.h"
// #include "json_parse.h"
//
// int main() {
//
//     LOG_INIT();
//     LOGW("START");
//     cxAllocPoolTests();
//     test_array();
//     test_hmap();
//     cxStrTests();
//     cxQueueTests();
//     cx_list_tests();
//     cx_timer_tests();
//     cx_var_tests();
//     json_build_tests();
//     json_parse_tests();
//     LOGW("END");
// }

