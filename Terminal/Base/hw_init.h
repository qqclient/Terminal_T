/*******************************************************************************
* File Name          : hw_init.h
* Author             : CSYXJ
* Version            : V0.1
* Date               : 2017-03-19
* Description        : MCU�ӿڶ����ļ�
*******************************************************************************/


#define __DEBUG             // ʹ�õ������
#define __KEY_VOICE__       // ������������
//#define __USE_UCOS__        // ʹ��uc/os
//#define __GPS_DMA__         // DMA����GPS����
//#define __HARDWARE_TEST__   // �ߵ��²���


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


// ��������
void MCU_Initialize(void);
void NVIC_Configuration(void);
unsigned short get_crc16(unsigned char* src, unsigned long sizes);
