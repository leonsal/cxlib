#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "cx_alloc_pool.h"
#include "chan.h"

#define cx_chan_name chu
#define cx_chan_type int
#define cx_chan_cap  8
#define cx_chan_static
#define cx_chan_allocator
#define cx_chan_implement
#include "cx_chan.h"

#define cx_chan_name chb
#define cx_chan_type int
#define cx_chan_cap  8
#define cx_chan_static
#define cx_chan_allocator
#define cx_chan_implement
#include "cx_chan.h"

#include "logger.h"

void cxChanTests(void) {

    // Use default allocator
    const size_t size = 100000;
    cxChanTest(cxDefaultAllocator());

    // Use pool allocator
    CxAllocPool* ba = cxAllocPoolCreate(4*1024, NULL);
    cxChanTest(cxAllocPoolGetAllocator(ba));
    cxAllocPoolDestroy(ba);
}

typedef struct Params {
    chu*  c;
    int   data;
    int*  results;
} Params;

typedef struct Params2 {
    chb*  c;
    int   data;
    int*  results;
} Params2;

static void* closer(void* d) {

    Params* p = d;
    // Waits 100ms before closing
    nanosleep(&(struct timespec){.tv_nsec=100000000}, NULL);
    chu_close(p->c);
    return NULL;
}

static void* writer(void* d) {

    Params* p = d;
    //printf("writer before send: %d\n", p->data);
    chu_send(p->c, p->data);
    //printf("writer after send: %d\n", p->data);
    free(p);
    return NULL;
}

static void* reader(void* d) {

    Params* p = d;
    int v = chu_recv(p->c);
    p->results[v]++;
    free(p);
    return NULL;
}

static void* closer2(void* d) {

    Params2* p = d;
    // Waits 100ms before closing
    nanosleep(&(struct timespec){.tv_nsec=100000000}, NULL);
    chb_close(p->c);
    return NULL;
}

static void* writer2(void* d) {

    Params2* p = d;
    //printf("writer2 before send: %d\n", p->data);
    chb_send(p->c, p->data);
    //printf("writer2 after send: %d\n", p->data);
    free(p);
    return NULL;
}

static void* reader2(void* d) {

    Params2* p = d;
    //printf("reader2 before recv\n");
    int v = chb_recv(p->c);
    //printf("reader2 after recv:%d\n", v);
    p->results[v]++;
    free(p);
    return NULL;
}

void cxChanTest(const CxAllocator* alloc) {

    LOGI("chan. alloc=%p", alloc);

    // Creates unbuffered channel, starts channel closer,
    // sends data and waits for return
    {
        chu c = chu_init(alloc, 0);
        assert(chu_len(&c) == 0);
        assert(chu_cap(&c) == 0);
        pthread_t t;
        Params p = {.c = &c};
        assert(pthread_create(&t, NULL, closer, &p) == 0);
        assert(pthread_detach(t) == 0);
        assert(chu_send(&c, 1) == false);
        assert(chu_send(&c, 2) == false);
        assert(chu_isclosed(&c));
        chu_free(&c); 
    }

    // Creates unbuffered channel, starts channel closer,
    // try to receive data and waits for return
    {
        chu c = chu_init(alloc, 0);
        pthread_t t;
        Params p = {.c = &c};
        assert(pthread_create(&t, NULL, closer, &p) == 0);
        assert(pthread_detach(t) == 0);
        assert(chu_recv(&c) == 0);
        assert(chu_recv(&c) == 0);
        assert(chu_isclosed(&c));
        chu_free(&c); 
    }

    // Creates unbuffered channel, starts N senders and N receivers
    {
        chu c = chu_init(alloc, 0);
        size_t const count = 10;
        pthread_t senders[count];
        pthread_t receivers[count];
        int results[count];
        memset(results, 0, sizeof(results[0]) * count);
        for (size_t i = 0; i < count; i++) {
            Params* p = malloc(sizeof(*p));
            p->c = &c;
            p->data = i;
            assert(pthread_create(&senders[i], NULL, writer, p) == 0);
        }
        for (size_t i = 0; i < count; i++) {
            Params* p = malloc(sizeof(*p));
            p->c = &c;
            p->results = results;
            assert(pthread_create(&receivers[i], NULL, reader, p) == 0);
        }
        // Wait for all threads
        for (size_t i = 0; i < count; i++) {
            pthread_join(senders[i], NULL);
            pthread_join(receivers[i], NULL);
        }
        // Check results
        for (size_t i = 0; i < count; i++) {
            assert(results[i] == 1);
        }
        chu_free(&c); 
    }

    // Creates buffered channel, send data without blocking,
    // starts closer and sends data which will block.
    {
        chb c = chb_init(alloc, 1);
        assert(chb_send(&c, 10));
        pthread_t t;
        Params2 p = {.c = &c};
        assert(pthread_create(&t, NULL, closer2, &p) == 0);
        assert(pthread_detach(t) == 0);
        assert(chb_send(&c, 20) == false); // blocks till closed
        assert(chb_recv(&c) == 10);
        assert(chb_recv(&c) == 0);
        assert(chb_isclosed(&c));
        chb_free(&c);
    }

    // Buffered channel
    // Write data till capacity without blocking,
    // then reads all
    {
        size_t cap = 10;
        chb c = chb_init(alloc, cap);
        assert(chb_cap(&c) == cap);
        for (size_t i = 0; i < cap; i++) {
            assert(chb_send(&c, i));
        }
        assert(chb_len(&c) == cap);
        // Reads data without blocking
        for (size_t i = 0; i < cap; i++) {
            assert(chb_recv(&c) == i);
        }
        assert(chb_len(&c) == 0);

        // Write some data and closes the channel
        for (size_t i = 0; i < cap/2; i++) {
            assert(chb_send(&c, i));
        }
        chb_close(&c);
        assert(chb_send(&c, 2) == false);
        assert(chb_len(&c) == cap/2);
        assert(chb_isclosed(&c));
        // Reads valid data and data after close
        for (size_t i = 0; i < cap; i++) {
            if (i < cap/2) {
                assert(chb_recv(&c) == i);
            } else {
                assert(chb_recv(&c) == 0);
            }
        }
        assert(chb_len(&c) == 0);
        chb_free(&c);
    }

    // Creates buffered channel, starts N senders and N receivers
    {
        size_t cap = 10;
        chb c = chb_init(alloc, cap);
        size_t const tcount = 100;
        pthread_t senders[tcount];
        pthread_t receivers[tcount];
        int results[tcount];
        memset(results, 0, sizeof(results[0]) * tcount);
        for (size_t i = 0; i < tcount; i++) {
            Params2* p = malloc(sizeof(*p));
            p->c = &c;
            p->data = i;
            assert(pthread_create(&senders[i], NULL, writer2, p) == 0);
        }
        for (size_t i = 0; i < tcount; i++) {
            Params2* p = malloc(sizeof(*p));
            p->c = &c;
            p->results = results;
            assert(pthread_create(&receivers[i], NULL, reader2, p) == 0);
        }
        // Wait for all threads
        for (size_t i = 0; i < tcount; i++) {
            pthread_join(senders[i], NULL);
            pthread_join(receivers[i], NULL);
        }
        // Check results
        for (size_t i = 0; i < tcount; i++) {
            assert(results[i] == 1);
        }
        chb_free(&c); 
    }
}


