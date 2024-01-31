#include <stdio.h>
#include <asm-generic/errno.h>
#include <stdio.h>
#include "cx_alloc.h"
#include "cx_pool_allocator.h"

#include "logger.h"
#include "alloc.h"
#include "array.h"
#include "hmap.h"
#include "string.h"
#include "queue.h"


int main() {

    LOG_INIT();
    LOGW("START");
    cxAllocPoolTests();
    cxArrayTests();
    cxHmapTests();
    cxStrTests();
    cxQueueTests();
    LOGW("END");
}

