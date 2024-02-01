#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

#include "cx_alloc.h"
#include "cx_timer.h"

// Type for timer task
typedef struct TimerTask {
    size_t          task_id;
    struct timespec abstime;
    CxTimerFunc     fn;
    void*           arg;
} TimerTask;

// Define linked list of timer tasks
#define cx_list_name list_task
#define cx_list_type TimerTask
#define cx_list_instance_allocator
#define cx_list_implement
#define cx_list_static
#include "cx_list.h"

// Thread command codes
typedef enum {
    CmdWait,
    CmdTask,
    CmdTime,
    CmdExit,
} TCommand;

// Timer state
typedef struct CxTimerMan {
    pthread_mutex_t lock;
    pthread_cond_t  cond;
    int             cmd;
    size_t          next_id;
    pthread_t       tid;
    list_task       tasks;
} CxTimerMan;

#define NANOSECS_PER_SEC    (1000000000)


// Forward declarations of local functions
static void* cx_timer_thread(void* arg);
static int cx_timer_cmp_timespec(struct timespec t1, struct timespec t2);


CxTimerMan* cx_timer_create(const CxAllocator* alloc) {

    if (alloc == NULL) {
        alloc = cxDefaultAllocator();
    }
    CxTimerMan* tm = cx_alloc_malloc(alloc, sizeof(CxTimerMan));
    if (tm == NULL) {
        return NULL;
    }
    assert(pthread_mutex_init(&tm->lock, NULL) == 0); 
    assert(pthread_cond_init(&tm->cond, NULL) == 0); 
    tm->cmd = CmdWait;
    tm->next_id = 1;
    tm->tasks = list_task_init(alloc);
    assert(pthread_create(&tm->tid, NULL, cx_timer_thread, tm) == 0);
    return tm;
}

void  cx_timer_destroy(CxTimerMan* tm) {

    printf("cx_timer_destroy \n");
    tm->cmd = CmdExit;
    assert(pthread_cond_signal(&tm->cond) == 0);
    pthread_join(tm->tid, NULL);
}

int cx_timer_set(CxTimerMan* tm, struct timespec reltime, CxTimerFunc fn, void* arg, size_t* ptid) {

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
    size_t task_id = tm->next_id++;
    TimerTask task = {
        .task_id = task_id,
        .abstime = abstime,
        .fn = fn,
        .arg = arg,
    };

    if (list_task_count(&tm->tasks) == 0) {
        list_task_push(&tm->tasks, task);
    } else {
        list_task_iter iter;
        TimerTask* ct = list_task_first(&tm->tasks, &iter);
        while (ct) {
            if (cx_timer_cmp_timespec(abstime, ct->abstime) < 0) {
                list_task_ins_before(&iter, task);
                break;
            }
            ct = list_task_next(&iter);
        }
    }
    *ptid = task_id;
    tm->cmd = CmdTask;
    assert(pthread_cond_signal(&tm->cond) == 0);
    assert(pthread_mutex_unlock(&tm->lock) == 0);
    return 0;
}


static void* cx_timer_thread(void* arg) {

#define CHKPT(CALL) {int res = CALL; if (res) { return NULL; }}

    printf("cx_timer_thread started\n");
    CxTimerMan* tm = arg;
    CHKPT(pthread_mutex_lock(&tm->lock));
    TimerTask task = {0};
    while (1) {
        // Wait for initial command
        while (tm->cmd == CmdWait) {
            printf("cx_timer_thread cond wait\n");
            CHKPT(pthread_cond_wait(&tm->cond, &tm->lock));
        }
        printf("cx_timer_thread cmd: %d\n", tm->cmd);
        // Checks for exit command
        if (tm->cmd == CmdExit) {
            break;
        }
        // Task command
        if (tm->cmd == CmdTask) {
            list_task_iter iter;
            TimerTask* ptask = list_task_first(&tm->tasks, &iter);
            if (ptask == NULL) {
                printf("cx_timer_thread NO FIRST TASK");
                break;
            }
            task = *ptask;
            printf("cx_timer_thread WAIT for timeout or command\n");
            tm->cmd = CmdWait;
            while (tm->cmd == CmdWait) {
                int res = pthread_cond_timedwait(&tm->cond, &tm->lock, &task.abstime);
                if (res == ETIMEDOUT) {
                    printf("cx_timer_thread TASK TIMEOUT\n");
                    CHKPT(pthread_mutex_unlock(&tm->lock));
                    task.fn(tm, task.arg);
                    CHKPT(pthread_mutex_lock(&tm->lock));
                    break;
                }
                if (res) {
                    printf("cx_timer_thread TASK NEW COMMAND\n");
                    break;
                }
            }
        }
    }
    CHKPT(pthread_mutex_unlock(&tm->lock));
    printf("cx_timer_thread finished\n");
    return NULL;
}

static int cx_timer_cmp_timespec(struct timespec t1, struct timespec t2) {
    
    if (t1.tv_sec < t2.tv_sec) {
        return -1;
    }
    if (t1.tv_sec > t2.tv_sec) {
        return 1;
    }
    if (t1.tv_nsec < t2.tv_nsec) {
        return -1;
    }
    if (t1.tv_nsec > t2.tv_nsec) {
        return 1;
    }
    return 0;
}

