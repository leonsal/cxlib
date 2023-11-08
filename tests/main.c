#include <stdio.h>
// #include "alloc.h"
// #include "array.h"
// #include "hmap.h"
// #include "string.h"

#define cx_str_name cxstr
#define cx_str_cap 8
#define cx_str_implement
#include "cx_str.h"

int main() {

    cxstr a1 = cxstr_init();
    cxstr_cpy(&a1, "test");
    cxstr_free(&a1);

    // cxAllocBlockTests();
    // cxArrayTests();
    // cxHmapTests();
    // cxStrTests();
    return 0;
}

