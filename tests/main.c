#include <stdio.h>
#include <asm-generic/errno.h>
#include <stdio.h>
#include <assert.h>
#include "cx_alloc.h"
#include "cx_alloc_pool.h"

#include "logger.h"
// #include "alloc.h"
// #include "array.h"
// #include "hmap.h"
// #include "string.h"
#include "queue.h"

#define cx_queue_name qu64
#define cx_queue_type uint64_t
#include "cx_queue.h"

int main() {

    LOG_INIT();
    LOGW("START");

    const size_t cap = 8;
    qu64 q = qu64_init(cap);
    assert(qu64_cap(&q) == cap);
    assert(qu64_len(&q) == 0);

    // Single thread
    {
        uint64_t bufin[] = {0,1,2,3,4,5};
        assert(qu64_putn(&q, bufin, 6) == 0);
        assert(qu64_len(&q) == 6);

        uint64_t bufout[cap];
        assert(qu64_getn(&q, bufout, 3) == 0);
        assert(bufout[0] == 0);
        assert(bufout[1] == 1);
        assert(bufout[2] == 2);
        assert(qu64_len(&q) == 3);

        uint64_t bufin2[] = {6,7,8,9};
        assert(qu64_putn(&q, bufin2, 4) == 0);
        assert(qu64_len(&q) == 7);
        assert(qu64_close(&q) == 0);
        bool closed;
        assert(qu64_is_closed(&q, &closed) == 0 && closed);

        assert(qu64_getn(&q, bufout, 7) == 0);
        assert(qu64_len(&q) == 0);
        for (size_t i = 0; i <= 6; i++) {
            assert(bufout[i] == i + 3);
        }
        assert(qu64_get(&q, &bufout[0]) == ECANCELED);
    }

    // cxAllocPoolTests();
    // cxArrayTests();
    // cxHmapTests();
    // cxStrTests();
    // cxQueueTests();
    // //cxChanTests();
    // LOGW("END");
}


