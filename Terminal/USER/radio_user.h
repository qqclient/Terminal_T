#ifndef __RADIO_USER_H
#define __RADIO_USER_H

#include "stm32f10x.h"
#include "gtw_type.h"

extern i2b_Def                  CheckerAnswer_ID;       // 采集仪ID后四位
extern Msg_Sign_DataCollector_Struct DataCollectorMsg; // 数据采集仪稽查命令的标志信息
extern Msg_Sign_DataCollector_Struct GTW02_BoardCast_Msg;
extern volatile unsigned char   Si4432_SendTimeOut;
extern volatile unsigned short  Si4432_TimeSendChangeLight;
extern volatile unsigned short  Si4432_TimeSendSn;
extern unsigned char RF433_MsgMask;
extern unsigned char RF433_MsgData;

void Si4432_SendTimer(u8* TmpSN);
void Si4432_CheckerAnswer(void);
void Si4432_Initialize(void);
void Si4432_CheckLight(void);
void Msg_Set_Sign_DataColl(u8 *src);
void Si4432_RxBufferProcess(void);
void Si4432_SendSnToLightMode(void);

#endif
