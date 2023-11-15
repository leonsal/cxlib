#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "cx_alloc_pool.h"
#include "chan.h"

#define cx_chan_name chu
#define cx_chan_type double
#define cx_chan_static
#define cx_chan_implement
#include "cx_chan.h"

void cxChanTests(void) {

}

typedef struct Params {
    chu*    c;
    double  data;
} Params;

static int closer(void* d) {

    Params* p = d;
    thrd_sleep(&(struct timespec){.tv_sec=1}, NULL);
    chu_close(p->c);
    return thrd_success;
}

static int writer(void* d) {

    Params* p = d;
    printf("writer send: %f\n", p->data);
    chu_send(p->c, p->data);
    return thrd_success;
}

void cxChanTest(const CxAllocator* alloc) {

    // Creates unbuffered channel, starts channel closer,
    // sends data and waits for return
    chu c1 = chu_init(0);
    thrd_t t1;
    Params p1 = {.c = &c1};
    assert(thrd_create(&t1, closer, &p1) == thrd_success);
    assert(chu_send(&c1, 1) == false);
    chu_free(&c1); 


    // thrd_t tids[10];
    // size_t count = sizeof(tids)/sizeof(tids[0]);
    // for (size_t i = 0; i < count; i++) {
    //     assert(thrd_create(&tids[i], writer, NULL) == thrd_success);
    // }


    // Unbuffered channel


}


