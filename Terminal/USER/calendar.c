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
_calendar_typedef Sys_CalendarTime;//ʱ�ӽṹ��
u32 Sys_TimeCount = 0;

/* Private variables ---------------------------------------------------------*/
//�·����ݱ�
uc8 table_week[12] = {0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5}; //���������ݱ�
//ƽ����·����ڱ�
uc8 mon_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/* Private functions ---------------------------------------------------------*/
static u8 Is_Leap_Year(u16 year);//ƽ��,�����ж�
static u8 RTC_Get_Week(u16 year,u8 month,u8 day);

/*******************************************************************************
* ������  : TimeToCharArray
* ����    : ������ת���ɳ���Ϊ14���ַ�������
*******************************************************************************/
void TimeToCharArray(_calendar_typedef Sys_CalendarTime, u8 *TimeStr)
{
    sprintf((char*)TimeStr, "%04d%02d%02d%02d%02d%02d", Sys_CalendarTime.w_year, Sys_CalendarTime.w_month, Sys_CalendarTime.w_day,
            Sys_CalendarTime.hour, Sys_CalendarTime.min, Sys_CalendarTime.sec);
}


/*******************************************************************************
* ������  : TimeSync
* ����    : ������Ϊ14���ַ�������ת��������,�ڽ���ʱ��ͬ��ʱ���øú���
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
                Sys_CalendarTime->sec); //����ʱ��

#ifdef __DEBUG
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "��ǰʱ��:%04d-%02d-%02d %02d:%02d:%02d",
                     Sys_CalendarTime->w_year,
                     Sys_CalendarTime->w_month,
                     Sys_CalendarTime->w_day,
                     Sys_CalendarTime->hour,
                     Sys_CalendarTime->min,
                     Sys_CalendarTime->sec);
#endif
}

/*******************************************************************************
* ������  : RTC_SetTime
* ����    : ����ʱ��,�������ʱ��ת��Ϊ����,��1970��1��1��Ϊ��׼,1970~2099��Ϊ�Ϸ����
*******************************************************************************/
u8 RTC_SetTime(u16 syear, u8 smon, u8 sday, u8 hour, u8 min, u8 sec)
{
    u16 t;
    u32 SecCount = 0;

    if(syear < 1970 || syear > 2099) {//��ݷǷ�,��������
        return 1;
    }
    for(t = 1970; t < syear; t++) { //��������ݵ��������
        if(Is_Leap_Year(t)) {
            SecCount += 31622400;   //�����������
        }
        else {
            SecCount += 31536000;   //ƽ���������
        }
    }
    smon -= 1;
    for(t = 0; t < smon; t++) { //��ǰ���·ݵ����������
        SecCount += (u32)mon_table[t] * 86400; //�·����������
        if(Is_Leap_Year(syear) && t == 1) {
            SecCount += 86400; //����2�·�����һ���������
        }
    }
    SecCount += (u32)(sday - 1) * 86400; //��ǰ�����ڵ����������
    SecCount += (u32)hour * 3600; //Сʱ������
    SecCount += (u32)min * 60;	 //����������
    SecCount += sec; //�������Ӽ���ȥ

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��
    PWR_BackupAccessCmd(ENABLE);	//ʹ��RTC�ͺ󱸼Ĵ�������

    RTC_SetCounter(SecCount);	//����RTC��������ֵ

    RTC_WaitForLastTask();	    //�ȴ����һ�ζ�RTC�Ĵ�����д�������
    return 0;
}


/*******************************************************************************
* ������  : RTC_GetTime
* ����    : ����ʱ������ת����������
*******************************************************************************/
u8 RTC_GetTime(void)
{
    static u16 DayCount = 0;
    u32 DayNum = 0;
    u16 TempYear = 0;

    Sys_TimeCount = RTC_GetCounter();

    DayNum = Sys_TimeCount / 86400; //�õ�����,һ��86400��

    if(DayCount != DayNum) //����һ����
    {
        DayCount = DayNum;
        TempYear = 1970;	//��1970�꿪ʼ
        while(DayNum >= 365)
        {
            if(Is_Leap_Year(TempYear))//������
            {
                if(DayNum >= 366)
                {
                    DayNum -= 366; //�����������
                } else
                {
                    TempYear++;
                    break;
                }
            } else
            {
                DayNum -= 365;	 //ƽ��
            }
            TempYear++;
        }
        Sys_CalendarTime.w_year = TempYear; //�õ����
        TempYear = 0;
        while(DayNum >= 28) //������һ����
        {
            if(Is_Leap_Year(Sys_CalendarTime.w_year) && TempYear == 1) //�����ǲ�������/2�·�
            {
                if(DayNum >= 29)
                {
                    DayNum -= 29; //�����������
                } else
                {
                    break;
                }
            } else
            {
                if(DayNum >= mon_table[TempYear])
                {
                    DayNum -= mon_table[TempYear]; //ƽ��
                } else
                {
                    break;
                }
            }
            TempYear++;
        }
        Sys_CalendarTime.w_month = TempYear + 1;	//�õ��·�
        Sys_CalendarTime.w_day = DayNum + 1;  	//�õ�����
    }
    DayNum = Sys_TimeCount % 86400;     		  //�õ�������
    Sys_CalendarTime.hour = DayNum / 3600;     	  //Сʱ
    Sys_CalendarTime.min = (DayNum % 3600) / 60; 	//����
    Sys_CalendarTime.sec = (DayNum % 3600) % 60; 	//����
    Sys_CalendarTime.week = RTC_Get_Week(Sys_CalendarTime.w_year, Sys_CalendarTime.w_month, Sys_CalendarTime.w_day); //��ȡ����
    return 0;
}

/*******************************************************************************
* ������  : RTC_Get_Week
* ˵��    :���빫�����ڵõ�����(ֻ����1901-2099��)
*******************************************************************************/
static u8 RTC_Get_Week(u16 year, u8 month, u8 day)
{
    u16 temp2;
    u8 yearH, yearL;

    yearH = year / 100;
    yearL = year % 100;
    // ���Ϊ21����,�������100
    if (yearH > 19)yearL += 100;
    // ����������ֻ��1900��֮���
    temp2 = yearL + yearL / 4;
    temp2 = temp2 % 7;
    temp2 = temp2 + day + table_week[month - 1];
    if (yearL % 4 == 0 && month < 3)temp2--;
    return(temp2 % 7);
}



/*******************************************************************************
* ������  : Is_Leap_Year
* ����    : �ж��Ƿ������꺯��
*******************************************************************************/
static u8 Is_Leap_Year(u16 year)
{
    if(year % 4 == 0) //�����ܱ�4����
    {
        if(year % 100 == 0) {
            if(year % 400 == 0) {
                return 1; //�����00��β,��Ҫ�ܱ�400����
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
* ������  : RTC_IRQHandler
* ����    : ÿ�봥��һ��
*******************************************************************************/
void RTC_IRQHandler(void)
{
    if (RTC_GetITStatus(RTC_IT_SEC) != RESET)//�����ж�
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
    if(RTC_GetITStatus(RTC_IT_ALR) != RESET) //�����ж�
    {
        RTC_ClearITPendingBit(RTC_IT_ALR);		//�������ж�
        //RTC_GetTime();				//����ʱ��
    }
    RTC_ClearITPendingBit(RTC_IT_SEC | RTC_IT_OW);		//�������ж�
    RTC_WaitForLastTask();
}

