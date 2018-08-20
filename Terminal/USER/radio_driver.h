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
//  CarTerminal = 1,  //车载智能终端
//  DataCollector = 2,//数据采集仪
//  PolicePhone = 3,  //警务通
//  UserPhone = 4,    //车主手机
//  EleDriverBall = 5,//电子驾照球
//  PoliceServer = 6, //服务器
//  CarTerManager = 7,//智能终端管理工具
//  OBD_Module = 8    //OBD模块
//}MsgSource_Typedef;


//typedef struct
//{
//  unsigned short int MsgID;//消息ID
//  unsigned short int MsgLen;//消息长度
//  MsgSource_Typedef  MsgSource;//消息源头
//  unsigned short int MsgNum;//消息流水号
//  unsigned short int MsgCheck;//校验码
//}Msg_Struct;

/*  variables ---------------------------------------------------------*/
#define  SI4432_buf_Len  64
extern unsigned char Si4432_RxBuf[SI4432_buf_Len];
extern unsigned char Si4432_TxBuf[SI4432_buf_Len];
extern volatile unsigned char Flag_si4432_RxBuf;
extern volatile unsigned char ItStatus1, ItStatus2;

/*  functions ---------------------------------------------------------*/
extern void Si4432_SendChangeLight(unsigned char* src);
extern void Si4432_SetRxMode(void);//设置为接受数据模式
extern unsigned char Si4432_ReadBuffer(void);
//extern void Si4432_RxBufferProcess(void);
extern void Si4432_SetTxMode(void);
extern void Si4432_WaitTxFinished(void);
extern void Si4432_Initialize(void);
extern void Si4432_WriteByte(unsigned char TRdata);//写一个字节的数据
extern unsigned char Si4432_ReadByte(void);//读一个字节的数据
extern void Si4432_WriteRegister(unsigned char reg, unsigned char TRdata);//往寄存器写值
extern unsigned char Si4432_ReadRegister(unsigned char reg);//

#endif
