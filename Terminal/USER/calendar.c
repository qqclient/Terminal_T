/******************** (C) COPYRIGHT soarsky ********************
* File Name          : rtc.c
* Author             :
* Version            :
* Date               :
* Description        :
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"

/* Global variables ---------------------------------------------------------*/
_calendar_typedef Sys_CalendarTime;//时钟结构体
u32 Sys_TimeCount = 0;

/* Private variables ---------------------------------------------------------*/
//月份数据表
uc8 table_week[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5}; //月修正数据表
//平年的月份日期表
uc8 mon_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/* Private functions ---------------------------------------------------------*/
static u8 Is_Leap_Year(u16 year);//平年,闰年判断
static u8 RTC_Get_Week(u16 year,u8 month,u8 day);

/*******************************************************************************
* 函数名  : TimeToCharArray
* 描述    : 将日历转换成长度为14的字符串数组
*******************************************************************************/
void TimeToCharArray(_calendar_typedef Sys_CalendarTime, u8 *TimeStr)
{
    sprintf((char*)TimeStr, "%04d%02d%02d%02d%02d%02d", Sys_CalendarTime.w_year, Sys_CalendarTime.w_month, Sys_CalendarTime.w_day,
            Sys_CalendarTime.hour, Sys_CalendarTime.min, Sys_CalendarTime.sec);
}


/*******************************************************************************
* 函数名  : TimeSync
* 描述    : 将长度为14的字符串数组转换成日历,在进行时间同步时调用该函数
*******************************************************************************/
void TimeSync(_calendar_typedef *Sys_CalendarTime, u8 *TimeStr)
{
    Sys_CalendarTime->w_year = Asc2Word((char*)TimeStr+0, 4);//(TimeStr[0]-0x30)*1000+(TimeStr[1]-0x30)*100+(TimeStr[2]-0x30)*10+TimeStr[3]-0x30;
    Sys_CalendarTime->w_month= Asc2Byte((char*)TimeStr+4, 2);//(TimeStr[4]-0x30)*10 + TimeStr[5]-0x30;
    Sys_CalendarTime->w_day  = Asc2Byte((char*)TimeStr+6, 2);//(TimeStr[6]-0x30)*10 + TimeStr[7]-0x30;
    Sys_CalendarTime->hour   = Asc2Byte((char*)TimeStr+8, 2);//(TimeStr[8]-0x30)*10 + TimeStr[9]-0x30;
    Sys_CalendarTime->min    = Asc2Byte((char*)TimeStr+10, 2);//(TimeStr[10]-0x30)*10+ TimeStr[11]-0x30;
    Sys_CalendarTime->sec    = Asc2Byte((char*)TimeStr+12, 2);//(TimeStr[12]-0x30)*10+ TimeStr[13]-0x30;

    RTC_SetTime(Sys_CalendarTime->w_year,
                Sys_CalendarTime->w_month,
                Sys_CalendarTime->w_day,
                Sys_CalendarTime->hour,
                Sys_CalendarTime->min,
                Sys_CalendarTime->sec); //设置时间

#ifdef __DEBUG
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "当前时间:%04d-%02d-%02d %02d:%02d:%02d",
                     Sys_CalendarTime->w_year,
                     Sys_CalendarTime->w_month,
                     Sys_CalendarTime->w_day,
                     Sys_CalendarTime->hour,
                     Sys_CalendarTime->min,
                     Sys_CalendarTime->sec);
#endif
}

/*******************************************************************************
* 函数名  : RTC_SetTime
* 描述    : 设置时钟,把输入的时钟转换为秒钟,以1970年1月1日为基准,1970~2099年为合法年份
*******************************************************************************/
u8 RTC_SetTime(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
    u16 t;
    u32 SecCount = 0;

    if(syear < 1970 || syear > 2099) {//年份非法,跳出程序
        return 1;
    }
    for(t = 1970; t < syear; t++) { //把所有年份的秒钟相加
        if(Is_Leap_Year(t)) {
            SecCount += 31622400;   //闰年的秒钟数
        }
        else {
            SecCount += 31536000;   //平年的秒钟数
        }
    }
    smon -= 1;
    for(t = 0; t < smon; t++) { //把前面月份的秒钟数相加
        SecCount += (u32)mon_table[t] * 86400; //月份秒钟数相加
        if(Is_Leap_Year(syear) && t == 1) {
            SecCount += 86400; //闰年2月份增加一天的秒钟数
        }
    }
    SecCount += (u32)(sday - 1) * 86400; //把前面日期的秒钟数相加
    SecCount += (u32)hour * 3600; //小时秒钟数
    SecCount += (u32)min * 60;	 //分钟秒钟数
    SecCount += sec; //最后的秒钟加上去

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟
    PWR_BackupAccessCmd(ENABLE);	//使能RTC和后备寄存器访问

    RTC_SetCounter(SecCount);	//设置RTC计数器的值

    RTC_WaitForLastTask();	    //等待最近一次对RTC寄存器的写操作完成
    return 0;
}


/*******************************************************************************
* 函数名  : RTC_GetTime
* 描述    : 将定时器的秒转换成年月日
*******************************************************************************/
u8 RTC_GetTime(void)
{
    static u16 DayCount = 0;
    u32 DayNum = 0;
    u16 TempYear = 0;

    Sys_TimeCount = RTC_GetCounter();

    DayNum = Sys_TimeCount / 86400; //得到天数,一天86400秒

    if(DayCount != DayNum) //超过一天了
    {
        DayCount = DayNum;
        TempYear = 1970;	//从1970年开始
        while(DayNum >= 365)
        {
            if(Is_Leap_Year(TempYear))//是闰年
            {
                if(DayNum >= 366)
                {
                    DayNum -= 366; //闰年的秒钟数
                } else
                {
                    TempYear++;
                    break;
                }
            } else
            {
                DayNum -= 365;	 //平年
            }
            TempYear++;
        }
        Sys_CalendarTime.w_year = TempYear; //得到年份
        TempYear = 0;
        while(DayNum >= 28) //超过了一个月
        {
            if(Is_Leap_Year(Sys_CalendarTime.w_year) && TempYear == 1) //当年是不是闰年/2月份
            {
                if(DayNum >= 29)
                {
                    DayNum -= 29; //闰年的秒钟数
                } else
                {
                    break;
                }
            } else
            {
                if(DayNum >= mon_table[TempYear])
                {
                    DayNum -= mon_table[TempYear]; //平年
                } else
                {
                    break;
                }
            }
            TempYear++;
        }
        Sys_CalendarTime.w_month = TempYear + 1;	//得到月份
        Sys_CalendarTime.w_day = DayNum + 1;  	//得到日期
    }
    DayNum = Sys_TimeCount % 86400;     		  //得到秒钟数
    Sys_CalendarTime.hour = DayNum / 3600;     	  //小时
    Sys_CalendarTime.min = (DayNum % 3600) / 60; 	//分钟
    Sys_CalendarTime.sec = (DayNum % 3600) % 60; 	//秒钟
    Sys_CalendarTime.week = RTC_Get_Week(Sys_CalendarTime.w_year, Sys_CalendarTime.w_month, Sys_CalendarTime.w_day); //获取星期
    return 0;
}

/*******************************************************************************
* 函数名  : RTC_Get_Week
* 说明    :输入公历日期得到星期(只允许1901-2099年)
*******************************************************************************/
static u8 RTC_Get_Week(u16 year, u8 month, u8 day)
{
    u16 temp2;
    u8 yearH, yearL;

    yearH = year / 100;
    yearL = year % 100;
    // 如果为21世纪,年份数加100
    if (yearH > 19)yearL += 100;
    // 所过闰年数只算1900年之后的
    temp2 = yearL + yearL / 4;
    temp2 = temp2 % 7;
    temp2 = temp2 + day + table_week[month - 1];
    if (yearL % 4 == 0 && month < 3)temp2--;
    return(temp2 % 7);
}



/*******************************************************************************
* 函数名  : Is_Leap_Year
* 描述    : 判断是否是闰年函数
*******************************************************************************/
static u8 Is_Leap_Year(u16 year)
{
    if(year % 4 == 0) //必须能被4整除
    {
        if(year % 100 == 0) {
            if(year % 400 == 0) {
                return 1; //如果以00结尾,还要能被400整除
            }
            else {
                return 0;
            }
        }
        else {
            return 1;
        }
    }
    else {
        return 0;
    }
}

/*******************************************************************************
* 函数名  : RTC_IRQHandler
* 描述    : 每秒触发一次
*******************************************************************************/
void RTC_IRQHandler(void)
{
    if (RTC_GetITStatus(RTC_IT_SEC) != RESET)//秒钟中断
    {
        Sys_TimeCount = RTC_GetCounter();

        Sys_CalendarTime.sec++;
        if( Sys_CalendarTime.sec > 59 ) {
            Sys_CalendarTime.sec = 0;

            Sys_CalendarTime.min++;
            if( Sys_CalendarTime.min > 59 ) {
                Sys_CalendarTime.min = 0;

                Sys_CalendarTime.hour++;
                if( Sys_CalendarTime.hour > 23 ) {
                    Sys_CalendarTime.hour = 0;

                    Sys_CalendarTime.w_day++;
                    if( Sys_CalendarTime.w_day > (mon_table[Sys_CalendarTime.w_month] + ( Sys_CalendarTime.w_month == 2 ? Is_Leap_Year(Sys_CalendarTime.w_year) : 0 ))) {
                        Sys_CalendarTime.w_day = 1;

                        Sys_CalendarTime.w_month++;
                        if( Sys_CalendarTime.w_month > 12 ) {
                            Sys_CalendarTime.w_month = 1;
                            Sys_CalendarTime.w_year++;
                        }

                        m_carinfo.Count_IllegalPark = 0;
                        MX25L128_CreatSaveTask();
                    }
                }
            }
        }
    }
    if(RTC_GetITStatus(RTC_IT_ALR) != RESET) //闹钟中断
    {
        RTC_ClearITPendingBit(RTC_IT_ALR);		//清闹钟中断
        //RTC_GetTime();				//更新时间
    }
    RTC_ClearITPendingBit(RTC_IT_SEC | RTC_IT_OW);		//清闹钟中断
    RTC_WaitForLastTask();
}

