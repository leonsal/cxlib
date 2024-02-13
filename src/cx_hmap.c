#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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

// Utility function for comparing C nul terminated strings
int cx_hmap_cmp_key_str(const void* k1, const void* k2, size_t size) {
    return strcmp(*(char**)k1, *(char**)k2);
}

// Utility function for hashing C nul terminated strings
size_t cx_hmap_hash_key_str(void* key, size_t keySize) {
    return cx_hmap_hash_fnv1a32(*((char**)key), strlen(*(char**)key));
}

// Utility function for freeing key/val "malloc" allocated C nul terminated string
void cx_hmap_free_str(char** str) {
    free(*str);
}



