#ifndef __ELECDRIVERLIC_H
#define __ELECDRIVERLIC_H

#include "stm32f10x.h"

#define STACK_SIZE 5	//��ջ��С


/* ---ĳһ���ȼ��������¼��ʻԱ�ļ��պ� ���ݽṹ --------------------------------------------*/
typedef struct {
    unsigned char ApplyLicStack[STACK_SIZE][ __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ ];//
    int pointer;
} ApplyLic_Struct;

/* ---�������ȼ��������¼��ʻԱ�ļ��պ� ���ݽṹ --------------------------------------------*/
typedef struct {
    ApplyLic_Struct FirApplyLic;		//��һ���ȼ��������˼�����Ϣ
    ApplyLic_Struct SecApplyLic;		//�ڶ����ȼ��������˼�����Ϣ
    ApplyLic_Struct ThiApplyLic;		//�������ȼ��������˼�����Ϣ
    ApplyLic_Struct FouApplyLic;		//�������ȼ��������˼�����Ϣ
    ApplyLic_Struct FifApplyLic;		//�������ȼ��������˼�����Ϣ
    ApplyLic_Struct NoneApplyLic;		//�����ȼ��������˼�����Ϣ
} AllApplyLic_Struct;

/***************���س�������*************/
//#define STACK_TYPE (unsigned char) /* ��ջ���洢��ֵ���������� */

/***************���غ�������*************/
//void PopApplyStack(ApplyLic_Struct tmpLic);
void  ApplyLicInit(void);
void  PushApplyStack(ApplyLic_Struct *tmpLic, u8 *str,u8 len);
void  GetApplyData(AllApplyLic_Struct* alltmpLic, u8 * idNo);
int   is_emptyApplyData(AllApplyLic_Struct *alltmpLic);
int   is_full(ApplyLic_Struct *tmpLic);//�ж϶�ջ�Ƿ���ջ
int   is_empty(ApplyLic_Struct *tmpLic);//�ж϶�ջ�Ƿ�Ϊ��
void  PopApplyStack(ApplyLic_Struct *tmpLic);//�����ݵ�����ջ


/***************ȫ�ֱ�������*************/
extern AllApplyLic_Struct ApplyLicense;		//�����˵ļ��պ�volatile


#endif
