#include <ctype.h>
#include <pthread.h>

#include "cx_logger.h"

// Terminal color codes
#define TERM_RESET          "\x1b[0m"
#define TERM_FG_BLACK       "\x1b[30m"
#define TERM_FG_RED         "\x1b[31m"
#define TERM_FG_GREEN       "\x1b[32m"
#define TERM_FG_YELLOW      "\x1b[33m"
#define TERM_FG_BLUE        "\x1b[34m"
#define TERM_FG_MAGENTA     "\x1b[35m"
#define TERM_FG_CYAN        "\x1b[36m"
#define TERM_FG_GRAY        "\x1b[37m"
#define TERM_FG_BRED        "\x1b[91m"
#define TERM_FG_BGREEN      "\x1b[92m"
#define TERM_FG_BYELLOW     "\x1b[93m"
#define TERM_FG_BBLUE       "\x1b[94m"
#define TERM_FG_BMAGENTA    "\x1b[95m"
#define TERM_FG_BCYAN       "\x1b[96m"
#define TERM_FG_BWHITE      "\x1b[97m"

// Logger handler information
typedef struct HandlerInfo {
    CxLoggerHandler handler;    // Pointer to handler function or NULL if empty
    void*           data;       // Optional associated handler data
} HandlerInfo;

// Define dynamic array of HandlerInfo
#define cx_array_name arr_handler
#define cx_array_type HandlerInfo
#define cx_array_instance_allocator
#define cx_array_static
#define cx_array_implement
#include "cx_array.h"

// CxLogger state
typedef struct CxLogger {
    const CxAllocator*  alloc;
    const char*         prefix;
    bool                enabled;
    CxLoggerFlags       flags;
    CxLoggerLevel       level;
    arr_handler         handlers;
    pthread_mutex_t     lock;
} CxLogger;

// Level names
static const char* level_strings[] = {
    "DEBUG", "INFO", "WARN", "ERROR", "FATAL", NULL
};

// Level colors
static const char* level_colors[] = {
    TERM_FG_CYAN, TERM_FG_GREEN, TERM_FG_YELLOW, TERM_FG_RED, TERM_FG_MAGENTA
};

CxLogger* cx_logger_new(const CxAllocator* alloc, const char* prefix) {

    const CxAllocator* use_alloc = alloc ? alloc : cx_def_allocator();
    CxLogger* logger = cx_alloc_mallocz(use_alloc, sizeof(CxLogger));
    if (logger == NULL) {
        return NULL;
    }
    logger->alloc = use_alloc;
    logger->prefix = prefix;
    logger->enabled = true;
    logger->handlers = arr_handler_init(alloc);
    CXCHKZ(pthread_mutex_init(&logger->lock, NULL));

    return logger;
}

void cx_logger_del(CxLogger* logger) {

    arr_handler_free(&logger->handlers);
    CXCHKZ(pthread_mutex_destroy(&logger->lock));
    cx_alloc_free(logger->alloc, logger, sizeof(CxLogger));
}

CxLoggerLevel cx_logger_get_level(const CxLogger* logger) {

    return logger->level;
}

CxError cx_logger_set_level(CxLogger* logger, CxLoggerLevel level) {

    if (level < 0 || level >= CxLoggerFatal) {
        return CXERROR(1, "invalid log level");
    }
    logger->level = level;
    return CXERROR_OK();
}

const char* cx_logger_get_level_str(const CxLogger* logger) {

    return level_strings[logger->level];
}

CxError cx_logger_set_level_str(CxLogger* logger, const char* level) {

    // Converts user specified level to upper case
    char upper[32];
    const size_t len = strlen(level);
    if (len >= sizeof(upper)-1) {
        return CXERROR(1, "invalid level name");
    }
    for (size_t i = 0; i < len; i++) {
        upper[i] = toupper(level[i]);
    }
    upper[len] = 0;

    // Looks for the specified level
    for (size_t i = 0;; i++) {
        const char* curr = level_strings[i];
        if (curr == NULL) {
            break;
        }
        if (strcmp(curr, upper) == 0){
            logger->level = i;
            return CXERROR_OK();
        }
    }
    return CXERROR(1, "invalid level name");
}

CxLoggerFlags cx_logger_get_flags(const CxLogger* logger) {

    return logger->flags;
}

CxError cx_logger_set_flags(CxLogger* logger, CxLoggerFlags flags) {

    logger->flags = flags;
    return CXERROR_OK();
}

CxError cx_logger_set_enabled(CxLogger* logger, bool enabled) {

    logger->enabled = enabled;
}

CxError cx_logger_add_handler(CxLogger* logger, CxLoggerHandler handler, void* handler_data) {

    for (size_t i = 0; i < arr_handler_len(&logger->handlers); i++) {
        const HandlerInfo* hinfo = &logger->handlers.data[i];
        if (hinfo->handler && hinfo->handler == handler && hinfo->data == handler_data) {
            return CXERROR(1, "handler already installed");
        }
    }
    arr_handler_push(&logger->handlers, (HandlerInfo){.handler = handler, .data = handler_data});
    return CXERROR_OK();
}

void cx_logger_del_handler(CxLogger* logger, CxLoggerHandler handler, void* handler_data) {

    for (size_t i = 0; i < arr_handler_len(&logger->handlers); i++) {
        HandlerInfo* hinfo = &logger->handlers.data[i];
        if (hinfo->handler == handler && hinfo->data == handler_data) {
            hinfo->handler = NULL;
            return;
        }
    }
}

void cx_logger_log(CxLogger* logger, CxLoggerLevel level, const char* fmt, ...) {

    if (!logger->enabled) {
        return;
    }

    if (level < logger->level) {
        return;
    }

    // Initializes log event
    CxLoggerEvent ev = {
        .fmt   = fmt,
        .level = level,
    };
    timespec_get(&ev.time, TIME_UTC);

    // Formats the event time date
    struct tm* time = localtime(&ev.time.tv_sec);
    char date[32] = {};
    if (logger->flags & CxLoggerFlagDate) {
        date[strftime(date, sizeof(date), "%Y-%m-%d", time)] = 0;
        strcat(ev.time_prefix, date);
    }
    // Formats the event time hour , minutes and seconds
    char time_str[32] = {};
    if (logger->flags & CxLoggerFlagTime) {
        time_str[strftime(time_str, sizeof(time_str), "%H:%M:%S", time)] = 0;
        if (strlen(date)) {
            strcat(ev.time_prefix, " ");
        }
        strcat(ev.time_prefix, time_str);
    }
    // Formats the event time milliseconds or microsecodns
    char time_frac[32] = {};
    if (logger->flags & CxLoggerFlagMs) {
        snprintf(time_frac, sizeof(time_frac), "%03ld", ev.time.tv_nsec/1000000);
    } else if (logger->flags & CxLoggerFlagUs) {
        snprintf(time_frac, sizeof(time_frac), "%06ld", ev.time.tv_nsec/1000);
    }
    // Appends formatted milliseconds/microseconds to date/time
    if (strlen(time_frac)) {
        if (strlen(ev.time_prefix)) {
            strcat(ev.time_prefix, ".");
        }
        strcat(ev.time_prefix, time_frac);
    }
    if (strlen(ev.time_prefix)) {
        strcat(ev.time_prefix, " ");
    }

    // Appends optional log previx
    if (logger->prefix) {
        size_t space = sizeof(ev.time_prefix) - strlen(ev.time_prefix);
        strncat(ev.time_prefix, logger->prefix, space-1);
        strncat(ev.time_prefix, ":", strlen(ev.time_prefix));
    }

    va_list ap;
    va_start(ap, fmt);

    // Call installed handlers
    for (size_t i = 0; i < arr_handler_len(&logger->handlers); i++) {
        const HandlerInfo* hinfo = &logger->handlers.data[i];
        va_copy(ev.ap, ap);
        ev.hdata = hinfo->data;
        CXCHKZ(pthread_mutex_lock(&logger->lock));
        hinfo->handler(logger, &ev);
        CXCHKZ(pthread_mutex_unlock(&logger->lock));
        va_end(ev.ap);
    }

    if (level == CxLoggerFatal) {
        abort();
    }
}

void cx_logger_console_handler(const CxLogger* logger, CxLoggerEvent *ev) {

    FILE* out = stderr;
    if (logger->flags & CxLoggerFlagColor) {
        fprintf(out, TERM_RESET TERM_FG_GRAY "%s%s%-5s ", ev->time_prefix, level_colors[ev->level], level_strings[ev->level]);
        fprintf(out, TERM_FG_BWHITE);
        vfprintf(out, ev->fmt, ev->ap);
        fprintf(out, TERM_RESET);
    } else {
        fprintf(out, "%s%-5s: ", ev->time_prefix, level_strings[ev->level]);
        vfprintf(out, ev->fmt, ev->ap);
    }
    fprintf(out, "\n");
    fflush(out);
}

void cx_logger_file_handler(const CxLogger* logger, CxLoggerEvent* ev) {

    FILE* out = ev->hdata;
    fprintf(out, "%s%-5s: ", ev->time_prefix, level_strings[ev->level]);
    vfprintf(out, ev->fmt, ev->ap);
    fprintf(out, "\n");
    fflush(out);
}


