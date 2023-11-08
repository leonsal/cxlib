#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "cx_alloc.h"

#ifndef cx_hmap_internal
    #ifndef cx_hmap_name
        #error "cx_hmap_name not defined"
    #endif
    #ifndef cx_hmap_key
        #error "cx_hmap_key not defined"
    #endif
    #ifndef cx_hmap_val
        #error "cx_hmap_val not defined"
    #endif
#endif

#ifndef cx_hmap_def_nbuckets
    #define cx_hmap_def_nbuckets (17)
#endif

// Default key comparison function
#ifndef cx_hmap_cmp_key
    #define cx_hmap_cmp_key cxHmapCmpKey
#endif

// Default key hash function
#ifndef cx_hmap_hash_key
    #define cx_hmap_hash_key cxHmapHashKey
#endif

// Auxiliary internal macros
#define cx_hmap_concat2_(a, b) a ## b
#define cx_hmap_concat1_(a, b) cx_hmap_concat2_(a, b)
#define cx_hmap_name_(name) cx_hmap_concat1_(cx_hmap_name, name)

// API attributes
#if defined(cx_hmap_static) && defined(cx_hmap_inline)
    #define cx_hmap_api_ static inline
#elif defined(cx_hmap_static)
    #define cx_hmap_api_ static
#elif defined(cx_hmap_inline)
    #define cx_hmap_api_ inline
#else
    #define cx_hmap_api_
#endif

// Use custom instance allocator
#ifdef cx_hmap_allocator
    #define cx_hmap_alloc_field_\
        const CxAllocator* alloc;
    #define cx_hmap_alloc_global_
    #define cx_hmap_alloc_(s,n)\
        cx_alloc_alloc(s->alloc, n)
    #define cx_hmap_free_(s,p,n)\
        cx_alloc_free(s->alloc, p, n)
// Use global type allocator
#else
    #define cx_hmap_alloc_field_
    #define cx_hmap_alloc_global_\
        static const CxAllocator* cx_hmap_name_(_allocator) = NULL;
    #define cx_hmap_alloc_(s,n)\
        cx_alloc_alloc(cx_hmap_name_(_allocator),n)
    #define cx_hmap_free_(s,p,n)\
        cx_alloc_free(cx_hmap_name_(_allocator),p,n)
#endif

//
// Declarations
//

typedef struct cx_hmap_name_(_entry) {
    struct cx_hmap_name_(_entry)* next_;
    cx_hmap_key key;
    cx_hmap_val val;
} cx_hmap_name_(_entry);

typedef struct cx_hmap_name {
    cx_hmap_alloc_field_
    void*       userdata;
    size_t      entryCount;
    size_t      bucketCount;
    cx_hmap_name_(_entry)* buckets;
} cx_hmap_name;

typedef struct cx_hmap_name_(_iter) {
    size_t bucket;
    cx_hmap_name_(_entry)*  next_;
} cx_hmap_name_(_iter);

typedef enum {
    cx_hmap_op_set,
    cx_hmap_op_get,
    cx_hmap_op_del,
} cx_hmap_op;

#ifdef cx_array_allocator
    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init)(const CxAllocator* alloc, size_t nbuckets);
    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_clone)(cx_hmap_name* src, size_t nbuckets, const CxAllocator* alloc);
#else
    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init)(size_t nbuckets);
    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_clone)(cx_hmap_name* src, size_t nbuckets);
#endif
cx_hmap_api_ void cx_hmap_name_(_free)(cx_hmap_name* m);
cx_hmap_api_ void cx_hmap_name_(_set)(cx_hmap_name* m, cx_hmap_key k, cx_hmap_val v);
cx_hmap_api_ cx_hmap_val* cx_hmap_name_(_get)(cx_hmap_name* m, cx_hmap_key k);
cx_hmap_api_ bool cx_hmap_name_(_del)(cx_hmap_name* m, cx_hmap_key k);
cx_hmap_api_ size_t cx_hmap_name_(_count)(cx_hmap_name* m);
cx_hmap_api_ cx_hmap_name_(_entry)* cx_hmap_name_(_next)(cx_hmap_name* m, cx_hmap_name_(_iter)* iter);

// Declaration of generic implementation functions
// int cxHmapCmpKey(void* k1, void* k2, size_t size);
// size_t cxHmapHashKey(char* key, size_t keySize);
// void cxHmapFreeFn(CxHmapState *ms);
// void* cxHmapOperFn(CxHmapState* ms, cx_hmap_op op, void *key);
// void* cxHmapNext(const CxHmapState* ms, CxHmapIter* iter);

//
// Implementations
//
#ifdef cx_hmap_implement
    cx_hmap_alloc_global_;

    // Default key hash functions
    size_t cxHmapHashKey(char* key, size_t keySize);
    int cxHmapCmpKey(void* k1, void* k2, size_t size);

    // Creates a new entry and inserts it after specified parent
    cx_hmap_name_(_entry)* cx_hmap_name_(_add_entry_)(cx_hmap_name* m, cx_hmap_name_(_entry)* par, cx_hmap_key* key) {

        cx_hmap_name_(_entry)* new = cx_hmap_alloc_(m, sizeof(cx_hmap_name_(_entry)));
        memcpy(&new->key, key, sizeof(cx_hmap_key));
        new->next_ = NULL;
        par->next_ = new;
        m->entryCount++;
        return new;
    }

    cx_hmap_name_(_entry)* cx_hmap_name_(_oper_)(cx_hmap_name* m, cx_hmap_op op, cx_hmap_key* key) {

        if (m->buckets == NULL) {
            if (op == cx_hmap_op_get) {
                return NULL;
            }
            if (op == cx_hmap_op_del) {
                return NULL;
            }
            if (op == cx_hmap_op_set) {
                const size_t allocSize = m->bucketCount * sizeof(*m->buckets);
                m->buckets = cx_hmap_alloc_(m, allocSize);
                memset(m->buckets, 0, allocSize);
            }
        }
        // Hash the key, calculates the bucket index and get its pointer
        const size_t hash = cx_hmap_hash_key((char*)key, sizeof(cx_hmap_key));
        const size_t idx = hash % m->bucketCount;
        cx_hmap_name_(_entry)* e = m->buckets + idx * sizeof(*m->buckets);

        // If bucket next pointer is NULL, the bucket is empty
        if (e->next_ == NULL) {
            // For "Get" or "Del" returns NULL pointer indicating entry not found
            if (op == cx_hmap_op_get || op == cx_hmap_op_del) {
                return NULL;
            }
            memcpy(&e->key, key, sizeof(cx_hmap_key));
            e->next_ = e;
            m->entryCount++;
            return e;
        }

        // This bucket is used, checks its key
        if (cx_hmap_cmp_key(&e->key, &key, sizeof(cx_hmap_key)) == 0) {
            // For "Get" or "Set" just returns the pointer to this entry.
            if (op == cx_hmap_op_get || op == cx_hmap_op_set) {
                return e;
            }
            // For "Del" sets this bucket as empty
            // For string keys, free allocated key
            m->entryCount--;
            if (e == e->next_) {
                e->next_ = NULL;
                return e;
            }
            // Moves the first linked entry to the bucket area and
            // frees allocated link entry.
            cx_hmap_name_(_entry)* next = e->next_;
            memcpy(e, next, sizeof(cx_hmap_name_(_entry)));
            if (e->next_ == NULL) {
                e->next_ = e;
            }
            cx_hmap_free_(m, next, sizeof(cx_hmap_name_(_entry)));
            return e;
        }

        // If bucket next pointer is equal to itself, it contains single entry, returns NULL
        if (e == e->next_) {
            // For "Get" or "Del" just returns NULL pointer indicating entry not found.
            if (op == cx_hmap_op_get || op == cx_hmap_op_del) {
                return NULL;
            }
            // For "Set" adds first link to this bucket, returning its pointer
            return cx_hmap_name_(_add_entry_)(m, e, key);
        }

        // Checks the linked list of entries starting at this bucket.
        cx_hmap_name_(_entry)* prev = e;
        cx_hmap_name_(_entry)* curr = e->next_;
        size_t maxSearch = 1;
        while (curr != NULL) {
            maxSearch++;
            if (cx_hmap_cmp_key(&curr->key, key, sizeof(cx_hmap_key)) == 0) {
                // For "Get" or "Set" just returns the pointer
                if (op == cx_hmap_op_get) {
                    return curr;
                }
                if (op == cx_hmap_op_set) {
                    // if (maxSearch > m->maxSearch) {
                    //     m->maxSearch = maxSearch;
                    // }
                    return curr;
                }
                // For "Del" removes this entry from the linked list
                prev->next_ = curr->next_;
                if (prev == e && prev->next_ == NULL) {
                    e->next_ = e;
                }
                cx_hmap_free_(m, curr, sizeof(cx_hmap_name_(_entry)));
                m->entryCount--;
                return curr;
            }
            prev = curr;
            curr = curr->next_;
        }
        // Entry not found
        if (op == cx_hmap_op_get || op == cx_hmap_op_del) {
            return NULL;
        }
        // Adds new entry to this bucket at the end of the linked list and returns its pointer
        cx_hmap_name_(_entry)* new = cx_hmap_name_(_add_entry_)(m, prev, key);
        return new;
    }

#ifdef cx_hmap_allocator

    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init)(const CxAllocator* alloc, size_t nbuckets) {
        return (cx_hmap_name){
            .alloc = alloc == NULL ? cxDefaultAllocator() : alloc,
            .bucketCount = nbuckets == 0 ? cx_hmap_def_nbuckets : nbuckets,
            .entryCount = 0,
            .buckets = NULL,
        };
    }

    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_clone)(cx_hmap_name* src, size_t nbuckets, const CxAllocator* alloc) {

        cx_hmap_name dst = cx_hmap_name_(_init)(alloc == NULL ? src->s.alloc : alloc, nbuckets);
        cx_hmap_name_(_iter) iter = {0};
        while (true) {
            cx_hmap_name_(_entry)* e = cx_hmap_name_(_next)(src, &iter);
            if (e == NULL) {
                return dst;
            }
            cx_hmap_name_(_set)(&dst, e->key, e->val);
        }
        return dst;
    }
#else

    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init)(size_t nbuckets) {
        if (cx_hmap_name_(_allocator) == NULL) {
            cx_hmap_name_(_allocator) = cxDefaultAllocator();
        }
        return (cx_hmap_name){
            .bucketCount = nbuckets == 0 ? cx_hmap_def_nbuckets : nbuckets,
            .entryCount = 0,
            .buckets = NULL,
        };
    }

    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_clone)(cx_hmap_name* src, size_t nbuckets) {

        cx_hmap_name dst = cx_hmap_name_(_init)(nbuckets);
        cx_hmap_name_(_iter) iter = {0};
        while (true) {
            cx_hmap_name_(_entry)* e = cx_hmap_name_(_next)(src, &iter);
            if (e == NULL) {
                return dst;
            }
            cx_hmap_name_(_set)(&dst, e->key, e->val);
        }
        return dst;
    }

#endif

cx_hmap_api_ void cx_hmap_name_(_free)(cx_hmap_name* m) {

    if (m == NULL) {
         return;
    }
    for (size_t i = 0; i < m->bucketCount; i++) {
        cx_hmap_name_(_entry)* e = m->buckets + (i * sizeof(cx_hmap_name_(_entry)));
        // If bucket is empty or is a single entry, continue
        if (e->next_ == NULL) {
            continue;
        }
        // Free this bucket
        if (e->next_ == e) {
            e->next_ = NULL;
            continue;
        }
        // Free the linked list of entries
        cx_hmap_name_(_entry)* curr = e->next_;
        while (curr != NULL) {
            cx_hmap_name_(_entry)* prev = curr;
            curr = curr->next_;
            cx_hmap_free_(m, prev, sizeof(cx_hmap_name_(_entry)));
        }
        e->next_ = NULL;
    }
    cx_hmap_free_(m, m->buckets, m->bucketCount * sizeof(cx_hmap_name_(_entry)));
    m->entryCount = 0;
    m->buckets = NULL;
}

cx_hmap_api_ void cx_hmap_name_(_set)(cx_hmap_name* m, cx_hmap_key k, cx_hmap_val v) {

    cx_hmap_name_(_entry)* e = cx_hmap_name_(_oper_)(m, cx_hmap_op_set, &k);
    e->val = v;
}

cx_hmap_api_ cx_hmap_val* cx_hmap_name_(_get)(cx_hmap_name* m, cx_hmap_key k) {
    cx_hmap_name_(_entry)* e = cx_hmap_name_(_oper_)(m, cx_hmap_op_get, &k);
    return e == NULL ? NULL : &e->val;
}

cx_hmap_api_ bool cx_hmap_name_(_del)(cx_hmap_name* m, cx_hmap_key k) {
    cx_hmap_name_(_entry)* e = cx_hmap_name_(_oper_)(m, cx_hmap_op_del, &k);
    return e == NULL ? false : true;
}

cx_hmap_api_ size_t cx_hmap_name_(_count)(cx_hmap_name* m) {
    return m->entryCount;
}

cx_hmap_api_ cx_hmap_name_(_entry)* cx_hmap_name_(_next)(cx_hmap_name* m, cx_hmap_name_(_iter)* iter) {

    for (size_t i = iter->bucket; i < m->bucketCount; i++) {
        if (iter->next_ != NULL) {
            cx_hmap_name_(_entry)* e = iter->next_;
            iter->next_ = e->next_;
            if (iter->next_ == NULL) {
                iter->bucket++;
            }
            return e;
        }
        cx_hmap_name_(_entry)* e = m->buckets + (i * sizeof(*e));
        if (e->next_ == NULL) {
            iter->bucket++;
            continue;
        }
        if (e->next_ == e) {
            iter->bucket++;
            iter->next_ = NULL;
            return e;
        }
        iter->next_ = e->next_;
        return e;
    }
    return NULL;
}

#endif

// Undefine config  macros
#undef cx_hmap_name
#undef cx_hmap_key
#undef cx_hmap_val
#undef cx_hmap_implement

// Undefine internal macros
#undef cx_hmap_concat2_
#undef cx_hmap_concat1_
#undef cx_hmap_name_
#undef cx_hmap_api_



