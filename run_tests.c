#include "cx_tests.h"

#define cx_str_name str
#include "cx_str.h"

#define cx_str_name str8
#define cx_str_cap 8
#include "cx_str.h"

int main() {

    cxStrTests();
    return 0;
}


// #include <stdint.h>
// #include <assert.h>
// #include <stdio.h>
// #include "cx_alloc.h"
// #include "cx_tests.h"
//
// #define cx_str_cap 16
// #define cx_str_name str16
// #define cx_str_implement
// #include "cx_str.h"
//
// #define cx_str_cap 32
// #define cx_str_name str32
// #define cx_str_allocator
// #define cx_str_implement
// #include "cx_str.h"
//
//
// int main() {
//
//     str16_allocator = NULL;
//     str16 s1 = str16_init();
//     str16 s2 = str16_initc("1234");
//     assert(str16_cmpc(&s2, "1234") == 0);
//
//     str16 s3 = str16_initn("1234", 2);
//     assert(str16_cmpn(&s3, "1234", 2) == 0);
//
//     str16 s4 = str16_inits(&s1);
//     assert(str16_cmps(&s4, &s1) == 0);
//
//     str16_setc(&s1, "hello");
//     str16_setc(&s1, "12345");
//     str16_setc(&s1, "1234");
//     str16_setc(&s1, "123456");
//     str16_setn(&s1, "hello world", 4);
//     str16_reserve(&s1, 2);
//
//     str16_sets(&s2, &s1);
//
//     str16_catc(&s4, "1234");
//     str16_catc(&s4, "5678");
//     str16_catn(&s4, "901", 3);
//     str16_cats(&s4, &s1);
//
//     str16_deln(&s4, 3, 2);
//     
//     str16 s5 = str16_init();
//     str16_setc(&s5, "012345");
//     str16_insc(&s5, "abc", 3);
//     str16_insc(&s5, "6789", 9);
//     str16_findn(&s5, "12", 2);
//
//
//     str16_printf(&s1, "%d %f TEXT\n", 12, 34.34);
//
//     str32 s10 = str32_initc(cxDefaultAllocator(), "XYZ");
//
//     str16_free(&s1);
//     str16_free(&s2);
//     str16_free(&s3);
//     str16_free(&s4);
//     str16_free(&s5);
//     str32_free(&s10);
//
//     //CxStr s1 = CxStr_init2(cxDefaultAllocator());
//     // cxAllocBlockTests();
//     // cxArrayTests();
//     // cxHmapTests();
//     return 0;
// }
//
//
