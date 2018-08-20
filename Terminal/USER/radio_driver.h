#ifndef __RADIO_DRIVER_H
#define __RADIO_DRIVER_H


#define set_Si4432_SCK		GPIO_WriteBit(Si4432_SCK_PORT, Si4432_SCK, (BitAction)1)
#define rst_Si4432_SCK		GPIO_WriteBit(Si4432_SCK_PORT, Si4432_SCK, (BitAction)0)
#define set_Si4432_SDI		GPIO_WriteBit(Si4432_SDI_PORT, Si4432_SDI, (BitAction)1)
#define rst_Si4432_SDI		GPIO_WriteBit(Si4432_SDI_PORT, Si4432_SDI, (BitAction)0)
#define set_Si4432_SEL		GPIO_WriteBit(Si4432_SEL_PORT, Si4432_SEL, (BitAction)1)
#define rst_Si4432_SEL		GPIO_WriteBit(Si4432_SEL_PORT, Si4432_SEL, (BitAction)0)
#define set_Si4432_SDN		GPIO_WriteBit(Si4432_SDN_PORT, Si4432_SDN, (BitAction)1)
#define rst_Si4432_SDN		GPIO_WriteBit(Si4432_SDN_PORT, Si4432_SDN, (BitAction)0)
#define read_Si4432_SDO		GPIO_ReadInputDataBit(Si4432_SDO_PORT, Si4432_SDO)
#define set_Si4432_SDO		GPIO_WriteBit(Si4432_SDO_PORT, Si4432_SDO, (BitAction)1)
#define read_Si4432_IRQ		GPIO_ReadInputDataBit(Si4432_IRQ_PORT, Si4432_IRQ)
#define set_Si4432_IRQ		GPIO_WriteBit(Si4432_IRQ_PORT, Si4432_IRQ, (BitAction)1)

//typedef enum
//{
//  CarTerminal = 1,  //���������ն�
//  DataCollector = 2,//���ݲɼ���
//  PolicePhone = 3,  //����ͨ
//  UserPhone = 4,    //�����ֻ�
//  EleDriverBall = 5,//���Ӽ�����
//  PoliceServer = 6, //������
//  CarTerManager = 7,//�����ն˹�����
//  OBD_Module = 8    //OBDģ��
//}MsgSource_Typedef;


//typedef struct
//{
//  unsigned short int MsgID;//��ϢID
//  unsigned short int MsgLen;//��Ϣ����
//  MsgSource_Typedef  MsgSource;//��ϢԴͷ
//  unsigned short int MsgNum;//��Ϣ��ˮ��
//  unsigned short int MsgCheck;//У����
//}Msg_Struct;

/*  variables ---------------------------------------------------------*/
#define  SI4432_buf_Len  64
extern unsigned char Si4432_RxBuf[SI4432_buf_Len];
extern unsigned char Si4432_TxBuf[SI4432_buf_Len];
extern volatile unsigned char Flag_si4432_RxBuf;
extern volatile unsigned char ItStatus1, ItStatus2;

/*  functions ---------------------------------------------------------*/
extern void Si4432_SendChangeLight(unsigned char* src);
extern void Si4432_SetRxMode(void);//����Ϊ��������ģʽ
extern unsigned char Si4432_ReadBuffer(void);
//extern void Si4432_RxBufferProcess(void);
extern void Si4432_SetTxMode(void);
extern void Si4432_WaitTxFinished(void);
extern void Si4432_Initialize(void);
extern void Si4432_WriteByte(unsigned char TRdata);//дһ���ֽڵ�����
extern unsigned char Si4432_ReadByte(void);//��һ���ֽڵ�����
extern void Si4432_WriteRegister(unsigned char reg, unsigned char TRdata);//���Ĵ���дֵ
extern unsigned char Si4432_ReadRegister(unsigned char reg);//

#endif
