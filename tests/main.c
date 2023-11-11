#include <stdio.h>
#include <assert.h>
#define cx_hmap_name map
#define cx_hmap_key int
#define cx_hmap_val double
#define cx_hmap_implement
#include "cx_hmap.h"

int main() {

    // Initialize map with default number of buckets
    map m1 = map_init(0);

    // Set keys and values
    size_t size = 100;
    for (size_t i = 0; i < size; i++) {
        map_set(&m1, i, i * 2.0);
    }
    assert(map_count(&m1) == size);

    // Get keys and values
    for (size_t i = 0; i < size; i++) {
        assert(*map_get(&m1, i) == i * 2.0);
    }

    // Iterate over keys and values
    map_iter iter = {0};
    map_entry* e = NULL;
    while ((e = map_next(&m1, &iter)) != NULL) {
        printf("key:%d val:%f\n", e->key, e->val);
    }

    // Delete even keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2 == 0) {
            map_del(&m1, i);
        }
    }
    assert(map_count(&m1) == size/2);
    return 0;
}
// #include <stdio.h>
// #include <assert.h>
// #include "alloc.h"
// #include "array.h"
// #include "hmap.h"
// #include "string.h"
//
// #define cx_hmap_name map1
// #define cx_hmap_key int
// #define cx_hmap_val double
// #define cx_hmap_allocator
// #define cx_hmap_stats
// #define cx_hmap_implement
// #include "cx_hmap2.h"
//
// // #define cx_hmap_name mapt2
// // #define cx_hmap_key int
// // #define cx_hmap_val double
// // #define cx_hmap_allocator
// // #define cx_hmap_implement
// // #include "cx_hmap.h"
//
// int main() {
//
//     cxHmapTests();
//
//     // map1 m1 = map1_init(NULL, 0);
//     // for (size_t i = 0; i < 13000; i++) {
//     //     map1_set(&m1, i, i*2);
//     // }
//     // map1_stats s = map1_get_stats(&m1);
//     // map1_print_stats(&s);
//     //cxAllocBlockTests();
//     //cxArrayTests();
//     //cxHmapTests();
//     //cxStrTests();
//     return 0;
// }
//
