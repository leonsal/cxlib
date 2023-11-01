#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include "cx_alloc.h"
#include "cx_tests.h"

#define cx_str_cap 16
//#define cx_str_allocator
#define cx_str_implement
#include "cx_str.h"


int main() {

    CxStr s1 = CxStr_init();
    //CxStr s1 = CxStr_init2(cxDefaultAllocator());
    // cxAllocBlockTests();
    // cxArrayTests();
    // cxHmapTests();
    return 0;
}


