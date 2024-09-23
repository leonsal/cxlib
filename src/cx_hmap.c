#include <stdlib.h>
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

// Utility function for freeing key/val "malloc" allocated C nul terminated string
void cx_hmap_free_str(char** str) {
    free(*str);
}

static size_t cx_hmap_prime_numbers[] = {
    5,
    11,
    23,
    53,
    97,
    193,
    389,
    769,
	1543,
	3079,
	6151,
	12289,
	24593,
	49157,
	98317,
	196613,
	393241,
	786433,
	1572869,
	3145739,
	6291469,
	12582917,
	25165843,
	50331653,
	100663319,
	201326611,
	402653189,
	805306457,
	1610612741,
    3221225549,
    6442451111,
    0,
};

// Returns 'n' if 'n' is prime or the next prime ~2*n
size_t cx_hmap_next_prime(size_t n) {

    size_t* curr = cx_hmap_prime_numbers;
    while (*curr < n) {
        curr++;
        if (*curr == 0) {
            return 2*n;
        }
    }
    return *curr;
}

