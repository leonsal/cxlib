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
#define cx_hmap_implement
#include "cx_hmap.h"
//
// #define cx_hmap_name mapt2
// #define cx_hmap_key int
// #define cx_hmap_val double
// #define cx_hmap_allocator
// #define cx_hmap_implement
// #include "cx_hmap.h"

int main() {

    map1 m1 = map1_init(NULL, 1);
    map1_set(&m1, 1, 2.0);
    map1_set(&m1, 2, 4.0);

    assert(*map1_get(&m1, 1) == 2.0);
    assert(*map1_get(&m1, 2) == 4.0);
    map1_free(&m1);
    //cxAllocBlockTests();
    //cxArrayTests();
    //cxHmapTests();
    //cxStrTests();
    return 0;
}

