#include <ctype.h>
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
#ifdef cx_log_mtsafe
    #include <pthread.h>
    #define cx_log_locker_      pthread_mutex_t locker;
    #define cx_log_lock_(l)     assert(pthread_mutex_lock(&l->locker)==0)
    #define cx_log_unlock_(l)   assert(pthread_mutex_unlock(&l->locker)==0)
#else
    #define cx_log_locker_  
    #define cx_log_lock_(l) 
    #define cx_log_unlock_(l)
#endif

// Terminal color codes
#ifndef CX_LOG_TERM_COLORS
#define CX_LOG_TERM_COLORS
#define CX_LOG_TERM_RESET          "\x1b[0m"
#define CX_LOG_TERM_FG_BLACK       "\x1b[30m"
#define CX_LOG_TERM_FG_RED         "\x1b[31m"
#define CX_LOG_TERM_FG_GREEN       "\x1b[32m"
#define CX_LOG_TERM_FG_YELLOW      "\x1b[33m"
#define CX_LOG_TERM_FG_BLUE        "\x1b[34m"
#define CX_LOG_TERM_FG_MAGENTA     "\x1b[35m"
#define CX_LOG_TERM_FG_CYAN        "\x1b[36m"
#define CX_LOG_TERM_FG_GRAY        "\x1b[37m"
#define CX_LOG_TERM_FG_BRED        "\x1b[91m"
#define CX_LOG_TERM_FG_BGREEN      "\x1b[92m"
#define CX_LOG_TERM_FG_BYELLOW     "\x1b[93m"
#define CX_LOG_TERM_FG_BBLUE       "\x1b[94m"
#define CX_LOG_TERM_FG_BMAGENTA    "\x1b[95m"
#define CX_LOG_TERM_FG_BCYAN       "\x1b[96m"
#define CX_LOG_TERM_FG_BWHITE      "\x1b[97m"
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

#ifndef CX_LOG_H 
#define CX_LOG_H
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
    const char *fmt;            // Format string passed by user
    struct timespec time;       // Event time
    void *hdata;                // Optional handler data
    char timeFormatted[128];    // Buffer with formatted time
} CxLogEvent;
#endif

typedef struct cx_log_name cx_log_name;
typedef void (*cx_log_name_(_handler))(cx_log_name* l, CxLogEvent *ev);

typedef struct cx_log_name_(_handler_info) {
    cx_log_name_(_handler) hfunc;
    void*           hdata;
} cx_log_name_(_handler_info);

typedef struct cx_log_name {
    cx_log_locker_ 
    bool            disabled_;
    CxLogFlag       flags_;
    CxLogLevel      level_;
    cx_log_name_(_handler_info) handlers_[cx_log_max_handlers];
} cx_log_name;

cx_log_api_ cx_log_name cx_log_name_(_init)();
cx_log_api_ void cx_log_name_(_free)(cx_log_name* l);
cx_log_api_ CxLogLevel cx_log_name_(_get_level)(cx_log_name* l);
cx_log_api_ int cx_log_name_(_set_level)(cx_log_name* l, CxLogLevel level);
cx_log_api_ const char* cx_log_name_(_get_level_str)(cx_log_name* l);
cx_log_api_ int cx_log_name_(_set_level_str)(cx_log_name* l, const char* lstr);
cx_log_api_ void cx_log_name_(_set_flags)(cx_log_name* l, CxLogFlag flags);
cx_log_api_ void cx_log_name_(_enable)(cx_log_name* l, bool enable); 
cx_log_api_ CxLogFlag cx_log_name_(_flags)(const cx_log_name* l);
cx_log_api_ void cx_log_name_(_emit)(cx_log_name* l, CxLogLevel level, const char *fmt, va_list ap);
cx_log_api_ int cx_log_name_(_add_handler)(cx_log_name* l, cx_log_name_(_handler) h, void* data);
cx_log_api_ int cx_log_name_(_del_handler)(cx_log_name* l, cx_log_name_(_handler) h);
cx_log_api_ void cx_log_name_(_console_handler)(cx_log_name* l, CxLogEvent *ev);
cx_log_api_ void cx_log_name_(_file_handler)(cx_log_name* l, CxLogEvent *ev);
cx_log_api_ void cx_log_name_(_deb)(cx_log_name* l, const char* fmt, ...) __attribute__((format(printf,2,3)));
cx_log_api_ void cx_log_name_(_info)(cx_log_name* l, const char* fmt, ...) __attribute__((format(printf,2,3)));
cx_log_api_ void cx_log_name_(_warn)(cx_log_name* l, const char* fmt, ...) __attribute__((format(printf,2,3)));
cx_log_api_ void cx_log_name_(_error)(cx_log_name* l, const char* fmt, ...) __attribute__((format(printf,2,3)));
cx_log_api_ void cx_log_name_(_fatal)(cx_log_name* l, const char* fmt, ...) __attribute__((format(printf,2,3)));


//
// Implementation
//
#ifdef cx_log_implement
    static const char* cx_log_name_(_level_strings)[] = {
        "DEBUG", "INFO", "WARN", "ERROR", "FATAL",
    };

    const char* cx_log_name_(_level_colors)[] = {
        CX_LOG_TERM_FG_CYAN, CX_LOG_TERM_FG_GREEN, CX_LOG_TERM_FG_YELLOW, CX_LOG_TERM_FG_RED, CX_LOG_TERM_FG_MAGENTA
    };

    cx_log_api_ cx_log_name cx_log_name_(_init)() {
        cx_log_name log = {0};
#ifdef cx_log_mtsafe
        assert(pthread_mutex_init(&log.locker, NULL) == 0);
#endif
        return log;
    }

    cx_log_api_ void cx_log_name_(_free)(cx_log_name* l) {
#ifdef cx_log_mtsafe
        assert(pthread_mutex_destroy(&l->locker) == 0);
#endif
    }

    cx_log_api_ CxLogLevel cx_log_name_(_get_level)(cx_log_name* l) {
        return l->level_;
    }

    cx_log_api_ int cx_log_name_(_set_level)(cx_log_name* l, CxLogLevel level) {
        if (level > CX_LOG_FATAL) {
            return 1;
        }
        l->level_ = level;
        return 0;
    }

    cx_log_api_ const char* cx_log_name_(_get_level_str)(cx_log_name* l) {
        return cx_log_name_(_level_strings)[l->level_];
    }

    cx_log_api_ int cx_log_name_(_set_level_str)(cx_log_name* l, const char* lstr) {
        char upper[32];
        const size_t len = strlen(lstr);
        if (len >= sizeof(upper)) {
            return 1;
        }
        for (size_t i = 0; i < len; i++) {
            upper[i] = toupper(lstr[i]);
        }
        upper[len] = 0;
        const size_t lcount = sizeof(cx_log_name_(_level_strings))/sizeof(const char*);
        for (size_t i = 0; i < lcount; i++) {
            if (strcmp(cx_log_name_(_level_strings)[i], upper) == 0) {
                l->level_ = i;
                return 0;
            }
        }
        return 1;
    }

    cx_log_api_ void cx_log_name_(_set_flags)(cx_log_name* l, CxLogFlag flags) {
        l->flags_ = flags;
    }

    cx_log_api_ void cx_log_name_(_enable)(cx_log_name* l, bool enable) {
        l->disabled_ = !enable;
    }

    cx_log_api_ CxLogFlag cx_log_name_(_flags)(const cx_log_name* l) {
        return l->flags_;
    }

    cx_log_api_ void cx_log_name_(_emit)(cx_log_name* l, CxLogLevel level, const char* fmt, va_list ap) {

        if (l->disabled_) {
            return;
        }

        if (level < l->level_) {
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
        if (l->flags_ & CX_LOG_FLAG_DATE) {
            dateStr[strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", time)] = 0;
            strcat(ev.timeFormatted, dateStr);
        }
        char timeStr[32] = {};
        if (l->flags_ & CX_LOG_FLAG_TIME) {
            timeStr[strftime(timeStr, sizeof(timeStr), "%H:%M:%S", time)] = 0;
            if (strlen(dateStr)) {
                strcat(ev.timeFormatted, " ");
            }
            strcat(ev.timeFormatted, timeStr);
        }
        char fracStr[32] = {};
        if (l->flags_ & CX_LOG_FLAG_MS) {
            snprintf(fracStr, sizeof(fracStr), "%03ld", ev.time.tv_nsec/1000000);
        } else if (l->flags_ & CX_LOG_FLAG_US) {
            snprintf(fracStr, sizeof(fracStr), "%06ld", ev.time.tv_nsec/1000);
        }
        if (strlen(fracStr)) {
            if (strlen(ev.timeFormatted)) {
                strcat(ev.timeFormatted, ".");
            }
            strcat(ev.timeFormatted, fracStr);
        }

        cx_log_lock_(l);
        // Call installed handlers
        for (size_t i = 0; i < cx_log_max_handlers && l->handlers_[i].hfunc; i++) {
            cx_log_name_(_handler_info)* hi = &l->handlers_[i];
            va_copy(ev.ap,ap);
            ev.hdata = hi->hdata;
            hi->hfunc(l, &ev);
            va_end(ev.ap);
        }
        cx_log_unlock_(l);

        if (level == CX_LOG_FATAL) {
            abort();
        }
    }

    cx_log_api_ int cx_log_name_(_add_handler)(cx_log_name* l, cx_log_name_(_handler) h, void* data) {

        for (size_t i = 0; i < cx_log_max_handlers; i++) {
            if (!l->handlers_[i].hfunc) {
                l->handlers_[i] = (cx_log_name_(_handler_info)){h, data};
                return i;
            }
        }
        return -1;
    }

    cx_log_api_ int cx_log_name_(_del_handler)(cx_log_name* l, cx_log_name_(_handler) h) {

        for (size_t i = 0; i < cx_log_max_handlers; i++) {
            if (l->handlers_[i].hfunc == h) {
                l->handlers_[i].hfunc = NULL;
                return i;
            }
        }
        return -1;
    }

    cx_log_api_ void cx_log_name_(_console_handler)(cx_log_name* l, CxLogEvent *ev) {

        FILE* out = stderr;
        if (l->flags_ & CX_LOG_FLAG_COLOR) {
            fprintf(out, CX_LOG_TERM_RESET CX_LOG_TERM_FG_GRAY "%s %s%-5s ",
                ev->timeFormatted, cx_log_name_(_level_colors)[ev->level], cx_log_name_(_level_strings)[ev->level]);
            fprintf(out,  CX_LOG_TERM_FG_BWHITE);
            vfprintf(out, ev->fmt, ev->ap);
            fprintf(out,  CX_LOG_TERM_RESET);
        } else {
            fprintf(out, "%s %-5s: ", ev->timeFormatted, cx_log_name_(_level_strings)[ev->level]);
            vfprintf(out, ev->fmt, ev->ap);
        }
        fprintf(out, "\n");
        fflush(out);
    }
    
    cx_log_api_ void cx_log_name_(_file_handler)(cx_log_name* l, CxLogEvent *ev) {

        FILE* out = ev->hdata;
        fprintf(out, "%s %-5s: ", ev->timeFormatted, cx_log_name_(_level_strings)[ev->level]);
        vfprintf(out, ev->fmt, ev->ap);
        fprintf(out, "\n");
        fflush(out);
    }

    cx_log_api_ void cx_log_name_(_deb)(cx_log_name* l, const char* fmt, ...) {
        if (l->disabled_) return;
        va_list ap;
        va_start(ap, fmt);
        cx_log_name_(_emit)(l, CX_LOG_DEBUG, fmt, ap);
        va_end(ap);
    }

    cx_log_api_ void cx_log_name_(_info)(cx_log_name* l, const char* fmt, ...) {
        if (l->disabled_) return;
        va_list ap;
        va_start(ap, fmt);
        cx_log_name_(_emit)(l, CX_LOG_INFO, fmt, ap);
        va_end(ap);
    }

    cx_log_api_ void cx_log_name_(_warn)(cx_log_name* l, const char* fmt, ...) {
        if (l->disabled_) return;
        va_list ap;
        va_start(ap, fmt);
        cx_log_name_(_emit)(l, CX_LOG_WARN, fmt, ap);
        va_end(ap);
    }

    cx_log_api_ void cx_log_name_(_error)(cx_log_name* l, const char* fmt, ...) {
        if (l->disabled_) return;
        va_list ap;
        va_start(ap, fmt);
        cx_log_name_(_emit)(l, CX_LOG_ERROR, fmt, ap);
        va_end(ap);
    }

    cx_log_api_ void cx_log_name_(_fatal)(cx_log_name* l, const char* fmt, ...) {
        if (l->disabled_) return;
        va_list ap;
        va_start(ap, fmt);
        cx_log_name_(_emit)(l, CX_LOG_FATAL, fmt, ap);
        va_end(ap);
    }


#endif // cx_log_implement

// Undefine config  macros
#undef cx_log_name
#undef cx_log_type
#undef cx_log_static
#undef cx_log_inline
#undef cx_log_max_handlers
#undef cx_log_mtsafe
#undef cx_log_implement

// Undefine internal macros
#undef cx_log_concat2_
#undef cx_log_concat1_
#undef cx_log_name_
#undef cx_log_api_
#undef cx_log_locker_
#undef cx_log_lock_
#undef cx_log_unlock_


