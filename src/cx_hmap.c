#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

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

    return memcmp(k1, k2, size);
}

// #define cx_hmap_internal
// #include "cx_hmap.h"
//
// // Uses the supplied map allocator to allocated memory
// #define map_alloc(m,n)   m->s.alloc->alloc(m->s.alloc->ctx,(n))
//
// // Uses the supplied map allocator to free previous allocate memory
// #define map_free(m,p,n)  m->s.alloc->free(m->s.alloc->ctx,(p),(n))
//
// // Generic map state
// typedef struct hmap {
//     CxHmapState s;
//     void *buckets;
// } hmap;
//
// // Generic Bucket/entry header
// typedef struct EntryHeader EntryHeader;
// typedef struct EntryHeader{
//     EntryHeader*    next;   // NULL|pointer to itself|pointer to next entry
//     char            key[];  // entry key
// } EntryHeader;
//
//
// // Forward declaration of local functions
// static void cxHmapSetEntryKey(hmap* m, EntryHeader* h, void* key);
// static EntryHeader* cxHmapAddEntryLink(hmap* m, EntryHeader* par, char* key);
//
//
// void* cxHmapOperFn(CxHmapState* ms, cx_hmap_op op, void *key) {
//
//     hmap* m = (hmap*)ms;
//     if (m->buckets == NULL) {
//         if (op == cx_hmap_op_get) {
//             return NULL;
//         }
//         if (op == cx_hmap_op_del) {
//             return NULL;
//         }
//         if (op == cx_hmap_op_set) {
//             const size_t allocSize = m->s.bucketCount * m->s.entrySize;
//             m->buckets = map_alloc(m, allocSize);
//             memset(m->buckets, 0, allocSize);
//         }
//     }
//
//     // Hash the key, calculates the bucket index and get its pointer
//     const size_t hash = cxHmapHashKey((char*)key, m->s.keySize);
//     const size_t idx = hash % m->s.bucketCount;
//     EntryHeader* h = (EntryHeader*)(m->buckets + idx * m->s.entrySize);
//
//     // If bucket next pointer is NULL, the bucket is empty
//     if (h->next == NULL) {
//         // For "Get" or "Del" returns NULL pointer indicating entry not found
//         if (op == cx_hmap_op_get || op == cx_hmap_op_del) {
//             return NULL;
//         }
//         cxHmapSetEntryKey(m, h, key);
//         h->next = h;
//         m->s.entryCount++;
//         return h;
//      }
//
//     // This bucket is used, checks its key
//     if (cx_hmap_cmp_key(h->key, key, m->s.keySize) == 0) {
//         // For "Get" or "Set" just returns the pointer to this entry.
//         if (op == cx_hmap_op_get || op == cx_hmap_op_set) {
//             return h;
//         }
//         // For "Del" sets this bucket as empty
//         // For string keys, free allocated key
//         m->s.entryCount--;
//         if (h == h->next) {
//             h->next = NULL;
//             return h;
//         }
//         // Moves the first linked entry to the bucket area and
//         // frees allocated link entry.
//         EntryHeader* next = h->next;
//         memcpy(h, next, m->s.entrySize);
//         if (h->next == NULL) {
//             h->next = h;
//         }
//         map_free(m, next, sizeof(m->s.entrySize));
//         return h;
//     }
//
//     // If bucket next pointer is equal to itself, it contains single entry, returns NULL
//     if (h == h->next) {
//         // For "Get" or "Del" just returns NULL pointer indicating entry not found.
//         if (op == cx_hmap_op_get || op == cx_hmap_op_del) {
//             return NULL;
//         }
//         // For "Set" adds first link to this bucket, returning its pointer
//         return cxHmapAddEntryLink(m, h, key);
//     }
//
//     // Checks the linked list of entries starting at this bucket.
//     EntryHeader* prev = h;
//     EntryHeader* curr = h->next;
//     size_t maxSearch = 1;
//     while (curr != NULL) {
//         maxSearch++;
//         if (cxHmapCmpKey(curr->key, key, m->s.keySize) == 0) {
//             // For "Get" or "Set" just returns the pointer
//             if (op == cx_hmap_op_get) {
//                 return curr;
//             }
//             if (op == cx_hmap_op_set) {
//                 // if (maxSearch > m->maxSearch) {
//                 //     m->maxSearch = maxSearch;
//                 // }
//                 return curr;
//             }
//             // For "Del" removes this entry from the linked list
//             prev->next = curr->next;
//             if (prev == h && prev->next == NULL) {
//                 h->next = h;
//             }
//             map_free(m, curr, sizeof(m->s.entrySize));
//             m->s.entryCount--;
//             return curr;
//         }
//         prev = curr;
//         curr = curr->next;
//     }
//     // Entry not found
//     if (op == cx_hmap_op_get || op == cx_hmap_op_del) {
//         return NULL;
//     }
//     // Adds new entry to this bucket at the end of the linked list and returns its pointer
//     EntryHeader* new = cxHmapAddEntryLink(m, prev, key);
//     return new;
// }
//
// // Frees all allocated memory and resets the map
// void cxHmapFreeFn(CxHmapState *ms) {
//
//     hmap* m = (hmap*)ms;
//     if (m == NULL) {
//          return;
//     }
//     for (int i = 0; i < m->s.bucketCount; i++) {
//         EntryHeader* h = (EntryHeader*)(m->buckets + i * m->s.entrySize);
//         // If bucket is empty or is a single entry, continue
//         if (h->next == NULL) {
//             continue;
//         }
//         // Free this bucket
//         if (h->next == h) {
//             h->next = NULL;
//             continue;
//         }
//         // Free the linked list of entries
//         EntryHeader* curr = h->next;
//         while (curr != NULL) {
//             EntryHeader* prev = curr;
//             curr = curr->next;
//             map_free(m, prev, m->s.entrySize);
//         }
//         h->next = NULL;
//     }
//     map_free(m, m->buckets, m->s.bucketCount * m->s.entrySize);
//     m->s.entryCount = 0;
//     m->buckets = NULL;
// }
//
// // Returns pointer to the next entry in the hashmap or NULL if
// // there are no more entries.
// // The iterator must be zero initialized before the first call.
// void* cxHmapNext(const CxHmapState* ms, CxHmapIter* iter) {
//
//     hmap* m = (hmap*)ms;
//     for (size_t i = iter->bucket; i < m->s.bucketCount; i++) {
//         if (iter->next != NULL) {
//             EntryHeader* e = iter->next;
//             iter->next = e->next;
//             if (iter->next == NULL) {
//                 iter->bucket++;
//             }
//             return e;
//         }
//         EntryHeader* e = (EntryHeader*)(m->buckets + i * m->s.entrySize);
//         if (e->next == NULL) {
//             iter->bucket++;
//             continue;
//         }
//         if (e->next == e) {
//             iter->bucket++;
//             iter->next = NULL;
//             return e;
//         }
//         iter->next = e->next;
//         return e;
//     }
//     return NULL;
// }
//
// // Default key comparison function
// int cxHmapCmpKey(void* k1, void* k2, size_t size) {
//
//     return memcmp(k1, k2, size);
// }
//
// // Key comparison function for C strings
// int cxHmapCmpKeyStr(void* k1, void* k2, size_t size) {
//
//     return strcmp(k1, k2);
// }
//
// // Sets the key in the specified entry.
// static void cxHmapSetEntryKey(hmap* m, EntryHeader* h, void* key) {
//
//     memcpy(h->key, key, m->s.keySize);
// }
//
// // Default key hash functions
// size_t cxHmapHashKey(char* key, size_t keySize) {
//
//     size_t hash = 0;
//     for (size_t i = 0; i < keySize; i++) {
//         hash = 31 * hash + key[i];
//     }
//     return hash;
// }
//
// // Creates a new entry and inserts it after specified parent
// static EntryHeader* cxHmapAddEntryLink(hmap* m, EntryHeader* par, char* key) {
//
//     EntryHeader* new = m->s.alloc->alloc(m->s.alloc->ctx, m->s.entrySize);
//     cxHmapSetEntryKey(m, new, key);
//     new->next = NULL;
//     par->next = new;
//     m->s.entryCount++;
//     return new;
// }
