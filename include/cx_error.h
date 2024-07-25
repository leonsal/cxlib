#ifndef CX_ERROR_H
#define CX_ERROR_H

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct CxError {
    char*       msg;        // Static or dynamically allocated error message
    const char* func;       // static string with name of function where the CXERROR macro was called
    int         code;       // Optional error code
    int         alloc_;     // Used internally to indicate if error message was allocated
} CxError;

// Generates empty CxError (no error)
#define CXOK()\
    (CxError){0}

// Generates CxError from specified static message
#define CXERR(MSG)\
    (CxError){.msg=(char*)MSG, .func=__func__, .code=1}

// Generates CxError from specified error code and static message
#define CXERR2(CODE,MSG)\
    (CxError){.msg=(char*)MSG, .func=__func__, .code=CODE}

// Internal function to generates CxError with specified error code and dynamically allocated error message (printf syntax)
#define CXERROR_MAX_DMSG    (256)
static inline CxError cx_error_printf(int code, const char* func, const char* fmt, ...) {
    CxError error = {.code=code, .func = func, .alloc_=1};
    error.msg = (char*)malloc(CXERROR_MAX_DMSG);
    if (error.msg == NULL) {
        abort();
    }
    va_list args;
    va_start(args, fmt);
    vsnprintf((char*)error.msg, CXERROR_MAX_DMSG, fmt, args);
    va_end(args);
    return error;
}

// Generates CxError with dynamically allocated and formatted error messsage
#define CXERRF(FMT, ...)\
    cx_error_printf(1,__func__,FMT,__VA_ARGS__)

// Generates CxError with specified error code and dynamically allocated and formatted error messsage
#define CXERRF2(CODE,FMT, ...)\
    cx_error_printf(CODE,__func__,FMT,__VA_ARGS__)

// Free CxError with dynamically allocated error message
#define CXERR_FREE(ERR)\
    {if (ERR.alloc_ && ERR.msg) {free(ERR.msg); ERR.msg=NULL;}}

// if ERR is not OK, prints error and aborts
#define CXERR_CHK(ERR)\
    {if (ERR.msg) {printf("ERROR file:%s line:%d code:%d func:%s msg:%s\n", __FILE__, __LINE__, ERR.code, ERR.func, ERR.msg); abort();}}

// If ERR is not OK, returns the CxError struct
#define CXERR_RET(ERR)\
    {if (ERR.msg) {return ERR;}}

// If COND is not true, prints error and aborts
#define CXCHK(COND)\
    {if (!(COND)) {printf("CXCHK ERROR file:%s line:%d func:%s\n", __FILE__, __LINE__, __func__);abort();}}

// If COND is not zero, prints error and aborts
#define CXCHKZ(COND)\
    {if (COND) {printf("CXCHKZ ERROR file:%s line:%d func:%s\n", __FILE__, __LINE__, __func__);abort();}}

// If COND is NULL, prints error and aborts
#define CXCHKN(CALL)\
    {if (COND==NULL) {printf("CXCHKN ERROR file:%s line:%d func:%s\n", __FILE__, __LINE__, __func__);abort();}}

#endif


