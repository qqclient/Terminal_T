#ifndef __FLASH_USER_H
#define __FLASH_USER_H

#include "stm32f10x.h"
#include "gtw_type.h"

/**************���ñ����궨��************/
//#define IsElecTicket 5  // �� 5Ϊ�е��ӷ���д�뵽Flash


extern unsigned char m_flagSave;    // �����豸��������
extern unsigned short m_delaySave;  // ������ʱ[����ms����]
extern void MX25L128_CreatSaveTask(void); // ���ñ�����

extern unsigned int ViolenceTotal;//���ڼ�¼Υ������
extern unsigned int ViolenceUntreated;

void MX25L128_SaveBase( unsigned char nDelay );
void MX25L128_Initialize(void);
void MX25L128_Process(void);
unsigned int   MX25L128_Illegal_Write(Illegal_Def* ill);
unsigned int   MX25L128_Illegal_Write_Base(Illegal_Def* ill, unsigned char flag);
void History_UseCar(unsigned char* src, unsigned char type);

#endif
