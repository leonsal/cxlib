#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cx_alloc_pool.h"
#include "alloc.h"

#include "logger.h"

void cxAllocBlockTests(void) {

    cxAllocBlockTest(100, 4*1024);
    cxAllocBlockTest(200, 1*1024);
}

void cxAllocBlockTest(size_t allocs, size_t blockSize) {

    LOGI("alloc pool tests. allocs=%lu blockSize=%lu", allocs, blockSize);
    // Allocation group
    typedef struct Group {
        int     start;  // start value of the first int
        size_t  count;  // number of ints allocated in the group
        int*    p;      // pointer to first int
        struct Group* next;
    } Group;

    Group* groups = NULL;
    CxAllocPool* a0 = cxAllocPoolCreate(blockSize, NULL);
    CxAllocPool* a1 = cxAllocPoolCreate(blockSize, NULL);
    CxAllocPool* a2 = cxAllocPoolCreate(blockSize, NULL);
    srand(time(NULL));
    size_t start = 0;

    for (size_t an = 0; an < allocs; an++) {
        // Random number of ints to allocate
        size_t count = rand() % 1000;
        // Choose arena 1 or arena 2
        CxAllocPool* a = a2;
        if (an % 2) {
            a = a1;
        }
        // Creates group and adds to the linked list of groups
        Group* g = cxAllocPoolAlloc(a0, sizeof(Group));
        *g = (Group){
            .start = start,
            .count = count,
            .p = cxAllocPoolAlloc(a, count * sizeof(int)),
        };
        g->next = groups;
        groups = g;
        // Initialize group data
        for (size_t idx = 0; idx < count; idx++) {
            g->p[idx] = start++;
        }
    }

    // Checks data in all groups
    Group* curr = groups;
    while (curr != NULL) {
        for (size_t i = 0 ; i < curr->count; i++) {
            if (curr->p[i] != curr->start + i) {
                printf("ERROR\n");
                abort();
            }
        }
        curr = curr->next;
    }

    cxAllocPoolDestroy(a0);
    cxAllocPoolDestroy(a1);
    cxAllocPoolDestroy(a2);
}

void cxAllocPoolTest(size_t allocs, size_t blockSize, size_t ncycles) {

    // Creates pool allocator
    CxAllocPool* pa = cxAllocPoolCreate(blockSize, NULL);
    const CxAllocator* alloc = cxAllocPoolGetAllocator(pa);

    // Allocation group
    typedef struct Group {
        size_t  count;  // number of ints allocated in the group
        int     value;  // value of all the elements
        int*    p;      // pointer to first element
    } Group;
    Group* groups = malloc(sizeof(Group) * allocs);

    srand(time(NULL));
    // Allocate groups with random number of elements and
    // fill the data with random value
    size_t maxCount = blockSize / sizeof(*groups[0].p);
    maxCount *= 1.1;    // 10 % percent of larger blocks than blockSize

    for (size_t ci = 0; ci < ncycles; ci++) {
        for (size_t i = 0; i < allocs; i++) {
            size_t count = rand() % maxCount;
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
        // Clear the allocator
        cxAllocPoolClear(pa);
    }


    cxAllocPoolDestroy(pa);
    free(groups);
}

