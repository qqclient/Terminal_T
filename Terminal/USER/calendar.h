#ifndef __CALENDAR_H
#define __CALENDAR_H

#include "stm32f10x.h"

//ʱ��ṹ��
typedef struct
{
    volatile unsigned char hour;
    volatile unsigned char min;
    volatile unsigned char sec;
    //������������
    volatile unsigned short w_year;
    volatile unsigned char  w_month;
    volatile unsigned char  w_day;
    volatile unsigned char  week;
} _calendar_typedef;

extern _calendar_typedef Sys_CalendarTime;//�����ṹ��
extern unsigned int Sys_TimeCount;//���ʱ

void TimeSync(_calendar_typedef * Sys_CalendarTime,u8 *TimeStr);
void TimeToCharArray(_calendar_typedef Sys_CalendarTime,u8 TimeStr[]);//������ʱ��ת����14byte������,�Է���洢
u8 RTC_GetTime(void);//��ȡ��ʱ��
u8 RTC_SetTime(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec);//����ʱ��

#endif


