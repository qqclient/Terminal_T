
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

u8 SendBuffer[] = {//0xaa,0xc1,0x00,0xa1,0x0a,//命令
    0x00,0x01,0x02,0x03,0x04,05,06,07,  //数据
//    0x80  //bit7:最后一个包  bit0~bit6:包号
//    0xE8,0xAE,0xFC,0x86
};//校验码CRC32



u8 RevBuffer[256];

SC_State SCState ;
SC_ADPU_Commands SC_ADPU;
SC_ADPU_Responce SC_Responce;


/*******************************************************************************
* Function Name  : RJMU101_Test
* Description    :
* Input          : Buffer:需要加密的数据  len:数据长度  SecType:加密方式  DataType:数据类型
* Output         : None
* Return         : None
*******************************************************************************/
void RJMU_SendBuffer(u8 *Buffer,u8 len,INS_Typedef SecType,u8 type)
{
    SC_ADPU.Header.CLA = 0xaa;
    SC_ADPU.Header.INS = SecType;//加密方式
    SC_ADPU.Header.P1 = type;//00加密,  01解密
    SC_ADPU.Header.P2 = 0xa1;

    SC_ADPU.Body.LC = len+2;//len 数据长度 ;1byte包号 ; 1byte校验和;
    SC_ADPU.Body.LE = 0;

    if(SC_ADPU.Body.LC)
    {
        memcpy(SC_ADPU.Body.Data,Buffer,len);
        SC_ADPU.Body.Data[len++] = 0x80;//bit7 是否为最后一包   bit0-bit6 包号
        SC_ADPU.Body.Data[len] = CheckSum(SC_ADPU.Body.Data,len);//len 数据长度
    }
    SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
}

/*******************************************************************************
* Function Name  : RJMU101_SendKey
* Description    :
* Input          : Buffer:需要加密的数据
* Output         : None
* Return         : None
*******************************************************************************/
void RJMU_SendKey(u8 *Buffer)
{
    SC_ADPU.Header.CLA = 0xaa;
    SC_ADPU.Header.INS = RSA;//加密方式
    SC_ADPU.Header.P1 = 0x02;//RSA:私钥解密
    SC_ADPU.Header.P2 = INS_SecretKey;

    SC_ADPU.Body.LC = 130;//128byte RSA私密  1byte包号  1byte校验和
    SC_ADPU.Body.LE = 0;

    if(SC_ADPU.Body.LC)
    {
        memcpy(SC_ADPU.Body.Data,Buffer,128);
        SC_ADPU.Body.Data[128] = 0x80;//bit7 是否为最后一包   bit0-bit6 包号
        SC_ADPU.Body.Data[129] = CheckSum(SC_ADPU.Body.Data,129);
    }
    SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
}



/*******************************************************************************
* Function Name  : RJMU101_Test
* Description    :
* Input          : Buffer 指令
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
    SC_ADPU.Body.LE = len;//接受数据长度

    SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
    memcpy(buffer,SC_Responce.Data,SC_ADPU.Body.LE);
}


/*******************************************************************************
* Function Name  : SmartcardTest
* Description    : 测试程序   SW1 =90 成功    SW1=6A 校验错误   SW1 = 6E CMD错误
* Input          : None DES 8字节对齐   AES16字节对齐   SM4  16字节对齐 (自动补0)
* Output         : None
* Return         : NoneSW


2.秘钥管理方式
3.数据销毁(秘钥,明文,结果)
4.CRC校验方式
5.非对称加密方式选择

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

    RJMU_SendKey(KeyBuffer);//发送密钥

    RJMU_SendBuffer(SendBuffer,sizeof(SendBuffer),DES,RESET);//发送明文数据

    RJMU_RevBuffer(RevBuffer,8);//接受加密结果

    RJMU_SendBuffer(RevBuffer,sizeof(SendBuffer),DES,SET);

    //接受解密结果
    RJMU_RevBuffer(RevBuffer,8);//接受加密结果
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
    //下发数据长度
    case 0:
    {
        SC_ADPU.Header.INS = 0xC0;
        SC_ADPU.Header.P1 = 0x00;
        SC_ADPU.Header.P2 = 0xA3;
        break;
    }
    //下发明文/密文校验码
    case 1:
    {
        break;
    }
    //下发秘钥的校验码
    case 2:
    {
        break;
    }
    //加密:下发AES明文数据
    case 3:
    {
        break;
    }
    //加密:下发AES秘钥数据
    case 4:
    {
        break;
    }
    //解密:下发AES密文数据
    case 5:
    {
        break;
    }
    //解密:下发AES秘钥数据
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
    //为0时表示发送数据
    case 0:
    {
        SC_ADPU.Body.LC = Len;//不为0时发送的字节数
        SC_ADPU.Body.LE = 0x00;//不为0时接受的字节数

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
    //为1时表示接受数据
    case 1:
    {
        SC_ADPU.Body.LC = 0x00;//不为0时发送的字节数
        SC_ADPU.Body.LE = Len;//不为0时接受的字节数
        SC_Handler(&SCState, &SC_ADPU, &SC_Responce);
        break;
    }
    default :
        break;
    }

}




