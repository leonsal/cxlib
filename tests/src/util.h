#ifndef UTIL_H
#define UTIL_H

#define CHK(cond)\
    {if (!(cond)) {printf("CHK ERROR file:%s line:%d func:%s\n", __FILE__, __LINE__, __func__);abort();}}

// Checks if call returns ZERO value
#define CHKZ(CALL)\
    {if (CALL) {printf("CHK ERROR file:%s line:%d func:%s\n", __FILE__, __LINE__, __func__);abort();}}

// Checks if call returns NON-NULL value
#define CHKN(CALL)\
    {if (CALL==NULL) {printf("CHK ERROR file:%s line:%d func:%s\n", __FILE__, __LINE__, __func__);abort();}}
#endif

