#ifndef CX_LOGGER_H
#define CX_LOGGER_H

#include <stdarg.h>
#include <stdbool.h>

#include "cx_alloc.h"
#include "cx_error.h"

typedef enum {
    CxLoggerDebug,
    CxLoggerInfo,
    CxLoggerWarn,
    CxLoggerError,
    CxLoggerFatal,
} CxLoggerLevel;

typedef enum {
    CxLoggerFlagDate    = 1 << 0,   // Show date
    CxLoggerFlagTime    = 1 << 1,   // Show time
    CxLoggerFlagMs      = 1 << 2,   // Show milliseconds
    CxLoggerFlagUs      = 1 << 3,   // Show microseconds
    CxLoggerFlagColor   = 1 << 4,   // Use color for console handler
} CxLoggerFlags;

typedef struct CxLoggerEvent {
    CxLoggerLevel   level;          // Current log level
    const char*     fmt;            // Format string passed by user
    va_list         ap;             // For formatting user message
    struct timespec time;           // Event time
    void            *hdata;         // Optional handler data
    char time_prefix[128];          // formatted time plus optional prefix
} CxLoggerEvent;

// Type for logger handlers
typedef struct CxLogger CxLogger;
typedef void (*CxLoggerHandler)(const CxLogger* logger, CxLoggerEvent* event);

// Creates and returns a new logger using the specified optional allocator and prefix.
CxLogger* cx_logger_new(const CxAllocator* alloc, const char* prefix);

// Deletes previously created logger
void cx_logger_del(CxLogger* logger);

// Get the current log level
CxLoggerLevel cx_logger_get_level(CxLogger* logger);

// Sets the log level
CxError cx_logger_set_level(CxLogger* logger, CxLoggerLevel level);

// Get the current log level string
const char* cx_logger_get_level_str(CxLogger* logger);

// Sets the log level by the level name string
CxError cx_logger_set_level_str(CxLogger* logger, const char* level_str);

// Get current logger flags
CxLoggerFlags cx_logger_get_flags(CxLogger* logger);

// Set the logger flags
CxError cx_logger_set_flags(CxLogger* logger, CxLoggerFlags flags);

// Sets logger enabled state
CxError cx_logger_set_enabled(CxLogger* logger, bool enabled);

// Add a new handler to the logger
CxError cx_logger_add_handler(CxLogger* logger, CxLoggerHandler handler, void* handler_data);

// Removes a previously added handler to the logger
void cx_logger_del_handler(CxLogger* logger, CxLoggerHandler handler, void* handler_data);

// Logs event
void cx_logger_log(CxLogger* logger, CxLoggerLevel level, const char* fmt, ...) __attribute__((format(printf,3,4)));

// Default console handler
void cx_logger_console_handler(const CxLogger* logger, CxLoggerEvent *ev);

// Default file handler
void cx_logger_file_handler(const CxLogger* logger, CxLoggerEvent* ev);

#endif

