#include <stdio.h>
#include <assert.h>
#include "alloc.h"
#include "array.h"
#include "hmap.h"
#include "string.h"
#include "queue.h"
#include "chan.h"

int main() {

    cxAllocBlockTests();
    cxArrayTests();
    cxHmapTests();
    cxStrTests();
    cxQueueTests();
    cxChanTests();
    return 0;
}

