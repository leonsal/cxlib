#include <stdlib.h>
#include <stdio.h>
#include "cx_bqueue.h"

typedef struct Buffer {
    void*   data;   // Pointer to buffer data
    size_t  cap;    // Buffer capacity
    size_t  len;    // Length of block in the buffer
} Buffer;

// Define internal list of Buffers
#define cx_list_name        cxlist
#define cx_list_type        Buffer
#define cx_list_static
#define cx_list_instance_allocator
#define cx_list_implement
#include "cx_list.h"

typedef struct CxBqueue {
    const CxAllocator* alloc;
    cxlist          free_bufs;
    cxlist          used_bufs;
    CxBqueueStats   s;
} CxBqueue;

CxBqueue* cx_bqueue_new(const CxAllocator* alloc) {

    CxBqueue* q = cx_alloc_mallocz(alloc, sizeof(CxBqueue));
    q->free_bufs = cxlist_init(alloc);
    q->used_bufs = cxlist_init(alloc);
    return q;
}

void cx_bqueue_del(CxBqueue* q) {

    cxlist_iter iter = {0};

    // Deallocate free buffers
    Buffer* buf = cxlist_first(&q->free_bufs, &iter);
    while (buf) {
        cx_alloc_free(q->alloc, buf->data, buf->cap);
        buf = cxlist_next(&iter);
    }
    cxlist_free(&q->free_bufs);

    // Deallocate used buffers
    buf = cxlist_first(&q->used_bufs, &iter);
    while (buf) {
        cx_alloc_free(q->alloc, buf->data, buf->cap);
        buf = cxlist_next(&iter);
    }
    cxlist_free(&q->used_bufs);

    cx_alloc_free(q->alloc, q, sizeof(CxBqueue));
}

void cx_bqueue_clear(CxBqueue* q) {

    // Move buffers from used list to free list
    while (cxlist_count(&q->used_bufs)) {
        Buffer buf = cxlist_pop(&q->used_bufs);
        cxlist_push(&q->free_bufs, buf);
    }
}

size_t cx_bqueue_count(const CxBqueue* q) {

    return cxlist_count(&q->used_bufs);
}

static inline size_t next_pow2(size_t val) {

    size_t res = 2;
    while (res <= val) {
        res = res << 1;
    }
    return res;
}

void* cx_bqueue_put(CxBqueue* q, size_t nbytes) {

    Buffer buf;
    // Recycle/reallocate free buffer
    if (cxlist_count(&q->free_bufs)) {
        buf = cxlist_popf(&q->free_bufs);
        if (buf.cap < nbytes) {
            const size_t new_cap =  next_pow2(nbytes+1);
            void* data = cx_alloc_malloc(q->alloc, new_cap);
            cx_alloc_free(q->alloc, buf.data, buf.cap);
            q->s.allocmem += new_cap - buf.cap;
            buf.data = data;
            buf.cap = new_cap;
            q->s.nreallocs++;
        }
    // Allocates new buffer
    } else {
        buf.cap = next_pow2(nbytes+1);
        buf.data = cx_alloc_malloc(q->alloc, buf.cap);
        q->s.allocmem += buf.cap;
        q->s.nallocs++;
    }

    buf.len = nbytes;
    cxlist_push(&q->used_bufs, buf);
    return buf.data;
}

void* cx_bqueue_get(CxBqueue* q, size_t* nbytes) {

    if (cxlist_count(&q->used_bufs) == 0) {
        return NULL;
    }
    // Remove buffer from used list and inserts into free list
    Buffer buf = cxlist_popf(&q->used_bufs);
    cxlist_push(&q->free_bufs, buf);

    *nbytes = buf.len;
    return buf.data;
}

CxBqueueStats cx_bqueue_stats(const CxBqueue* q) {

    CxBqueueStats s = q->s;
    s.used_blocks = cxlist_count(&q->used_bufs);
    s.free_blocks = cxlist_count(&q->free_bufs);
    return s;
}

void cx_bqueue_pstats(const CxBqueue* q) {

    CxBqueueStats s = cx_bqueue_stats(q);
    printf("used:%zu free:%zu nallocs:%zu nreallocs:%zu allocmem:%zu\n",
            s.used_blocks,
            s.free_blocks,
            s.nallocs,
            s.nreallocs,
            s.allocmem
    );
}


