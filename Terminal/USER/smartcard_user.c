
#include "smartcard_user.h"
#include "smartcard.h"
#include "utility.h"
#include "string.h"

u8 TestBuff7[20];

u8 KeyBuffer[] =
{   0x37,0x07,0x58,0xB2,0xA4,0x8B,0x56,0xF1,0x74,0x88,
    0x25,0x1D,0xE6,0x63,0x7E,0x3B,0x32,0xBB,0x78,0x53,
    0xB9,0xA8,0x83,0x7A,0x21,0x07,0x58,0xEA,0xA3,0xF4,
    0x71,0x72,0x42,0xB3,0x9E,0x6B,0x6C,0x51,0x55,0x63,
    0x9E,0x23,0x93,0x99,0xE1,0x34,0x2A,0x63,0x53,0x88,
    0x25,0x43,0x4E,0xE1,0x3B,0xD3,0x98,0x48,0x52,0x84,
    0x09,0xA1,0x8F,0x99,0x05,0x04,0x08,0xE7,0xD5,0xB5,
    0xA9,0x60,0xEF,0x30,0x91,0xC4,0x0B,0xFD,0x69,0xFA,
    0xC5,0x2C,0xE8,0x09,0x5D,0x65,0x9F,0x10,0xEC,0x83,
    0xD8,0xBD,0x1B,0x33,0x87,0xA4,0x99,0x4F,0x15,0x63,
    0x00,0xC5,0x61,0x10,0x67,0x75,0x90,0x37,0x18,0x16,
    0xB0,0x1F,0xCD,0x64,0x73,0x80,0x3F,0x35,0x84,0x4E,
    0xD0,0xD4,0x57,0x04,0x1A,0x13,0x66,0x82
};

u8 SendBuffer[] = {//0xaa,0xc1,0x00,0xa1,0x0a,//����
    0x00,0x01,0x02,0x03,0x04,05,06,07,  //����
//    0x80  //bit7:���һ����  bit0~bit6:����
//    0xE8,0xAE,0xFC,0x86
};//У����CRC32



u8 RevBuffer[256];

SC_State SCState ;
SC_ADPU_Commands SC_ADPU;
SC_ADPU_Responce SC_Responce;


/*******************************************************************************
* Function Name  : RJMU101_Test
* Description    :
* Input          : Buffer:��Ҫ���ܵ�����  len:���ݳ���  SecType:���ܷ�ʽ  DataType:��������
* Output         : None
* Return         : None
*******************************************************************************/
void RJMU_SendBuffer(u8 *Buffer,u8 len,INS_Typedef SecType,u8 type)
{
    SC_ADPU.Header.CLA = 0xaa;
    SC_ADPU.Header.INS = SecType;//���ܷ�ʽ
    SC_ADPU.Header.P1 = type;//00����,  01����
    SC_ADPU.Header.P2 = 0xa1;

    SC_ADPU.Body.LC = len+2;//len ���ݳ��� ;1byte���� ; 1byteУ���;
    SC_ADPU.Body.LE = 0;

    if(SC_ADPU.Body.LC)
    {
        memcpy(SC_ADPU.Body.Data,Buffer,len);
        SC_ADPU.Body.Data[len++] = 0x80;//bit7 �Ƿ�Ϊ���һ��   bit0-bit6 ����
        SC_ADPU.Body.Data[len] = CheckSum(SC_ADPU.Body.Data,len);//len ���ݳ���
    }
    SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
}

/*******************************************************************************
* Function Name  : RJMU101_SendKey
* Description    :
* Input          : Buffer:��Ҫ���ܵ�����
* Output         : None
* Return         : None
*******************************************************************************/
void RJMU_SendKey(u8 *Buffer)
{
    SC_ADPU.Header.CLA = 0xaa;
    SC_ADPU.Header.INS = RSA;//���ܷ�ʽ
    SC_ADPU.Header.P1 = 0x02;//RSA:˽Կ����
    SC_ADPU.Header.P2 = INS_SecretKey;

    SC_ADPU.Body.LC = 130;//128byte RSA˽��  1byte����  1byteУ���
    SC_ADPU.Body.LE = 0;

    if(SC_ADPU.Body.LC)
    {
        memcpy(SC_ADPU.Body.Data,Buffer,128);
        SC_ADPU.Body.Data[128] = 0x80;//bit7 �Ƿ�Ϊ���һ��   bit0-bit6 ����
        SC_ADPU.Body.Data[129] = CheckSum(SC_ADPU.Body.Data,129);
    }
    SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
}



/*******************************************************************************
* Function Name  : RJMU101_Test
* Description    :
* Input          : Buffer ָ��
* Output         : None
* Return         : None
*******************************************************************************/
void RJMU_RevBuffer( u8 *buffer,u8 len)
{
    SC_ADPU.Header.CLA = 0xaa;
    SC_ADPU.Header.INS = 0xca;
    SC_ADPU.Header.P1 = 0x00;
    SC_ADPU.Header.P2 = 0x00;
    SC_ADPU.Body.LC = 0;
    SC_ADPU.Body.LE = len;//�������ݳ���

    SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
    memcpy(buffer,SC_Responce.Data,SC_ADPU.Body.LE);
}


/*******************************************************************************
* Function Name  : SmartcardTest
* Description    : ���Գ���   SW1 =90 �ɹ�    SW1=6A У�����   SW1 = 6E CMD����
* Input          : None DES 8�ֽڶ���   AES16�ֽڶ���   SM4  16�ֽڶ��� (�Զ���0)
* Output         : None
* Return         : NoneSW


2.��Կ����ʽ
3.��������(��Կ,����,���)
4.CRCУ�鷽ʽ
5.�ǶԳƼ��ܷ�ʽѡ��

*******************************************************************************/
void SmartcardTest(void)
{
    /* Start SC Demo ---------------------------------------------------------*/
    /* Wait A2R --------------------------------------------------------------*/
    SCState = SC_POWER_ON;
    SC_ADPU.Header.CLA = 0x00;
    SC_ADPU.Header.INS = SC_GET_A2R;
    SC_ADPU.Header.P1 = 0x00;
    SC_ADPU.Header.P2 = 0x00;
    SC_ADPU.Body.LC = 0x00;

    while(SCState != SC_ACTIVE_ON_T0)
    {
        SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
    }

    /* Apply the Procedure Type Selection (PTS) */
    SC_PTSConfig();

    RJMU_SendKey(KeyBuffer);//������Կ

    RJMU_SendBuffer(SendBuffer,sizeof(SendBuffer),DES,RESET);//������������

    RJMU_RevBuffer(RevBuffer,8);//���ܼ��ܽ��

    RJMU_SendBuffer(RevBuffer,sizeof(SendBuffer),DES,SET);

    //���ܽ��ܽ��
    RJMU_RevBuffer(RevBuffer,8);//���ܼ��ܽ��
}





/*******************************************************************************
* Function Name  : RJMU101_SendBuff
* Description    :
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RJMU101_SendBuff(u8 *TxBuff,u8 Len,u8 RevOrSend,u8 Type)
{
//  RJMU101_Buffer[0] = 0xAA;
//  RJMU101_Buffer[0] = 0xAA;
    u16 i;
    SC_ADPU.Header.CLA = 0xAA;

    switch(Type)
    {
    //�·����ݳ���
    case 0:
    {
        SC_ADPU.Header.INS = 0xC0;
        SC_ADPU.Header.P1 = 0x00;
        SC_ADPU.Header.P2 = 0xA3;
        break;
    }
    //�·�����/����У����
    case 1:
    {
        break;
    }
    //�·���Կ��У����
    case 2:
    {
        break;
    }
    //����:�·�AES��������
    case 3:
    {
        break;
    }
    //����:�·�AES��Կ����
    case 4:
    {
        break;
    }
    //����:�·�AES��������
    case 5:
    {
        break;
    }
    //����:�·�AES��Կ����
    case 6:
    {
        break;
    }
    case 7:
    {
        break;
    }
    case 8:
    {
        break;
    }
    }

    switch (RevOrSend)
    {
    //Ϊ0ʱ��ʾ��������
    case 0:
    {
        SC_ADPU.Body.LC = Len;//��Ϊ0ʱ���͵��ֽ���
        SC_ADPU.Body.LE = 0x00;//��Ϊ0ʱ���ܵ��ֽ���

        if(SC_ADPU.Body.LC)
        {
            for(i=0; i<SC_ADPU.Body.LC; i++)
            {
                SC_ADPU.Body.Data[i]=TxBuff[i+6];
            }

            while(i < LCmax)
            {
                SC_ADPU.Body.Data[i++] = 0;
            }
        }
        SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
        break;
    }
    //Ϊ1ʱ��ʾ��������
    case 1:
    {
        SC_ADPU.Body.LC = 0x00;//��Ϊ0ʱ���͵��ֽ���
        SC_ADPU.Body.LE = Len;//��Ϊ0ʱ���ܵ��ֽ���
        SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
        break;
    }
    default :
        break;
    }

}




