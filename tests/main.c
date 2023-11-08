#include <stdio.h>
#include "alloc.h"
#include "array.h"
#include "hmap.h"
#include "string.h"

#define cx_str_name str
#define cx_str_implement
#include "cx_str.h"

int main() {

    str a1 = str_init();
    str_cpy(&a1, "test");

    cxAllocBlockTests();
    cxArrayTests();
    cxHmapTests();
    cxStrTests();
    return 0;
}

