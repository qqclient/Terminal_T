#ifndef __IO_DETECTION_H
#define __IO_DETECTION_H

#include "stm32f10x.h"

typedef enum {
    HighBeam = 0,//Զ���
    LowBeam = 1,//�����
    Ignite = 2,//���
    Honk = 3,//����
    DriverBelt = 4,//����ȫ��
    DeputyBelt = 5,//����ȫ��
    DriverDoor = 6,//��ʻ�೵��
    SecondDoor = 7//����ʻ�೵��
} KeyType_Typedef; //��������

typedef struct {
    unsigned char Times_HighBeam;      //Զ���
    unsigned char Times_LowBeam;       //�����
    unsigned char Times_Ignite;        //���
    unsigned char Times_Honk;          //����
    unsigned char Times_DriverBelt;    //����ȫ��
    unsigned char Times_DeputyBelt;    //����ȫ��
    unsigned char Times_DriverDoor;    //��ʻ�೵��
    unsigned char Times_SecondDoor;    //����ʻ�೵��
} KeyTimes_Typedef;     //������������



typedef struct {
    volatile unsigned char Value_HighBeam;  // Զ���
    volatile unsigned char Value_LowBeam;   // �����
    volatile unsigned char Value_Ignite;    // ���
    volatile unsigned char Value_Honk;      // ����
    volatile unsigned char Value_DriverBelt;// ����ȫ��
    volatile unsigned char Value_DeputyBelt;// ����ȫ��1
    volatile unsigned char Value_DeputyBelt2;// ����ȫ��2
    volatile unsigned char Value_DriverDoor;// �ſ���1
    volatile unsigned char Value_SecondDoor;// �ſ���2
    volatile unsigned char Value_IC1;       // ָʾ��
    volatile unsigned char Value_KEY;       //
    volatile unsigned char Value_C_KEY;     // �Բ����������ż��
} Car_InputCheck_Typedef;
extern Car_InputCheck_Typedef Car_InputCheck;
extern Car_InputCheck_Typedef Car_SetupCheck;
extern Car_InputCheck_Typedef Car_InputCheck_Last;


void Study_CarStatus(void);
void Input_Study(void);
void Read_CarStatus(unsigned char real);
void Read_Key(void);;

#endif

