#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "cx_pool_allocator.h"
#include "alloc.h"
#include "logger.h"

void cxAllocPoolTests(void) {

    cxAllocPoolTest(1000, 1*1024, 10);
    cxAllocPoolTest(2000, 2*1024, 10);
    cxAllocPoolTest(3000, 3*1024, 10);
}

void cxAllocPoolTest(size_t allocs, size_t blockSize, size_t ncycles) {

    LOGI("alloc pool test. allocs=%lu blockSize=%lu cycles:%lu ", allocs, blockSize, ncycles);

    // Creates pool allocator
    CxPoolAllocator* pa = cx_pool_allocator_create(blockSize, NULL);
    const CxAllocator* alloc = cx_pool_allocator_iface(pa);

    // Allocation group
    typedef struct Group {
        size_t  count;  // number of elements allocated in the group
        int     value;  // value of all the elements
        int*    p;      // pointer to first element
    } Group;
    Group* groups = malloc(sizeof(Group) * allocs);

    // Calculate the maximum count of elements to allocate
    // allowing for allocations of blocks of size greater than blockSize
    size_t maxCount = blockSize / sizeof(*groups[0].p);
    maxCount *= 1.2;    // 20 % percent of larger blocks than blockSize
    srand(time(NULL));

    // For specified number of cycles
    for (size_t ci = 0; ci < ncycles; ci++) {

        // Allocate groups with random number of elements and fill data with random value
        for (size_t i = 0; i < allocs; i++) {
            size_t count = rand() % maxCount;
            //printf("COUNT:%lu\n", count);
            groups[i] = (Group){
                .count = count,
                .value = rand(),
                .p = alloc->alloc(alloc->ctx, count*sizeof(*groups[i].p)),
            };
            for (size_t j = 0; j < count; j++) {
                groups[i].p[j] = groups[i].value;
            }
        }

        // Check all groups
        for (size_t i = 0; i < allocs; i++) {
            for (size_t j = 0; j < groups[i].count; j++) {
                assert(groups[i].p[j] == groups[i].value);
            }
        }
        // Clear the allocator before restarting the cycle
        //cxAllocPoolTestPrint(pa);
        cx_pool_allocator_clear(pa);
    }

    cx_pool_allocator_destroy(pa);
    free(groups);
}

void cxAllocPoolTestPrint(CxPoolAllocator* pa) {

    CxPoolAllocatorStats stats = cx_pool_allocator_stats(pa);
    printf("nallocs:%lu "
            "nbytes:%lu "
            "usedBlocks: %lu "
            "freeBlocks: %lu "
            "\n",
            stats.nallocs,
            stats.nbytes,
            stats.usedBlocks,
            stats.freeBlocks
    );
}

