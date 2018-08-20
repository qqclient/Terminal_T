/******************** (C) COPYRIGHT soarsky ************************************
* File Name          : gps_user.c
* Author             : 
* Version            : 
* Date               : 
* Description        : 待优化
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"

/***************全局变量定义*************/

unsigned char GetLocation( unsigned char* dsr )
{
    char buf[32] = { 0 };

    if(dsr == NULL)
        return 0;

    sprintf( buf, "%3.7f,%2.7f        ",
             (double)gpsx.longitude / 100000,
             (double)gpsx.latitude  / 100000 );
    strcpy( (char*)dsr, buf );

    return( strlen(buf) );
}

/*******************************************************************************
* Function Name  : GetSpeed
* Description    : 从GPS获得车辆当前的速度
*******************************************************************************/
int GetSpeed(void)
{
    if( ( m_carinfo.Flag_Simulation & 0x01 ) == SET ) {
        if( Car_InputCheck.Value_DeputyBelt == SET )
            return 30;
        else
            return 0 ;
    }
    else {
        if(( gpsx.gpssta == 1 || gpsx.gpssta == 2 )) {
            return gpsx.speed / 1000;
        }
        else {
            return 0;
        }
    }
}

/*******************************************************************************
* Function Name  : isSubtend
* Description    : 用于判断两车是否为对向来车
*******************************************************************************/
u8 isSubtend(u8 dir1,u8 dir2)
{
    if((dir1 == East && dir2 == West)  ||
            (dir1 == South && dir2 == North)  ||
            (dir1 == West && dir2 == East)    ||
            (dir1 == North && dir2 == South)  ||
            (dir1 == Southeast && dir2 == Northwest)  ||
            (dir1 == Southwest && dir2 == Northeast)  ||
            (dir1 == Northeast && dir2 == Southwest)  ||
            (dir1 == Northwest && dir2 == Southeast)  ) {

        return 1;
    }
    else {
        return 0;
    }
}



/*******************************************************************************
* Function Name  : GetDistance
* Description    : 从GPS获得两点之间的距离
*******************************************************************************/
unsigned int GetDistance(u32 lat1,u32 lng1,unsigned int lat2,unsigned int lng2)
{
    return 0;
}

/*******************************************************************************
* Function Name  : RecordAbnormitySpeed
* Description    : 从GPS判断出车辆急加速or急减速后，记录十组车速、不足补空格。
*******************************************************************************/
void RecordAbnormitySpeed(int TmpSpeed,unsigned char *RecoSpeed)
{

}

/****************************************END OF FILE**********************************/
