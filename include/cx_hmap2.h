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

// Use custom instance allocator
#ifdef cx_hmap_allocator
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

// These are declared only once
#ifndef CX_HMAP_H
#define CX_HMAP_H
    typedef enum {
        cx_hmap_status_empty,
        cx_hmap_status_full,
        cx_hmap_status_del,
    } cx_hmap_status;

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
    cx_hmap_alloc_global_;

    // External functions defined in 'cx_hmap.c'
    size_t cxHashFNV1a32(void* key, size_t keySize);

    // Resize hash map if load exceeded
    void cx_hmap_name_(_check_resize_)(cx_hmap_name* m) {

        if (m->count_ + m->deleted_ + 1 < (float)(m->nbuckets_) * cx_hmap_resize_load) {
            return;
        }
        cx_hmap_name resized = cx_hmap_name_(_clone)(m, (m->nbuckets_ * 2) + 0);
        cx_hmap_name_(_free)(m);
        *m = resized;
    }

    // Map operations
    cx_hmap_name_(_entry)* cx_hmap_name_(_oper_)(cx_hmap_name* m, cx_hmap_op op, cx_hmap_key* key, size_t* nprobes) {

        if (m->buckets_ == NULL) {
            if (op == cx_hmap_op_get) {
                return NULL;
            }
            if (op == cx_hmap_op_del) {
                return NULL;
            }
            if (op == cx_hmap_op_set) {
                size_t allocSize = m->nbuckets_ * sizeof(*m->buckets_);
                m->buckets_ = cx_hmap_alloc_(m, allocSize);
                allocSize = m->nbuckets_ * sizeof(*m->status_);
                m->status_ = cx_hmap_alloc_(m, m->nbuckets_ * sizeof(*m->status_));
                memset(m->status_, cx_hmap_status_empty, allocSize);
            }
        }

        if (op == cx_hmap_op_set) {
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
            if (m->status_[idx] == cx_hmap_status_empty) {
                // For "Get" or "Del" returns NULL pointer indicating entry not found
                if (op == cx_hmap_op_get || op == cx_hmap_op_del) {
                    return NULL;
                }
                // Sets the bucket key and value
                memcpy(&e->key, key, sizeof(cx_hmap_key));
                m->count_++;
                m->status_[idx] = cx_hmap_status_full;
                return e;
            }
            // Bucket is full
            if (m->status_[idx] == cx_hmap_status_full) {
                // Checks current bucket key
                if (cx_hmap_cmp_key(&e->key, key, sizeof(cx_hmap_key)) == 0) {
                    // For "Get" or "Set" just returns the pointer to this entry.
                    if (op == cx_hmap_op_get || op == cx_hmap_op_set) {
                        return e;
                    }
                    // For "Del" sets this bucket as deleted
                    m->count_--;
                    m->deleted_++;
                    m->status_[idx] = cx_hmap_status_del;
                    return e;
                }
            }
            // Bucket is deleted
            if (m->status_[idx] == cx_hmap_status_del) {
                if (op == cx_hmap_op_set && idx == startIdx) {
                    memcpy(&e->key, key, sizeof(cx_hmap_key));
                    m->count_++;
                    m->deleted_--;
                    m->status_[idx] = cx_hmap_status_full;
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

#ifdef cx_hmap_allocator

    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init)(const CxAllocator* alloc, size_t nbuckets) {
        return (cx_hmap_name){
            .alloc_ = alloc == NULL ? cxDefaultAllocator() : alloc,
            .nbuckets_ = nbuckets == 0 ? cx_hmap_def_nbuckets : nbuckets,
        };
    }

#else

    cx_hmap_api_ cx_hmap_name cx_hmap_name_(_init)(size_t nbuckets) {
        if (cx_hmap_name_(_allocator) == NULL) {
            cx_hmap_name_(_allocator) = cxDefaultAllocator();
        }
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
    memset(cloned.status_, cx_hmap_status_empty, allocSize);
    for (size_t i = 0; i < m->nbuckets_; i++) {
        if (m->status_[i] == cx_hmap_status_full) {
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
    cx_hmap_name_(_entry)* e = cx_hmap_name_(_oper_)(m, cx_hmap_op_set, &k, NULL);
    e->val = v;
}

cx_hmap_api_ cx_hmap_val* cx_hmap_name_(_get)(const cx_hmap_name* m, cx_hmap_key k) {

    assert(m);
    cx_hmap_name_(_entry)* e = cx_hmap_name_(_oper_)((cx_hmap_name*)m, cx_hmap_op_get, &k, NULL);
    return e == NULL ? NULL : &e->val;
}

cx_hmap_api_ bool cx_hmap_name_(_del)(cx_hmap_name* m, cx_hmap_key k) {

    assert(m);
    cx_hmap_name_(_entry)* e = cx_hmap_name_(_oper_)(m, cx_hmap_op_del, &k, NULL);
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
        if (m->status_[i] == cx_hmap_status_full) {
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
            if (m->status_[i] == cx_hmap_status_empty) {
                s.empty++;
                continue;
            }
            if (m->status_[i] == cx_hmap_status_full) {
                cx_hmap_name_(_entry)* e = m->buckets_ + i;
                size_t nprobes = 89;
                cx_hmap_name_(_oper_)((cx_hmap_name*)m, cx_hmap_op_get, &e->key, &nprobes);
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
            if (m->status_[i] == cx_hmap_status_del) {
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




