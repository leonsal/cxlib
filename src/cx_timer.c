#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>

#include "cx_alloc.h"
#include "cx_timer.h"

// Type for timer task
typedef struct TimerTask {
    size_t          id;
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
    CmdExit,
} TCommand;

// Timer state
typedef struct CxTimer {
    const CxAllocator*  alloc;
    pthread_mutex_t     lock;
    pthread_cond_t      cond;
    int                 cmd;
    size_t              next_id;
    pthread_t           tid;
    list_task           tasks;
    void*               userdata;
} CxTimer;

#define NANOSECS_PER_SEC    (1000000000)

#define CHKPTN(CALL) {int res = CALL; if (res) { return NULL; }}
#define CHKPTI(CALL) {int res = CALL; if (res) { return res;  }}

// Forward declarations of local functions
static void* cx_timer_thread(void* arg);
static bool cx_timer_del_task(CxTimer* tm, size_t task_id);


CxTimer* cx_timer_create(const CxAllocator* alloc) {

    if (alloc == NULL) {
        alloc = cxDefaultAllocator();
    }
    CxTimer* tm = cx_alloc_malloc(alloc, sizeof(CxTimer));
    if (tm == NULL) {
        return NULL;
    }

    tm->alloc = alloc;
    CHKPTN(pthread_mutex_init(&tm->lock, NULL));
    CHKPTN(pthread_cond_init(&tm->cond, NULL));
    tm->cmd = CmdWait;
    tm->next_id = 1;
    tm->tasks = list_task_init(alloc);
    tm->userdata = NULL;

    CHKPTN(pthread_create(&tm->tid, NULL, cx_timer_thread, tm));
    return tm;
}

int cx_timer_destroy(CxTimer* tm) {

    // Signals thread to exit and waits for it to finish
    CHKPTI(pthread_mutex_lock(&tm->lock));
    tm->cmd = CmdExit;
    CHKPTI(pthread_cond_signal(&tm->cond));
    CHKPTI(pthread_mutex_unlock(&tm->lock));
    CHKPTI(pthread_join(tm->tid, NULL));

    // Destroy allocated resources
    CHKPTI(pthread_mutex_destroy(&tm->lock));
    CHKPTI(pthread_cond_destroy(&tm->cond));
    list_task_free(&tm->tasks);
    cx_alloc_free(tm->alloc, tm, sizeof(CxTimer));
    return 0;
}

void cx_timer_set_userdata(CxTimer* tm, void* userdata) {

    tm->userdata = userdata;
}

void* cx_timer_get_userdata(CxTimer* tm) {

    return tm->userdata;
}

int cx_timer_set(CxTimer* tm, struct timespec reltime, CxTimerFunc fn, void* arg, size_t* tid) {

    CHKPTI(pthread_mutex_lock(&tm->lock));

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
    const size_t task_id = tm->next_id++;
    TimerTask task = {
        .id = task_id,
        .abstime = abstime,
        .fn = fn,
        .arg = arg,
    };

    // Adds new task to the list
    if (list_task_count(&tm->tasks) == 0) {
        list_task_push(&tm->tasks, task);
    } else {
        list_task_iter iter;
        TimerTask* ct = list_task_first(&tm->tasks, &iter);
        while (ct) {
            if (cx_timer_cmp_ts(abstime, ct->abstime) <= 0) {
                list_task_ins_before(&iter, task);
                break;
            }
            ct = list_task_next(&iter);
        }
        if (ct == NULL) {
            list_task_push(&tm->tasks, task);
        }
    }

    // Saves task id if requested
    if (tid) {
        *tid = task_id;
    }

    // Prepare command to send to timer thread
    tm->cmd = CmdTask;
    CHKPTI(pthread_cond_signal(&tm->cond));
    CHKPTI(pthread_mutex_unlock(&tm->lock));
    return 0;
}

int cx_timer_clear(CxTimer* tm, size_t task_id) {

    CHKPTI(pthread_mutex_lock(&tm->lock));
    cx_timer_del_task(tm, task_id);
    CHKPTI(pthread_mutex_unlock(&tm->lock));
    CHKPTI(pthread_cond_signal(&tm->cond));
}

int cx_timer_clear_all(CxTimer* tm) {

    CHKPTI(pthread_mutex_lock(&tm->lock));
    list_task_clear(&tm->tasks);
    CHKPTI(pthread_mutex_unlock(&tm->lock));
    CHKPTI(pthread_cond_signal(&tm->cond));
}

size_t cx_timer_count(CxTimer* tm) {

    size_t count;
    CHKPTI(pthread_mutex_lock(&tm->lock));
    count = list_task_count(&tm->tasks);
    CHKPTI(pthread_mutex_unlock(&tm->lock));
    return count;
}

struct timespec cx_timer_ts_from_secs(double secs) {

    double intpart;
    const double fracpart = modf(secs, &intpart);
    return (struct timespec){.tv_sec = intpart, .tv_nsec = fracpart * NANOSECS_PER_SEC};
}

double cx_timer_secs_from_ts(struct timespec ts) {

    return (double)ts.tv_sec + (double)ts.tv_nsec / (double)NANOSECS_PER_SEC;
}

int cx_timer_cmp_ts(struct timespec t1, struct timespec t2) {
    
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

static void* cx_timer_thread(void* arg) {

    CxTimer* tm = arg;
    TimerTask task = {0};       // Current task
    list_task_iter iter;
    int res;

    CHKPTN(pthread_mutex_lock(&tm->lock));
    while (1) {

        // Wait for command or task wait time expired
        while (tm->cmd == CmdWait) {
            // If no current task, waits for command
            if (task.fn == NULL) {
                res = pthread_cond_wait(&tm->cond, &tm->lock);
            // Waits for command or task timeout
            } else {
                res = pthread_cond_timedwait(&tm->cond, &tm->lock, &task.abstime);
            }

            // If timeout, excutes the task function
            if (res == ETIMEDOUT) {
                CHKPTN(pthread_mutex_unlock(&tm->lock));
                task.fn(tm, task.arg);
                CHKPTN(pthread_mutex_lock(&tm->lock));
                // Deletes task if still exists
                cx_timer_del_task(tm, task.id);
                // Get next task
                task.fn = NULL; // No current task
                tm->cmd = CmdTask;
                break;
            }
            // Other pthread errors
            if (res) {
                printf("cx_timer_thread PTHREAD ERROR:%d\n", res);
                goto exit;
            }
        }

        // Checks for exit command
        if (tm->cmd == CmdExit) {
            goto exit;
        }

        // Task command: get next task (first) from the list
        // NOTE: could be the same task it was waiting before.
        if (tm->cmd == CmdTask) {
            TimerTask* ptask = list_task_first(&tm->tasks, &iter);
            if (ptask == NULL) {
                tm->cmd = CmdWait;
                continue;
            }
            task = *ptask;
            tm->cmd = CmdWait;
            continue;
        }
    }

exit:
    CHKPTN(pthread_mutex_unlock(&tm->lock));
    return NULL;
}

static bool cx_timer_del_task(CxTimer* tm, size_t task_id) {

    list_task_iter iter;
    TimerTask* ptask = list_task_first(&tm->tasks, &iter);
    while (ptask) {
        if (ptask->id == task_id) {
            list_task_del(&iter, true);
            return true;
        }
    }
    return false;
}


