#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "cx_alloc_block.h"
#include "alloc.h"

void cxAllocBlockTests(void) {

    cxAllocBlockTest(100, 4*1024);
}

void cxAllocBlockTest(size_t allocs, size_t blockSize) {

    // Allocation group
    typedef struct Group {
        int     start;  // start value of the first int
        size_t  count;  // number of ints allocated in the group
        int*    p;      // pointer to first int
        struct Group* next;
    } Group;

    Group* groups = NULL;
    CxAllocBlock* a0 = cxAllocBlockCreate(blockSize, NULL);
    CxAllocBlock* a1 = cxAllocBlockCreate(blockSize, NULL);
    CxAllocBlock* a2 = cxAllocBlockCreate(blockSize, NULL);
    srand(time(NULL));
    size_t start = 0;

    for (size_t an = 0; an < allocs; an++) {
        // Random number of ints to allocate
        size_t count = rand() % 1000;
        // Choose arena 1 or arena 2
        CxAllocBlock* a = a2;
        if (an % 2) {
            a = a1;
        }
        // Creates group and adds to the linked list of groups
        Group* g = cxAllocBlockAlloc(a0, sizeof(Group));
        *g = (Group){
            .start = start,
            .count = count,
            .p = cxAllocBlockAlloc(a, count * sizeof(int)),
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

    cxAllocBlockDestroy(a0);
    cxAllocBlockDestroy(a1);
    cxAllocBlockDestroy(a2);
}

