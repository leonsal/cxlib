#include <stdio.h>
#include "alloc.h"
#include "array.h"
#include "hmap.h"
#include "string.h"

#define cx_hmap_name mapt1
#define cx_hmap_key int
#define cx_hmap_val double
#define cx_hmap_allocator
#define cx_hmap_implement
#include "cx_hmap.h"


int main() {

    mapt1 m1 = mapt1_init(NULL, 0);
    mapt1_set(&m1, 1, 2.0);
    mapt1_free(&m1);

    //cxAllocBlockTests();
    //cxArrayTests();
    //cxHmapTests();
    //cxStrTests();
    return 0;
}

