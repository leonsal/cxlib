#include <stdio.h>
#include "alloc.h"
#include "array.h"
#include "hmap.h"
#include "string.h"

#define cx_hmap_name map
#define cx_hmap_key int
#define cx_hmap_val double
#define cx_hmap_implement
#include "cx_hmap.h"

int main() {

    //cxAllocBlockTests();
    //cxArrayTests();
    cxHmapTests();
    //cxStrTests();
    return 0;
}

