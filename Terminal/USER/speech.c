/******************** (C) COPYRIGHT soarsky ************************************
* File Name          :
* Author             :
* Version            :
* Date               :
* Description        : 语音模块使用XFS5152芯片
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"


/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
vu8  XFS_UART_RxBuffer = 0;//用于存放串口2接受到的数据

/* Global variables ---------------------------------------------------------*/
vu16 XFS_TimeCount = 0;//用于语音模块定时，在中断中自减

/*芯片设置命令---------------------------------------------------------------*/
//static u8 XFS_Stop[]   ={0xFD,0X00,0X01,0X02};//停止合成
//static u8 XFS_Suspend[]={0XFD,0X00,0X01,0X03};//暂停合成
//static u8 XFS_Recover[]={0XFD,0X00,0X01,0X04};//恢复合成
//static u8 XFS_StatusCmd[4] = {0XFD,0X00,0X01,0X21}; //状态查询,
//static u8 XFS_PwrDown[4]= {0XFD,0X00,0X01,0X88}; //进入省电模式命令
//static u8 XFS_PwrUp[4]  = {0XFD,0X00,0X01,0XFF}; //退出省电模式命令
//static u8 XFS_Volume[9] = {0xFD,0x00,0x06,0x01,0x00,'[','v','9',']'}; //设置音量


u8  XFS_Buffer[MAXNUM_TTS][TTS_LEN] = {0};
vu8 iwTTS;//写入第i条语音
vu8 irTTS;//读出第i条语音


/*******************************************************************************
* Function Name  : XFS_WriteBuffer
* Description    : 将数据写入缓存
* Input          : fmt: 需要播报的语音;
*                  nType: 0=数字自动处理; 1=数字做号码处理; 2=数字做数值处理;
*******************************************************************************/
void XFS_WriteBuffer(unsigned char nType, char *fmt,...)
{
//    unsigned char *pb;
    unsigned char strLen = strlen(fmt); //获取字符串长度

    int   d;
    float f1;
    char  s1[ 16 ] = { 0 };
    char  s2[ 64 ] = { 0 };
    const char *s;
    char* lpsz1;
    va_list ap;

    memset( XFS_Buffer[iwTTS], 0, TTS_LEN ); // 数据清0
    lpsz1 = (char*)XFS_Buffer[iwTTS];

    *lpsz1++ = strLen + 9;  // 以FD开头的长度
    *lpsz1++ = 0xfd;
    *lpsz1++ = 0; 	        // 数据区长度的高字节
    *lpsz1++ = strLen + 6;  // 数据区长度的低字节
    *lpsz1++ = 1;           // 命令字：合成播放命令
    *lpsz1++ = 0;           // 文本编码格式,0:GB2312,1:GBK,2:BIG5,3:UNICODE
    *lpsz1++ = '[';
    *lpsz1++ = 'n';
    *lpsz1++ = nType;
    *lpsz1++ = ']';

    va_start(ap, fmt);

    while( *fmt != 0 ) {    // 判断是否到达字符串结束符
        if( *fmt == '\\' ) {
            switch(*++fmt) {
            case 'r':       // 回车符
                *lpsz1++ = 0xd;
                fmt++;
                break;

            case 'n':       // 换行符
                *lpsz1++ = 0xa;
                fmt++;
                break;

            default:
                fmt++;
                break;
            }
        }
        else if( *fmt == '%' ) {
            switch (*++fmt) {
            case 'c':
                d = va_arg(ap, int);
                sprintf(s2, "%c", d);
                strcpy(lpsz1, s2);
                lpsz1 += strlen(s2);
                fmt++;
                break;

            case 's':   //字符串数
                s = va_arg(ap, const char *);
                strcpy(lpsz1, s);
                lpsz1 += strlen(s);
                fmt++;
                break;

            case 'd':   //十进制数
                d = va_arg(ap, int);
                sprintf(s2, "%d", d);
                strcpy(lpsz1, s2);
                lpsz1 += strlen(s2);
                fmt++;
                break;

            case 'x':   //十六进制数
                d = va_arg(ap, int);
                sprintf(s2, "%x", d);
                strcpy(lpsz1, s2);
                lpsz1 += strlen(s2);
                fmt++;
                break;

            case '.':   // %.7f 浮点数
                f1 = va_arg(ap, double);
                s1[0] = '%';
                s1[1] = *fmt++;
                s1[2] = *fmt++;
                s1[3] = *fmt;
                s1[4] = 0;
                sprintf(s2, s1, f1);
                strcpy(lpsz1, s2);
                lpsz1 += strlen(s2);
                fmt++;
                break;

            case '0':   // %04d/%04x
                s1[0] = '%';
                s1[1] = *fmt++;
                s1[2] = *fmt++;
                s1[3] = *fmt;
                s1[4] = 0;
                d = va_arg(ap, int);
                sprintf(s2, s1, d);
                strcpy(lpsz1, s2);
                lpsz1 += strlen(s2);
                fmt++;
                break;

            default:
                fmt++;
                break;
            }
        }
        else {
            *lpsz1++ = *fmt++;
        }
    }

//    memcpy(pb, fmt, strLen);
//    pb += strLen;
    XFS_Buffer[iwTTS][0] = lpsz1 - (char*)XFS_Buffer[iwTTS] - 1;
    XFS_Buffer[iwTTS][3] = XFS_Buffer[iwTTS][0] - 3;

    iwTTS ++;
    if(iwTTS >= MAXNUM_TTS)
        iwTTS = 0;
}


/*******************************************************************************
* Function Name  : XFS_Initialize
* Description    : 语音芯片的初始化设置
*******************************************************************************/
void XFS_Initialize(void)
{
    OFF_AMP();

    // RESET引脚置低电平,时间持续40ms以上
    rstXFS5152();
    Delay_ms(100);
    setXFS5152();

    XFS_TimeCount = 300;
    while(1) {
        if(XFS_TimeCount == 0) {
            break;
        }
        if(XFS_UART_RxBuffer == 0x4A) {
            //XFS_Status = Idle;
            break;
        }
        IWDG_ReloadCounter();
    }
    XFS_SetVolume( m_carinfo.Volume_Value );//设置音量
}



/*******************************************************************************
* Function Name  : XFS_Process
* Description    : 检测语音芯片状态
*******************************************************************************/
void XFS_Process(void)
{
#ifdef XFS_UART
    static vu8  XFS_SendCmdStep = 0;//用于标志当前命令是否执行
    static u8   TempLen;
        
    switch(XFS_SendCmdStep)
    {
    case 0:
        if(iwTTS != irTTS) {
            XFS_TimeCount = 300;
            XFS_SendCmdStep = 1;//有语音播放内容
            ON_AMP();
//            if(READ_AMP() == RESET){
//                XFS_SendCmd(XFS_PwrUp,4);//退出省电模式
//                XFS_TimeCount = 200;
//            }
        }
        else {
//            if(READ_AMP() == SET){
//                XFS_SendCmd(XFS_PwrDown,4);//进入省电模式
//            }
            OFF_AMP();
        }
        break;
    case 1:
        if( XFS_TimeCount == 0 )
            XFS_SendCmdStep = 2;
        break;
    case 2:
        TempLen = XFS_Buffer[irTTS][0];//第0位为以FD开头的数据长度
        if(TempLen > 5) {
            Usart_SendNByte(XFS_UART, &XFS_Buffer[irTTS][1], TempLen);//发送帧配置
        }
        XFS_Buffer[irTTS][0] = 0;//清掉
        XFS_TimeCount = 300 * TempLen;
        XFS_UART_RxBuffer = 0;
        XFS_SendCmdStep = 3;
        break;
    case 3:
        if(XFS_UART_RxBuffer == 0x4F || XFS_TimeCount == 0) {
            XFS_SendCmdStep = 4;
        }
        break;
    case 4:
        XFS_SendCmdStep = 0;
        irTTS ++;
        if(irTTS >= MAXNUM_TTS)
            irTTS = 0;
        break;
    default :
        XFS_SendCmdStep = 0;
        break;
    }
#endif
}


/*******************************************************************************
* Function Name  : XFS_SetVolume
* Description    : 设置XFS的音量
*******************************************************************************/
void XFS_SetVolume(char Volume)
{
    unsigned char XFS_Volume[10] = { 0xfd, 0, 7, 1, 0, '[', 'v', '1', '0', ']' };

    if(Volume <= 0 || Volume > 10) {
        Volume = 5;
    }
    if(Volume < 10) {
        XFS_Volume[2] = 6;
        XFS_Volume[7] = '0' + Volume;
        XFS_Volume[8] = ']';
        XFS_Buffer[iwTTS][0] = 9;
        memcpy(&XFS_Buffer[iwTTS][1],XFS_Volume,9);
    }
    else {
        XFS_Buffer[iwTTS][0] = 10;
        memcpy(&XFS_Buffer[iwTTS][1],XFS_Volume,10);
    }
    iwTTS ++;
    if(iwTTS >= MAXNUM_TTS)
        iwTTS = 0;
}

#ifdef XFS_UART
/*******************************************************************************
* Function Name  : XFS_UART_IRQHandler
* Description    : 串口2中断服务函数
******************************************************************************/
void XFS_IRQHandler(void)
{
    u8 Clear = Clear;

    if(USART_GetITStatus(XFS_UART, USART_IT_RXNE) != RESET)	  //接收中断
    {
        USART_ClearITPendingBit(XFS_UART, USART_IT_RXNE);        //清除挂起标志位
        XFS_UART_RxBuffer = USART_ReceiveData(XFS_UART);
    }
    else if(USART_GetITStatus(XFS_UART,USART_IT_IDLE) != RESET)
    {
        Clear = XFS_UART->SR;
        Clear = XFS_UART->DR;
    }
}
#endif // #ifdef XFS_UART

