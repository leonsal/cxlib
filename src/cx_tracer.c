#include <bits/time.h>
#include <time.h>
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <pthread.h>

#include "cx_error.h"
#define cx_str_name cxstr
#define cx_str_static
#define cx_str_instance_allocator
#define cx_str_implement
#include "cx_str.h"

#include "cx_tracer.h"

typedef struct CxTracerEvent {
    cxstr       name;
    cxstr       cat;
    pid_t       pid;
    int         tid;
    char        ph;
    CxTracerScope scope;
    struct timespec ts;
} CxTracerEvent;

typedef struct CxTracer {
    const CxAllocator*  alloc;
    CxTracerEvent*      events;
    size_t              cap;
    size_t              count;
    size_t              next_tid;
    pthread_mutex_t     lock;
} CxTracer;

// Thread local generated thread id
static _Thread_local int thread_local_id = 0;

// Forward declaration of local functions
static CxTracerEvent* cx_tracer_append_event(CxTracer* tr, const char* name, const char* cat);

CxTracer* cx_tracer_new(const CxAllocator* alloc, size_t cap) {

    // Creates CxTracer state
    CxTracer* tr = cx_alloc_mallocz(alloc, sizeof(CxTracer));
    tr->alloc = alloc;
    CXCHKZ(pthread_mutex_init(&tr->lock, NULL));

    // Creates array of CxTracer events
    tr->events = cx_alloc_mallocz(alloc, sizeof(CxTracerEvent) * cap);
    tr->cap = cap;

    // Reserve events string size to avoid allocations when appending events.
    for (size_t i = 0; i < tr->cap; i++) {
        CxTracerEvent* ev = &tr->events[i];
        ev->name = cxstr_init(alloc);
        cxstr_reserve(&ev->name, 32);
        ev->cat = cxstr_init(alloc);
        cxstr_reserve(&ev->cat, 32);
    }

    tr->count = 0;
    tr->next_tid = 1;
    return tr;
}

void cx_tracer_del(CxTracer* tr) {

    for (size_t i = 0; i < tr->cap; i++) {
        cxstr_free(&tr->events[i].name);
        cxstr_free(&tr->events[i].cat);
    }
    cx_alloc_free(tr->alloc, tr->events, sizeof(CxTracerEvent) * tr->cap);
    CXCHKZ(pthread_mutex_destroy(&tr->lock));
    cx_alloc_free(tr->alloc, tr, sizeof(CxTracer));
}

size_t cx_tracer_get_count(CxTracer* tr) {

    CXCHKZ(pthread_mutex_lock(&tr->lock));
    size_t count = tr->count;
    CXCHKZ(pthread_mutex_unlock(&tr->lock));
    return count;
}

void cx_tracer_clear(CxTracer* tr) {

    CXCHKZ(pthread_mutex_lock(&tr->lock));
    tr->count = 0;
    CXCHKZ(pthread_mutex_unlock(&tr->lock));
}

void cx_tracer_begin(CxTracer* tr, const char* name, const char* cat) {

    CxTracerEvent* ev = cx_tracer_append_event(tr, name, cat);
    if (ev == NULL) {
        return ;
    }
    ev->ph = 'B';
}

void cx_tracer_end(CxTracer* tr, const char* name, const char* cat) {

    CxTracerEvent* ev = cx_tracer_append_event(tr, name, cat);
    if (ev == NULL) {
        return ;
    }
    ev->ph = 'E';
}

void cx_tracer_instant(CxTracer* tr, const char* name, const char* cat, CxTracerScope scope) {

    CxTracerEvent* ev = cx_tracer_append_event(tr, name, cat);
    if (ev == NULL) {
        return ;
    }
    ev->ph = 'i';
    ev->scope = scope;
}

CxError cx_tracer_json_write(CxTracer* tr, CxWriter* out) {

    cxstr evstr = cxstr_init(tr->alloc);
    if (cx_writer_write(out, "[", 1) < 1) {
        return CXERR("Error writing events");
    }
    for (size_t i = 0; i < tr->count; i++) {
        const CxTracerEvent* ev = &tr->events[i];
        size_t ts = (ev->ts.tv_sec * 1000000000 + ev->ts.tv_nsec) / 1000;
        cxstr_clear(&evstr);
        cxstr_printf(&evstr,
            "{\"name\":\"%s\",\"cat\":\"%s\",\"ph\":\"%c\",\"ts\":%lu,\"pid\":%d,\"tid\":%d",
            ev->name.data, ev->cat.data, ev->ph, ts, ev->pid, ev->tid
        );
        if (ev->scope == CxTracerScopeDefault) {
            cxstr_printf(&evstr, "}", 1);
        } else {
            cxstr_printf(&evstr, "\"s\":%c}", (char)ev->scope);
        }
        if (i < tr->count - 1) {
            cxstr_printf(&evstr, ",\n", (char)ev->scope);
        }
        if (cx_writer_write(out, evstr.data, cxstr_len(&evstr)) < (int)cxstr_len(&evstr)) {
            return CXERR("Error writing events");
        }
    }
    if (cx_writer_write(out, "]", 1) < 1) {
        return CXERR("Error writing events");
    }
    cxstr_free(&evstr);
    return CXOK();
}

CxError cx_tracer_json_write_file(CxTracer* tr, const char* path) {

    FILE* f = fopen(path, "w");
    if (f == NULL) {
        return CXERR("Error opening file");
    }
    CxWriter writer = cx_writer_file(f);
    CxError err = cx_tracer_json_write(tr, &writer);
    fclose(f);
    return err;
}

static CxTracerEvent* cx_tracer_append_event(CxTracer* tr, const char* name, const char* cat) {

    // Generates a unique id for the thread once
    CXCHKZ(pthread_mutex_lock(&tr->lock));
    if (thread_local_id == 0) {
        thread_local_id = tr->next_tid++;
    }

    // If event capacity reached, nothing to do
    const size_t idx = tr->count;
    if (idx >= tr->cap) {
        CXCHKZ(pthread_mutex_unlock(&tr->lock));
        return NULL;
    }
    tr->count++;
    CXCHKZ(pthread_mutex_unlock(&tr->lock));

    // Fills the event slot
    CxTracerEvent* ev = &tr->events[idx];
    clock_gettime(CLOCK_REALTIME, &ev->ts);
    ev->pid = getpid();
    ev->tid = thread_local_id;
    ev->scope = CxTracerScopeDefault;
    cxstr_cpy(&ev->name, name);
    cxstr_cpy(&ev->cat, cat);
    return ev;
}

