#include <stdio.h>
#include <assert.h>
#include "alloc.h"
#include "array.h"
#include "hmap.h"
#include "string.h"

int main() {

    cxHmapTests();

    // map1 m1 = map1_init(NULL, 0);
    // for (size_t i = 0; i < 13000; i++) {
    //     map1_set(&m1, i, i*2);
    // }
    // map1_stats s = map1_get_stats(&m1);
    // map1_print_stats(&s);
    //cxAllocBlockTests();
    //cxArrayTests();
    //cxHmapTests();
    //cxStrTests();
    return 0;
}

