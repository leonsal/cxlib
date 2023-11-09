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

uint32_t cxHashFNV1a32(const void *buf, size_t len) {

    const uint8_t *bp = buf;
    const uint8_t *be = bp + len;
    uint32_t hval = FNV_32_INIT;
    while (bp < be) {
        hval ^= (uint32_t)*bp++;
	    hval *= FNV_32_PRIME;
    }
    return hval;
}

