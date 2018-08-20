#ifndef __CALENDAR_H
#define __CALENDAR_H

#include "stm32f10x.h"

//时间结构体
typedef struct
{
    volatile unsigned char hour;
    volatile unsigned char min;
    volatile unsigned char sec;
    //公历日月年周
    volatile unsigned short w_year;
    volatile unsigned char  w_month;
    volatile unsigned char  w_day;
    volatile unsigned char  week;
} _calendar_typedef;

extern _calendar_typedef Sys_CalendarTime;//日历结构体
extern unsigned int Sys_TimeCount;//秒计时

void TimeSync(_calendar_typedef * Sys_CalendarTime,u8 *TimeStr);
void TimeToCharArray(_calendar_typedef Sys_CalendarTime,u8 TimeStr[]);//将日历时间转换成14byte的数组,以方便存储
u8 RTC_GetTime(void);//获取新时间
u8 RTC_SetTime(u16 syear,u8 smon,u8 sday,u8 hour,u8 min,u8 sec);//设置时间

#endif


