#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>

#define cx_hmap_name map
#define cx_hmap_key int
#define cx_hmap_val double
#define cx_hmap_allocator
#define cx_hmap_implement
#include "cx_hmap.h"
#include "hmap.h"

void cxHmapTests(void) {

    cxHmapTest(100, 40, NULL);
}

void cxHmapTest(size_t size, size_t nbuckets, const CxAllocator* alloc) {

    // Initializes map type 1 and sets entries
    map m1 = map_init(alloc, nbuckets);
    for (size_t  i = 0; i < size; i++) {
        map_set(&m1, i, i*2.0);
    }
    assert(map_count(&m1) == size);
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        assert(*map_get(&m1, i) == i * 2.0);
    }
    // Checks entries using iterator
    map_iter iter1 = {};
    size_t m1Count = 0;
    while (true) {
        map_entry* e = map_next(&m1, &iter1);
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
        map_set(&m1, i, i*3.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        assert(*map_get(&m1, i) == i * 3.0);
    }

    // Delete even keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2 == 0) {
            map_del(&m1, i);
        }
    }
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        if (i % 2 == 0) {
            assert(map_get(&m1, i) == NULL);
        }
        else {
            assert(*map_get(&m1, i) == i * 3.0);
        }
    }

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        map_set(&m1, i, i*4.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        assert(*map_get(&m1, i) == i * 4.0);
    }

    // Delete odd keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2) {
            map_del(&m1, i);
        }
    }
    // Checks entries
    for (size_t  i = 0; i < size; i++) {
        if (i % 2) {
            assert(map_get(&m1, i) == NULL);
        }
        else {
            assert(*map_get(&m1, i) == i * 4.0);
        }
    }

    // Overwrites all keys
    for (size_t i = 0; i < size; i++) {
        map_set(&m1, i, i*5.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        assert(*map_get(&m1, i) == i * 5.0);
    }

    // Clones map and checks entries of cloned map
    map m2 = map_clone(&m1, nbuckets*2);
    iter1 = (map_iter){};
    while (true) {
        map_entry* e = map_next(&m2, &iter1);
        if (e == NULL) {
            break;
        }
        assert(*map_get(&m2, e->key) == *map_get(&m1, e->key));
    }
    // Frees map 1
    map_free(&m1);
    assert(map_count(&m1) == 0);

    // Removes all the keys from map 2
    for (size_t  i = 0; i < size; i++) {
        map_del(&m2, i);
    }
    assert(map_count(&m2) == 0);
    // Fill again
    for (size_t i = 0; i < size; i++) {
        map_set(&m2, i, i*6.0);
    }
    // Checks entries directly
    for (size_t  i = 0; i < size; i++) {
        assert(*map_get(&m2, i) == i * 6.0);
    }


    map_free(&m2);
}

