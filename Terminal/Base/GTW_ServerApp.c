/*******************************************************************************
* File Name          : gtw_serverapp.c
* Author             : csyxj@728920175@qq.com
* Version            : V0.1
* Date               : 2017-03-19
* Description        : Background server support function
*******************************************************************************/
#include "gtw_Head.h"

unsigned short GPRS_LocationTime = 0;           // 定时值(ms): 上报定位信息 => 后台服务器

Illegal_Process_Def GTW_BurglarAlarm;           // 防盗报警
Illegal_Process_Def GTW_IllegalPark;            // 违停
Illegal_Process_Def GTW_MoveCar;                // 移车

Illegal_Def Ill_ETicket;

unsigned char  Flag_IllegalParkCancel = RESET;  // 申请撤销违停标志

unsigned char M35_TxBuf[256] = { 0 };

M35_SendDef m_gprs;


/*******************************************************************************
* Function Name  : GTW_ServerAPP
* Description    : 解析服务器消息
******************************************************************************/
unsigned short GTW_ServerAPP(unsigned char* src, unsigned char* dsr)
{
    return(0);
}


/*******************************************************************************
* Function Name  : M35_Process
* Description    : 处理M35模块的打电话,发短信,发送GPRS数据业务
*******************************************************************************/
extern unsigned char Flag_Setup_SMS;    // 安装短信消息
void M35_Process(void)
{
    unsigned char buf[32] = { 0 };
    static unsigned short Alarm_Count = 0;

    // 开机后进行初始化
    M35_Initialize();

    // 区分任务
    if( M35_GetStatus() == SET ) {    // 当前SIM卡空闲

        if( Flag_Setup_SMS == SET ) { // 发送安装信息 => 后台服务器
            GPRS_CmdNow = SendTcp_Start;
            Flag_Setup_SMS = RESET;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "安装短信");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "安装短信" );
#endif

            m_gprs.count = M35_SendSMS( M35_TxBuf,
                                        4,
                                        (unsigned char*)"FFFFFFFFFFFFFFFF",
                                        m_carinfo.Family_Phone[0] );//组数据报文
            m_gprs.lpmsg = M35_TxBuf;
        }
        else if( GTW_BurglarAlarm.Flag_SMS == SET ) {           // 防盗报警短信
            GPRS_CmdNow = SendTcp_Start;
            GTW_BurglarAlarm.Flag_SMS = RESET;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "防盗报警短信");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "防盗报警短信" );
#endif

            m_gprs.count = M35_SendSMS( M35_TxBuf,
                                        1,
                                        (unsigned char*)"FFFFFFFFFFFFFFFF",
                                        m_carinfo.Family_Phone[0] );//组数据报文
            m_gprs.lpmsg = M35_TxBuf;
        }
        else if( GTW_IllegalPark.Flag_SMS == SET ) {     // 违停短信 => 后台服务器
            GPRS_CmdNow = SendTcp_Start;
            GTW_IllegalPark.Flag_SMS = RESET;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "违停短信");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "违停短信" );
#endif

            m_gprs.count = M35_SendSMS( M35_TxBuf,
                                        2,
                                        (unsigned char*)"FFFFFFFFFFFFFFFF",
                                        m_carinfo.Family_Phone[0] );//组数据报文
            m_gprs.lpmsg = M35_TxBuf;
        }
        else if( GTW_MoveCar.Flag_SMS == SET ) {         // 请人移车短信 => 后台服务器
            GPRS_CmdNow = SendTcp_Start;
            GTW_MoveCar.Flag_SMS = RESET;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "移车短信");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "移车短信" );
#endif

            m_gprs.count = M35_SendSMS( M35_TxBuf,
                                        3,
                                        GTW_MoveCar.lpszMsg,
                                        m_carinfo.Family_Phone[0] );   //组数据报文
            m_gprs.lpmsg = M35_TxBuf;
        }
        else if( Flag_IllegalParkCancel == SET ) {      // 发送撤销违停 => 后台服务器
            GPRS_CmdNow = SendTcp_Start;
            Flag_IllegalParkCancel = RESET;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "撤销违停短信");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "撤销违停" );
#endif

            m_gprs.count = M35_SendIllegalPark( M35_TxBuf );    //组数据报文
            m_gprs.lpmsg = M35_TxBuf;
        }
        else if( GPRS_LocationTime == 0 ) {             // 定时发送定位信息=>后台服务器
            // [[ 17.6.9 CsYxj Modify
            if(GTW_BurglarAlarm.Flag_Main == SET) {
                GPRS_LocationTime = m_carinfo.FD04_TimeOver;    // (默认30秒)触发防盗报警后定时发送位置信息
                if( Alarm_Count == 0 ) {
                    Alarm_Count = 2;
#ifdef __DEBUG
                   // XFS_WriteBuffer( xfs_num, "定位数据=%d", (int)gpsx.longitude / 100000 );
                    GetLocation( buf );
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf( SET, "定位数据=%s", buf );
#endif
                    // 位置信息 => 后台服务器
                    GPRS_CmdNow = SendTcp_Start;
                    m_gprs.count = M35_SendCarLocation( M35_TxBuf, '3' );//组数据报文 位置信息 => 后台服务器
                    m_gprs.lpmsg = M35_TxBuf;
                }
                else {
                    if( gpsx.gpssta == 1 || gpsx.gpssta == 2 ) {    // 未定位时不发送定位数据: 2017-6-1 CSYXJ(728920175@qq.com)
#ifdef __DEBUG
                       // XFS_WriteBuffer( xfs_num, "定位数据=%d", (int)gpsx.longitude / 100000 );
                        GetLocation( buf );
                        if(m_var.Flag_Debug_Enable == SET)
                            Debug_Printf( SET, "定位数据=%s", buf );
#endif
                        // 位置信息 => 后台服务器
                        GPRS_CmdNow = SendTcp_Start;
                        //组数据报文 位置信息 => 后台服务器
                        m_gprs.count = M35_SendCarLocation( M35_TxBuf,
                                                            m_var.Flag_ApplyDriver == SET ? '1' : '2' );
                        m_gprs.lpmsg = M35_TxBuf;
                    }
                }
            }
            else {
                Alarm_Count = 0;
                GPRS_LocationTime = m_carinfo.FD03_TimeOver;    // (10分钟)触发防盗报警前定时发送位置信息
                if( gpsx.gpssta == 1 || gpsx.gpssta == 2 ) {    // 未定位时不发送定位数据: 2017-6-1 CSYXJ(728920175@qq.com)
#ifdef __DEBUG
                  //  XFS_WriteBuffer( xfs_num, "定位数据=%d", (int)gpsx.longitude / 100000 );
                    GetLocation( buf );
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf( SET, "定位数据=%s", buf );
#endif
                    // 位置信息 => 后台服务器
                    GPRS_CmdNow = SendTcp_Start;
                    m_gprs.count = M35_SendCarLocation( M35_TxBuf, '0' );//组数据报文 位置信息 => 后台服务器
                    m_gprs.lpmsg = M35_TxBuf;
                }
            }
        }
        // 检测电话呼叫任务
        else if( GTW_BurglarAlarm.Flag_Call == SET ) {  // 防盗报警电话呼叫车主
            GTW_BurglarAlarm.Flag_Call = RESET;
            GPRS_CmdNow = Call_Start;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "防盗报警电话");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "防盗报警电话" );
#endif
        }
        else if( GTW_IllegalPark.Flag_Call == SET ) {   // 违停电话呼叫车主
            GTW_IllegalPark.Flag_Call = RESET;
            GPRS_CmdNow = Call_Start;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "违停电话");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "违停电话" );
#endif
        }
        else if( GTW_MoveCar.Flag_Call == SET ) {       // 请人移车电话呼叫车主
            GTW_MoveCar.Flag_Call = RESET;
            GPRS_CmdNow = Call_Start;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "请人移车电话");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "请人移车电话" );
#endif
        }
        else if( M35_TimeCount == 0 ) {                 // 检测信号
            GPRS_CmdNow = CheckSignal1;
        }
    }

    if( GTW_MoveCar.Flag_Main == SET ) {
        if( Sys_TimeCount - GTW_MoveCar.Sec_OverTime > m_carinfo.MoveCar_TimeOver ) {
            // 移车保护时间到，
            GTW_MoveCar.Flag_Main = RESET;  // 清除移车主标志, 实现指定时间内不再响应该消息
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_value, "请人移车%d秒保护结束", m_carinfo.MoveCar_TimeOver );
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "请人移车%d秒保护结束", m_carinfo.MoveCar_TimeOver );
#endif
        }
    }
}


/*******************************************************************************
* Function Name  : M35_SendCarStatus
* Description    : 通过GPRS将车辆状态发送到后台服务器
******************************************************************************/
unsigned short M35_SendCarStatus(unsigned char* dsr, unsigned char Car_InputCheck,unsigned char Additon)
{
    unsigned char* ptr = dsr + HEAD_LENGTH;

    if(GPRS_CmdNow < SendTcp_Start || GPRS_CmdNow > SendTcp_Finished) {
        return 0;
    }
    memcpy( ptr, m_carinfo.Car_9SN, __CAR9SN_LEN__ );   // 设置车牌号[9Byte]:
    ptr += __CAR9SN_LEN__;

    *ptr++ = Car_InputCheck;
    *ptr++ = Additon;
    return GTW_Packet( dsr, 1, dsr + HEAD_LENGTH, ptr-dsr-HEAD_LENGTH, m_carinfo.DTU_Type );
}


/*******************************************************************************
* Function Name  : M35_SendCarLocation
* Description    : 提交车辆的位置 => 后台服务器
******************************************************************************/
unsigned short M35_SendCarLocation( unsigned char* dsr, unsigned char Type )
{
    unsigned char* s1;

    if( NULL == dsr ) {
        return 0;
    }

    if(GPRS_CmdNow < SendTcp_Start || GPRS_CmdNow > SendTcp_Finished) {
        return 0;
    }

    s1 = dsr + HEAD_LENGTH;

    sprintf( (char*)s1, "%02d", m_carinfo.Car_Type );
    s1 += 2;

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    GetLocation( s1 );
    s1 += 24;
    TimeToCharArray(Sys_CalendarTime, s1 );        //14位当前时间值
    s1 += 14;

    *s1++ = Type;

    if( Type == '1' || Type == '3' ) {
        memset( s1, '0', __DRIVER_TYPE_LEN__  + __DRIVERID_LEN__ );
        s1 += __DRIVER_TYPE_LEN__  + __DRIVERID_LEN__;
    }
    else if( Type == '2' ) {
        sprintf( (char*)s1, "%02d", m_carinfo.Driver_Type );
        s1 += __DRIVER_TYPE_LEN__;

        memcpy( s1, m_carinfo.Driver_ID, __DRIVERID_LEN__ );
        s1 += __DRIVERID_LEN__;
    }

    return GTW_Packet( dsr, 3, dsr + HEAD_LENGTH, s1-dsr-HEAD_LENGTH, m_carinfo.DTU_Type );
}


/*******************************************************************************
* Function Name  : M35_SendIllegalPark
* Description    : 发送取消违停处罚申请 => 后台服务器
******************************************************************************/
unsigned short M35_SendIllegalPark(unsigned char* dsr)
{
    unsigned char* s1;
    unsigned short k = 0;

    if(GPRS_CmdNow < SendTcp_Start || GPRS_CmdNow > SendTcp_Finished) {
        return 0;
    }

    s1 = dsr + HEAD_LENGTH;

    sprintf( (char*)s1, "%02d", m_carinfo.Car_Type );       // 号牌种类[2Byte]: 0=黄牌, 1=蓝牌, 2=黑牌, 3=白牌
    s1 += 2;

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );        // 车牌号[9Byte]:
    s1 += __CAR9SN_LEN__;

    memcpy( s1, m_carinfo.Driver_ID[0] + 2, __DRIVERID_LEN__ ); // 申请人驾驶证号[18Byte]
    s1 += __DRIVERID_LEN__;

    memcpy( s1, Ill_ETicket.stime, sizeof( Ill_ETicket.stime ) );
    s1 += sizeof( Ill_ETicket.stime );

    TimeToCharArray( Sys_CalendarTime, s1 );
    s1 += sizeof( Ill_ETicket.stime );

    sprintf( (char*)s1, "%.7f,%.7f      ", Ill_ETicket.longitude, Ill_ETicket.latitude);
    s1 += 24;
    
    k = GetLocation( s1 );
    s1 += k;

    return GTW_Packet( dsr, 4, dsr + HEAD_LENGTH, s1-dsr-HEAD_LENGTH, m_carinfo.DTU_Type );
}


/*******************************************************************************
* Function Name  : M35_SendSMS
* Description    : 将待发短信消息发给后台服务器统一发送
******************************************************************************/
unsigned short M35_SendSMS(unsigned char* dsr, unsigned char sms_id, unsigned char* phone, unsigned char* recv_sms)
{
    unsigned char* s1;
    int k;

    if(GPRS_CmdNow < SendTcp_Start || GPRS_CmdNow > SendTcp_Finished) {
        return 0;
    }

    s1 = dsr + HEAD_LENGTH;

    if( strlen((char*)recv_sms) > 5 * __PHONE_LENGTH__ )
        *(recv_sms + 5 * __PHONE_LENGTH__ ) = 0;

    sprintf( (char*)s1, "%02d", m_carinfo.Car_Type );           // 号牌种类[2Byte]: 0=黄牌, 1=蓝牌, 2=黑牌, 3=白牌
    s1 += 2;

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );            // 车牌号[9Byte]:
    s1 += __CAR9SN_LEN__;

    sprintf( (char*)s1, "%03d", sms_id );
    s1 += 3;

    memcpy( s1, phone, __PHONE_LENGTH__ );
    PhoneAlign( s1, __PHONE_LENGTH__ );
    s1 += __PHONE_LENGTH__;

    k = strlen((char*)m_carinfo.Driver_ID[0] + 2);
    memcpy( s1, m_carinfo.Driver_ID[0] + 2, __DRIVERID_LEN__ );
    memset( s1 + k, ' ', 28 - k );
    s1 += 28;

    sprintf( (char*)s1, "%d   ", strlen((char*)recv_sms));
    s1 += 4;

    strcpy( (char*)s1, (char*)recv_sms );
    s1 += strlen( (char*)recv_sms );

    return GTW_Packet( dsr, 5, dsr + HEAD_LENGTH, s1-dsr-HEAD_LENGTH, m_carinfo.DTU_Type );
}

