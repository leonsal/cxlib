#include <assert.h>
#include <pthread.h>

#include "cx_timer_man.h"
#include "cx_alloc.h"

typedef struct TimerTask {
    struct timespec abstime;
    CxTimerFunc     fn;
    void*           arg;
} TimerTask;

// Define internal array of timer tasks
#define cx_array_name arr_task
#define cx_array_type TimerTask
#define cx_array_implement
#define cx_array_static
#define cx_array_instance_allocator
#include "cx_array.h"

typedef struct CxTimerMan {
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    pthread_t       tid;
    arr_task        tasks;
} CxTimerMan;

#define NANOSECS_PER_SEC    (1000000000)

static void* cx_timer_man_thread(void* arg);


CxTimerMan* cx_timer_man_create(const CxAllocator* alloc) {

    if (alloc == NULL) {
        alloc = cxDefaultAllocator();
    }
    CxTimerMan* tm = cx_alloc_malloc(alloc, sizeof(CxTimerMan));
    if (tm == NULL) {
        return NULL;
    }
    assert(pthread_mutex_init(&tm->lock, NULL) == 0); 
    assert(pthread_cond_init(&tm->cond, NULL) == 0); 
    assert(pthread_create(&tm->tid, NULL, cx_timer_man_thread, tm) == 0);
    tm->tasks = arr_task_init(alloc);
    return tm;
}

int cx_timer_man_set(CxTimerMan* tm, struct timespec reltime, CxTimerFunc fn, void* arg, size_t* task_id) {

    assert(pthread_mutex_lock(&tm->lock) == 0);

    // Calculates absolute time from now from the specified relative time
    struct timespec abstime;
    clock_gettime(CLOCK_REALTIME, &abstime);
    abstime.tv_sec += reltime.tv_sec;
    abstime.tv_nsec += reltime.tv_nsec;
    if (abstime.tv_nsec >= NANOSECS_PER_SEC) {
        abstime.tv_nsec -= NANOSECS_PER_SEC;
        abstime.tv_sec += 1;
    }

    // Prepare new task to saves in the tasks array
    TimerTask task = {
        .abstime = abstime,
        .fn = fn,
        .arg = arg,
    };

    // Looks for empty slot in tasks array
    size_t id = SIZE_MAX;
    for (size_t i = 0; i < arr_task_len(&tm->tasks); i++) {
        if (tm->tasks.data[i].fn == NULL) {
            tm->tasks.data[i] = task;
            id = i;
            break;
        }
    }

    // If empty slot not found, appends at the end of the array
    if (id == SIZE_MAX) {
        arr_task_push(&tm->tasks, task);
        id = arr_task_len(&tm->tasks) - 1;
    }

    *task_id = id;
    assert(pthread_mutex_unlock(&tm->lock) == 0);
    return 0;
}


static void* cx_timer_man_thread(void* arg) {

    CxTimerMan* tm = arg;




    return NULL;
}

