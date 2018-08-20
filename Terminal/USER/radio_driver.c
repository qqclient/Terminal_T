/******************** (C) COPYRIGHT soarsky ********************
* File Name          : radio.c
* Author             :
* Version            :
* Date               :
* Description        : ����ģ��ʹ�õ���si4432оƬ
            ----------------------------------------------
*���ݸ�ʽ��|Preamble|SYNC|HEADER ADDR|PK LENGTH|DATA|CRC |
            ----------------------------------------------

*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"

/* Private define ------------------------------------------------------------*/
#define  SI4432_PWRSTATE_READY	0x01		//READY Mode 
#define  SI4432_PWRSTATE_TX			0x09		//TX on in Manual Transmit Mode ;  READY Mode 	
#define  SI4432_PWRSTATE_RX			0x05		//RX on in Manual Receiver Mode ;  READY Mode 	

#define  TX1_RX0	Si4432_WriteRegister(0x0E,0x01)//I/O Port Configuration
#define  TX0_RX1	Si4432_WriteRegister(0x0E,0x02)//I/O Port Configuration
#define  TX0_RX0	Si4432_WriteRegister(0x0E,0x00)//I/O Port Configuration  

/*����si4432-----------------------------------------------------------------------*/
#define Si4432_IFBW_1C		0	//0. 1C --> IFBW: IF Filter Bandwidth
#define Si4432_COSR_20		1	//1. 20 --> COSR: Clock Recovery OverSampling Ratio
#define Si4432_CRO2_21		2	//2. 21 --> CRO2: Clock Recovery Offset 2
#define Si4432_CRO1_22		3	//3. 22 --> CRO1: Clock Recovery Offset 1
#define Si4432_CRO0_23		4	//4. 23 --> CRO0: Clock Recovery Offset 0
#define Si4432_CTG1_24		5	//5. 24 --> CTG1: Clock Recovery Timing Loop Gain 1
#define Si4432_CTG0_25		6	//6. 25 --> CTG0: Clock Recovery Timing Loop Gain 0
#define Si4432_TDR1_6E		7	//7. 6E --> TDR1: TX Data Rate 1
#define Si4432_TDR0_6F		8	//8. 6F --> TDR0: TX Data Rate 0
#define Si4432_MMC1_70		9	//9. 70 --> MMC1: Modulation Mode Control 1
#define Si4432_FDEV_72		10	//10.72 --> TXFDEV: Frequency Deviation
#define Si4432_RXFDEV_72	11	//11.72 --> RXFDEV: Frequency Deviation
#define Si4432_BTIME		  12	//12.   --> B_TIME:
#define Si4432_AFC_1D		  13	//13.1D --> AFC: AFC Loop Gearshift Override
#define Si4432_AFCL_2A		14	//14.2A --> AFCLimiter: 

static u8 Si4432VL[] = {0x83,0x5e,0x01,0x5D,0x86,0x02,0xBB,0x20,0xc5,0x00,0x66,0x66,8,0x40,0x50};

/* Private variables ---------------------------------------------------------*/

u8 Si4432_RxBuf[SI4432_buf_Len];
u8 Si4432_TxBuf[SI4432_buf_Len];
volatile unsigned char Flag_si4432_RxBuf = RESET;
volatile unsigned char ItStatus1, ItStatus2;

/*******************************************************************************
* Function Name  : Si4432_ReadBuffer
* Description    : ��ȡSi4432�����е�ֵ
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
unsigned char Si4432_ReadBuffer(void)
{
    unsigned char i;
    unsigned char TempLen = 0;

    if( read_Si4432_IRQ == 0)//�鿴�Ƿ����ж�
    {
        //����ģ�鴦�ڿ���ģʽ��������ܵ������ݰ�����������������
        Si4432_WriteRegister(0x07, SI4432_PWRSTATE_READY);	//READY Mode

        //nIRQ���Ž��ᱣ�ֵ͵�ƽֱ����Ƭ�������ж�״̬�Ĵ���03h~04h
        ItStatus1 = Si4432_ReadRegister(0x03);	//read the Interrupt Status1 register
        ItStatus2 = Si4432_ReadRegister(0x04);	//read the Interrupt Status2 register

        if((ItStatus1 & 0x02) == 0x02 )	//packet received interrupt occured,���ݽ����ж�
        {
            //��ȡ���ݰ�����
            TempLen = Si4432_ReadRegister(0x4B); //read the Received Packet Length register
            if(TempLen > 0) {
                rst_Si4432_SEL;//nSEL = 0;
                Si4432_WriteByte( 0x7F );//FIFO Access
                for(i = 0; i < TempLen; i++) {
                    Si4432_RxBuf[i] = Si4432_ReadByte();//�����ܵ������ݴ�������
                }
                Flag_si4432_RxBuf = SET;
            }
            set_Si4432_SEL; //nSEL = 1;
        }
        Si4432_WriteRegister(0x07, SI4432_PWRSTATE_RX);//���ݰ�������ɣ�����ģ�������������
    }
    return TempLen;
}


/*******************************************************************************
* Function Name  : Si4432_Init
* Description    : Si4432 RF����ģ���ʼ��
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void Si4432_Initialize(void)
{
    //-------------------------���ų�ʼ��---------------------------------------------------------
    set_Si4432_SDI;
    set_Si4432_SDO;
    rst_Si4432_SCK;
    set_Si4432_SEL;
    set_Si4432_IRQ;
    //--------------------------ģ���ϵ縴λ------------------------------------------------------
    set_Si4432_SDN;
    Delay_ms (10);
    rst_Si4432_SDN;
    Delay_ms (200);
    //---------------------------�Ĵ�����ʼ��----------------------------------------------------
    //nIRQ���Ž��ᱣ�ֵ͵�ƽֱ����Ƭ�������ж�״̬�Ĵ���03h~04h
    Si4432_ReadRegister(0x03);//read the Interrupt Status1 register
    Si4432_ReadRegister(0x04);//read the Interrupt Status2 register
    Si4432_WriteRegister(0x06,0x00);//Interrupt Enable 2
    Si4432_WriteRegister(0x07,SI4432_PWRSTATE_READY);//Operating Mode and Function Control 1  ; READY Mode
    Si4432_WriteRegister(0x09,0x7F);//30 MHz Crystal Oscillator Load Capacitance ; Reset value = 01111111 //���ص���=12p
    Si4432_WriteRegister(0x0A,0x05);//Microcontroller Output Clock ;  Reset value = xx000110
    Si4432_WriteRegister(0x0B,0xEA);//GPIO Configuration 0 ;   Direct Digital Output
    Si4432_WriteRegister(0x0C,0xEA);//GPIO Configuration 1 ;   Direct Digital Output
    Si4432_WriteRegister(0x0D,0xF4);//GPIO Configuration 2 ;   RX Data (output)
    Si4432_WriteRegister(0x70,Si4432VL[Si4432_MMC1_70]);//Modulation Mode Control 1
    Si4432_WriteRegister(0x1D,Si4432VL[Si4432_AFC_1D]); //AFC Loop Gearshift Override  AFCѭ������
    Si4432_WriteRegister(0x1C,Si4432VL[Si4432_IFBW_1C]);//IF Filter Bandwidth IF�˲�������
    Si4432_WriteRegister(0x20,Si4432VL[Si4432_COSR_20]);//Clock Recovery Oversampling Rate//ʱ�ӻظ���������
    Si4432_WriteRegister(0x21,Si4432VL[Si4432_CRO2_21]);//Clock Recovery Offset 2
    Si4432_WriteRegister(0x22,Si4432VL[Si4432_CRO1_22]);//Clock Recovery Offset 1
    Si4432_WriteRegister(0x23,Si4432VL[Si4432_CRO0_23]);//Clock Recovery Offset 0
    Si4432_WriteRegister(0x24,Si4432VL[Si4432_CTG1_24]);//Clock Recovery Timing Loop Gain 1//ʱ�ӻָ���ʱ������1
    Si4432_WriteRegister(0x25,Si4432VL[Si4432_CTG0_25]);//Clock Recovery Timing Loop Gain 0
    Si4432_WriteRegister(0x6E,Si4432VL[Si4432_TDR1_6E]);//TX Data Rate 1//����������
    Si4432_WriteRegister(0x6F,Si4432VL[Si4432_TDR0_6F]);//TX Data Rate 0
    Si4432_WriteRegister(0x30,0x8C);//Data Access Control ;  ʹ��PH+ FIFOģʽ����λ��ǰ�棬ʹ��CRCУ��
    Si4432_WriteRegister(0x32,0xFF);//Header Control 1  ;   byte0, 1,2,3 ��Ϊͷ��
    Si4432_WriteRegister(0x33,0x42);//Header Control 2  ;   byte 0,1,2,3 ��ͷ�룬ͬ����3,2 ��ͬ����
    Si4432_WriteRegister(0x34,16);  //Preamble Length   ;   �����볤��
    Si4432_WriteRegister(0x35,0x20);//Preamble Detection Control 1
    Si4432_WriteRegister(0x36,0x2D);//Synchronization Word 3//ͬ����Ϊ0x2DD4
    Si4432_WriteRegister(0x37,0xD4);//Synchronization Word 2
    Si4432_WriteRegister(0x38,0x00);//Synchronization Word 1
    Si4432_WriteRegister(0x39,0x00);//Synchronization Word 0
    Si4432_WriteRegister(0x3A,'s');//Transmit Header 3  ; �����ͷ��Ϊ�� ��swwx"
    Si4432_WriteRegister(0x3B,'o');//Transmit Header 2
    Si4432_WriteRegister(0x3C,'a');//Transmit Header 1
    Si4432_WriteRegister(0x3D,'r');//Transmit Header 0
    Si4432_WriteRegister(0x3E,10); //Packet Length//�ܹ�����10���ֽڵ�����
    Si4432_WriteRegister(0x3F,'s');//Check Header 3
    Si4432_WriteRegister(0x40,'o');//Check Header 2
    Si4432_WriteRegister(0x41,'a');//Check Header 1
    Si4432_WriteRegister(0x42,'r');//Check Header 0
    Si4432_WriteRegister(0x43,0xFF);//Header Enable 3//ͷ������λ����ҪУ��
    Si4432_WriteRegister(0x44,0xFF);//Header Enable 2
    Si4432_WriteRegister(0x45,0xFF);//Header Enable 1
    Si4432_WriteRegister(0x46,0xFF);//Header Enable 0
    Si4432_WriteRegister(0x6D,0x07);//TX Power  ����Ϊ����书��
    Si4432_WriteRegister(0x79,0x00);//Frequency Hopping Channel Select ���ŵ�ѡ��
    Si4432_WriteRegister(0x7A,0x00);//Frequency Hopping Step Size ; //��Ƶ
    //���Ʒ�ʽ��FSK �� TX Data CLK is available via the SDO pin. ��Direct Mode
    Si4432_WriteRegister(0x71,0x22);//Modulation Mode Control 2  ;
    //set the Tx deviation register (+-66kHz)
    Si4432_WriteRegister(0x72,Si4432VL[Si4432_FDEV_72]);//Frequency Deviation  ��Ƶ��ƫ��
    Si4432_WriteRegister(0x73,0x00);//Frequency Offset 1  :û��Ƶ��ƫ��
    Si4432_WriteRegister(0x74,0x00);//Frequency Offset 2
    Si4432_WriteRegister(0x75,0x53);//Frequency Band Select  ��Ƶ��
    Si4432_WriteRegister(0x76,0x57);//Nominal Carrier Frequency   ���ز�Ƶ�ʸ�λ
    Si4432_WriteRegister(0x77,0x80);//Nominal Carrier Frequency   ���ز�Ƶ�ʵ�λ
    TX0_RX0;//I/O Port Configuration
    //--------------------------------------------------------------------------
    Si4432_SetRxMode();//��ʼ����ɽ������ģʽ
}


/*******************************************************************************
* Function Name  : Si4432_SetRxMode
* Description    : ����RF ����ģ��Ϊ����ģʽ
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void Si4432_SetRxMode(void)
{
    Si4432_WriteRegister(0x07,SI4432_PWRSTATE_READY);// Operating Mode and Function Control 1  //����ģ�鴦�ڿ���״̬
    TX0_RX1;//I/O Port Configuration
    Delay_ms(1);
    //�������
    Si4432_WriteRegister(0x08,0x03);//Operating Mode and Function Control 2
    Si4432_WriteRegister(0x08,0x00);//Setting ffclrrx =1 followed by ffclrrx = 0 will clear the contents of the RX FIFO.
    Si4432_WriteRegister(0x07,SI4432_PWRSTATE_RX );//Operating Mode and Function Control 1
    Si4432_WriteRegister(0x05,0x02);//Interrupt Enable 1��Enable Valid Packet Received.
    //nIRQ���Ž��ᱣ�ֵ͵�ƽֱ����Ƭ�������ж�״̬�Ĵ���03h~04h
    Si4432_ReadRegister(0x03);//read the Interrupt Status1 register
    Si4432_ReadRegister(0x04);//read the Interrupt Status2 register
}


/*******************************************************************************
* Function Name  : Si4432_WaitTxFinished
* Description    : �ȴ������������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Si4432_WaitTxFinished(void)
{
    Si4432_SendTimeOut = 0;	//��ʼ���з����ʱ
    do
    {
        if(Si4432_SendTimeOut >= 1)	//1S��ʱ
        {
#ifdef __HARDWARE_TEST__
            Debug_Printf(SET, "\r\nSi4432 ERROR");
#endif
            Si4432_Initialize();
            break;
        }
        if( read_Si4432_IRQ == 0)
        {
            ItStatus1 = Si4432_ReadRegister(0x03);
            ItStatus2 = Si4432_ReadRegister(0x04);
#ifdef __HARDWARE_TEST__
            Debug_Printf(SET, "\r\nSi4432 OK");
#endif
            break;
        }
        IWDG_ReloadCounter();  // ι��

    } while(1);

    Si4432_SetRxMode();	//���ݷ�����ɣ��ָ�������ģʽ
}

/*******************************************************************************
* Function Name  : Si4432_SetTxMode
* Description    : �������е�����ͨ��RF�����ȥ
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Si4432_SetTxMode(void)
{

    //----------------------------------------------------------------
    Si4432_WriteRegister(0x07, SI4432_PWRSTATE_READY);	//Operating Mode and Function Control 1  ��
    TX1_RX0;		//I/O Port Configuration
    Delay_ms(1);
    //----------------------------------------------------------------
    //Setting ffclrrx =1 followed by ffclrrx = 0 will clear the contents of the RX FIFO.
    Si4432_WriteRegister(0x08,0x03);//Operating Mode and Function Control 2
    Si4432_WriteRegister(0x08,0x00);//Operating Mode and Function Control 2
    //---------------------------------------------------------------
    Si4432_WriteRegister(0x05,0x04);//Enable Packet Sent.//�������ݷ�����󣬲����ж�
    //nIRQ���Ž��ᱣ�ֵ͵�ƽֱ����Ƭ�������ж�״̬�Ĵ���03h~04h
    Si4432_ReadRegister(0x03);//Interrupt/Status 1
    Si4432_ReadRegister(0x04);//Interrupt/Status 2
    Si4432_WriteRegister(0x07,SI4432_PWRSTATE_TX);//Operating Mode and Function Control 1 //���뷢��ģʽ
    Si4432_WriteRegister(0x34,40);// Preamble Length ������40��Nibble��ǰ����
}


/*******************************************************************************
* Function Name  : Si4432_WriteByte
* Description    : дһ���ֽڵ�����
* Input          : TRdata����Ҫд������
* Output         : None
* Return         : None
******************************************************************************/
void Si4432_WriteByte(unsigned char TRdata)
{
    unsigned char i;
    for (i = 0; i < 8; i++)
    {

        if (TRdata & 0x80)
            set_Si4432_SDI;
        else
            rst_Si4432_SDI;

        //GPIO_WriteBit(GPIOB, Si4432_SCK, (BitAction)0);		//Si4432_SCK = 0;
        TRdata <<= 1;
        set_Si4432_SCK;
        set_Si4432_SCK;
        rst_Si4432_SCK;
    }
}


/*******************************************************************************
* Function Name  : Si4432_ReadByte
* Description    : ��һ���ֽڵ�����
* Input          : None
* Output         : None
* Return         : ����
******************************************************************************/
unsigned char Si4432_ReadByte(void)
{
    unsigned char i;
    unsigned char tmp = 0x00;

    for (i = 0; i < 8; i++)
    {
        set_Si4432_SCK;		//Si4432_SCK = 1;
        tmp <<= 1;
        if( read_Si4432_SDO != 0)
            tmp |= 0x01;
        else
            tmp &= 0xFE;
        rst_Si4432_SCK;		//Si4432_SCK = 0;
    }
    return tmp;
}


/*******************************************************************************
* Function Name  : Si4432_WriteRegister
* Description    : д�Ĵ���Ҫ���ַ�����λ��1�����Ե�ַ��Ҫ����0x80,
* Input          : reg���Ĵ�����ַ    TRdata������
* Output         : None
* Return         : None
*******************************************************************************/
void Si4432_WriteRegister(unsigned char reg, unsigned char TRdata)
{
    rst_Si4432_SEL;
    Si4432_WriteByte(reg|0x80);
    Si4432_WriteByte(TRdata);
    set_Si4432_SEL;		//SI4432_SEL = 1;
}

/*******************************************************************************
* Function Name  : Si4432_ReadRegister
* Description    : ��ȡSi4432�Ĵ�����ֵ
* Input          : reg���Ĵ�����ַ
* Output         : None
* Return         : ����
*******************************************************************************/
unsigned char Si4432_ReadRegister(unsigned char reg)
{
    unsigned char tmp;
    rst_Si4432_SEL;		//SI4432_SEL = 0;
    Si4432_WriteByte(reg);
    tmp = Si4432_ReadByte();
    set_Si4432_SEL;		//SI4432_SEL = 1;
    return tmp;
}

