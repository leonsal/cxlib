#include <stdio.h>
#include <assert.h>

#define cx_chan_name mychan
#define cx_chan_type int
#define cx_chan_implement
#include "cx_chan.h"

typedef struct Params {
    char*   name;
    mychan* c;
    int     data;
} Params;

int writer(void* a) {

    Params* p = a;
    printf("%s send:%d\n", p->name, p->data);
    mychan_send(p->c, p->data);
    printf("%s SENT:%d\n", p->name, p->data);
    return thrd_success;
}

int reader(void* a) {

    Params* p = a;
    printf("%s recv\n", p->name);
    int data = mychan_recv(p->c);
    printf("%s RECV:%d\n", p->name, data);
    return thrd_success;
}

int main() {

    mychan c = mychan_init(0);

    thrd_t w1;
    assert(thrd_create(&w1, writer,
        &(Params){.name = "writer1", .c=&c, .data=1}) == thrd_success);

    thrd_t w2;
    assert(thrd_create(&w2, writer,
        &(Params){.name = "writer2", .c=&c, .data=2}) == thrd_success);
   
    thrd_t r1;
    assert(thrd_create(&r1, reader,
        &(Params){.name = "reader1", .c=&c}) == thrd_success);

    thrd_t r2;
    assert(thrd_create(&r2, reader,
        &(Params){.name = "reader2", .c=&c}) == thrd_success);
    
    // thrd_t r2;
    // assert(thrd_create(&r2, reader2, &c1) == thrd_success);

    thrd_sleep(&(struct timespec){.tv_sec=10}, NULL);
    return 0;
}

// #include "alloc.h"
// #include "array.h"
// #include "hmap.h"
// #include "string.h"
// #include "queue.h"
//
// int main() {
//
//     cxAllocBlockTests();
//     cxArrayTests();
//     cxHmapTests();
//     cxStrTests();
//     cxQueueTests();
//
//     return 0;
// }
//
