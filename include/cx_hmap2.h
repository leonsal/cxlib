/*
Hashmap Implementation
----------------------
- Uses open addressing with linear probing.
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
#include "cx_hmap2.h"

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

Define optional custom allocator pointer or function call which return pointer to allocator.
Uses default allocator if not defined.
This allocator will be used for all instances of this type.
    #define cx_hmap_allocator <allocator>

Sets if map uses custom allocator per instance.
If set, it is necessary to initialize each array with the desired allocator.
    #define cx_hmap_instance_allocator

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
#include <assert.h>
#include <stdio.h>
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

#ifndef cx_hmap_resize_load
    #define cx_hmap_resize_load (0.8)
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

// Default allocator
#ifndef cx_hmap_allocator
    #define cx_hmap_allocator cxDefaultAllocator()
#endif

// Use custom instance allocator
#ifdef cx_hmap_instance_allocator
    #define cx_hmap_alloc_field_\
        const CxAllocator* alloc_;
    #define cx_hmap_alloc_global_
    #define cx_hmap_alloc_(s,n)\
        cx_alloc_alloc((s)->alloc_, n)
    #define cx_hmap_free_(s,p,n)\
        cx_alloc_free((s)->alloc_, p, n)
// Use global type allocator
#else
    #define cx_hmap_alloc_field_
    #define cx_hmap_alloc_(m,n)\
        cx_alloc_alloc(cx_hmap_allocator,n)
    #define cx_hmap_free_(m,p,n)\
        cx_alloc_free(cx_hmap_allocator,p,n)
#endif

//
// Declarations
//

typedef struct cx_hmap_name_(_entry) {
    cx_hmap_key key;
    cx_hmap_val val;
} cx_hmap_name_(_entry);

typedef struct cx_hmap_name {
    cx_hmap_alloc_field_
    size_t      nbuckets_;
    size_t      count_;
    size_t      deleted_;
    uint8_t*    status_;
    cx_hmap_name_(_entry)* buckets_;
} cx_hmap_name;

typedef struct cx_hmap_name_(_iter) {
    size_t bucket_;
} cx_hmap_name_(_iter);

#ifdef cx_hmap_instance_allocator
    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init)(const CxAllocator* alloc, size_t nbuckets);
#else
    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init)(size_t nbuckets);
#endif
cx_hmap_api_ cx_hmap_name cx_hmap_name_(_clone)(const cx_hmap_name* src, size_t nbuckets);
cx_hmap_api_ void cx_hmap_name_(_free)(cx_hmap_name* m);
cx_hmap_api_ void cx_hmap_name_(_set)(cx_hmap_name* m, cx_hmap_key k, cx_hmap_val v);
cx_hmap_api_ cx_hmap_val* cx_hmap_name_(_get)(const cx_hmap_name* m, cx_hmap_key k);
cx_hmap_api_ bool cx_hmap_name_(_del)(cx_hmap_name* m, cx_hmap_key k);
cx_hmap_api_ size_t cx_hmap_name_(_count)(const cx_hmap_name* m);
cx_hmap_api_ cx_hmap_name_(_entry)* cx_hmap_name_(_next)(cx_hmap_name* m, cx_hmap_name_(_iter)* iter);


//
// Implementation
//
#ifdef cx_hmap_implement
    #define cx_hmap_empty_  (0)
    #define cx_hmap_full_   (1)
    #define cx_hmap_del_    (2)
    #define cx_hmap_op_set_ (0)
    #define cx_hmap_op_get_ (1)
    #define cx_hmap_op_del_ (2)

    // External functions defined in 'cx_hmap.c'
    size_t cxHashFNV1a32(void* key, size_t keySize);

    // Resize hash map if load exceeded
    cx_hmap_api_ void cx_hmap_name_(_check_resize_)(cx_hmap_name* m) {

        if (m->count_ + m->deleted_ + 1 < (float)(m->nbuckets_) * cx_hmap_resize_load) {
            return;
        }
        cx_hmap_name resized = cx_hmap_name_(_clone)(m, (m->nbuckets_ * 2) + 0);
        cx_hmap_name_(_free)(m);
        *m = resized;
        //printf("RESIZED:%lu\n", m->nbuckets_);
    }

    // Map operations
    cx_hmap_api_ cx_hmap_name_(_entry)* cx_hmap_name_(_oper_)(cx_hmap_name* m, int op, cx_hmap_key* key, size_t* nprobes) {

        if (m->buckets_ == NULL) {
            if (op == cx_hmap_op_get_) {
                return NULL;
            }
            if (op == cx_hmap_op_del_) {
                return NULL;
            }
            if (op == cx_hmap_op_set_) {
                // Allows for static initialization of maps
                if (m->nbuckets_ == 0) {
                    m->nbuckets_ = cx_hmap_def_nbuckets;
                }
                size_t allocSize = m->nbuckets_ * sizeof(*m->buckets_);
                m->buckets_ = cx_hmap_alloc_(m, allocSize);
                allocSize = m->nbuckets_ * sizeof(*m->status_);
                m->status_ = cx_hmap_alloc_(m, m->nbuckets_ * sizeof(*m->status_));
                memset(m->status_, cx_hmap_empty_, allocSize);
            }
        }

        if (op == cx_hmap_op_set_) {
            cx_hmap_name_(_check_resize_)(m);
        }

        // Hash the key and calculates the bucket index
        const size_t hash = cx_hmap_hash_key((char*)key, sizeof(cx_hmap_key));
        size_t idx = hash % m->nbuckets_;

        if (nprobes) {
            *nprobes = 0;
        }
        size_t startIdx = idx;
        while (true) {
            cx_hmap_name_(_entry)* e = m->buckets_ + idx;
            // Bucket is empty
            if (m->status_[idx] == cx_hmap_empty_) {
                // For "Get" or "Del" returns NULL pointer indicating entry not found
                if (op == cx_hmap_op_get_ || op == cx_hmap_op_del_) {
                    return NULL;
                }
                // Sets the bucket key and value
                memcpy(&e->key, key, sizeof(cx_hmap_key));
                m->count_++;
                m->status_[idx] = cx_hmap_full_;
                return e;
            }
            // Bucket is full
            if (m->status_[idx] == cx_hmap_full_) {
                // Checks current bucket key
                if (cx_hmap_cmp_key(&e->key, key, sizeof(cx_hmap_key)) == 0) {
                    // For "Get" or "Set" just returns the pointer to this entry.
                    if (op == cx_hmap_op_get_ || op == cx_hmap_op_set_) {
                        return e;
                    }
                    // For "Del" sets this bucket as deleted
                    m->count_--;
                    m->deleted_++;
                    m->status_[idx] = cx_hmap_del_;
                    return e;
                }
            }
            // Bucket is deleted
            if (m->status_[idx] == cx_hmap_del_) {
                if (op == cx_hmap_op_set_ && idx == startIdx) {
                    memcpy(&e->key, key, sizeof(cx_hmap_key));
                    m->count_++;
                    m->deleted_--;
                    m->status_[idx] = cx_hmap_full_;
                    return e;
                }
            }
            // Linear probing
            idx++;
            idx %= m->nbuckets_;
            if (nprobes) {
                (*nprobes)++;
            }
            // This should never happen if cx_hmap_resize_load < 1.0
            if (idx == startIdx) {
                fprintf(stderr, "CX_HMAP OVERFLOW\n");
                abort();
            }
        }
    }

#ifdef cx_hmap_instance_allocator

    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init)(const CxAllocator* alloc, size_t nbuckets) {
        return (cx_hmap_name){
            .alloc_ = alloc == NULL ? cxDefaultAllocator() : alloc,
            .nbuckets_ = nbuckets == 0 ? cx_hmap_def_nbuckets : nbuckets,
        };
    }

#else

    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init)(size_t nbuckets) {
        return (cx_hmap_name){
            .nbuckets_ = nbuckets == 0 ? cx_hmap_def_nbuckets : nbuckets,
        };
    }

#endif

cx_hmap_api_ cx_hmap_name cx_hmap_name_(_clone)(const cx_hmap_name* m, size_t nbuckets) {

    assert(m);
    assert(nbuckets > 0);
    cx_hmap_name cloned = *m; // copy possible allocator
    cloned.nbuckets_ = nbuckets;
    cloned.count_ = 0;
    cloned.deleted_ = 0;
    cloned.buckets_ = cx_hmap_alloc_(&cloned, cloned.nbuckets_ * sizeof(*cloned.buckets_));
    const size_t allocSize = cloned.nbuckets_ * sizeof(*cloned.status_);
    cloned.status_ = cx_hmap_alloc_(&cloned, allocSize);
    memset(cloned.status_, cx_hmap_empty_, allocSize);
    for (size_t i = 0; i < m->nbuckets_; i++) {
        if (m->status_[i] == cx_hmap_full_) {
            cx_hmap_name_(_entry)* e = &m->buckets_[i];
            cx_hmap_name_(_set)(&cloned, e->key, e->val);
        }
    }
    return cloned;         
}

cx_hmap_api_ void cx_hmap_name_(_free)(cx_hmap_name* m) {

    assert(m);
    cx_hmap_free_(m, m->buckets_, m->nbuckets_ * sizeof(*m->buckets_));
    cx_hmap_free_(m, m->status_, m->nbuckets_ * sizeof(*m->status_));
    m->buckets_ = NULL;
    m->status_ = NULL;
    m->count_ = 0;
    m->deleted_ = 0;
}

cx_hmap_api_ void cx_hmap_name_(_set)(cx_hmap_name* m, cx_hmap_key k, cx_hmap_val v) {

    assert(m);
    cx_hmap_name_(_entry)* e = cx_hmap_name_(_oper_)(m, cx_hmap_op_set_, &k, NULL);
    e->val = v;
}

cx_hmap_api_ cx_hmap_val* cx_hmap_name_(_get)(const cx_hmap_name* m, cx_hmap_key k) {

    assert(m);
    cx_hmap_name_(_entry)* e = cx_hmap_name_(_oper_)((cx_hmap_name*)m, cx_hmap_op_get_, &k, NULL);
    return e == NULL ? NULL : &e->val;
}

cx_hmap_api_ bool cx_hmap_name_(_del)(cx_hmap_name* m, cx_hmap_key k) {

    assert(m);
    cx_hmap_name_(_entry)* e = cx_hmap_name_(_oper_)(m, cx_hmap_op_del_, &k, NULL);
    return e == NULL ? false : true;
}

cx_hmap_api_ size_t cx_hmap_name_(_count)(const cx_hmap_name* m) {

    assert(m);
    return m->count_;
}

cx_hmap_api_ cx_hmap_name_(_entry)* cx_hmap_name_(_next)(cx_hmap_name* m, cx_hmap_name_(_iter)* iter) {

    assert(m);
    assert(iter);
    for (size_t i = iter->bucket_; i < m->nbuckets_; i++) {
        if (m->status_[i] == cx_hmap_full_) {
            iter->bucket_ = i + 1;
            return &m->buckets_[i];
        }
    }
    iter->bucket_ = m->nbuckets_;
    return NULL;
}

#ifdef cx_hmap_stats

    typedef struct cx_hmap_name_(_stats) {
        size_t nbuckets;        // Number of buckets
        size_t count;           // Number of used buckets
        size_t deleted;         // Number of deleted buckets
        size_t empty;           // Number of empty buckets
        size_t probes;          // Total number of probes
        size_t max_probe;       // Maximum number of probes
        size_t min_probe;       // Minimum number of probes
        double avg_probe;       // Average number of probes
        double load_factor;     // Current load factor
    } cx_hmap_name_(_stats);

    cx_hmap_api_ cx_hmap_name_(_stats) cx_hmap_name_(_get_stats)(const cx_hmap_name* m) {

        assert(m);
        cx_hmap_name_(_stats) s = {0};
        s.nbuckets = m->nbuckets_;
        s.min_probe = UINT64_MAX;
        for (size_t i = 0; i < m->nbuckets_; i++) {
            if (m->status_[i] == cx_hmap_empty_) {
                s.empty++;
                continue;
            }
            if (m->status_[i] == cx_hmap_full_) {
                cx_hmap_name_(_entry)* e = m->buckets_ + i;
                size_t nprobes = 89;
                cx_hmap_name_(_oper_)((cx_hmap_name*)m, cx_hmap_op_get_, &e->key, &nprobes);
                s.probes += nprobes;
                if (nprobes > s.max_probe) {
                    s.max_probe = nprobes;
                }
                if (nprobes < s.min_probe) {
                    s.min_probe = nprobes;
                }
                s.count++;
                continue;
            }
            if (m->status_[i] == cx_hmap_del_) {
                s.deleted++;
                continue;
            }
            assert(0);
        }
        //s.avg_probe = (double)(s.max_probe + s.min_probe)/2.0;
        s.avg_probe = (double)s.probes/(double)s.count;
        s.load_factor = (double)s.count / (double)s.nbuckets ;
        return s;
    }

    cx_hmap_api_ void cx_hmap_name_(_print_stats)(const cx_hmap_name_(_stats)* ps) {

        printf( "nbuckets...: %lu\n"
                "count......: %lu\n"
                "deleted....: %lu\n"
                "empty......: %lu\n"
                "probes.....: %lu\n"
                "max_probe..: %lu\n"
                "min_probe..: %lu\n"
                "avg_probe..: %.2f\n"
                "load_factor: %.2f\n",
                ps->nbuckets,
                ps->count,
                ps->deleted,
                ps->empty,
                ps->probes,
                ps->max_probe,
                ps->min_probe,
                ps->avg_probe,
                ps->load_factor);
    }

#endif // cx_hmap_stats
#endif // cx_hmap_implement

// Undefine config  macros
#undef cx_hmap_name
#undef cx_hmap_key
#undef cx_hmap_val
#undef cx_hmap_def_nbuckets
#undef cx_hmap_resize_load
#undef cx_hmap_cmp_key
#undef cx_hmap_hash_key
#undef cx_hmap_allocator
#undef cx_hmap_instance_allocator
#undef cx_hmap_static
#undef cx_hmap_inline
#undef cx_hmap_implement

// Undefine internal macros
#undef cx_hmap_concat2_
#undef cx_hmap_concat1_
#undef cx_hmap_name_
#undef cx_hmap_api_
#undef cx_hmap_alloc_field_
#undef cx_hmap_alloc_global
#undef cx_hmap_alloc_
#undef cx_hmap_free_
#undef cx_hmap_empty_
#undef cx_hmap_full_
#undef cx_hmap_del_
#undef cx_hmap_op_set_
#undef cx_hmap_op_get_
#undef cx_hmap_op_del_




