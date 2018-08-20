#ifndef __GPRS_GSM_USER_H
#define __GPRS_GSM_USER_H

#include "stm32f10x.h"


extern unsigned short int GPRS_LocationTime;
extern void M35_SendCarLocation(unsigned char Type);
extern void M35_SendCarStatus(unsigned char CarStatus,unsigned char Additon);
extern void M35_Process(void);
extern unsigned char Flag_CallPhoneOK;
extern unsigned char Flag_SendMsgOK;
extern char IllegalParkStr1[];
extern char IllegalParkStr2[];


#endif


