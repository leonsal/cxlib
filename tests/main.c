#include <stdio.h>
#include <assert.h>
#include "alloc.h"
#include "array.h"
#include "hmap.h"
#include "string.h"

#define cx_hmap_name map1
#define cx_hmap_key int
#define cx_hmap_val double
#define cx_hmap_allocator
#define cx_hmap_stats
#define cx_hmap_implement
#include "cx_hmap2.h"

// #define cx_hmap_name mapt2
// #define cx_hmap_key int
// #define cx_hmap_val double
// #define cx_hmap_allocator
// #define cx_hmap_implement
// #include "cx_hmap.h"

int main() {

    //size_t size = 10;
    map1 m1 = map1_init(NULL, 4);
    map1_set(&m1, 0, 1);
    map1_set(&m1, 1, 2);
    map1_free(&m1);
    map1_set(&m1, 2, 3);
    map1_set(&m1, 3, 4);
    map1_set(&m1, 4, 5);
    // for (size_t i = 0; i < size; i++) {
    //     map1_set(&m1, i, i*2);
    // }
    // map1_stats stats;
    // map1_get_stats(&m1, &stats);
    // printf("entryCount:%lu emptyCount:%lu chainCount:%lu maxChain:%lu minChain:%lu avgChain:%.1f loadFactor:%f\n",
    //     stats.entryCount, stats.emptyCount, stats.chainCount, stats.maxChain, stats.minChain, stats.avgChain, stats.loadFactor);
    
    // assert(*map1_get(&m1, 1) == 2.0);
    // assert(*map1_get(&m1, 2) == 4.0);
    // map1_free(&m1);
    //cxAllocBlockTests();
    //cxArrayTests();
    //cxHmapTests();
    //cxStrTests();
    return 0;
}

