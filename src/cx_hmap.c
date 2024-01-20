#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>


// // Default key hash functions
// uint32_t cxHmapHashKey(void* key, size_t keySize) {
//
//     uint8_t* pkey = key;
//     uint32_t hash = 0;
//     for (size_t i = 0; i < keySize; i++) {
//         hash = 31 * hash + pkey[i];
//     }
//     return hash;
// }

// FNV1-a hash function for 32 bit hashes.
#define FNV_32_INIT     ((uint32_t)0x811c9dc5)
#define FNV_32_PRIME    ((uint32_t)0x01000193)

uint32_t cx_hmap_hash_fnv1a32(const void *buf, size_t len) {

    const uint8_t *bp = buf;
    const uint8_t *be = bp + len;
    uint32_t hval = FNV_32_INIT;
    while (bp < be) {
        hval ^= (uint32_t)*bp++;
	    hval *= FNV_32_PRIME;
    }
    return hval;
}

int cx_hmap_cmp_key_str_arr(const void* k1, const void* k2, size_t size) {
    return strcmp(k1, k2);
}

size_t cx_hmap_hash_key_str_arr(void* key, size_t keySize) {
    return cx_hmap_hash_fnv1a32(key, strlen((char*)key));
}

int cx_hmap_cmp_key_str_ptr(const void* k1, const void* k2, size_t size) {
    return strcmp(*(char**)k1, *(char**)k2);
}

size_t cx_hmap_hash_key_str_ptr(void* key, size_t keySize) {
    return cx_hmap_hash_fnv1a32(*((char**)key), strlen(*(char**)key));
}

