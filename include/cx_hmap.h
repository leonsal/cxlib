/*
Hashmap Implementation
----------------------
- Uses chaining.
- Uses global type allocator initialized with default allocator.
- Can be configured to use custom allocator per instance.

Example
-------

#include <stdio.h>
#include <assert.h>
#define cx_hmap_name map
#define cx_hmap_key int
#define cx_hmap_val double
#define cx_hmap_implement
#include "cx_hmap.h"

int main() {

    // Optionally sets this map type allocator:
    //map_allocator = cxDefaultAllocator();

    // Initialize map with default number of buckets
    map m1 = map_init(0);

    // Set keys and values
    size_t size = 100;
    for (size_t i = 0; i < size; i++) {
        map_set(&m1, i, i * 2.0);
    }
    assert(map_count(&m1) == size);

    // Get keys and values
    for (size_t i = 0; i < size; i++) {
        assert(*map_get(&m1, i) == i * 2.0);
    }

    // Iterate over keys and values
    map_iter iter = {0};
    map_entry* e = NULL;
    while ((e = map_next(&m1, &iter)) != NULL) {
        printf("key:%d val:%f\n", e->key, e->val);
    }

    // Delete even keys
    for (size_t i = 0; i < size; i++) {
        if (i % 2 == 0) {
            map_del(&m1, i);
        }
    }
    assert(map_count(&m1) == size/2);
    return 0;
}


Configuration
-------------

Define the name of the map type (mandatory):
    #define cx_hmap_name <name>

Define the type of the map key (mandatory):
    #define cx_hmap_key <type>

Define the type of the map value (mandatory):
    #define cx_hmap_val <type>

Define the default initial number of buckets,
when map is initialized with nbuckets = 0
    #define cx_hmap_def_nbuckets <n>

Define the load factor (number of entries/number of buckets)
which, if exceeded, will imply in the rehash of the hash map
(expensive operation)
    #define cx_hmap_load_factor <lf>

Define the key comparison function with type:
void (*cmp)(const void* k1, const void* k2, size_t size);
    #define cx_hmap_cmp_key <cmp_func>

Define the key hash function with type:
uint32_t (*hash)(const void* key, size_t size);
The default hash function implements FNV-1a algorithm
    #define cx_hmap_hash_key <hash_func>

Sets if map uses custom allocator per instance
    #define cx_hmap_allocator

Sets if all map functions are prefixed with 'static'
    #define cx_hmap_static

Sets if all map functions are prefixed with 'inline'
    #define cx_hmap_inline

Sets to implement functions in this translation unit:
    #define cx_hmap_implement

Enable implementation of stats function.
Used mainly for development and benchmarking
    #define cx_hmap_stats


API
---

Assuming:
#define cx_hmap_name hmap       // Map type name
#define cx_hmap_key  ktype      // Type of key 
#define cx_hmap_val  vtype      // Type of value
               
Initialize hashmap defined with custom allocator
If the specified number of bucket is 0, the default will be used.
    hmap hmap_init(const CxAllocator* alloc, size_t nbuckets);

Initialize hashmap NOT defined with custom allocator
If the specified number of bucket is 0, the default will be used.
    hmap hmap_init(size_t nbuckets);

Clones hashmap returning a new one with possibly different number of buckets.
    hmap hmap_clone(const hmap* src, size_t nbuckets);

Free hashmap allocated memory
    void hmap_free(hmap* m);

Inserts or updates specified key and value
    void hmap_set(hmap* m, ktype k, vtype v);

Returns pointer to value associated with specified key.
Returns NULL if not found.
    vtype* hmap_get(const hmap* m, ktype k);

Deletes entry with the specified key.
Returns true if found or false otherwise.
    bool hmap_del(hmap* m, ktype k);

Returns the number of entries in the hashmap
    size_t hmap_count(const hmap* m);

Returns the next hashmap entry from the specified iterator.
Returns NULL after the last entry.
    hmap_entry* hmap_next(const hmap* m, hmap_iter* iter);

Returns statistics for the specified map (if enabled)
    hmap_stats hmap_get_stats(const cx_hmap_name* m);

*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "cx_alloc.h"

#ifndef cx_hmap_name
    #error "cx_hmap_name not defined"
#endif
#ifndef cx_hmap_key
    #error "cx_hmap_key not defined"
#endif
#ifndef cx_hmap_val
    #error "cx_hmap_val not defined"
#endif

#ifndef cx_hmap_def_nbuckets
    #define cx_hmap_def_nbuckets (17)
#endif

#ifndef cx_hmap_load_factor
    #define cx_hmap_load_factor (2.0)
#endif

// Default key comparison function
#ifndef cx_hmap_cmp_key
    #define cx_hmap_cmp_key memcmp
#endif

// Default key hash function
#ifndef cx_hmap_hash_key
    #define cx_hmap_hash_key cxHashFNV1a32
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
        const CxAllocator* alloc_;
    #define cx_hmap_alloc_global_
    #define cx_hmap_alloc_(s,n)\
        cx_alloc_alloc(s->alloc_, n)
    #define cx_hmap_free_(s,p,n)\
        cx_alloc_free(s->alloc_, p, n)
// Use global type allocator
#else
    #define cx_hmap_alloc_field_
    #define cx_hmap_alloc_global_\
        static const CxAllocator* cx_hmap_name_(_allocator) = NULL;
    #define cx_hmap_alloc_(m,n)\
        cx_alloc_alloc(cx_hmap_name_(_allocator),n)
    #define cx_hmap_free_(m,p,n)\
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
    size_t      count_;
    size_t      nbuckets_;
    cx_hmap_name_(_entry)* buckets_;
} cx_hmap_name;

typedef struct cx_hmap_name_(_iter) {
    size_t bucket_;
    cx_hmap_name_(_entry)*  next_;
} cx_hmap_name_(_iter);

// This is declared only once
#ifndef CX_HMAP_H
#define CX_HMAP_H
typedef enum {
    cx_hmap_op_set,
    cx_hmap_op_get,
    cx_hmap_op_del,
} cx_hmap_op;
#endif

#ifdef cx_hmap_allocator
    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init)(const CxAllocator* alloc, size_t nbuckets);
#else
    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init)(size_t nbuckets);
#endif
cx_hmap_api_ cx_hmap_name cx_hmap_name_(_clone)(cx_hmap_name* src, size_t nbuckets);
cx_hmap_api_ void cx_hmap_name_(_free)(cx_hmap_name* m);
cx_hmap_api_ void cx_hmap_name_(_set)(cx_hmap_name* m, cx_hmap_key k, cx_hmap_val v);
cx_hmap_api_ cx_hmap_val* cx_hmap_name_(_get)(cx_hmap_name* m, cx_hmap_key k);
cx_hmap_api_ bool cx_hmap_name_(_del)(cx_hmap_name* m, cx_hmap_key k);
cx_hmap_api_ size_t cx_hmap_name_(_count)(cx_hmap_name* m);
cx_hmap_api_ cx_hmap_name_(_entry)* cx_hmap_name_(_next)(cx_hmap_name* m, cx_hmap_name_(_iter)* iter);

//
// Implementation
//
#ifdef cx_hmap_implement
    cx_hmap_alloc_global_;

    // External functions defined in 'cx_hmap.c'
    size_t cxHashFNV1a32(void* key, size_t keySize);

    // Resize hash map if load exceeded
    cx_hmap_api_ void cx_hmap_name_(_check_resize_)(cx_hmap_name* m) {

        if (m->count_ + 1 < (float)(m->nbuckets_) * cx_hmap_load_factor) {
            return;
        }
        cx_hmap_name resized = cx_hmap_name_(_clone)(m, (m->nbuckets_ * 2));
        cx_hmap_name_(_free)(m);
        *m = resized;
        printf("RESIZED:%lu\n", m->nbuckets_);
    }

    // Creates a new entry and inserts it after specified parent
    cx_hmap_name_(_entry)* cx_hmap_name_(_add_entry_)(cx_hmap_name* m, cx_hmap_name_(_entry)* par, cx_hmap_key* key) {

        cx_hmap_name_(_entry)* new = cx_hmap_alloc_(m, sizeof(cx_hmap_name_(_entry)));
        memcpy(&new->key, key, sizeof(cx_hmap_key));
        new->next_ = NULL;
        par->next_ = new;
        m->count_++;
        return new;
    }

    // Map operations
    cx_hmap_name_(_entry)* cx_hmap_name_(_oper_)(cx_hmap_name* m, cx_hmap_op op, cx_hmap_key* key) {

        if (m->buckets_ == NULL) {
            if (op == cx_hmap_op_get) {
                return NULL;
            }
            if (op == cx_hmap_op_del) {
                return NULL;
            }
            if (op == cx_hmap_op_set) {
                const size_t allocSize = m->nbuckets_ * sizeof(*m->buckets_);
                m->buckets_ = cx_hmap_alloc_(m, allocSize);
                memset(m->buckets_, 0, allocSize);
            }
        }

        if (op == cx_hmap_op_set) {
            cx_hmap_name_(_check_resize_)(m);
        }

        // Hash the key, calculates the bucket index and get its pointer
        const size_t hash = cx_hmap_hash_key((char*)key, sizeof(cx_hmap_key));
        const size_t idx = hash % m->nbuckets_;
        cx_hmap_name_(_entry)* e = m->buckets_ + idx;

        // If bucket next pointer is NULL, the bucket is empty
        if (e->next_ == NULL) {
            // For "Get" or "Del" returns NULL pointer indicating entry not found
            if (op == cx_hmap_op_get || op == cx_hmap_op_del) {
                return NULL;
            }
            memcpy(&e->key, key, sizeof(cx_hmap_key));
            e->next_ = e;
            m->count_++;
            return e;
        }

        // This bucket is used, checks its key
        if (cx_hmap_cmp_key(&e->key, key, sizeof(cx_hmap_key)) == 0) {
            // For "Get" or "Set" just returns the pointer to this entry.
            if (op == cx_hmap_op_get || op == cx_hmap_op_set) {
                return e;
            }
            // For "Del" sets this bucket as empty
            // For string keys, free allocated key
            m->count_--;
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
        while (curr != NULL) {
            if (cx_hmap_cmp_key(&curr->key, key, sizeof(cx_hmap_key)) == 0) {
                // For "Get" or "Set" just returns the pointer
                if (op == cx_hmap_op_get) {
                    return curr;
                }
                if (op == cx_hmap_op_set) {
                    return curr;
                }
                // For "Del" removes this entry from the linked list
                prev->next_ = curr->next_;
                if (prev == e && prev->next_ == NULL) {
                    e->next_ = e;
                }
                cx_hmap_free_(m, curr, sizeof(cx_hmap_name_(_entry)));
                m->count_--;
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
            .alloc_ = alloc == NULL ? cxDefaultAllocator() : alloc,
            .nbuckets_ = nbuckets == 0 ? cx_hmap_def_nbuckets : nbuckets,
            .count_ = 0,
            .buckets_ = NULL,
        };
    }

#else

    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init)(size_t nbuckets) {
        if (cx_hmap_name_(_allocator) == NULL) {
            cx_hmap_name_(_allocator) = cxDefaultAllocator();
        }
        return (cx_hmap_name){
            .nbuckets_ = nbuckets == 0 ? cx_hmap_def_nbuckets : nbuckets,
            .count_ = 0,
            .buckets_ = NULL,
        };
    }

#endif

cx_hmap_api_ cx_hmap_name cx_hmap_name_(_clone)(cx_hmap_name* src, size_t nbuckets) {

    cx_hmap_name dst = *src;
    dst.nbuckets_ = nbuckets;
    dst.count_ = 0;
    dst.buckets_ = NULL;
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

cx_hmap_api_ void cx_hmap_name_(_free)(cx_hmap_name* m) {

    if (m == NULL) {
         return;
    }
    for (size_t i = 0; i < m->nbuckets_; i++) {
        cx_hmap_name_(_entry)* e = m->buckets_ + i;
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
    cx_hmap_free_(m, m->buckets_, m->nbuckets_ * sizeof(cx_hmap_name_(_entry)));
    m->count_ = 0;
    m->buckets_ = NULL;
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
    return m->count_;
}

cx_hmap_api_ cx_hmap_name_(_entry)* cx_hmap_name_(_next)(cx_hmap_name* m, cx_hmap_name_(_iter)* iter) {

    for (size_t i = iter->bucket_; i < m->nbuckets_; i++) {
        if (iter->next_ != NULL) {
            cx_hmap_name_(_entry)* e = iter->next_;
            iter->next_ = e->next_;
            if (iter->next_ == NULL) {
                iter->bucket_++;
            }
            return e;
        }
        cx_hmap_name_(_entry)* e = m->buckets_ + i;
        if (e->next_ == NULL) {
            iter->bucket_++;
            continue;
        }
        if (e->next_ == e) {
            iter->bucket_++;
            iter->next_ = NULL;
            return e;
        }
        iter->next_ = e->next_;
        return e;
    }
    return NULL;
}

#ifdef cx_hmap_stats

    typedef struct cx_hmap_name_(_stats) {
        size_t nbuckets;        // Number of buckets
        size_t count;           // Number of entries found
        size_t empty;           // Number of empty buckets
        size_t links;           // Total number of links
        size_t max_chain;       // Number of links of the longest chain
        size_t min_chain;       // Number of links of the shortest chain
        float  avg_chain;       // Average chain length
        float  load_factor;     // Load factor: count / nbuckets
    } cx_hmap_name_(_stats);

    cx_hmap_api_ cx_hmap_name_(_stats) cx_hmap_name_(_get_stats)(const cx_hmap_name* m) {
        cx_hmap_name_(_stats) s = {0};
        s.nbuckets = m->nbuckets_;
        s.min_chain = UINT64_MAX;
        for (size_t i = 0; i < m->nbuckets_; i++) {
            cx_hmap_name_(_entry)* e = &m->buckets_[i];
            if (e->next_ == NULL) {
                s.min_chain = 0;
                s.empty++;
                continue;
            }
            s.count++;    
            if (e->next_ == e) {
                s.min_chain = 0;
                continue;
            }
            size_t chains = 0;
            cx_hmap_name_(_entry)* curr = e->next_;
            while (curr) {
                s.links++;
                chains++;
                s.count++;    
                curr = curr->next_;
            }
            if (chains > s.max_chain) {
                s.max_chain = chains;
            }
            if (chains < s.min_chain) {
                s.min_chain = chains;
            }
        }
        //s.avg_chain = (float)(s.max_chain + s.min_chain)/2.0;
        s.avg_chain = (float)s.links / (float)s.count;
        s.load_factor = (float)s.count / (float)m->nbuckets_;
        return s;
    }

    cx_hmap_api_ void cx_hmap_name_(_print_stats)(const cx_hmap_name_(_stats)* ps) {

        printf( "nbuckets...: %lu\n"
                "count......: %lu\n"
                "empty......: %lu\n"
                "links......: %lu\n"
                "max_chain..: %lu\n"
                "min_chain..: %lu\n"
                "avg_chain..: %.2f\n"
                "load_factor: %.2f\n",
                ps->nbuckets,
                ps->count,
                ps->empty,
                ps->links,
                ps->max_chain,
                ps->min_chain,
                ps->avg_chain,
                ps->load_factor);
    }

#endif // cx_hmap_stats
#endif // cx_hmap_implement

// Undefine config  macros
#undef cx_hmap_name
#undef cx_hmap_key
#undef cx_hmap_val
#undef cx_hmap_cmp_key
#undef cx_hmap_hash_key
#undef cx_hmap_static
#undef cx_hmap_inline
#undef cx_hmap_implement
#undef cx_hmap_stats

// Undefine internal macros
#undef cx_hmap_concat2_
#undef cx_hmap_concat1_
#undef cx_hmap_name_
#undef cx_hmap_api_
#undef cx_hmap_alloc_field_
#undef cx_hmap_alloc_global
#undef cx_hmap_alloc_
#undef cx_hmap_free_



