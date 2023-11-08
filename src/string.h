

void cxStrTests(void) {

    // Use default allocator
    cxStrTest(cxDefaultAllocator()); 

    // Use block allocator
    CxAllocBlock* ba = cxAllocBlockCreate(4*1024, NULL);
    cxStrTest(cxAllocBlockGetAllocator(ba));
    cxAllocBlockDestroy(ba);
}

void cxStrTest(const CxAllocator* alloc) {

    // init
#ifdef CX_STR_ALLOCATOR
    cxstr s1 = cxstr_init(alloc);
    cxstr s2 = cxstr_init(alloc);
#else
    cxstr_allocator = alloc;
    cxstr s1 = cxstr_initc(NULL);
    cxstr s2 = cxstr_initc(NULL);
#endif
    assert(cxstr_len(&s1) == 0);
    assert(cxstr_cap(&s1) == 0);
    assert(cxstr_empty(&s1));
    assert(cxstr_cmp(&s1, "") == 0);

    // cpy
    cxstr_cpyn(&s1, "123\0x00456", 7);
    assert(strlen(s1.data) == 3);
    assert(cxstr_len(&s1) == 7);
    cxstr_cpy(&s1, "hello áéíóú");
    assert(cxstr_len(&s1) == strlen("hello áéíóú"));
    assert(!cxstr_empty(&s1));
    assert(cxstr_lencp(&s1) == 11);
    assert(cxstr_lencp(&s1) != cxstr_len(&s1));
    assert(cxstr_validu8(&s1));
    cxstr_cpys(&s2, &s1);
    assert(cxstr_cmps(&s1, &s2) == 0);
    cxstr_free(&s1);
    cxstr_free(&s2);

    // case insentive cmp
    cxstr_cpy(&s1, "áéíóú");
    assert(cxstr_cmp(&s1,"ÁÉÍÓÚ") != 0);
    assert(cxstr_icmp(&s1,"ÁÉÍÓÚ") == 0);
    assert(cxstr_icmp(&s1,"ÁÉÍÓú") == 0);
    cxstr_cpy(&s2, "ÁÉÍÓÚ");
    assert(cxstr_cmps(&s1, &s2) != 0);
    assert(cxstr_icmps(&s1, &s2) == 0);
    cxstr_cpy(&s2, "ÁÉÍÓú");
    assert(cxstr_icmps(&s1, &s2) == 0);

    // cat
    cxstr_cpy(&s1, "hello");
    cxstr_cat(&s1, " áéíóú");
    assert(cxstr_len(&s1) == strlen("hello áéíóú"));
    assert(cxstr_cmp(&s1, "hello áéíóú") == 0);
    assert(cxstr_validu8(&s1));

    // cat codepoint
    cxstr_free(&s1);
    int32_t codes[] = {225, 233, 237, 243, 250};
    for (int i = 0; i < 5; i++) {
        cxstr_catcp(&s1, codes[i]);
    }
    assert(cxstr_cmp(&s1,"áéíóú") == 0);

    // ins
    cxstr_cpy(&s1, "áéíóú");
    cxstr_ins(&s1, "hello ", 0);
    assert(cxstr_cmp(&s1, "hello áéíóú") == 0);
    assert(cxstr_validu8(&s1));
    cxstr_cpy(&s2, " 123");
    cxstr_inss(&s1, &s2, 5);
    assert(cxstr_cmp(&s1, "hello 123 áéíóú") == 0);
    assert(cxstr_validu8(&s1));

    // del
    cxstr_deln(&s1, 5, 4);
    assert(cxstr_cmp(&s1, "hello áéíóú") == 0);
    assert(cxstr_validu8(&s1));

    // printf
    cxstr_clear(&s1);
    cxstr_printf(&s1, "x=%d y=%d", 1, 2);
    cxstr_printf(&s1, " z=%d", 3);
    cxstr_printf(&s1, " %s", "áéíóú");
    assert(cxstr_cmp(&s1, "x=1 y=2 z=3 áéíóú") == 0);
    assert(cxstr_validu8(&s1));

    // find
    cxstr_clear(&s1);
    cxstr_cpy(&s1, "012345678");
    assert(cxstr_find(&s1, "23") == 2);
    assert(cxstr_find(&s1, "23X") < 0);
    cxstr_cpy(&s2, "678");
    assert(cxstr_finds(&s1, &s2) == 6);

    // insensitive case find
    cxstr_cpy(&s1, u8"áéíóú");
    assert(cxstr_ifind(&s1, "ÓÚ") == 6); // byte index NOT codepoint index
    cxstr_cpy(&s2, "éÍó");
    assert(cxstr_ifinds(&s1, &s2) == 2); // byte index NOT codepoint index

    // Find codepoint
    cxstr_cpy(&s1, "áéíóú");
    assert(cxstr_findcp(&s1, 233) == 2); // é
    assert(cxstr_findcp(&s1, 300) == -1);

    // substr
    cxstr_cpy(&s1, "abcdefghijklm");
    cxstr_substr(&s1, 3, 4, &s2);
    cxstr_cmp(&s2, "defg");
    cxstr_substr(&s1, 10, 100, &s2);
    cxstr_cmp(&s2, "klm");

    // upper/lower case
    cxstr_cpy(&s1, "áéíóúABCDE");
    cxstr_upper(&s1);
    assert(cxstr_cmp(&s1, "ÁÉÍÓÚABCDE") == 0);
    cxstr_lower(&s1);
    assert(cxstr_cmp(&s1, "áéíóúabcde") == 0);

    // iterate codepoints
    int32_t codepoint;
    cxstr_cpy(&s1, "áéíóú");
    int32_t expected[] = {225, 233, 237, 243, 250};
    int i = 0;
    for (char* iter = s1.data; (iter = cxstr_ncp(&s1, iter, &codepoint));) {
        assert(codepoint == expected[i]);
        i++;
    }
    assert(i == 5);

    // left trim
    cxstr_cpy(&s1, "áéíóúABCDE");
    cxstr_ltrim(&s1, "éáó");
    assert(cxstr_cmp(&s1, "íóúABCDE") == 0);
    
    // right trim
    cxstr_cpy(&s1, "ABCDEáéíóú");
    cxstr_rtrim(&s1, "íúóá");
    assert(cxstr_cmp(&s1, "ABCDEáé") == 0);

    // replace
    // cxstr_cpy(&s1, "01234567890123456789");
    // cxstr_replace(&s1, "567", "abcdefg", 0);
    cxstr_free(&s1);
    cxstr_cpy(&s1, "ABCD");
    cxstr_replace(&s1, "BC", "xxx", 0);
    assert(cxstr_cmp(&s1, "AxxxD") == 0);
    cxstr_replace(&s1, "xxx", "", 0);
    assert(cxstr_cmp(&s1, "AD") == 0);
    cxstr_cpy(&s1, "AB_AB_AB_AB");
    cxstr_replace(&s1, "AB", "C", 3);
    assert(cxstr_cmp(&s1, "C_C_C_AB") == 0);

    cxstr_free(&s1);
    cxstr_free(&s2);
}



