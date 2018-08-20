#ifndef __SMARTCARD_USER_H
#define __SMARTCARD_USER_H

#include "stm32f10x.h"


typedef enum
{
    AES = 0xC0,//对称
    DES = 0xC1,//对称
    SM4 = 0xC3,//对称
    RSA = 0xC4,//非对称,传输秘钥
    GetResult = 0xCA,
} INS_Typedef;

typedef enum
{
    INS_Data = 0xa1,
    INS_Key = 0xa2,
    INS_SecretKey = 0xa3,
} INS_Data_Typedef;


void SmartcardTest(void);

#endif

