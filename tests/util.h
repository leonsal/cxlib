#ifndef UTIL_H
#define UTIL_H

#define CHK(cond)\
    {if (!(cond)) {printf("CHK ERROR file:%s line:%d func:%s\n", __FILE__, __LINE__, __func__);abort();}}

#define CHKZ(CALL)\
    {if (CALL) {printf("CHK ERROR file:%s line:%d func:%s\n", __FILE__, __LINE__, __func__);abort();}}
#endif

