#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <pthread.h>

#include "cx_alloc_pool.h"
#include "logger.h"

#define cx_queue_name qu64
#define cx_queue_type uint64_t
#define cx_queue_static
#define cx_queue_inline
#define cx_queue_instance_allocator
#define cx_queue_implement
#include "cx_queue.h"

typedef struct Test {
    size_t  wstart;
    size_t  wcount;
    size_t  wsum;
    bool    wclose;
    size_t  rcount;
    size_t  rsum;
    qu64*   q;
} Test;

static void *writer(void* arg) {

    Test* t = arg;
    size_t start = t->wstart;
    size_t count = t->wcount;
    while (count) {
        assert(qu64_put(t->q, start) == 0);
        t->wsum += start;
        start++;
        count--;
    }
    if (t->wclose) {
        assert(qu64_close(t->q) == 0);
    }
    return NULL;
}

static void* reader(void* arg) {

    Test* t = arg;
    while (1) {
        uint64_t v;
        int res = qu64_get(t->q, &v);
        if (res == ECANCELED) {
            break;
        }
        assert(res == 0);
        t->rcount++;
        t->rsum += v;
    }

    return NULL;
}

void cxQueueTests(void) {

    // Use pool allocator
    CxAllocPool* pa = cxAllocPoolCreate(4*1024, NULL);
    CxAllocator* alloc = (CxAllocator*)cxAllocPoolGetAllocator(pa);

    // Single thread
    {
        LOGI("queue 1T");
        const size_t cap = 8;
        qu64 q = qu64_init(alloc, cap);
        assert(qu64_cap(&q) == cap);
        assert(qu64_len(&q) == 0);

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
        assert(qu64_put(&q, 0) == ECANCELED);

        assert(qu64_getn(&q, bufout, 7) == 0);
        assert(qu64_len(&q) == 0);
        for (size_t i = 0; i <= 6; i++) {
            assert(bufout[i] == i + 3);
        }
        assert(qu64_get(&q, &bufout[0]) == ECANCELED);
    }

    // 1 Writer and 1 reader
    {
        LOGI("queue 1WT/1RT");
        const size_t cap = 50;
        qu64 q = qu64_init(alloc, cap);
        assert(qu64_cap(&q) == cap);

        // Creates and starts writer thread
        Test wdata = {.q = &q, .wstart = 1, .wcount = 1000, .wclose = true};
        pthread_t write_id;
        pthread_create(&write_id, NULL, writer, &wdata);

        // Creates and starts reader thread
        Test rdata = {.q = &q};
        pthread_t read_id;
        pthread_create(&read_id, NULL, reader, &rdata);
        
        pthread_join(write_id, NULL);
        pthread_join(read_id, NULL);

        assert(wdata.wcount == rdata.rcount);
        assert(wdata.wsum == rdata.rsum);

        qu64_free(&q);
    }

    // 2 Writers and 1 reader
    {
        LOGI("queue 2WT/1RT");
        const size_t cap = 50;
        qu64 q = qu64_init(alloc, cap);
        assert(qu64_cap(&q) == cap);

        // Creates and starts writer1 thread
        Test wdata1 = {.q = &q, .wstart = 1, .wcount = 1000};
        pthread_t writer1_id;
        pthread_create(&writer1_id, NULL, writer, &wdata1);

        // Creates and starts writer1 thread
        Test wdata2 = {.q = &q, .wstart = 1000, .wcount = 500};
        pthread_t writer2_id;
        pthread_create(&writer2_id, NULL, writer, &wdata2);

        // Creates and starts reader thread
        Test rdata = {.q = &q};
        pthread_t reader_id;
        pthread_create(&reader_id, NULL, reader, &rdata);

        // Waits for writers to finish
        pthread_join(writer1_id, NULL);
        pthread_join(writer2_id, NULL);
        // Closes the queue and waits for reader
        assert(qu64_close(&q) == 0);
        pthread_join(reader_id, NULL);

        assert(rdata.rcount == wdata1.wcount + wdata2.wcount);
        assert(rdata.rsum == wdata1.wsum + wdata2.wsum);

        qu64_free(&q);
    }

    // 2 Writers and 2 readers
    {
        LOGI("queue 2WT/2RT");
        const size_t cap = 50;
        qu64 q = qu64_init(alloc, cap);
        assert(qu64_cap(&q) == cap);

        // Creates and starts writer1 thread
        Test wdata1 = {.q = &q, .wstart = 1, .wcount = 1000};
        pthread_t writer1_id;
        pthread_create(&writer1_id, NULL, writer, &wdata1);

        // Creates and starts writer1 thread
        Test wdata2 = {.q = &q, .wstart = 1000, .wcount = 1500};
        pthread_t writer2_id;
        pthread_create(&writer2_id, NULL, writer, &wdata2);

        // Creates and starts reader1 thread
        Test rdata1 = {.q = &q};
        pthread_t reader1_id;
        pthread_create(&reader1_id, NULL, reader, &rdata1);

        // Creates and starts reader2 thread
        Test rdata2 = {.q = &q};
        pthread_t reader2_id;
        pthread_create(&reader2_id, NULL, reader, &rdata2);

        // Waits for writers to finish
        pthread_join(writer1_id, NULL);
        pthread_join(writer2_id, NULL);
        // Closes the queue and waits for readers
        assert(qu64_close(&q) == 0);
        pthread_join(reader1_id, NULL);
        pthread_join(reader2_id, NULL);

        assert(rdata1.rcount + rdata2.rcount == wdata1.wcount + wdata2.wcount);
        assert(rdata1.rsum + rdata2.rsum == wdata1.wsum + wdata2.wsum);

        qu64_free(&q);
    }

    cxAllocPoolDestroy(pa);
}


