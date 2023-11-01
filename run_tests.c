#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include "cx_alloc.h"
#include "cx_tests.h"

#define cx_str_cap 16
#define cx_str_name str16
#define cx_str_implement
#include "cx_str.h"

#define cx_str_cap 32
#define cx_str_name str32
#define cx_str_allocator
#define cx_str_implement
#include "cx_str.h"

int main() {

    str16 s1 = str16_init();
    str16_set(&s1, "hello");
    str16_set(&s1, "12345");
    str16_set(&s1, "1234");
    str16_set(&s1, "123456");
    str16_setn(&s1, "hello world", 4);
    str16 s2 = str16_init();
    str16_set_str(&s2, &s1);
    str16 cloned = str16_clone(&s1);


    //CxStr s1 = CxStr_init2(cxDefaultAllocator());
    // cxAllocBlockTests();
    // cxArrayTests();
    // cxHmapTests();
    return 0;
}


