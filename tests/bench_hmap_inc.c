#include <bits/time.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "util.h"

void BENCH_(HMAP)(const CxAllocator* alloc, size_t size, size_t count) {

    // Fill map with 'size' random entries
    HMAP m = HMAP_(_init)(alloc, 0);
    srand(1);
    const int N=100000;
    for (size_t i = 0; i < size; i++) {
        uint64_t key = rand() % (N + 1);
        HMAP_(_set)(&m, key, key * 2);
    }

    // Get 'count' random entries
    size_t low = SIZE_MAX;
    size_t high = 0;
    size_t sum = 0;
    size_t found = 0;
    struct timespec start;
    struct timespec stop;
    srand(10);
    for (size_t i = 0; i < count; i++) {
        uint64_t key = rand() % (N + 1);

        // Measure get time
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
        const uint64_t* val = HMAP_(_get)(&m, key);
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &stop);
        const size_t elapsed = (stop.tv_sec - stop.tv_sec)*1000000000 + stop.tv_nsec-start.tv_nsec;

        if (elapsed < low)  { low = elapsed; }
        if (elapsed > high) { high = elapsed; }
        sum += elapsed;

        if (val) {
            CHK(*val == key*2); 
            found++;
        }
    }

    printf("size  :%zu   \n", size);
    printf("count :%zu   \n", count);
    printf("\tlow   :%zuns \n", low);
    printf("\thigh  :%zuns \n", high);
    printf("\tavg   :%zuns \n", sum/count);
    printf("\tfound :%zu   \n", found);
}

