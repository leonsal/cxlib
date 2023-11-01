#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include "cx_alloc.h"
#include "cx_tests.h"

#define cx_str_cap 16
#define cx_str_name str16
#define cx_str_implement
#include "cx_str.h"

// #define cx_str_cap 32
// #define cx_str_name str32
// #define cx_str_allocator
// #define cx_str_implement
// #include "cx_str.h"

int main() {

    str16 s1 = str16_init();
    str16 s2 = str16_initc("1234");
    str16 s3 = str16_initn("1234",2);
    str16 s4 = str16_inits(&s1);

    str16_setc(&s1, "hello");
    str16_setc(&s1, "12345");
    str16_setc(&s1, "1234");
    str16_setc(&s1, "123456");
    str16_setn(&s1, "hello world", 4);

    str16_sets(&s2, &s1);

    // str16 s4 = str16_init();
    // str16_cat(&s4, "1234");
    // str16_cat(&s4, "5678");
    // str16_catn(&s4, "901", 3);
    // str16_cats(&s4, &s4);
    //
    // str16 s5 = str16_init();
    // str16_set(&s5, "012345");
    // str16_ins(&s5, "abc", 2);
    // str16_ins(&s5, "6789", 9);
    // //str16_inss(&s5, &s5, 0);
    //
    //
    // str16_free(&s1);
    // str16_free(&s2);
    // str16_free(&s3);
    // str16_free(&s4);
    //CxStr s1 = CxStr_init2(cxDefaultAllocator());
    // cxAllocBlockTests();
    // cxArrayTests();
    // cxHmapTests();
    return 0;
}


