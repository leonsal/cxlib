#include "cx_logger.h"

// Global logger defined in 'main'
extern CxLogger* g_logger;

// Utility macros
#define GLOGD(...) cx_logger_log(g_logger, CxLoggerDebug, __VA_ARGS__)
#define GLOGI(...) cx_logger_log(g_logger, CxLoggerInfo, __VA_ARGS__)
#define GLOGW(...) cx_logger_log(g_logger, CxLoggerWarn, __VA_ARGS__)
#define GLOGE(...) cx_logger_log(g_logger, CxLoggerError, __VA_ARGS__)
#define GLOGF(...) cx_logger_log(g_logger, CxLoggerFatal, __VA_ARGS__)
