#ifndef __GPRS_GSM_DRIVER_H
#define __GPRS_GSM_DRIVER_H

#include "stm32f10x.h"

typedef enum
{
    M35_IDLE = 0,

    M35_PwrOn_Start    = 1,
    M35_PwrOn_Step1    = 2,
    M35_PwrOn_Step2    = 3,
    M35_PwrOn_Finished = 4,

    M35_PwrOff_Start    = 7,
    M35_PwrOff_End      = 8,
    M35_PwrOff_Finished = 9,



    Call_Start = 10,
    Call_Step1 = 11,
    Call_Step2 = 12,
    Call_Step3 = 13,
    Call_Step4 = 14,
    Call_Step5 = 15,
    Call_Finished  = 16,

    SendTcp_Start = 20,
    SendTcp_Step1 = 21,
    SendTcp_Step2 = 22,
    SendTcp_Step3 = 23,
    SendTcp_Step4 = 24,
    SendTcp_Step5 = 25,
    SendTcp_Step6 = 26,
    SendTcp_Step7 = 27,
    SendTcp_Step8 = 28,
    SendTcp_Step9 = 29,
    SendTcp_Finished = 30,

    CheckSignal1 = 40,
    CheckSignal2   = 41,
    CheckSignal_Finished = 42,

    M35_Init_Start = 49,
    M35_Init_Step1 = 50,
    M35_Init_Step2 = 51,
    M35_Init_Step3 = 52,
    M35_Init_Step4 = 53,
    M35_Init_Step5 = 54,
    M35_Init_Step6 = 55,
    M35_Init_Step7 = 56,
    M35_Init_Step8 = 57,
    M35_Init_Step9 = 58,
    M35_Init_Step10 = 59,
    M35_Init_Step11 = 60,
    M35_Init_Finished= 61,


} GPRS_CmdNow_Typedef;

extern volatile unsigned char GPRS_CmdNow;
extern unsigned char m_Phone[];

/* Private functions ---------------------------------------------------------*/
extern void M35_SendCmd(char *cmd);
extern unsigned char *M35_CheckCmd(char *str);
extern void M35_PowerOff(void);
extern unsigned char M35_CheckSignal(char *src);
extern FlagStatus M35_GetStatus(void);
extern volatile unsigned short int M35_TimeCount;
extern void M35_Initialize(void);

#endif


