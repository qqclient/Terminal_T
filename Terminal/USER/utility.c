/******************** (C) COPYRIGHT soarsky ********************
* File Name          : utility
* Author             :
* Version            :
* Date               :
* Description        :
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"

/* Global variables ---------------------------------------------------------*/
volatile unsigned int TimingDelay;//用于Delay_ms定时，在中断中自减


/*******************************************************************************
* 函数名  : ReadDataBit
* 描述    : 读取data的bit位
*返回     : SET 指定bit为1,  RESET指定bit为0
*******************************************************************************/
u8 ReadDataBit(u8 data,u8 bit)
{
    if((data & bit) != 0) {
        return SET;
    }
    else {
        return RESET;
    }
}


/*******************************************************************************
* 函数名  : ReadDataBit
* 描述    : 读取data的bit位
*返回     : SET 指定bit为1,  RESET指定bit为0
*******************************************************************************/
u8 ReadData32Bit(u32 data,u32 bit)
{
    if((data & bit) != 0) {
        return SET;
    }
    else {
        return RESET;
    }
}


/*******************************************************************************
* Function Name  :
* Description    :
******************************************************************************/
void SystemReset(void)
{
    //给主板复位
    __set_FAULTMASK(1);   // 关闭所有中断
    NVIC_SystemReset();   // 系统复位
}


/*******************************************************************************
* Function Name  :Bcd2Deci
* Description    :
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
unsigned char IsDayOrNight(void)
{
    if(Sys_CalendarTime.hour >= 18 ||
            Sys_CalendarTime.hour < 6 )//晚上六点至第二天早上6点
    {
        return night_time;//夜间
    }
    else {
        return day_time;//白天
    }

}

/*******************************************************************************
* Function Name  :Bcd2Deci
* Description    :
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
unsigned char Bcd2Deci(unsigned char* asc, unsigned char* bcd)
{
    unsigned char i;
    if(asc == NULL || bcd == NULL ) return 0;

    for(i = 0; i<9; i++) {
        asc[i*2]   = bcd[i] >> 4;
        asc[i*2+1] = bcd[i] & 0x0F;
    }

    if(asc[17] == 0xA) {
        asc[17] = 'X';
    }
    return 1;
}


/*******************************************************************************
* Function Name  :Deci2Bcd
* Description    :
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
unsigned char Str2BcdDriverNo(unsigned char* str,unsigned char* bcd)
{
    u8 i;

    if(str == NULL || bcd == NULL ) return 0;

    for(i = 0; i<8; i++)
    {
        bcd[i] = (str[i*2]-0x30)<<4 | (str[i*2+1]-0x30);
    }

    if(str[17] == 'x' || str[17] == 'X')
    {
        bcd[8] = (str[16]-0x30)<<4 | 0xA;
    }
    else {
        bcd[8] = (str[16]-0x30)<<4 | (str[17]-0x30);
    }



    return 1;
}

/*******************************************************************************
* Function Name  :Deci2BcdTime
* Description    :将违章开始结束时间转换为BCD码
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
unsigned char Str2BcdTime(unsigned char* str,unsigned char* bcd,unsigned char len)
{
    u8 i;

    if(str == NULL || bcd == NULL ) return 0;

    for(i = 0; i<len; i++)
    {
        bcd[i] = (str[i*2] - 0x30)<<4 | (str[i*2+1] -0x30);
    }
    return 1;
}


/*******************************************************************************
* Function Name  :Word2Bcd
* Description    :
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
unsigned char Year2Bcd(u16 dsr,unsigned char * src)
{
    if(src == NULL ) return 0;
    src[0] = (dsr/1000%10)<<4 | dsr/100%10;//取千位与百位
    src[1] = (dsr/10%10)<<4 | dsr%10;//取十位与个位
    return 1;
}

/*******************************************************************************
* Function Name  :Byte2Bcd
* Description    :
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
unsigned char Byte2Bcd(u8 dsr)
{
    u8 TempData = 0;
    TempData = (dsr/10%10)<<4 | dsr%10;//取十位与个位
    return TempData;
}

/*******************************************************************************
* Function Name  : fputc
* Description    : 重写printf函数
* Input          : ch    *f
* Output         : None
* Return         : None
******************************************************************************/
int fputc(int ch, FILE *f)
{
#ifdef DEBUG_UART
    USART_SendData(DEBUG_UART, (unsigned char) ch);
    while (!(DEBUG_UART->SR & USART_FLAG_TC));
#endif
    return (ch);
}

/*******************************************************************************
* Function Name  : Delay_ms
* Description    : ms延时函数
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void Delay_ms(volatile u32 nTime)
{
#ifdef __USE_UCOS__
    // 允许调度
    OSTimeDly(  (OS_TICK    )nTime,
                (OS_OPT     )OS_OPT_TIME_DLY,
                (OS_ERR    *)0);

#else
    TimingDelay = nTime;
    while(TimingDelay != 0);
#endif
}

/*******************************************************************************
* Function Name  : Delay_us
* Description    : ms延时函数
******************************************************************************/
void Delay_us(volatile u32 nTime)
{
    u16 i=0;
    while(nTime--)
    {
        i=10;  //自己定义
        while(i--) ;
    }
}

/*******************************************************************************
* Function Name  : Usart_SendByte
* Description    : 发送一个字节
* Input          : pUSARTx：串口号
                   ch：
* Output         : None
* Return         : None
******************************************************************************/
void Usart_SendByte( USART_TypeDef * pUSARTx, u8 data )
{
    USART_SendData(pUSARTx, data);
    while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
}


/*******************************************************************************
* Function Name  : Usart_SendNByte
* Description    : 发送指定长度的字符串
* Input          : pUSARTx：串口号
                   array：数组名
                   len：数组长度
* Output         : None
* Return         : None
******************************************************************************/
void Usart_SendNByte( USART_TypeDef * pUSARTx, u8 *array, u16 len )
{
    unsigned int k = 0;
    do
    {
        Usart_SendByte( pUSARTx, *(array + k) );
        k++;
    } while(k < len);
}


/*******************************************************************************
* Function Name  : Usart_SendString
* Description    : 发送字符串
* Input          : pUSARTx：串口号
                   str：需要发送的字符串
* Output         : None
* Return         : None
******************************************************************************/
void Usart_SendString( USART_TypeDef * pUSARTx, char *str)
{
    unsigned int k = 0;
    do
    {
        Usart_SendByte( pUSARTx, *(str + k) );
        k++;
    } while(*(str + k) != '\0');
}


/*******************************************************************************
* 函数名  : TimeToCharArray
* 描述    : 将一个word转换成char数组
* 输入    :
* 返回值  :
* 说明    :
*******************************************************************************/
void WordToCharArray(u32 Word,u8 *CharArray)
{
//    *(CharArray+0) = Word/1000+0x30;
//    *(CharArray+1) = Word/100%10+0x30;
//    *(CharArray+2) = Word/10%100+0x30;
//    *(CharArray+3) = Word%10+0x30;
    sprintf( (char*)CharArray, "%04d", Word );
}

/*******************************************************************************
* Function Name  : GetLockCode
* Description    :
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
unsigned int GetLockCode(void)
{
    static unsigned int CpuID[3];
    static unsigned int Lock_Code;

    //获取CPU唯一ID
    CpuID[0] = *(volatile unsigned int*)(0x1ffff7e8);
    CpuID[1] = *(volatile unsigned int*)(0x1ffff7ec);
    CpuID[2] = *(volatile unsigned int*)(0x1ffff7f0);
#ifdef __DEBUG
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "MCU = (0x%x, 0x%x, 0x%x )", CpuID[0], CpuID[1], CpuID[2]);
#endif

    //加密算法,很简单的加密算法
    Lock_Code = (CpuID[0] >> 1) + (CpuID[1] >> 2) + (CpuID[2] >> 3);
    return( Lock_Code );
}

///*******************************************************************************
//* Function Name  : CRC16
//* Description    : CRC16计算函数, 采用1021多项式(CRC-ITU)实现
//* Input          : ptr = 待验证数据地址
//                   len = 待验证数据长度
//* Output         : None
//* Return         : None
//******************************************************************************/
//// CRC余式表
//const unsigned char crch[16] = { 0x00,0x10,0x20,0x30,0x40,0x50,0x60,0x70,
//                                 0x81,0x91,0xa1,0xb1,0xc1,0xd1,0xe1,0xf1
//                               };

//const unsigned char crcl[16] = { 0x00,0x21,0x42,0x63,0x84,0xa5,0xc6,0xe7,
//                                 0x08,0x29,0x4a,0x6b,0x8c,0xad,0xce,0xef
//                               };

//typedef union {
//    unsigned char  c[2];
//    unsigned short x;
//} crc_def;
//unsigned short CRC16(unsigned char* src, unsigned short len)
//{
//    crc_def crc;
//    unsigned char  t = 0;
//    unsigned short i;
//
//    // 检查指针
//    if( NULL == src || len <= 0 )
//        return 0;

//    crc.x = 0;

//    for( i=0; i<len; i++ ) {
//        t = (crc.c[0] >> 4) ^ ( (src[i] >> 4) & 0x0f );
//        crc.x <<= 4;
//        crc.c[0] ^= crch[t];
//        crc.c[1] ^= crcl[t];

//        t = (crc.c[0] >> 4) ^ ( src[i] & 0x0f );
//        crc.x <<= 4;
//        crc.c[0] ^= crch[t];
//        crc.c[1] ^= crcl[t];
//    }

//    return crc.x;
//}



/*******************************************************************************
* Function Name  : CheckSum
* Description    :
* Input          : ptr = 待验证数据地址
                   len = 待验证数据长度
* Output         : None
* Return         : None
******************************************************************************/
unsigned char CheckSum(unsigned char* src, unsigned short len)
{
    unsigned short sum = 0,i;
    for(i = 0; i<len; i++) {
        sum+= src[i];
    }
    sum ^= 0xA5;
    sum += 0x5A;
    return ((unsigned char)sum);
}
/*******************************************************************************
* 函数名  : Asc2Byte
* 描述    : 将char数组转换成1Byte
* 输入    : src = 地址指针
*           len = 数据长度
* 返回值  :
* 说明    :
*******************************************************************************/
unsigned char Byte2Asc(unsigned char* src, unsigned char len)
{
    char buf[22] = { 0 };
    unsigned char dt = 0,i = 0;

    if ( src == NULL || len == 0 ) return 0;
    memcpy( buf, src, len );
    for( i = 0; i<len; i++) {
        sprintf((char*)(src+i), "%d", buf[i]);
    }
    return dt;
}

/*******************************************************************************
* 函数名  : Asc2Byte
* 描述    : 将char数组转换成1Byte
* 输入    : src = 地址指针
*           len = 数据长度
* 返回值  :
* 说明    :
*******************************************************************************/
unsigned char Asc2Byte(char* src, unsigned char len)
{
    char buf[22] = { 0 };
    unsigned char dt = 0;

    if ( src == NULL || len == 0 ) return 0;

    memcpy( buf, src, len );
    dt = atoi( buf );
    return dt;
}

/*******************************************************************************
* 函数名  : Asc2Word
* 描述    : 将char数组转换成1Byte
* 输入    : src = 地址指针
*           len = 数据长度
* 返回值  :
* 说明    :
*******************************************************************************/
unsigned short Asc2Word(char* src, unsigned char len)
{
    char buf[10] = { 0 };
    unsigned short dt = 0;

    if ( src == NULL || len == 0 ) return 0;

    memcpy( buf, src, len );
    dt = atoi( buf );
    return dt;
}

/*******************************************************************************
* 函数名  : Asc2Float
* 描述    : 将char数组转换成float
* 输入    : src = 地址指针
*           len = 数据长度
* 返回值  :
* 说明    :
*******************************************************************************/
float Asc2Float(char* src, unsigned char len)
{
    char buf[14] = { 0 };
    float dt = 0;

    if ( src == NULL || len == 0 ) return 0;

    memcpy( buf, src, len );
    dt = atof( buf );
    return dt;
}


/*******************************************************************************
* 函数名  : PhoneAlign
* 描述    : 以'F'补齐电话达到设定个数
* 输入    : src = 待处理的号码地址; width = 对齐宽度
* 返回值  :
* 说明    : 对指定地址用'F'补足号码达到要求个数
*******************************************************************************/
void  PhoneAlign( unsigned char* src, unsigned char width )
{
    int i = 0;
    for( i = 0; i< strlen( (char*)src ); i++ ) {
        if( src[i] == 'F' || src[i] == 'f' ) {
            break;
        }
    }
    for( ; i<__PHONE_LENGTH__; i++ ) {
        src[i] = 'F';
    }
}


