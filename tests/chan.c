#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "cx_alloc_pool.h"
#include "chan.h"

#define cx_chan_name chu
#define cx_chan_type int
#define cx_chan_static
#define cx_chan_implement
#include "cx_chan.h"

void cxChanTests(void) {

}

typedef struct Params {
    chu*  c;
    int   data;
    int*  results;
} Params;

static int closer(void* d) {

    Params* p = d;
    // Waits 100ms before closing
    thrd_sleep(&(struct timespec){.tv_nsec=100000000}, NULL);
    chu_close(p->c);
    return thrd_success;
}

static int writer(void* d) {

    Params* p = d;
    printf("writer before send: %d\n", p->data);
    chu_send(p->c, p->data);
    printf("writer after send: %d\n", p->data);
    free(p);
    return thrd_success;
}

static int reader(void* d) {

    Params* p = d;
    int v = chu_recv(p->c);
    p->results[v]++;
    free(p);
    return thrd_success;
}

void cxChanTest(const CxAllocator* alloc) {

    // Creates unbuffered channel, starts channel closer,
    // sends data and waits for return
    chu c1 = chu_init(0);
    assert(chu_len(&c1) == 0);
    assert(chu_cap(&c1) == 0);
    thrd_t t1;
    Params p1 = {.c = &c1};
    assert(thrd_create(&t1, closer, &p1) == thrd_success);
    assert(chu_send(&c1, 1) == false);
    assert(chu_send(&c1, 2) == false);
    assert(chu_isclosed(&c1));
    chu_free(&c1); 

    // Creates unbuffered channel, starts channel closer,
    // try to receive data and waits for return
    chu c2 = chu_init(0);
    thrd_t t2;
    Params p2 = {.c = &c2};
    assert(thrd_create(&t1, closer, &p2) == thrd_success);
    assert(chu_recv(&c2) == 0);
    assert(chu_recv(&c2) == 0);
    assert(chu_isclosed(&c2));
    chu_free(&c2); 

    // Creates unbuffered channel, starts N senders and N receivers
    chu c3 = chu_init(0);
    size_t const count = 10;
    thrd_t senders[count];
    thrd_t receivers[count];
    int results[count];
    memset(results, 0, sizeof(results[0]) * count);
    for (size_t i = 0; i < count; i++) {
        Params* p = malloc(sizeof(Params));
        p->c = &c3;
        p->data = i;
        assert(thrd_create(&senders[i], writer, p) == thrd_success);
    }
    for (size_t i = 0; i < count; i++) {
        Params* p = malloc(sizeof(Params));
        p->c = &c3;
        p->results = results;
        assert(thrd_create(&receivers[i], reader, p) == thrd_success);
    }
    // Wait for all threads
    for (size_t i = 0; i < count; i++) {
        thrd_join(senders[i], NULL);
        thrd_join(receivers[i], NULL);
    }
    // Check results
    for (size_t i = 0; i < count; i++) {
        assert(results[i] == 1);
    }
    chu_free(&c3); 


    // Unbuffered channel


}


