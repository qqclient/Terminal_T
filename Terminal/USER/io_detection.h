#ifndef __IO_DETECTION_H
#define __IO_DETECTION_H

#include "stm32f10x.h"

typedef enum {
    HighBeam = 0,//远光灯
    LowBeam = 1,//近光灯
    Ignite = 2,//点火
    Honk = 3,//喇叭
    DriverBelt = 4,//主安全带
    DeputyBelt = 5,//副安全带
    DriverDoor = 6,//驾驶侧车门
    SecondDoor = 7//副驾驶侧车门
} KeyType_Typedef; //按键类型

typedef struct {
    unsigned char Times_HighBeam;      //远光灯
    unsigned char Times_LowBeam;       //近光灯
    unsigned char Times_Ignite;        //点火
    unsigned char Times_Honk;          //喇叭
    unsigned char Times_DriverBelt;    //主安全带
    unsigned char Times_DeputyBelt;    //副安全带
    unsigned char Times_DriverDoor;    //驾驶侧车门
    unsigned char Times_SecondDoor;    //副驾驶侧车门
} KeyTimes_Typedef;     //按键消抖计数



typedef struct {
    volatile unsigned char Value_HighBeam;  // 远光灯
    volatile unsigned char Value_LowBeam;   // 近光灯
    volatile unsigned char Value_Ignite;    // 点火
    volatile unsigned char Value_Honk;      // 喇叭
    volatile unsigned char Value_DriverBelt;// 主安全带
    volatile unsigned char Value_DeputyBelt;// 副安全带1
    volatile unsigned char Value_DeputyBelt2;// 副安全带2
    volatile unsigned char Value_DriverDoor;// 门开关1
    volatile unsigned char Value_SecondDoor;// 门开关2
    volatile unsigned char Value_IC1;       // 指示灯
    volatile unsigned char Value_KEY;       //
    volatile unsigned char Value_C_KEY;     // 自擦除输入引脚检测
} Car_InputCheck_Typedef;
extern Car_InputCheck_Typedef Car_InputCheck;
extern Car_InputCheck_Typedef Car_SetupCheck;
extern Car_InputCheck_Typedef Car_InputCheck_Last;


void Study_CarStatus(void);
void Input_Study(void);
void Read_CarStatus(unsigned char real);
void Read_Key(void);;

#endif

