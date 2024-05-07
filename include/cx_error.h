#ifndef CX_ERROR_H
#define CX_ERROR_H

typedef struct CxError {
    int code;           // Error code (0 == no error)
    const char* emsg;   // Static error message 
    const char* func;   // Function which generated the error
} CxError;

// Generates empty CxError (no error)
#define CXERROR_OK()\
    (CxError){}

// Generates CxError with code and error message
#define CXERROR(CODE,EMSG)\
    (CxError){.code=CODE, .emsg=EMSG, .func=__func__}

// if ERR is not OK, prints error and aborts
#define CXERROR_CHK(ERR)\
    {const CxError err = (ERR); if (err.code) {printf("CXERROR file:%s line:%d func:%s\n", __FILE__, __LINE__, err.func); abort();}}

// If ERR is not OK, returns the CxErr
#define CXERROR_RET(ERR)\
    {const CxError err = (ERR); if (err.code) {return err;}}

// If INTERR (int error number) is not zero returns CxError
#define CXERROR_RETNZ(INTERR,EMSG)\
    {if ((INTERR)) {return (CxError){.code=INTERR, .emsg=EMSG, .func=__func__};}}

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


