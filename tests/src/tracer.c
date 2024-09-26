#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include "cx_tracer.h"

struct GenArgs {
    CxTracer*   tr;         // Tracer object
    const char* name;       // Event name
    const char* cat;        // Event category
    bool        begin_end;  // Generates begin/end OR instant
    size_t      count;      // Number of executions
    size_t      us;         // Sleep time in microseconds
};

static void* gen_events(void* arg) {

    // Copy received argument
    struct GenArgs ga = *(struct GenArgs*)arg;
    while (ga.count--) {
        if (ga.begin_end) {
            cx_tracer_begin(ga.tr, ga.name, ga.cat);
            usleep(ga.us);
            cx_tracer_end(ga.tr, ga.name, ga.cat);
        } else {
            cx_tracer_instant(ga.tr, ga.name, ga.cat, CxTracerScopeThread);
            usleep(ga.us);
        }
    }
    return NULL;
}


void test_tracer(const CxAllocator* alloc) {

    CxTracer* tr = cx_tracer_new(alloc, 16*1024);

    // Creates event generation thread 1
    pthread_t t1;
    struct GenArgs args1 = {
        .tr = tr,
        .name = "t1",
        .cat = "cat1",
        .begin_end = true,
        .count = 102,
        .us = 1000,
    };
    pthread_create(&t1, NULL, gen_events, &args1);

    // Creates event generation thread 2
    pthread_t t2;
    struct GenArgs args2 = {
        .tr = tr,
        .name = "t2",
        .begin_end = true,
        .cat = "cat2",
        .count = 110,
        .us = 2000,
    };
    pthread_create(&t2, NULL, gen_events, &args2);

    // Creates event generation thread 3
    pthread_t t3;
    struct GenArgs args3 = {
        .tr = tr,
        .name = "t3",
        .begin_end = false,
        .cat = "cat3",
        .count = 120,
        .us = 1500,
    };
    pthread_create(&t3, NULL, gen_events, &args3);

    // Wait for threads to finish
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    const size_t nevs = cx_tracer_get_count(tr);
    CXCHK(nevs == (args1.count + args2.count)*2 + args3.count);

    CxError err = cx_tracer_json_write_file(tr, "test_tracer.json");
    CXCHK(err.msg == NULL);

    cx_tracer_del(tr);

}

