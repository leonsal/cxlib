#ifndef LOGGER_H
#define LOGGER_H

#include "cx_logger.h"

// Global logger defined in 'main'
extern CxLogger* g_logger;

// Utility macros
#define LOGD(...) cx_logger_log(g_logger, CxLoggerDebug, __VA_ARGS__)
#define LOGI(...) cx_logger_log(g_logger, CxLoggerInfo, __VA_ARGS__)
#define LOGW(...) cx_logger_log(g_logger, CxLoggerWarn, __VA_ARGS__)
#define LOGE(...) cx_logger_log(g_logger, CxLoggerError, __VA_ARGS__)
#define LOGF(...) cx_logger_log(g_logger, CxLoggerFatal, __VA_ARGS__)

#endif





