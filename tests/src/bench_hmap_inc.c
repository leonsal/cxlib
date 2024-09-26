#include <bits/time.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "util.h"
#include "logger.h"

typedef struct ARR_(_Entry){uint64_t key; uint64_t val;} ARR_(_Entry);
#define cx_array_name ARR_()
#define cx_array_type ARR_(_Entry)
#define cx_array_static
#define cx_array_instance_allocator
#define cx_array_implement
#include "cx_array.h"

void BENCH_(HMAP)(const CxAllocator* alloc, size_t elcount, size_t lookups) {

    LOGI("%s: elcount:%zu lookups:%zu", __func__, elcount, lookups);
    // Creates array for comparison
    ARR_() a = ARR_(_init)(alloc);

    // Fill map with 'size' random entries
    HMAP m = HMAP_(_init)(alloc, 0);
    srand(1);
    const int N=100000;
    for (size_t i = 0; i < elcount; i++) {
        uint64_t key = rand() % (N + 1);
        uint64_t val = key * 2;
        HMAP_(_set)(&m, key, val);
        ARR_(_push)(&a, (ARR_(_Entry)){key, val});
    }

    // Get 'count' random entries
    size_t m_low = SIZE_MAX;
    size_t m_high = 0;
    size_t m_sum = 0;
    size_t m_found = 0;

    size_t a_low = SIZE_MAX;
    size_t a_high = 0;
    size_t a_sum = 0;
    size_t a_found = 0;
    struct timespec start;
    struct timespec stop;
    srand(2);
    for (size_t i = 0; i < lookups; i++) {
        uint64_t key = rand() % (N + 1);
        uint64_t* val = NULL;

        // Measure hmap get time
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        val = HMAP_(_get)(&m, key);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);
        const size_t m_elapsed = (stop.tv_sec - start.tv_sec)*1000000000 + stop.tv_nsec-start.tv_nsec;
        if (m_elapsed < m_low)  { m_low = m_elapsed; }
        if (m_elapsed > m_high) { m_high = m_elapsed; }
        m_sum += m_elapsed;
        if (val) {
            CHK(*val == key*2); 
            m_found++;
        }

        // Measure array find time
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        for (size_t i = 0; i < elcount; i++) {
            if (a.data[i].key == key) {
                val = &a.data[i].val;
                CHK(*val == key*2); 
                a_found++;
                break;
            }
        }
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);
        const size_t a_elapsed = (stop.tv_sec - start.tv_sec)*1000000000 + stop.tv_nsec-start.tv_nsec;
        if (a_elapsed < a_low)  { a_low = a_elapsed; }
        if (a_elapsed > a_high) { a_high = a_elapsed; }
        a_sum += a_elapsed;
    }

    LOGI("\tarray low:%5zuns high:%6zuns avg:%5zuns found:%zu", a_low, a_high, a_sum/lookups, a_found);
    LOGI("\thmap  low:%5zuns high:%6zuns avg:%5zuns found:%zu", m_low, m_high, m_sum/lookups, m_found);

    ARR_(_free)(&a);
    HMAP_(_free)(&m);
}

