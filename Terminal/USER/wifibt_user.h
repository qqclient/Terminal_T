#ifndef __WIFIBT_USER_H
#define __WIFIBT_USER_H

#include "stm32f10x.h"

/***************本地变量声明*************/
extern unsigned char FlagTerminalStat;
extern volatile unsigned short int I2411E_TimeCount;
extern unsigned short int Count_I2411E_SCAN;
extern unsigned short int  Count_FindCop;
extern unsigned char Flag_RevCopData;

extern volatile unsigned int Time_OperDoor;        // 切换到状态二的系统时间

extern void I2411E_Inspection(void);

extern void I2411E_Process(void);
extern void I2411E_ChangeCarID(u8 * TmpCarInfor);
extern void I2411E_ChangeCarStatus(u8 Car_InputCheck);
extern void I2411E_Process(void);


extern void I24_SendBuffer(unsigned short object, unsigned int handle, unsigned char* src, unsigned short len);
extern void I24_SendBuffer_pack(unsigned short object, unsigned int handle, unsigned char* src, unsigned short len);



#endif
