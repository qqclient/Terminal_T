/*******************************************************************************
* File Name          : hw_init.h
* Author             : CSYXJ
* Version            : V0.1
* Date               : 2017-03-19
* Description        : MCU接口定义文件
*******************************************************************************/


#define __DEBUG             // 使用调试输出
#define __KEY_VOICE__       // 按键语音播报
//#define __USE_UCOS__        // 使用uc/os
//#define __GPS_DMA__         // DMA接收GPS数据
//#define __HARDWARE_TEST__   // 高低温测试


#define __PCB_V32__  

#ifdef __PCB_V1__ 
#include "hw_v1.h"
#endif

#ifdef __PCB_V2__
#include "hw_v2.h"
#endif

#ifdef __PCB_V3__
#include "hw_v3.h"
#endif

#ifdef __PCB_V31__
#include "hw_v31.h"
#endif

#ifdef __PCB_V32__
#include "hw_v32.h"
#endif

#define __VER_YEAR      2017
#define __VER_MONTH     11
#define __VER_DAY       01

extern const char __ver_id[];
extern const char __pcb_ver_id[];


// 函数定义
void MCU_Initialize(void);
void NVIC_Configuration(void);
unsigned short get_crc16(unsigned char* src, unsigned long sizes);
