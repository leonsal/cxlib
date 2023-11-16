#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include "cx_alloc.h"

// log type name must be defined
#ifndef cx_log_name
    #error "cx_log_name not defined"
#endif
#ifndef cx_log_max_handlers
    #define cx_log_max_handlers  (4)
#endif
#ifdef cx_log_tsafe
    #include <pthread.h>
    #define cx_log_locker_  pthread_mutex_t locker;
#else
    #define cx_log_locker_  
#endif

// Auxiliary internal macros
#define cx_log_concat2_(a, b) a ## b
#define cx_log_concat1_(a, b) cx_log_concat2_(a, b)
#define cx_log_name_(name) cx_log_concat1_(cx_log_name, name)

// API attributes
#if defined(cx_log_static) && defined(cx_log_inline)
    #define cx_log_api_ static inline
#elif defined(cx_log_static)
    #define cx_log_api_ static
#elif defined(cx_log_inline)
    #define cx_log_api_ inline
#else
    #define cx_log_api_
#endif

//
// Declarations
//

// Log levels
typedef enum {
    CX_LOG_DEBUG,
    CX_LOG_INFO,
    CX_LOG_WARN,
    CX_LOG_ERROR,
    CX_LOG_FATAL,
} CxLogLevel;

// Log flags
typedef enum {
    CX_LOG_FLAG_DATE   = 1 << 0,   // Show date
    CX_LOG_FLAG_TIME   = 1 << 1,   // Show time
    CX_LOG_FLAG_MS     = 1 << 2,   // Show milliseconds
    CX_LOG_FLAG_US     = 1 << 3,   // Show microseconds
    CX_LOG_FLAG_COLOR  = 1 << 4,   // Use color for console handler
} CxLogFlag;

typedef struct CxLogEvent {
    va_list ap;
    CxLogLevel level;           // Current log level
    const char *func;           // Name of the function where the log was emitted
    const char *fmt;            // Format string passed by user
    struct timespec time;       // Event time
    void *hdata;                // Optional handler data
    char timeFormatted[128];    // Buffer with formatted time
} CxLogEvent;

typedef void (*CxLogHandler)(CxLogEvent *ev);

// Log handler info
typedef struct CxLogHandlerInfo {
    CxLogHandler    hfunc;
    void*           hdata;
    CxLogLevel      level;
} CxLogHandlerInfo;

typedef struct cx_log_name {
    cx_log_locker_ 
    bool            disabled;
    CxLogFlag       flags;
    CxLogLevel      level;
    CxLogHandlerInfo handlers[cx_log_max_handlers];
} cx_log_name;

cx_log_api_ cx_log_name cx_log_name_(_init)();
cx_log_api_ void cx_log_name_(_set_level)(cx_log_name* l, CxLogLevel level);
cx_log_api_ void cx_log_name_(_set_flags)(cx_log_name* l, CxLogFlag flags);
cx_log_api_ void cx_log_name_(_enable)(cx_log_name* l, bool enable); 
cx_log_api_ CxLogFlag cx_log_name_(_flags)(const cx_log_name* l);
cx_log_api_ void cx_log_name_(_emit)(cx_log_name* l, CxLogLevel level, const char *fmt, ...);
cx_log_api_ int cx_log_name_(_add_handler)(cx_log_name* l, CxLogHandler h, void* data, CxLogLevel level);
cx_log_api_ int cx_log_name_(_del_handler)(cx_log_name* l, CxLogHandler h);
cx_log_api_ int cx_log_name_(_deb)(cx_log_name* l, const char* fmt, ...);


//
// Implementation
//
#define cx_log_implement
#ifdef cx_log_implement

    cx_log_api_ cx_log_name cx_log_name_(_init)() {
        cx_log_name log = {0};
#ifdef cx_log_tsafe
        assert(pthread_mutex_init(&log.locker, NULL) == 0);
#endif
        return log;
    }
    cx_log_api_ void cx_log_name_(_set_level)(cx_log_name* l, CxLogLevel level) {
        l->level = level;
    }

    cx_log_api_ void cx_log_name_(_set_flags)(cx_log_name* l, CxLogFlag flags) {
        l->flags = flags;
    }

    cx_log_api_ void cx_log_name_(_enable)(cx_log_name* l, bool enable) {
        l->disabled = !enable;
    }

    cx_log_api_ void cx_log_name_(_emit)(cx_log_name* l, CxLogLevel level, const char* fmt, ...) {

        if (l->disabled) {
            return;
        }

        // Initializes log event
        CxLogEvent ev = {
            .fmt   = fmt,
            .level = level,
        };
        timespec_get(&ev.time, TIME_UTC);

        // Formats the event time
        struct tm* time = localtime(&ev.time.tv_sec);
        char dateStr[32] = {};
        if (l->flags & CX_LOG_FLAG_DATE) {
            dateStr[strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", time)] = 0;
            strcat(ev.timeFormatted, dateStr);
        }
        char timeStr[32] = {};
        if (l->flags & CX_LOG_FLAG_TIME) {
            timeStr[strftime(timeStr, sizeof(timeStr), "%H:%M:%S", time)] = 0;
            if (strlen(dateStr)) {
                strcat(ev.timeFormatted, " ");
            }
            strcat(ev.timeFormatted, timeStr);
        }
        char fracStr[32] = {};
        if (l->flags & CX_LOG_FLAG_MS) {
            snprintf(fracStr, sizeof(fracStr), "%03ld", ev.time.tv_nsec/1000000);
        } else if (l->flags & CX_LOG_FLAG_US) {
            snprintf(fracStr, sizeof(fracStr), "%06ld", ev.time.tv_nsec/1000);
        }
        if (strlen(fracStr)) {
            if (strlen(ev.timeFormatted)) {
                strcat(ev.timeFormatted, ".");
            }
            strcat(ev.timeFormatted, fracStr);
        }

        // LOCK

        // Call installed handlers
        for (size_t i = 0; i < cx_log_max_handlers && l->handlers[i].hfunc; i++) {
            CxLogHandlerInfo* hi = &l->handlers[i];
            if (level >= hi->level) {
                va_start(ev.ap, fmt);
                ev.hdata = hi->hdata;
                hi->hfunc(&ev);
                va_end(ev.ap);
            }
        }

        // UNLOCK

        if (level == CX_LOG_FATAL) {
            abort();
        }
    }

#endif // cx_log_implement

// Undefine config  macros
#undef cx_log_name
#undef cx_log_type
#undef cx_log_static
#undef cx_log_inline
#undef cx_log_implement

// Undefine internal macros
#undef cx_log_concat2_
#undef cx_log_concat1_
#undef cx_log_name_
#undef cx_log_api_


