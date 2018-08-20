/*******************************************************************************
* File Name          : gtw_setupapp.c
* Author             : 宁建文
* Version            : V0.1
* Date               : 2017-03-19
* Description        : Instrument application support function
*******************************************************************************/

#include "gtw_Head.h"

/*******************************************************************************
* Function Name  : GTW_Broadcast
* Description    : 广播消息
*******************************************************************************/
unsigned short GTW_Broadcast(unsigned char* src, unsigned char* dsr)
{
    unsigned char iMsg;

    iMsg = Asc2Byte((char*)src+22, 2);

    switch(iMsg) {
    case 1://禁用远光
        DataCollectorMsg.MsgHighBeams = SET;
        break;
    case 2://禁用喇叭
        DataCollectorMsg.MsgWhistle = SET;
        break;
    case 3://限速
        Flag_LimitSpeed = SET;
        break;
    case 4://限行
        Flag_LimitRun = SET;
        break;
    case 5://广告信息
    case 6://进入服务区
    case 7://进入高速
    case 8://进入城区
    case 9://进入郊区
        break;
    }

    return 0;
}


/*******************************************************************************
* Function Name  : GTW_ReadTicket
* Description    : 读取罚单
******************************************************************************/
unsigned short GTW_ReadTicket(unsigned char* src, unsigned char* dsr)
{
    unsigned short send_pack_len = 0;

    return send_pack_len;
}


/*******************************************************************************
* Function Name  : GTW_DataCollector
* Description    : 解析采集仪消息
******************************************************************************/
unsigned short GTW_DataCollector(unsigned char* src, unsigned char* dsr)
{
    unsigned short send_pack_len = 0,imsg = 0;

    // 检测该条信息是否发给本车
    if( memcmp(src + HEAD_LENGTH, m_carinfo.Car_9SN, __CAR9SN_LEN__ ) != 0) {
        return 0; //不是发给本车的信息
    }

    // 消息ID
    imsg = MAKEWORD( src[1], src[2] );
    switch(imsg) {
    case 0x1:   // 读取罚单
        send_pack_len = GTW_ReadTicket(src, dsr);
        break;

    case 0x2:   // 广播信息
        send_pack_len = GTW_Broadcast(src, dsr);
        break;

    default:
        break;
    }

    return send_pack_len;
}


/*******************************************************************************
* Function Name  : GTW_CarTerminal
* Description    : 解析智能终端消息
******************************************************************************/
unsigned short GTW_CarTerminal(unsigned char* src, unsigned char* dsr)
{
    MsgHead_Struct Si4432RevMsg;

    // 消息信息分解
    Si4432RevMsg.MsgID =   MAKEWORD (src[1],src[2]);
    Si4432RevMsg.MsgSource = (MsgSource_Typedef)MAKEWORD(src[6],src[7]);

    if( Si4432RevMsg.MsgSource == CarTerminal ) {
        if(Si4432RevMsg.MsgID == 0x01) {
//                  if( isSubtend( Si4432_RxBuf[10], gpsx.direction) == 1 )
            // 收到的航向与本车航向对比
            Flag_ChangeLight = SET;
        }
    }
    return 0;
}

