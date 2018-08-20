#ifndef __FLASH_USER_H
#define __FLASH_USER_H

#include "stm32f10x.h"
#include "gtw_type.h"

/**************常用变量宏定义************/
//#define IsElecTicket 5  // ＝ 5为有电子罚单写入到Flash


extern unsigned char m_flagSave;    // 保存设备基本参数
extern unsigned short m_delaySave;  // 保存延时[基于ms级别]
extern void MX25L128_CreatSaveTask(void); // 设置保存标记

extern unsigned int ViolenceTotal;//用于记录违章总数
extern unsigned int ViolenceUntreated;

void MX25L128_SaveBase( unsigned char nDelay );
void MX25L128_Initialize(void);
void MX25L128_Process(void);
unsigned int   MX25L128_Illegal_Write(Illegal_Def* ill);
unsigned int   MX25L128_Illegal_Write_Base(Illegal_Def* ill, unsigned char flag);
void History_UseCar(unsigned char* src, unsigned char type);

#endif
