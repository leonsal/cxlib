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

#define CHKPTN(CALL) {int res = CALL; if (res) { return NULL; }}
#define CHKPTI(CALL) {int res = CALL; if (res) { return res;  }}

// Forward declarations of local functions
static void* cx_timer_thread(void* arg);
static bool cx_timer_del_task(CxTimer* tm, size_t task_id);
static int cx_timer_cmp_timespec(struct timespec t1, struct timespec t2);


CxTimerMan* cx_timer_create(const CxAllocator* alloc) {

    if (alloc == NULL) {
        alloc = cxDefaultAllocator();
    }
    CxTimerMan* tm = cx_alloc_malloc(alloc, sizeof(CxTimerMan));
    if (tm == NULL) {
        return NULL;
    }
    CHKPTN(pthread_mutex_init(&tm->lock, NULL));
    CHKPTN(pthread_cond_init(&tm->cond, NULL));
    tm->cmd = CmdWait;
    tm->next_id = 1;
    tm->tasks = list_task_init(alloc);
    CHKPTN(pthread_create(&tm->tid, NULL, cx_timer_thread, tm));
    return tm;
}

int cx_timer_destroy(CxTimerMan* tm) {

    tm->cmd = CmdExit;
    CHKPTI(pthread_cond_signal(&tm->cond));
    CHKPTI(pthread_join(tm->tid, NULL));
}

int cx_timer_set(CxTimerMan* tm, struct timespec reltime, CxTimerFunc fn, void* arg, size_t* ptid) {

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
            if (cx_timer_cmp_timespec(abstime, ct->abstime) <= 0) {
                list_task_ins_before(&iter, task);
                break;
            }
            ct = list_task_next(&iter);
        }
        if (ct == NULL) {
            list_task_push(&tm->tasks, task);
        }
    }

    list_task_iter iter;
    TimerTask* ct = list_task_first(&tm->tasks, &iter);
    while (ct) {
        printf("count:%zu task_id:%zu abstime:%zu.%zu\n", list_task_count(&tm->tasks), ct->task_id, ct->abstime.tv_sec, ct->abstime.tv_nsec);
        ct = list_task_next(&iter);
    }
    printf("---------------------------------\n");


    if (ptid) {
        *ptid = task_id;
    }
    tm->cmd = CmdTask;
    CHKPTI(pthread_cond_signal(&tm->cond));
    CHKPTI(pthread_mutex_unlock(&tm->lock));
    return 0;
}

int cx_timer_unset(CxTimer* tm, size_t task_id) {

    CHKPTI(pthread_mutex_lock(&tm->lock));
    bool found = cx_timer_del_task(tm, task_id);
    CHKPTI(pthread_mutex_unlock(&tm->lock));
}


static void* cx_timer_thread(void* arg) {


    printf("cx_timer_thread started\n");
    CxTimerMan* tm = arg;
    TimerTask task = {0};       // Current task
    list_task_iter iter;
    int res;

    CHKPTN(pthread_mutex_lock(&tm->lock));
    while (1) {

        // Wait for command or task timeout
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
                //printf("cx_timer_thread TASK TIMEOUT\n");
                CHKPTN(pthread_mutex_unlock(&tm->lock));
                task.fn(tm, task.arg);
                CHKPTN(pthread_mutex_lock(&tm->lock));
                // Deletes task if still exists
                bool found = cx_timer_del_task(tm, task.task_id);
                if (!found) {
                    printf("cx_timer_thread TASK TO DELETE NOT FOUND:%zu\n", task.task_id);
                }
                // Get next task
                task.fn = NULL; // No current task
                tm->cmd = CmdTask;
                break;
            }
            // Other pthread error
            if (res) {
                printf("cx_timer_thread PTHREAD ERROR:%d\n", res);
                goto exit;
            }
        }

        //printf("cx_timer_thread cmd: %d\n", tm->cmd);
        // Checks for exit command
        if (tm->cmd == CmdExit) {
            goto exit;
        }

        // Task command
        if (tm->cmd == CmdTask) {
            TimerTask* ptask = list_task_first(&tm->tasks, &iter);
            if (ptask == NULL) {
                //printf("cx_timer_thread NO MORE TASKS\n");
                tm->cmd = CmdWait;
                continue;
            }
            task = *ptask;
            //printf("cx_timer_thread WAIT: %zu.%zu\n", task.abstime.tv_sec, task.abstime.tv_nsec);
            tm->cmd = CmdWait;
            continue;
        }
    }

exit:
    CHKPTN(pthread_mutex_unlock(&tm->lock));
    printf("cx_timer_thread finished\n");
    return NULL;
}

static bool cx_timer_del_task(CxTimer* tm, size_t task_id) {

    list_task_iter iter;
    TimerTask* ptask = list_task_first(&tm->tasks, &iter);
    while (ptask) {
        if (ptask->task_id == task_id) {
            list_task_del(&iter, true);
            return true;
        }
    }
    return false;
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

