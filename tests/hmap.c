#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>

#define cx_hmap_name mapt1
#define cx_hmap_key int
#define cx_hmap_val double
#define cx_hmap_allocator
#define cx_hmap_implement
#include "cx_hmap.h"

#define cx_hmap_name mapt2
#define cx_hmap_key const char*
#define cx_hmap_val double
#define cx_hmap_allocator
#define cx_hmap_implement
#include "cx_hmap.h"

#include "hmap.h"

void cxHmapTests(void) {

    cxHmapTest(10000, 1000/4, NULL);
}

void cxHmapTest(size_t size, size_t nbuckets, const CxAllocator* alloc) {

    // Size must be greater than this value for tests
    assert(size >= 100);

    //
    // map type 1: int -> double
    //
    {
        // Initializes map type 1 and sets entries
        mapt1 m1 = mapt1_init(alloc, nbuckets);
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
            //fprintf(stderr, "k:%u v:%f\n", e->key, e->val);
        }
        assert(m1Count == size);

        // Overwrites all keys
        for (size_t i = 0; i < size; i++) {
            mapt1_set(&m1, i, i*3.0);
        }
        // Checks entries directly
        for (size_t  i = 0; i < size; i++) {
            assert(*mapt1_get(&m1, i) == i * 3.0);
        }

        // Delete even keys
        for (size_t i = 0; i < size; i++) {
            if (i % 2 == 0) {
                mapt1_del(&m1, i);
            }
        }
        // Checks entries
        for (size_t  i = 0; i < size; i++) {
            if (i % 2 == 0) {
                assert(mapt1_get(&m1, i) == NULL);
            }
            else {
                assert(*mapt1_get(&m1, i) == i * 3.0);
            }
        }

        // Overwrites all keys
        for (size_t i = 0; i < size; i++) {
            mapt1_set(&m1, i, i*4.0);
        }
        // Checks entries directly
        for (size_t  i = 0; i < size; i++) {
            assert(*mapt1_get(&m1, i) == i * 4.0);
        }

        // Delete odd keys
        for (size_t i = 0; i < size; i++) {
            if (i % 2) {
                mapt1_del(&m1, i);
            }
        }
        // Checks entries
        for (size_t  i = 0; i < size; i++) {
            if (i % 2) {
                assert(mapt1_get(&m1, i) == NULL);
            }
            else {
                assert(*mapt1_get(&m1, i) == i * 4.0);
            }
        }

        // Overwrites all keys
        for (size_t i = 0; i < size; i++) {
            mapt1_set(&m1, i, i*5.0);
        }
        // Checks entries directly
        for (size_t  i = 0; i < size; i++) {
            assert(*mapt1_get(&m1, i) == i * 5.0);
        }

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
        mapt2 m1 = mapt2_init(alloc, nbuckets);
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

