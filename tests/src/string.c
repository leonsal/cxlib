#include <stdio.h>
#include <time.h>
#include "cx_alloc.h"
#include "cx_pool_allocator.h"

#include "util.h"

#define cx_str_name cxstr
#define cx_str_cap 8
#define cx_str_static
#define cx_str_error_handler(msg,func)\
    printf("CXSTR ERROR:%s at %s\n", msg, func);abort()
#define cx_str_instance_allocator
#define cx_str_implement
#include "cx_str.h"

#include "string.h"
#include "logger.h"

void cxStrTests(void) {

    // Use default allocator
    cxStrTest(cx_def_allocator()); 

   // Use pool allocator
    CxPoolAllocator* ba = cx_pool_allocator_create(4*1024, NULL);
    cxStrTest(cx_pool_allocator_iface(ba));
    cx_pool_allocator_destroy(ba);
}

void cxStrTest(const CxAllocator* alloc) {

    LOGI("strings. alloc=%p", alloc);

    // init
    cxstr s1 = cxstr_init(alloc);
    cxstr s2 = cxstr_init(alloc);
    CHK(cxstr_len(&s1) == 0);
    CHK(cxstr_cap(&s1) == 0);
    CHK(cxstr_empty(&s1));
    CHK(cxstr_cmp(&s1, "") == 0);

    // cpy
    cxstr_cpy(&s1, "");
    CHK(strlen(s1.data) == 0);
    CHK(cxstr_len(&s1) == 0);

    cxstr_cpyn(&s1, "123\0x00456", 7);
    CHK(strlen(s1.data) == 3);
    CHK(cxstr_len(&s1) == 7);
    cxstr_cpy(&s1, "hello áéíóú");
    CHK(cxstr_len(&s1) == strlen("hello áéíóú"));
    CHK(!cxstr_empty(&s1));
    CHK(cxstr_lencp(&s1) == 11);
    CHK(cxstr_lencp(&s1) != cxstr_len(&s1));
    CHK(cxstr_validu8(&s1));
    cxstr_cpys(&s2, &s1);
    CHK(cxstr_cmps(&s1, &s2) == 0);
    cxstr_free(&s1);
    cxstr_free(&s2);

    // case insentive cmp
    cxstr_cpy(&s1, "áéíóú");
    CHK(cxstr_cmp(&s1,"ÁÉÍÓÚ") != 0);
    CHK(cxstr_icmp(&s1,"ÁÉÍÓÚ") == 0);
    CHK(cxstr_icmp(&s1,"ÁÉÍÓú") == 0);
    cxstr_cpy(&s2, "ÁÉÍÓÚ");
    CHK(cxstr_cmps(&s1, &s2) != 0);
    CHK(cxstr_icmps(&s1, &s2) == 0);
    cxstr_cpy(&s2, "ÁÉÍÓú");
    CHK(cxstr_icmps(&s1, &s2) == 0);

    // cat
    cxstr_cpy(&s1, "hello");
    cxstr_cat(&s1, " áéíóú");
    CHK(cxstr_len(&s1) == strlen("hello áéíóú"));
    CHK(cxstr_cmp(&s1, "hello áéíóú") == 0);
    CHK(cxstr_validu8(&s1));

    // cat codepoint
    cxstr_free(&s1);
    int32_t codes[] = {225, 233, 237, 243, 250};
    for (int i = 0; i < 5; i++) {
        cxstr_catcp(&s1, codes[i]);
    }
    CHK(cxstr_cmp(&s1,"áéíóú") == 0);

    // ins
    cxstr_cpy(&s1, "áéíóú");
    cxstr_ins(&s1, "hello ", 0);
    CHK(cxstr_cmp(&s1, "hello áéíóú") == 0);
    CHK(cxstr_validu8(&s1));
    cxstr_cpy(&s2, " 123");
    cxstr_inss(&s1, &s2, 5);
    CHK(cxstr_cmp(&s1, "hello 123 áéíóú") == 0);
    CHK(cxstr_validu8(&s1));

    // del
    cxstr_deln(&s1, 5, 4);
    CHK(cxstr_cmp(&s1, "hello áéíóú") == 0);
    CHK(cxstr_validu8(&s1));

    // printf
    cxstr_clear(&s1);
    cxstr_printf(&s1, "x=%d y=%d", 1, 2);
    cxstr_printf(&s1, " z=%d", 3);
    cxstr_printf(&s1, " %s", "áéíóú");
    CHK(cxstr_cmp(&s1, "x=1 y=2 z=3 áéíóú") == 0);
    CHK(cxstr_validu8(&s1));

    // find
    cxstr_clear(&s1);
    cxstr_cpy(&s1, "012345678");
    CHK(cxstr_find(&s1, "23") == 2);
    CHK(cxstr_find(&s1, "23X") < 0);
    cxstr_cpy(&s2, "678");
    CHK(cxstr_finds(&s1, &s2) == 6);

    // insensitive case find
    cxstr_cpy(&s1, u8"áéíóú");
    CHK(cxstr_ifind(&s1, "ÓÚ") == 6); // byte index NOT codepoint index
    cxstr_cpy(&s2, "éÍó");
    CHK(cxstr_ifinds(&s1, &s2) == 2); // byte index NOT codepoint index

    // Find codepoint
    cxstr_cpy(&s1, "áéíóú");
    CHK(cxstr_findcp(&s1, 233) == 2); // é
    CHK(cxstr_findcp(&s1, 300) == -1);

    // substr
    cxstr_cpy(&s1, "abcdefghijklm");
    cxstr_substr(&s1, 3, 4, &s2);
    cxstr_cmp(&s2, "defg");
    cxstr_substr(&s1, 10, 100, &s2);
    cxstr_cmp(&s2, "klm");

    // upper/lower case
    cxstr_cpy(&s1, "áéíóúABCDE");
    cxstr_upper(&s1);
    CHK(cxstr_cmp(&s1, "ÁÉÍÓÚABCDE") == 0);
    cxstr_lower(&s1);
    CHK(cxstr_cmp(&s1, "áéíóúabcde") == 0);

    // iterate codepoints
    int32_t codepoint;
    cxstr_cpy(&s1, "áéíóú");
    int32_t expected[] = {225, 233, 237, 243, 250};
    int i = 0;
    for (char* iter = s1.data; (iter = cxstr_ncp(&s1, iter, &codepoint));) {
        CHK(codepoint == expected[i]);
        i++;
    }
    CHK(i == 5);

    // left trim
    cxstr_cpy(&s1, "áéíóúABCDE");
    cxstr_ltrim(&s1, "éáó");
    CHK(cxstr_cmp(&s1, "íóúABCDE") == 0);
    
    // right trim
    cxstr_cpy(&s1, "ABCDEáéíóú");
    cxstr_rtrim(&s1, "íúóá");
    CHK(cxstr_cmp(&s1, "ABCDEáé") == 0);

    // replace
    // cxstr_cpy(&s1, "01234567890123456789");
    // cxstr_replace(&s1, "567", "abcdefg", 0);
    cxstr_free(&s1);
    cxstr_cpy(&s1, "ABCD");
    cxstr_replace(&s1, "BC", "xxx", 0);
    CHK(cxstr_cmp(&s1, "AxxxD") == 0);
    cxstr_replace(&s1, "xxx", "", 0);
    CHK(cxstr_cmp(&s1, "AD") == 0);
    cxstr_cpy(&s1, "AB_AB_AB_AB");
    cxstr_replace(&s1, "AB", "C", 3);
    CHK(cxstr_cmp(&s1, "C_C_C_AB") == 0);

    cxstr_free(&s1);
    cxstr_free(&s2);
}


