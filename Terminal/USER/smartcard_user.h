#ifndef __SMARTCARD_USER_H
#define __SMARTCARD_USER_H

#include "stm32f10x.h"


typedef enum
{
    AES = 0xC0,//�Գ�
    DES = 0xC1,//�Գ�
    SM4 = 0xC3,//�Գ�
    RSA = 0xC4,//�ǶԳ�,������Կ
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

