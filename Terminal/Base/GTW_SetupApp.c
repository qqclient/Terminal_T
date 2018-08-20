/*******************************************************************************
* File Name          : gtw_setupapp.c
* Author             : csyxj@728920175@qq.com
* Version            : V0.1
* Date               : 2017-03-19
* Description        : Installing application support functions
*******************************************************************************/

#include "gtw_Head.h"

unsigned char Flag_Setup_SMS = RESET;
unsigned char Flag_Setup_LAMP = RESET;

unsigned short Get_DetectInput(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "手动检测");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "手动检测");
#endif

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];
    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;
    *s1++ = Car_SetupCheck.Value_Ignite;
    *s1++ = Car_SetupCheck.Value_Honk;
    *s1++ = Car_SetupCheck.Value_HighBeam;
    *s1++ = Car_SetupCheck.Value_LowBeam;
    *s1++ = Car_SetupCheck.Value_KEY;
    *s1++ = Car_SetupCheck.Value_DriverDoor;
    *s1++ = Car_SetupCheck.Value_DeputyBelt;

    return GTW_Packet( dsr, 0x801, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}


unsigned short Get_DetectModular(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "自动检测");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "自动检测");
#endif

    Count_FindCop = m_carinfo.SD01_TimeOver * 10;
    Flag_Setup_LAMP = SET;

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];
    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;
    *s1++ = 1;
    *s1++ = 1;
    *s1++ = 1;
    *s1++ = 1;

    return GTW_Packet( dsr, 0x802, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

unsigned short Get_StudyInput(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;

    m_var.Flag_InputStudy = 0xff;

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];
    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;

    return GTW_Packet( dsr, 0x803, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}


unsigned short Get_LAMPSelect(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
#ifdef __DEBUG
    char buf[64] = {0};
#endif

    switch( src[19] ) {
    case 0x01:
        m_carinfo.Flag_LAMPInput = SET;
        break;
    case 0x02:
        m_carinfo.Flag_LAMPInput = RESET;
        break;
    }

#ifdef __DEBUG
    sprintf( (char*)buf,
             "%s检测大灯",
             m_carinfo.Flag_LAMPInput == SET ? "直接" : "模块" );
    XFS_WriteBuffer(xfs_auto, buf);
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, buf);
#endif

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];
    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;

    return GTW_Packet( dsr, 0x804, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}



/*******************************************************************************
* Function Name  : GTW_SetupAPP
* Description    : 解析用户手机APP消息
******************************************************************************/
unsigned short GTW_SetupAPP(unsigned char* src, unsigned char* dsr)
{
    unsigned short imsg;
    unsigned short send_pack_len = 0;
#ifdef __DEBUG
    unsigned char  buf[24] = {0};
#endif

    if( memcmp(src + HEAD_LENGTH, m_carinfo.Car_9SN, __CAR9SN_LEN__ ) != 0) {
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "车牌号不对");
        if(m_var.Flag_Debug_Enable == SET) {
            memset( buf, 0, sizeof(buf) );
            memcpy( buf, src + HEAD_LENGTH, __CAR9SN_LEN__ );
            Debug_Printf(SET,
                         "安装APP操作的车牌号(%s)[%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x]不对",
                         buf, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8] );
        }
#endif
        return 0;
    }

    // 消息ID
    imsg = MAKEWORD( src[1], src[2] );
    switch(imsg) {
    case 0x1:
        send_pack_len = Get_DetectInput( src, dsr );
        break;

    case 0x2:
        send_pack_len = Get_DetectModular( src, dsr );
        break;

    case 0x3:
        send_pack_len = Get_StudyInput( src, dsr );
        break;

    case 0x4:
        send_pack_len = Get_LAMPSelect( src, dsr );
        break;

    default:
        break;
    }

#ifdef __DEBUG
    XFS_WriteBuffer( xfs_auto, "返回给安装端%d个数据", send_pack_len );
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "消息=%04x,数据返回%d个", imsg, send_pack_len );
#endif

    return send_pack_len;
}


