/******************** (C) COPYRIGHT soarsky ************************************
* File Name          : GPRS_GSM.c
* Author             :
* Version            :
* Date               :
* Description        : GSM/GPRS使用M35模块
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"

/* Private define ------------------------------------------------------------*/
#define M35_DTR_LOW			GPIO_WriteBit(M35_DTR_PORT, M35_DTR, (BitAction)0)
#define M35_DTR_HIGH		GPIO_WriteBit(M35_DTR_PORT, M35_DTR, (BitAction)1)
#define M35_PWR_LOW			GPIO_WriteBit(M35_PWR_PORT, M35_PWR, (BitAction)0)
#define M35_PWR_HIGH		GPIO_WriteBit(M35_PWR_PORT, M35_PWR, (BitAction)1)
#define M35_RTS_LOW			GPIO_WriteBit(M35_RTS_PORT, M35_RTS, (BitAction)0)
#define M35_RTS_HIGH		GPIO_WriteBit(M35_RTS_PORT, M35_RTS, (BitAction)1)
#define Read_M35_STATUS	    GPIO_ReadInputDataBit(M35_STATUS_PORT, M35_STATUS)

#define M35_ENABLE          GPIO_ResetBits(M35_EN_PORT, M35_EN)
#define M35_DISABLE         GPIO_SetBits(M35_EN_PORT, M35_EN)


/* Private variables ---------------------------------------------------------*/
static volatile unsigned char Flag_M35RevFrame = RESET;
static volatile unsigned char M35_RxCounter = 0;
static char  M35_RxBuffer[100];

volatile unsigned char GPRS_CmdNow = M35_PwrOn_Start;//当前命令

/* Global variables ---------------------------------------------------------*/
volatile unsigned char  M35_SignalQ = 0;//信号质量
volatile unsigned short M35_TimeCount = 0;//延时计数器
unsigned char m_Phone[ __PHONE_LENGTH__ + 1 ] = {0}; //打电话号码


/*******************************************************************************
* Function Name  : M35_GetStatus
* Description    : 获取M35的状态
******************************************************************************/
FlagStatus M35_GetStatus(void)
{
    if( m_var.Flag_InitDone_M35 == SET ) {
        if( GPRS_CmdNow == SendTcp_Finished      ||//发送TCP数据完成
                GPRS_CmdNow == M35_Init_Finished ||//初始化完成
                GPRS_CmdNow == Call_Finished     ||//打电话完成
                GPRS_CmdNow == CheckSignal_Finished) {//检测信号完成
            return SET;
        }
    }
    return RESET;
}


/*******************************************************************************
* Function Name  : M35_Initialize
* Description    : GSM初始化设置
******************************************************************************/
void M35_Initialize(void)
{
#define __MAX_RETRY_COUNT__ 3
#define __DELAY_LENGTH__    800

    char* s1;
    char* s2;
    unsigned char sign_vol = 0;

    char SetIPTelStr[80]= {0};
    static volatile unsigned char GPRS_CmdFlag = 0;//用于标志当前命令是否执行
    static volatile unsigned char M35_ReTry_Count = 0;  // 重试次数计数
    static volatile unsigned char GPRS_ReSend_Count = 0;  //

    switch( GPRS_CmdNow ) {
        /////////////////////////////////////////////////////////////////////
        // [[ M35模块进行开机流程
        {
        case M35_PwrOn_Start:
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "GSM开始开机");
#endif
            GPRS_CmdFlag = 0;
            GPRS_CmdNow = M35_PwrOn_Step1;
            break;

        case M35_PwrOn_Step1:
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                GPRS_CmdFlag = GPRS_CmdNow;
                M35_ENABLE;
                M35_DTR_LOW;
                M35_PWR_LOW;
                M35_TimeCount = 2000;
            }
            else if(M35_TimeCount == 0) {
                M35_PWR_HIGH;
                GPRS_CmdFlag = 0;
                GPRS_CmdNow = M35_PwrOn_Step2;
            }
            break;

        case M35_PwrOn_Step2:
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                GPRS_CmdFlag = GPRS_CmdNow;
                M35_TimeCount = 15000;
            }
            else if(M35_TimeCount == 0) {
                GPRS_CmdFlag = 0;
                GPRS_CmdNow = M35_Init_Start;
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "GSM开机完成");
#endif
            }
            break;
        }
        // M35模块进行开机流程 ]]
        /////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////
        // [[ M35模块初始化序列状态
        {
        case M35_Init_Start:
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "GSM初始化开始...");
#endif
            GPRS_CmdFlag = 0;
            M35_ReTry_Count = 0;
            GPRS_CmdNow = M35_Init_Step1;
            break;

        case M35_Init_Step1:  // 同步波特率
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                GPRS_CmdFlag = GPRS_CmdNow;
                M35_SendCmd("AT\r");
                M35_TimeCount = __DELAY_LENGTH__;
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "AT");
#endif
            }
            else if(M35_CheckCmd("OK")) {       // 命令执行成功
                GPRS_CmdFlag = 0;
                M35_ReTry_Count = 0;
#ifdef __HARDWARE_TEST__
                GPRS_CmdNow = M35_Init_Finished;
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "\r\nM35 OK");
#endif
#else
                GPRS_CmdNow = M35_Init_Step2;   // 跳转到写一步执行
#endif
            }
            else if(M35_TimeCount == 0) {       // 等待GSM模块回应超时
                GPRS_CmdFlag = 0;

                M35_ReTry_Count++;
                if( M35_ReTry_Count >= __MAX_RETRY_COUNT__ * 5 ) {
                    GPRS_CmdNow = M35_Init_Finished;
#ifdef __DEBUG
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf(SET, "\r\nM35 ERROR");
#endif
                }
            }
            break;

        case (M35_Init_Step2):  // 关闭回显
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                GPRS_CmdFlag = GPRS_CmdNow;
                M35_SendCmd("ATE0\r");//关闭回显
                M35_TimeCount = __DELAY_LENGTH__;
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "ATE0:关闭回显");
#endif
            }
            else if(M35_CheckCmd("OK")) { //命令执行成功
                GPRS_CmdFlag = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = M35_Init_Step3;
            }
            else if(M35_TimeCount == 0) { //等待GSM模块回应超时
                GPRS_CmdFlag = 0;
                // 检查重试次数
                M35_ReTry_Count++;
                if( M35_ReTry_Count >= __MAX_RETRY_COUNT__ ) {
                    GPRS_CmdNow = M35_Init_Finished;
                }
            }
            break;

        case (M35_Init_Step3):  // 查询PIN码
            if( GPRS_CmdNow != GPRS_CmdFlag ) {
                GPRS_CmdFlag = GPRS_CmdNow;
                M35_SendCmd("AT+CPIN?\r");
                M35_TimeCount = __DELAY_LENGTH__;
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "AT+CPIN?");
#endif
            }
            else if(M35_CheckCmd("READY")) { //等待GSM模块回应超时
                GPRS_CmdFlag = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = M35_Init_Step4;
            }
            else if( M35_CheckCmd("ERROR") || M35_TimeCount == 0 ) {
                GPRS_CmdFlag  = 0;
                M35_ReTry_Count++;
                if( M35_ReTry_Count >= __MAX_RETRY_COUNT__ ) {
#ifdef __DEBUG
                    //XFS_WriteBuffer(xfs_auto, "未插卡");
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf(SET, "未插卡");
#endif
                    GPRS_CmdNow = M35_PwrOff_Start;
                }
            }
            break;

        case (M35_Init_Step4):  // 查询信号强度
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                GPRS_CmdFlag = GPRS_CmdNow;
                M35_SendCmd("AT+CSQ\r");
                M35_TimeCount = __DELAY_LENGTH__;
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "AT+CSQ");
#endif
            }
            else if(M35_CheckCmd("OK") || M35_TimeCount == 0) {
                GPRS_CmdFlag  = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = M35_Init_Step5;
            }
            break;

        case (M35_Init_Step5):  // 设置SMS格式
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                GPRS_CmdFlag = GPRS_CmdNow;
                M35_SendCmd("AT+CMGF=1\r");//设置为PDU模式,只有在该模式下才能发送中文;0:PDU  ,1 :文本
                M35_TimeCount = __DELAY_LENGTH__;
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "AT+CMGF=1");
#endif
            }
            else if(M35_CheckCmd("OK")) {
                GPRS_CmdFlag  = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = M35_Init_Step6;
            }
            else if(M35_TimeCount == 0) {
                GPRS_CmdFlag = 0;
                M35_ReTry_Count++;
                if( M35_ReTry_Count >= __MAX_RETRY_COUNT__ ) {
                    GPRS_CmdNow = M35_Init_Finished;
                }
            }
            break;

        case (M35_Init_Step6):  // 设置字符集格式
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                GPRS_CmdFlag = GPRS_CmdNow;
                M35_SendCmd("AT+CSCS=\"GSM\"\r");
                M35_TimeCount = __DELAY_LENGTH__;
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "AT+CSCS=\"GSM\"");
#endif
            }
            else if(M35_CheckCmd("OK")) {
                GPRS_CmdFlag  = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = M35_Init_Step7;
            }
            else if(M35_TimeCount == 0) {
                GPRS_CmdFlag = 0;
                M35_ReTry_Count++;
                if( M35_ReTry_Count >= __MAX_RETRY_COUNT__ ) {
                    GPRS_CmdNow = M35_Init_Finished;
                }
            }
            break;

        case (M35_Init_Step7):  // 设置GPRS模式
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                GPRS_CmdFlag = GPRS_CmdNow;
                M35_SendCmd("AT+QIMODE=0\r");//1:透传模式    0:非透传模式（默认）
                M35_TimeCount = __DELAY_LENGTH__;
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "AT+QIMODE=0");
#endif
            }
            else if(M35_CheckCmd("OK")) {
                GPRS_CmdFlag  = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = M35_Init_Step8;
            }
            else if(M35_TimeCount == 0) {
                GPRS_CmdFlag = 0;
                M35_ReTry_Count++;
                if( M35_ReTry_Count >= __MAX_RETRY_COUNT__ ) {
                    GPRS_CmdNow = M35_Init_Finished;
                }
            }
            break;

        case (M35_Init_Step8):  // 是否启用多路连接
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                GPRS_CmdFlag = GPRS_CmdNow;
                M35_SendCmd("AT+QIMUX=0\r");
                M35_TimeCount = __DELAY_LENGTH__;
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "AT+QIMUX=0");
#endif
            }
            else if(M35_CheckCmd("OK")) {
                GPRS_CmdFlag  = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = M35_Init_Step9;
            }
            else if(M35_TimeCount == 0) {
                GPRS_CmdFlag = 0;
                M35_ReTry_Count++;
                if( M35_ReTry_Count >= __MAX_RETRY_COUNT__ ) {
                    GPRS_CmdNow = M35_Init_Finished;
                }
            }
            break;

        case (M35_Init_Step9):  // 配置连接方式: 0=IP/1=域名
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                GPRS_CmdFlag = GPRS_CmdNow;
                M35_TimeCount = __DELAY_LENGTH__;
                if( m_carinfo.NetAddrType == 0 )
                    M35_SendCmd("AT+QIDNSIP=0\r");// 使用IP地址连接
                else
                    M35_SendCmd("AT+QIDNSIP=1\r");// 使用域名方式连接
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET) {
                    if( m_carinfo.NetAddrType == 0 )
                        Debug_Printf(SET, "AT+QIDNSIP=0");
                    else
                        Debug_Printf(SET, "AT+QIDNSIP=1");
                }
#endif
            }
            else if(M35_CheckCmd("OK")) {
                GPRS_CmdFlag  = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = M35_Init_Step10;
            }
            else if(M35_TimeCount == 0) {
                GPRS_CmdFlag = 0;
                M35_ReTry_Count++;
                if( M35_ReTry_Count >= __MAX_RETRY_COUNT__ ) {
                    GPRS_CmdNow = M35_Init_Finished;
                }
            }
            break;

        case (M35_Init_Step10):
            if(GPRS_CmdNow != GPRS_CmdFlag) {

                GPRS_CmdFlag = GPRS_CmdNow;
                M35_SendCmd("AT+QIHEAD=1\r"); // 显示IP头
                M35_TimeCount = __DELAY_LENGTH__;
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET) {
                    Debug_Printf(SET, "AT+QIHEAD=1");
#endif
                }
            }
            else if(M35_CheckCmd("OK")) {
                GPRS_CmdFlag  = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = M35_Init_Step11;
            }
            else if(M35_TimeCount == 0) {

                GPRS_CmdFlag = 0;
                M35_ReTry_Count++;
                if( M35_ReTry_Count >= __MAX_RETRY_COUNT__ ) {
                    GPRS_CmdNow = M35_Init_Finished;
                }
            }
            break;

        case (M35_Init_Step11): // 设置为GPRS连接
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                GPRS_CmdFlag = GPRS_CmdNow;
                M35_SendCmd("AT+QICSGP=1,\"CMWAP\"\r");
                M35_TimeCount = __DELAY_LENGTH__;
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET) {
                    Debug_Printf(SET, "AT+QICSGP=1,\"CMWAP\"");
#endif
                }
            }
            else if(M35_CheckCmd("OK")) {
                GPRS_CmdFlag  = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = M35_Init_Finished;
                m_var.Flag_InitDone_M35 = SET;
#ifdef __DEBUG
                XFS_WriteBuffer(xfs_auto, "GSM初始化完成");
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "GSM初始化完成");
#endif
            }
            else if(M35_TimeCount == 0) {
                GPRS_CmdFlag = 0;
                M35_ReTry_Count++;
                if( M35_ReTry_Count >= __MAX_RETRY_COUNT__ ) {
                    GPRS_CmdNow = M35_Init_Finished;
                }
            }
            break;
        }
        // M35模块初始化序列状态 ]]
        /////////////////////////////////////////////////////////////////////


        /////////////////////////////////////////////////////////////////////
        // [[ GPRS发送数据状态
        {
        case SendTcp_Start:
            if( m_var.Flag_InitDone_M35 == RESET ) {
                GPRS_CmdNow = SendTcp_Finished;
                break;
            }

            GPRS_ReSend_Count = 0;

            // 检查待拨号码
            memset( SetIPTelStr, 0, sizeof(SetIPTelStr) );
            s2 = SetIPTelStr;
            s1 = (char*)m_carinfo.Family_Phone[0];
            while(1) {
                if( *s1 == 'F' || *s1 == 0xd || *s1 == 0xa ) {
                    break;
                }
                *s2++ = *s1++;
            }
            if( s2 - SetIPTelStr == 0  ) {
#ifdef __DEBUG
                XFS_WriteBuffer( xfs_auto, "空号码" );
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf( SET, "空号码,不用发送" );
#endif
                GPRS_CmdNow = SendTcp_Finished;
            }
            else {
#ifdef __DEBUG
                //XFS_WriteBuffer(xfs_auto, "数据发送开始");
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "GPRS发送开始...");
#endif
                GPRS_CmdFlag = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = SendTcp_Step1;
            }
            break;

        case SendTcp_Step1:
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                if( m_carinfo.NetAddrType == 0 ) {
                    // ip地址连接方式
                    sprintf((char *)SetIPTelStr,
                            "AT+QIOPEN=\"TCP\",\"%d.%d.%d.%d\",%ld\r",
                            m_carinfo.ServerName[0],
                            m_carinfo.ServerName[1],
                            m_carinfo.ServerName[2],
                            m_carinfo.ServerName[3],
                            (long)m_carinfo.NetPort);
                }
                else {
                    // 域名连接方式
                    sprintf((char *)SetIPTelStr,
                            "AT+QIOPEN=\"TCP\",\"%s\",%ld\r",
                            m_carinfo.ServerName,
                            (long)m_carinfo.NetPort);
                }
                GPRS_CmdFlag = GPRS_CmdNow;
                M35_SendCmd( SetIPTelStr );//设置IP地址,建立TCP连接
                M35_TimeCount = 5000;
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "发送服务器连接请求");
#endif
            }
            // else if( M35_CheckCmd("OK") || M35_CheckCmd("ALREADY CONNECT")) { //命令执行成功
            else if( M35_CheckCmd("CONNECT OK") || M35_CheckCmd("ALREADY CONNECT") ) {
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "服务器连接成功");
#endif
                GPRS_CmdFlag  = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = SendTcp_Step5;
                M35_TimeCount = __DELAY_LENGTH__;
            }
            else if( M35_CheckCmd("CONNECT FAIL")) {
                GPRS_CmdFlag = 0;

                M35_ReTry_Count++;
                if( M35_ReTry_Count >= __MAX_RETRY_COUNT__ ) {
#ifdef __DEBUG
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf(SET, "服务器连接失败");
                    //XFS_WriteBuffer(xfs_auto, "连接失败");
#endif
                    GPRS_CmdNow = SendTcp_Finished;
                }
            }
            else if( M35_TimeCount == 0 ) {
                GPRS_CmdFlag = 0;

                M35_ReTry_Count++;
                if( M35_ReTry_Count >= __MAX_RETRY_COUNT__ ) {
                    GPRS_CmdNow = SendTcp_Finished;
                }
            }
            break;

        case SendTcp_Step5:
            // 延时
            if(M35_TimeCount == 0) {
                GPRS_CmdFlag  = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = SendTcp_Step6;
            }
            break;

        case SendTcp_Step6:
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                GPRS_CmdFlag = GPRS_CmdNow;
                M35_SendCmd("AT+QISEND\r");
                M35_TimeCount = 1000;
            }
            else if(M35_CheckCmd(">")) {
                GPRS_CmdFlag  = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = SendTcp_Step7;
            }
            else if(M35_CheckCmd("+CME ERROR:")) { //if(M35_CheckCmd("+CME ERROR: 100")) {
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "+CME ERROR:");
#endif
            }
            else if(M35_TimeCount == 0) {
                GPRS_CmdFlag = 0;
                M35_ReTry_Count++;
                if( M35_ReTry_Count >= __MAX_RETRY_COUNT__ ) {
                    GPRS_CmdNow = SendTcp_Finished;
                }
            }
            break;

        case SendTcp_Step7:
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                GPRS_CmdFlag = GPRS_CmdNow;
                memset(M35_RxBuffer, 0, sizeof(M35_RxBuffer));//清空接受数组
                Usart_SendNByte(M35_UART, m_gprs.lpmsg, m_gprs.count);//写入数据
                M35_UART->DR = 0x1A;//发送
                M35_TimeCount = 5000;
            }
            else if(M35_CheckCmd("SEND OK")) { //命令执行成功
                GPRS_CmdFlag  = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = SendTcp_Step8;
                M35_TimeCount = 1000;
#ifdef __DEBUG
                //XFS_WriteBuffer(xfs_auto, "数据发送成功");
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "发送数据成功");
#endif
            }
            else if( M35_CheckCmd("ERROR") || M35_TimeCount == 0 ) {
                GPRS_CmdFlag = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = SendTcp_Step1;
                GPRS_ReSend_Count++;
                if( GPRS_ReSend_Count >= __MAX_RETRY_COUNT__ ) {
                    GPRS_CmdNow = SendTcp_Finished;
                }
            }
            break;

        case SendTcp_Step8:
            if(M35_CheckCmd("IPD")) {

            }
            else if(M35_TimeCount == 0) {
                GPRS_CmdFlag = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = SendTcp_Step9;
            }
            break;

        case SendTcp_Step9:
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                GPRS_CmdFlag = GPRS_CmdNow;
                memset(M35_RxBuffer,0,sizeof(M35_RxBuffer));//清空接收数组
                Usart_SendString(M35_UART,"AT+QICLOSE\r");
                M35_TimeCount = 3000;
            }
            else if(M35_CheckCmd("CLOSE OK")) { //命令执行成功
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "断开与服务器的连接");
#endif
                GPRS_CmdFlag = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = SendTcp_Finished;
            }
            else if(M35_TimeCount == 0) { //等待GSM模块回应超时
                GPRS_CmdFlag = 0;
                // 检查重试次数
                M35_ReTry_Count++;
                if( M35_ReTry_Count >= __MAX_RETRY_COUNT__ ) {
                    GPRS_CmdNow = SendTcp_Finished;
                }
            }
            break;
        }
        // GPRS发送数据状态 ]]
        /////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////
        // [[ 拨打电话状态
        {
        case Call_Start:
            if( m_var.Flag_InitDone_M35 == RESET ) {
                GPRS_CmdNow = Call_Finished;
                break;
            }
            // 检查待拨号码
            memset( SetIPTelStr, 0, sizeof(SetIPTelStr) );
            s2 = SetIPTelStr;
            s1 = (char*)m_carinfo.Family_Phone[0];
            while(1) {
                if( *s1 == 'F' || *s1 == 'f' || *s1 == 0xd || *s1 == 0xa ) {
                    break;
                }
                *s2++ = *s1++;
            }
            if( s2 - SetIPTelStr == 0 ) {
#ifdef __DEBUG
                XFS_WriteBuffer( xfs_auto, "空号码" );
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf( SET, "空号码,不用呼叫" );
#endif
                GPRS_CmdNow = Call_Finished;
            }
            else {
                GPRS_CmdFlag = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = Call_Step2;
            }
            break;

        case Call_Step2:
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                GPRS_CmdFlag = GPRS_CmdNow;
                M35_TimeCount = 5000;   // 延时5sec后再开始拨号
            }
            else if(M35_TimeCount == 0) { //回应超时
                GPRS_CmdFlag = 0;
                GPRS_CmdNow = Call_Step3;
            }
            break;

        case Call_Step3:
            if(GPRS_CmdNow != GPRS_CmdFlag) {
                GPRS_CmdFlag = GPRS_CmdNow;

                sprintf((char *)SetIPTelStr, "ATD%s;\r", m_carinfo.Family_Phone[0]);
                s1 = SetIPTelStr;
                while(1) {
                    if( *s1 == 'F' || *s1 == 0xd || *s1 == 0xa ) {
                        *s1++ = ';';
                        *s1++ = 0xd;
                        *s1++ = 0;
                        break;
                    }
                    s1++;
                }

                M35_SendCmd(SetIPTelStr);//
                M35_TimeCount = 30000;
                GPRS_CmdNow = Call_Step4;
#ifdef __DEBUG
                *(s1 - 3) = 0;
                s1 = SetIPTelStr + 3;
                XFS_WriteBuffer(xfs_auto, "开始拨%s", s1);
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "开始拨%s", s1);
#endif
            }
            break;

        case Call_Step4:
            if(M35_CheckCmd("BUSY") || M35_CheckCmd("NO ANSWER") ) {
                // 线路忙或无应答, 5sec后重来
                GPRS_CmdFlag = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = Call_Step2;
            }
            else if( M35_CheckCmd("OK") || M35_TimeCount == 0 ) {
                GPRS_CmdFlag = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = Call_Step5;
            }
            break;

        case Call_Step5:
            if(M35_TimeCount == 0) {
                M35_SendCmd("ATH\r");
                GPRS_CmdNow = Call_Finished;
#ifdef __DEBUG
                XFS_WriteBuffer(xfs_auto, "拨号完成");
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "拨号流程完成");
#endif
            }
            break;
        }
        // 拨打电话状态 ]]
        /////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////
        // [[检测信号强度
        {
        case CheckSignal1:
            if(GPRS_CmdNow != GPRS_CmdFlag) { //检测是否第一次调用该函数
                M35_SendCmd("AT+CSQ\r");
                GPRS_CmdNow = CheckSignal2;
                M35_TimeCount = __DELAY_LENGTH__;
            }
            break;

        case CheckSignal2:
            if( M35_CheckCmd("+CSQ: ")) {
                sign_vol = M35_CheckSignal( M35_RxBuffer );//解析信号质量
                if( M35_SignalQ != sign_vol && sign_vol > 0 ) {
                    M35_SignalQ = sign_vol;
#ifdef __DEBUG
                    if( M35_SignalQ > 0 && M35_SignalQ < 32 ) {
                        if( ( m_carinfo.Flag_GSMSign % 0x1 ) == SET ) {
                            XFS_WriteBuffer(xfs_value, "信号%d", M35_SignalQ );
                        }
                        if( m_var.Flag_Debug_Enable == SET )
                            Debug_Printf(SET, "GSM信号=%d", M35_SignalQ );
                    }
                }
#endif
                GPRS_CmdNow = CheckSignal_Finished;
            }
            else if(M35_TimeCount == 0) {
                GPRS_CmdFlag = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = CheckSignal_Finished;
            }
            break;

        case CheckSignal_Finished:
            GPRS_CmdFlag = 0;
            M35_ReTry_Count = 0;
            break;
        }
        // 检测信号强度 ]]
        /////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////
        // [[ 关机状态
        {
        case M35_PwrOff_Start:
            M35_PWR_LOW;
            if(GPRS_CmdNow != GPRS_CmdFlag) { //检测是否第一次调用该函数
#ifdef __DEBUG
                //XFS_WriteBuffer(xfs_auto,"GSM开始关机" );
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "GSM开始关机");
#endif

                GPRS_CmdFlag = GPRS_CmdNow;//第一次调用该函数后会置位
                M35_TimeCount = 1000;
            }
            else if(M35_TimeCount == 0) {
                GPRS_CmdFlag = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = M35_PwrOff_End;
            }
            break;

        case M35_PwrOff_End:
            if(GPRS_CmdNow != GPRS_CmdFlag) { //检测是否第一次调用该函数
                GPRS_CmdFlag = GPRS_CmdNow;//第一次调用该函数后会置位
                M35_TimeCount = 5000;//延时12S
            }
            else if(M35_TimeCount == 0) {
                GPRS_CmdFlag = 0;
                M35_ReTry_Count = 0;
                GPRS_CmdNow = M35_PwrOff_Finished;
            }
            break;

        case M35_PwrOff_Finished:
            if(GPRS_CmdNow != GPRS_CmdFlag) { //检测是否第一次调用该函数
#ifdef __DEBUG
                //XFS_WriteBuffer(xfs_auto,"GSM完成关机" );
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "GSM完成关机");
#endif

                GPRS_CmdFlag = GPRS_CmdNow;//第一次调用该函数后会置位
            }
            break;
        }
    // 关机状态 ]]
    /////////////////////////////////////////////////////////////////////


    /////////////////////////////////////////////////////////////////////
    // [[ 操作结束状态
    case Call_Finished:
    case SendTcp_Finished:
    case M35_Init_Finished:
        break;
    // 操作结束状态 ]]
    /////////////////////////////////////////////////////////////////////

    default:
        GPRS_CmdNow = M35_PwrOff_Start;
        break;
    }
}

/*******************************************************************************
* Function Name  : M35_CheckCmd
* Description    : 发送命令后,检测接收到的应答
******************************************************************************/
unsigned char* M35_CheckCmd(char *str)
{
    char *strx = 0;
    if(str == NULL ) {
        return (uint8_t *)strx;
    }
    /* 接收到一次数据了 */
    if(Flag_M35RevFrame == SET) {
        strx = strstr((const char *)M35_RxBuffer, (const char *)str);
        if(strx != 0) {
            Flag_M35RevFrame = RESET;
        }
    }
    return (uint8_t *)strx;
}

/*******************************************************************************
* Function Name  : M35_SendCmd
* Description    : 向模块发送命令
******************************************************************************/
void M35_SendCmd(char *cmd)
{
    memset(M35_RxBuffer,0,sizeof(M35_RxBuffer));//清空接受数组
    Flag_M35RevFrame = RESET;
    Usart_SendString(M35_UART,cmd);//发送命令
}

/*******************************************************************************
* Description    : 检测信号质量
* Input          :
******************************************************************************/
unsigned char M35_CheckSignal(char *src)
{
    char* s1 = NULL;
    char* s2 = NULL;
    unsigned char val = 0;

    char buf[5] = {0};

    if( (u8*)strstr((const char *)src, "+CSQ: ") != NULL ) {
        s1 = strstr((const char *)src, "+CSQ: ");
        s1 += strlen("+CSQ: ");
        s2 = buf;
        while(1) {
            if( *s1 == ',' || (s2 - buf) > 3 ) {
                break;
            }
            *s2++ = *s1++;
        }
        val = atoi(buf);
    }
    return val;
}


/*******************************************************************************
* Function Name  : UART4_IRQHandler
* Description    : 串口4终端服务函数
******************************************************************************/
void M35_IRQHandler(void)
{
    unsigned char dt;
    unsigned char Clear = Clear;

    if(USART_GetITStatus(M35_UART, USART_IT_RXNE) != RESET)	    //接收中断
    {
        dt = USART_ReceiveData(M35_UART);
        USART_ClearITPendingBit(M35_UART, USART_IT_RXNE);       //清除挂起标志位

        M35_RxBuffer[M35_RxCounter++] = dt;
        if(M35_RxCounter >= sizeof(M35_RxBuffer)) {
            Flag_M35RevFrame = SET;
            M35_RxCounter = 0;
        }

        if(m_var.Flag_Init_M35 == 0 ) {
            Debug_Data( dt );
        }
    }
    else if(USART_GetITStatus(M35_UART, USART_IT_IDLE) != RESET)
    {
        Clear = M35_UART->SR;
        Clear = M35_UART->DR;
        M35_RxCounter = 0;
        Flag_M35RevFrame = SET;
    }
}

/*---------------------------------End of File----------------------------------------------*/
