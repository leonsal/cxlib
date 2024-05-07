#ifndef CX_ERROR_H
#define CX_ERROR_H

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct CxError {
    int         code;       // Error code (0 == no error)
    const char* func;       // static string with name of function where the CXERROR macro was called
    char*       msg;        // Static or dynamically allocated error message
    int         alloc_;     // Used internally to indicate if error message was allocated
} CxError;

// Generates empty CxError (no error)
#define CXERROR_OK()\
    (CxError){}

// Generates CxError with code and static error message
#define CXERROR(CODE,MSG)\
    (CxError){.code=CODE, .msg=MSG, .func=__func__}

// Generates CxError with code and dynamically allocated error message (printf syntax)
#define CXERROR_MAX_DMSG    (256)
static inline CxError cx_error_printf(int code, const char* func, const char* fmt, ...) {
    CxError error = {.code=code, .func = func, .alloc_=1};
    error.msg = malloc(CXERROR_MAX_DMSG);
    if (error.msg == NULL) {
        abort();
    }
    va_list args;
    va_start(args, fmt);
    vsnprintf((char*)error.msg, CXERROR_MAX_DMSG, fmt, args);
    va_end(args);
    return error;
}
#define CXERRORF(CODE, ...)\
    cx_error_printf(CODE, __func__,  __VA_ARGS__)

// Free CxError with dynamically allocated error message
#define CXERROR_FREE(ERR)\
    {if (ERR.alloc_) {free(ERR.msg); ERR.msg=NULL;}}

// if ERR is not OK, prints error and aborts
#define CXERROR_CHK(ERR)\
    {const CxError err = (ERR); if (err.code) {printf("CXERROR file:%s line:%d func:%s\n", __FILE__, __LINE__, err.func); abort();}}

// If ERR is not OK, returns the CxErr
#define CXERROR_RET(ERR)\
    {const CxError err = (ERR); if (err.code) {return err;}}

// If INTERR (int error number) is not zero returns CxError
#define CXERROR_RETNZ(INTERR,MSG)\
    {if ((INTERR)) {return (CxError){.code=INTERR, .msg=MSG, .func=__func__};}}

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


