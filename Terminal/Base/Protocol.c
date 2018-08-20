/*******************************************************************************
* File Name          : protocol.c
* Author             : csyxj@728920175@qq.com
* Version            : V0.1
* Date               : 2017-03-19
* Description        : GTW协议(V2.0)解析支持函数文件
*******************************************************************************/
#include "gtw_Head.h"

unsigned char gtw_RxBuf[ I2411E_RxBUF_LEN ];    // 接收缓冲区
unsigned char gtw_TxBuf[ I2411E_TxBUF_LEN ];    // 发送缓冲区

unsigned short GTW_Packet(unsigned char* dsr, unsigned short nMsg_id,
                          unsigned char* src, unsigned short len, unsigned short obj);

unsigned short GTW_Analysis( unsigned char* src, unsigned short len,
                             unsigned char* dsr, unsigned short* packet_size );

/*******************************************************************************
* Function Name  : GTW_Packet
* Description    : 数据打包
******************************************************************************/
unsigned short GTW_Packet(unsigned char* dsr, unsigned short nMsg_id, unsigned char* src, unsigned short len, unsigned short obj)
{
    static unsigned short MsgNumber = 0;
    unsigned short crc16 = 0;
    unsigned char* s1;
    unsigned short count = 0;

    if( src == NULL || len == 0 )
        return 0;

    if( dsr == NULL )
        return(len + 12);

    s1 = dsr;

    *s1++ = 0x7e;

    *s1++ = HIBYTE( nMsg_id );
    *s1++ = LOBYTE( nMsg_id );

    // 消息头
    *s1++ = 0;
    *s1++ = HIBYTE( len );
    *s1++ = LOBYTE( len );

    *s1++ = HIBYTE(obj);
    *s1++ = LOBYTE(obj);

    *s1++ = HIBYTE( MsgNumber );
    *s1++ = LOBYTE( MsgNumber );
    MsgNumber++;

    if( s1 != src )
        memcpy( s1, src, len );
    s1 += len;
    count = s1 - dsr;

    crc16 = get_crc16( dsr + 1, count ); 

    *s1++ = HIBYTE( crc16 );
    *s1++ = LOBYTE( crc16 );

    return( s1 - dsr );
}

/*******************************************************************************
* Function Name  : GTW_Analysis
* Description    : 数据解包
******************************************************************************/
unsigned short GTW_Analysis( unsigned char* src, unsigned short len, unsigned char* dsr, unsigned short* packet_size )
{
#define PHONE_BT_SIZE       155
#define DRVBALL_BT_SIZE     20
#define RF433_SIZE          64

    unsigned short send_pack_len = 0;
    MsgHead_Struct gtw_msg;

    if( src == NULL || len == 0 )
        return(0);

    if( dsr == NULL )
        return(0);

    if(*src == 0x7E) {
        /*消息头 分解-----------------------------------------------------------------------*/
        gtw_msg.MsgID       =   MAKEWORD( src[1], src[2] );
        gtw_msg.MsgLen      =   MAKEWORD( src[4], src[5] );
        gtw_msg.MsgSource   =   (MsgSource_Typedef)MAKEWORD( src[6], src[7] );
        gtw_msg.MsgNum      =   MAKEWORD( src[8], src[9] );
        gtw_msg.MsgCheck    =   MAKEWORD( src[gtw_msg.MsgLen+10], src[gtw_msg.MsgLen+11] );

        switch((u8)gtw_msg.MsgSource) {
        case (UserPhone):
            send_pack_len = GTW_UserAPP(src, dsr);
            *packet_size = m_carinfo.bt_packlen;
            break;

        case (PolicePhone):
            send_pack_len = GTW_PoliceAPP(src, dsr);
            *packet_size = m_carinfo.bt_packlen;
            break;

        case (EleDriverBall):
            send_pack_len = GTW_DriverBallApp(src, dsr);
            *packet_size = DRVBALL_BT_SIZE;
            break;

        case (LightModule):
            send_pack_len = GTW_LightAPP(src, dsr);
            *packet_size = RF433_SIZE;
            break;

        case SetupApp:
            send_pack_len = GTW_SetupAPP(src,dsr);
            *packet_size = m_carinfo.bt_packlen;
            break;

        case DataCollector:
            send_pack_len = GTW_DataCollector(src, dsr);
            *packet_size = RF433_SIZE;
            break;

        case CarTerminal:
            send_pack_len = GTW_CarTerminal(src, dsr);
            *packet_size = RF433_SIZE;
            break;

        default:
            break;
        }
    }
    return send_pack_len;
}


/*******************************************************************************
* Function Name  : GTW_Analysis
* Description    : 数据解包
******************************************************************************/
void GTW_ReceiveData( unsigned char dt, unsigned short recv_object, unsigned int recv_handle)
{
    // 状态定义
    enum {
        gtw_recv_st1 = 1,
        gtw_recv_st2,
        gtw_recv_st3,
        gtw_recv_st4,
        gtw_recv_st5,
        gtw_recv_st6,
        gtw_recv_st7,
        gtw_recv_st8,
        gtw_recv_st9,
        gtw_recv_st10,
        gtw_recv_st11,
        gtw_recv_st12,
        gtw_recv_st13
    };

    static unsigned char  recv_status = gtw_recv_st1;
    static unsigned short recv_point_gtw = 0;
    static unsigned short recv_count = 0;
    static unsigned short send_count = 0;
    static unsigned short recv_crc16 = 0;
    static unsigned short pack_len = 155;
    unsigned short i;

    if(m_var.gtw_recv_timeout == 0) {
        recv_status = gtw_recv_st1;
        recv_point_gtw = 0;
    }
    m_var.gtw_recv_timeout = 200;

    gtw_RxBuf[recv_point_gtw++] = dt;

    // 状态转换
    switch( recv_status ) {
    case gtw_recv_st1:
        recv_status = gtw_recv_st2;
        if(gtw_RxBuf[0] != 0x7E) {
            recv_status = gtw_recv_st1;
            recv_point_gtw = 0;
        }
        break;

    case gtw_recv_st2:
        recv_status = gtw_recv_st3;
        break;

    case gtw_recv_st3:
        recv_status = gtw_recv_st4;
        break;

    case gtw_recv_st4:
        recv_status = gtw_recv_st5;
        break;

    case gtw_recv_st5:
        recv_count = dt << 8;
        recv_status = gtw_recv_st6;
        break;

    case gtw_recv_st6:
        recv_count += dt;
        recv_status = gtw_recv_st7;
        break;

    case gtw_recv_st7:
        recv_status = gtw_recv_st8;
        break;

    case gtw_recv_st8:
        recv_status = gtw_recv_st9;
        break;

    case gtw_recv_st9:
        recv_status = gtw_recv_st10;
        break;

    case gtw_recv_st10:
        recv_status = gtw_recv_st11;
        break;

    case gtw_recv_st11:
        if(recv_point_gtw >= recv_count + HEAD_LENGTH ) {
            recv_status = gtw_recv_st12;
        }
        break;

    case gtw_recv_st12:
        recv_crc16 = dt << 8;
        recv_status = gtw_recv_st13;
        break;

    case gtw_recv_st13:
        recv_crc16 += dt;

        recv_status = gtw_recv_st1;
        recv_point_gtw = 0;

        send_count = GTW_Analysis(gtw_RxBuf, recv_count, gtw_TxBuf+(recv_object == 0x0201 ? 11 : 9), &pack_len); 

        if(send_count > 0) {
            if( recv_object == 0x0201 ) {
                I24_SendBuffer(recv_object, recv_handle, gtw_TxBuf, send_count); 
            }
            else if( recv_object == 0x0101 ) {
                for(i=0; i<send_count / pack_len; i++ ) {
                    I24_SendBuffer_pack(recv_object, recv_handle, (pack_len * i) + gtw_TxBuf + 9, pack_len);
                }
                if( send_count % pack_len > 0 ) {
                    I24_SendBuffer_pack(recv_object, recv_handle, (pack_len * i) + gtw_TxBuf + 9, send_count % pack_len);
                }
            }
        }
        break;

    default:
        break;
    }
}

