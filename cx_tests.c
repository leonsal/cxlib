#include <stdio.h>
#include <assert.h>
#include <time.h>
#include "cx_tests.h"

// For cxstr error handler
static const char* cxstr_error = NULL;

#define cx_array_name arri32
#define cx_array_type int
#define cx_array_implement
#define cx_array_static
#include "cx_array.h"

#define cx_hmap_name mapt1
#define cx_hmap_key int
#define cx_hmap_val double
#define cx_hmap_implement
#include "cx_hmap.h"

#define cx_hmap_name mapt2
#define cx_hmap_key const char*
#define cx_hmap_val double
#define cx_hmap_implement
#include "cx_hmap.h"

//#define CX_STR_ALLOCATOR
#define cx_str_name cxstr
#define cx_str_cap 8
#define cx_str_static
#define cx_str_error_handler(msg)\
    cxstr_error=msg
#ifdef CX_STR_ALLOCATOR
    #define cx_str_allocator
#endif
#define cx_str_implement
#include "cx_str.h"

#include "cx_alloc_block.h"

static int sort_int_desc(const int* v1, const int* v2) {
    return *v2 > *v1;
}


void cxTests(void) {

    cxAllocBlockTests();
    cxArrayTests();
    cxHmapTests();
    cxStrTests();
}

void cxAllocBlockTests(void) {

    cxAllocBlockTest(100, 4*1024);
}

void cxAllocBlockTest(size_t allocs, size_t blockSize) {

    // Allocation group
    typedef struct Group {
        int     start;  // start value of the first int
        size_t  count;  // number of ints allocated in the group
        int*    p;      // pointer to first int
        struct Group* next;
    } Group;

    Group* groups = NULL;
    CxAllocBlock* a0 = cxAllocBlockCreate(blockSize, NULL);
    CxAllocBlock* a1 = cxAllocBlockCreate(blockSize, NULL);
    CxAllocBlock* a2 = cxAllocBlockCreate(blockSize, NULL);
    srand(time(NULL));
    size_t start = 0;

    for (size_t an = 0; an < allocs; an++) {
        // Random number of ints to allocate
        size_t count = rand() % 1000;
        // Choose arena 1 or arena 2
        CxAllocBlock* a = a2;
        if (an % 2) {
            a = a1;
        }
        // Creates group and adds to the linked list of groups
        Group* g = cxAllocBlockAlloc(a0, sizeof(Group));
        *g = (Group){
            .start = start,
            .count = count,
            .p = cxAllocBlockAlloc(a, count * sizeof(int)),
        };
        g->next = groups;
        groups = g;
        // Initialize group data
        for (size_t idx = 0; idx < count; idx++) {
            g->p[idx] = start++;
        }
    }

    // Checks data in all groups
    Group* curr = groups;
    while (curr != NULL) {
        for (size_t i = 0 ; i < curr->count; i++) {
            if (curr->p[i] != curr->start + i) {
                printf("ERROR\n");
                abort();
            }
        }
        curr = curr->next;
    }

    cxAllocBlockDestroy(a0);
    cxAllocBlockDestroy(a1);
    cxAllocBlockDestroy(a2);
}

void cxArrayTests(void) {

    // Used default allocator
    const size_t size = 100000;
    cxArrayTest(size, cxDefaultAllocator());

    // Use block allocator
    CxAllocBlock* ba = cxAllocBlockCreate(4*1024, NULL);
    cxArrayTest(size, cxAllocBlockGetAllocator(ba));
    cxAllocBlockDestroy(ba);
}

void cxArrayTest(size_t size, const CxAllocator* alloc) {

    // Fill array with push
    arri32 a1 = arri32_init2(alloc);
    assert(arri32_len(&a1) == 0);
    assert(arri32_cap(&a1) == 0);
    assert(arri32_empty(&a1));
    for (size_t i = 0; i < size; i++) {
        arri32_push(&a1, i);
    }
    assert(arri32_len(&a1) == size);
    assert(!arri32_empty(&a1));
    assert(arri32_last(&a1) == size-1);
    // Check
    for (size_t i = 0; i < size; i++) {
        assert(*arri32_at(&a1, i) == i);
        assert(a1.data[i] == i);
    }
    // Pop
    for (size_t i = 0; i < size; i++) {
        assert(arri32_pop(&a1) == size-i-1);
    }
    arri32_free(&a1);
    assert(arri32_len(&a1) == 0);
    assert(arri32_cap(&a1) == 0);

    // Set length and fill array
    arri32_setlen(&a1, size);
    assert(arri32_len(&a1) == size);
    assert(!arri32_empty(&a1));
    for (size_t i = 0; i < size; i++) {
        a1.data[i] = i * 2;
    }
    // Check
    for (size_t i = 0; i < size; i++) {
        assert(*arri32_at(&a1, i) == i*2);
        assert(a1.data[i] == i*2);
    }
    // Clone and check
    arri32 a2 = arri32_clone(&a1);
    assert(arri32_len(&a2) == arri32_len(&a1));
    for (size_t i = 0; i < size; i++) {
        assert(*arri32_at(&a2, i) == *arri32_at(&a1, i));
        assert(a2.data[i] == a1.data[i]);
    }
    arri32_free(&a1);
    assert(arri32_len(&a1) == 0);
    assert(arri32_cap(&a1) == 0);
    arri32_free(&a2);
    assert(arri32_len(&a2) == 0);
    assert(arri32_cap(&a2) == 0);
    
    // Set capacity and fill array
    arri32_setcap(&a1, size);
    assert(arri32_len(&a1) == 0);
    assert(arri32_cap(&a1) == size);
    assert(arri32_empty(&a1));
    for (size_t i = 0; i < size; i++) {
        arri32_push(&a1, i*3);
    }
    assert(arri32_len(&a1) == size);
    assert(arri32_cap(&a1) == size);
    for (size_t i = 0; i < size; i++) {
        assert(*arri32_at(&a1, i) == i*3);
        assert(a1.data[i] == i*3);
    }
    arri32_clear(&a1);
    assert(arri32_len(&a1) == 0);
    assert(arri32_cap(&a1) == size);
    arri32_free(&a1);
    assert(arri32_cap(&a1) == 0);

    // Inserts space for data
    arri32_push(&a1, 100);
    arri32_insn(&a1, 0, 5);
    assert(arri32_len(&a1) == 1+5);
    assert(a1.data[5] == 100);
    arri32_free(&a1);

    // Appends data
    int buf[] = {1,2,3,4,5,6,7,8,9};
    size_t buf_size = sizeof(buf)/sizeof(int);
    arri32_push(&a1, 0);
    arri32_append(&a1, buf, buf_size);
    assert(arri32_len(&a1) == buf_size+1);
    for (size_t i = 0; i < buf_size+1; i++) {
        assert(*arri32_at(&a1, i) == i);
        assert(a1.data[i] == i);
    }
    // Sorts data in descending order
    arri32_sort(&a1, sort_int_desc);
    for (size_t i = 0; i < buf_size+1; i++) {
        assert(*arri32_at(&a1, i) == buf_size-i);
        assert(a1.data[i] == buf_size-i);
    }
    arri32_free(&a1);

    // Appends from another array
    arri32_push(&a1, 100);
    arri32_push(&a2, 0);
    arri32_push(&a2, 1);
    arri32_append_array(&a1, &a2);
    assert(arri32_len(&a1) == 3);
    assert(a1.data[0] == 100 && a1.data[1] == 0 && a1.data[2] == 1);
    arri32_free(&a1);
    arri32_free(&a2);

}

void cxHmapTests(void) {

    cxHmapTest(50, 50/2, NULL);
}

void cxHmapTest(size_t size, size_t nbuckets, const CxAllocator* alloc) {

    //
    // map type 1: int -> double
    //
    {
        // Initializes map type 1 and sets entries
        mapt1 m1 = mapt1_init2(nbuckets, alloc);
        for (size_t  i = 0; i < size; i++) {
            mapt1_set(&m1, i, i*2.0);
        }
        assert(mapt1_count(&m1) == size);

        // Checks entries directly
        for (size_t  i = 0; i < size; i++) {
            assert(*mapt1_get(&m1, i) == i * 2.0);
        }

        // Checks entries using iterator
        mapt1_iter iter1 = {};
        size_t m1Count = 0;
        while (true) {
            mapt1_entry* e = mapt1_next(&m1, &iter1);
            if (e == NULL) {
                break;
            }
            m1Count++;
            assert(e->val == e->key * 2.0);
            //printf("k:%u v:%f\n", e->key, e->val);
        }
        assert(m1Count == size);

        // Clones map and checks entries of cloned map
        mapt1 m2 = mapt1_clone(&m1, nbuckets*2, NULL);
        iter1 = (mapt1_iter){};
        while (true) {
            mapt1_entry* e = mapt1_next(&m2, &iter1);
            if (e == NULL) {
                break;
            }
            assert(*mapt1_get(&m2, e->key) == *mapt1_get(&m1, e->key));
        }
        // Frees map 1
        mapt1_free(&m1);
        assert(mapt1_count(&m1) == 0);

        // Removes all the keys from map 2
        for (size_t  i = 0; i < size; i++) {
            mapt1_del(&m2, i);
        }
        assert(mapt1_count(&m2) == 0);
        mapt1_free(&m2);
    }

    //
    // map type 2: const char* -> double
    //
    {
        // Initializes map type 2 and sets entries
        const char* keys[] = {
            "0","1","2","3","4","5","6","7","8","9",
            "10","11","12","13","14","15","16","17","18","19",
            "20","21","22","23","24","25","26","27","28","29",
            "30","31","32","33","34","35","36","37","38","39",
        };
        const size_t keyCount = sizeof(keys)/sizeof(const char*);
        mapt2 m1 = mapt2_init2(nbuckets, alloc);
        for (size_t  i = 0; i < keyCount; i++) {
            mapt2_set(&m1, keys[i], atof(keys[i]) * 2);
        }
        assert(mapt2_count(&m1) == keyCount);

        // Checks entries directly
        for (size_t  i = 0; i < mapt2_count(&m1); i++) {
            assert(*mapt2_get(&m1, keys[i]) == i * 2.0);
        }

        // Checks entries using iterator
        mapt2_iter iter = {};
        size_t count = 0;
        while (true) {
            mapt2_entry* e = mapt2_next(&m1, &iter);
            if (e == NULL) {
                break;
            }
            count++;
            //printf("%s: %f\n", e->key, e->val);
            assert(e->val == atof(e->key) * 2.0);
        }
        //printf("%lu / %lu\n", count, keyCount);
        assert(count == keyCount);

        // Clones map and checks entries of cloned map
        mapt2 m2 = mapt2_clone(&m1, nbuckets*2, NULL);
        iter = (mapt2_iter){};
        while (true) {
            mapt2_entry* e = mapt2_next(&m2, &iter);
            if (e == NULL) {
                break;
            }
            assert(*mapt2_get(&m2, e->key) == *mapt2_get(&m1, e->key));
        }
        // Frees map 1
        mapt2_free(&m1);
        assert(mapt2_count(&m1) == 0);

        // Removes all the keys from map 2
        for (size_t  i = 0; i < keyCount; i++) {
            mapt2_del(&m2, keys[i]);
        }
        assert(mapt2_count(&m2) == 0);
        mapt2_free(&m2);
    }
}

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

    // checks index error
    assert(cxstr_error == NULL);
    cxstr_ins(&s1, "error", cxstr_len(&s1)+1);
    assert(cxstr_error != NULL);
    cxstr_error = NULL;

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


