#include <stdio.h>
#include <assert.h>

#include "alloc.h"
#include "array.h"
#include "cx_alloc.h"
#include "hmap.h"
#include "string.h"
#include "queue.h"

int main() {

    cxAllocBlockTests();
    cxArrayTests();
    cxHmapTests();
    cxStrTests();
    cxQueueTest(cxDefaultAllocator());

    return 0;
}

