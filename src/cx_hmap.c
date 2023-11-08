#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Default key hash functions
size_t cxHmapHashKey(char* key, size_t keySize) {

    size_t hash = 0;
    for (size_t i = 0; i < keySize; i++) {
        hash = 31 * hash + key[i];
    }
    return hash;
}

// Default key comparison function
int cxHmapCmpKey(void* k1, void* k2, size_t size) {

    //printf("cmp:%p/%p/%lu\n", k1, k2, size);
    return memcmp(k1, k2, size);
}

