#ifndef LOGGER_H
#define LOGGER_H

#define cx_log_name logger
#define cx_log_tsafe
#ifdef CX_LOG_IMPLEMENT
#define cx_log_implement
#endif
#include "cx_log.h"

extern logger default_logger;
#define LOG_INIT()\
    default_logger = logger_init();\
    logger_set_flags(&default_logger, CX_LOG_FLAG_TIME|CX_LOG_FLAG_US|CX_LOG_FLAG_COLOR);\
    logger_add_handler(&default_logger, logger_console_handler, NULL, CX_LOG_DEBUG)

#define LOGD(...) logger_deb(&default_logger, __VA_ARGS__)
#define LOGI(...) logger_info(&default_logger, __VA_ARGS__)
#define LOGW(...) logger_warn(&default_logger, __VA_ARGS__)
#define LOGE(...) logger_error(&default_logger, __VA_ARGS__)
#define LOGF(...) logger_error(&default_logger, __VA_ARGS__)

#endif


