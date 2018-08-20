/*******************************************************************************
* File Name          : gtw_policeapp.c
* Author             : csyxj@728920175@qq.com
* Version            : V0.1
* Date               : 2017-03-19
* Description        : Policing support function
*******************************************************************************/

#include "gtw_Head.h"

unsigned char  Flag_NeweTicket = RESET;

/*******************************************************************************
* Function Name  : eTicket_VoicePrompt
* Description    : 开门后语音提示电子罚单
******************************************************************************/
void eTicket_VoicePrompt(void)
{
    if(Flag_NeweTicket == SET &&
            Car_InputCheck.Value_DriverDoor == SET)
    {
        Flag_NeweTicket = RESET;
        XFS_WriteBuffer(xfs_auto, "您有一条违章信息，请用手机及时查看处理");
#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "您有一条违章信息，请用手机及时查看处理");
#endif
    }
}


/*******************************************************************************
* Function Name  : Get_MatterTicket
* Description    : 罚单信息提醒
******************************************************************************/
unsigned short Get_MatterTicket(unsigned char* src, unsigned char* dsr)
{
    i2b_Def i2b;
    unsigned char* s1;
    unsigned short illTotal;

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "罚单信息提醒");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "罚单信息提醒");
#endif

    // 处理应答回发数据
    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    // 车牌号[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // 车牌号
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;

    // 驾驶证号
    if((is_emptyApplyData(&ApplyLicense) == 0)) {
        memset(s1, '0', __DRIVERID_LEN__ );
    }
    else {
        memcpy(s1, &m_carinfo.Driver_ID[0][0] + __DRIVER_TYPE_LEN__, __DRIVERID_LEN__);
    }
    s1 += __DRIVERID_LEN__;

    illTotal = MX25L128_Word_read(ADDR_ILLEGAL_UPLOAD);     // 读取违章总数

    memcpy( Ill_ETicket.stime, src+21, 14 );                // 来自PolicApp的违章开始时间
    memcpy( Ill_ETicket.etime, src+21, 14 );                // 违章结束时间

    Ill_ETicket.ewhemi = *(src + 35);                       // 东经/西经,E:东经;W:西经
    Ill_ETicket.longitude = Asc2Float((char*)src+36, 10);   // 违章时经度
    Ill_ETicket.nshemi = *(src + 46);                       // 北纬/南纬,N:北纬;S:南纬
    Ill_ETicket.latitude = Asc2Float((char*)src+47, 10);    // 违章时纬度

    if( Asc2Byte((char*)src+9, 2) == 0 )
        Ill_ETicket.ino = ino_IllegalPark;                  // 违章代号 = 违停
    else
        Ill_ETicket.ino = Asc2Byte((char*)src+9, 2);

    i2b.x = MX25L128_Illegal_Write_Base(&Ill_ETicket, RESET);       // 写违章数据

    *s1++ = i2b.c[3];
    *s1++ = i2b.c[2];
    *s1++ = i2b.c[1];
    *s1++ = i2b.c[0];

    *s1++ = HIBYTE(illTotal);
    *s1++ = LOBYTE(illTotal);

    Flag_NeweTicket = SET;      // 设置语音提示标志

    {
        // 违停电子罚单,发送短信及拨打电话
        GTW_IllegalPark.Flag_Main = SET;            // 用于检测是否符合撤销条件
        GTW_IllegalPark.Flag_Call = SET;
        GTW_IllegalPark.Flag_SMS = SET;
        GTW_IllegalPark.lpszMsg = NULL;
        GTW_IllegalPark.Sec_OverTime = 10 * 60;   // 接收违停罚单后10分钟内,车辆启动并离开被开罚单位置500米以上时，传输取消违停信号给手机
    }

    return GTW_Packet( dsr, 0x802, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}


/*******************************************************************************
* Function Name  : Get_EleTicket
* Description    : 收到电子罚单
******************************************************************************/
unsigned short Get_EleTicket(unsigned char* src, unsigned char* dsr)
{
    i2b_Def i2b;
    unsigned char* s1;
    unsigned short illTotal;

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "收到电子罚单");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "开电子罚单");
#endif

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    // 车牌号[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // 车牌号
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;

    // 驾驶证号
    if( m_carinfo.Driver_ID[0][0] == 0x00 ) {
        memset(s1, '0', __DRIVERID_LEN__ );
    }
    else {
        memcpy(s1, &m_carinfo.Driver_ID[0][0] + __DRIVER_TYPE_LEN__, __DRIVERID_LEN__);
    }

    s1 += __DRIVERID_LEN__;

    illTotal = MX25L128_Word_read(ADDR_ILLEGAL_UPLOAD);//读取违章总数

    memcpy( Ill_ETicket.stime, src+21, 14 );                // 来自PolicApp的违章开始时间
    memcpy( Ill_ETicket.etime, src+21, 14 );                // 违章结束时间

    Ill_ETicket.ewhemi = *(src + 35);                       // 东经/西经,E:东经;W:西经
    Ill_ETicket.longitude = Asc2Float((char*)src+36, 10);   // 违章时经度
    Ill_ETicket.nshemi = *(src + 46);                       // 北纬/南纬,N:北纬;S:南纬
    Ill_ETicket.latitude = Asc2Float((char*)src+47, 10);    // 违章时纬度

    Ill_ETicket.ino = ino_IllegalPark;                  // 违章代号 = 违停

    i2b.x = MX25L128_Illegal_Write_Base(&Ill_ETicket, RESET);       // 写违章数据

    *s1++ = i2b.c[3];
    *s1++ = i2b.c[2];
    *s1++ = i2b.c[1];
    *s1++ = i2b.c[0];

    *s1++ = HIBYTE(illTotal);
    *s1++ = LOBYTE(illTotal);

    Flag_NeweTicket = SET;

    {
        // 违停电子罚单,发送短信及拨打电话
        GTW_IllegalPark.Flag_Main = SET;            // 用于检测是否符合撤销条件
        GTW_IllegalPark.Flag_Call = SET;
        GTW_IllegalPark.Flag_SMS = SET;
        GTW_IllegalPark.lpszMsg = NULL;
        GTW_IllegalPark.Sec_OverTime = 10 * 60;   // 接收违停罚单后10分钟内,车辆启动并离开被开罚单位置500米以上时，传输取消违停信号给手机
    }

    return GTW_Packet( dsr, 0x803, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}


/*******************************************************************************
* Function Name  : Get_PoliceCheck
* Description    : 收到警务通稽查
******************************************************************************/
unsigned short Get_PoliceCheck(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;

#ifdef __DEBUG
    //XFS_WriteBuffer(xfs_auto, "警务通稽查");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "警务通稽查");
#endif

    Flag_RevCopData = SET;

    // 处理应答回发数据
    s1 = dsr + HEAD_LENGTH;


    *s1++ = src[8];
    *s1++ = src[9];


    *s1++ = src[1];
    *s1++ = src[2];

    // 车牌号[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // 车牌号
    s1 += __CAR9SN_LEN__;

    // 结果[1Byte]: 0=成功/确认;1=失败;2=消息有误;3=不支持
    *s1++ = 0;

    return GTW_Packet( dsr, 0x804, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}


/*******************************************************************************
* Function Name  : Get_ReadAccident
* Description    : 收到读取事故分析数据命令
******************************************************************************/
unsigned short Get_ReadAccident(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;

    const unsigned char CarFault[38] = "20161212080808P000120161212202020P0002";
    const unsigned char CarChangLight[15] = "201612121010101";
    const unsigned char DriveFlase[48] = "20161212090909120110100090080070060050040005";
    const unsigned char TestBuf[87] = {
        0x00,0x00,0x00,0x01,    //4byte 违章数据的唯一标志符01号
        '1','1',0xE8,0x92,0x99,'A','1','2','3','4','5',// 11byte
        '2','0','1','6','1','2','1','2','1','4','1','4','1','4',//    14byte
        '2','0','1','6','1','2','1','2','1','4','1','4','3','0',//          14byte
        '1','1','1','2','3','4','5','6','7','8','9','1','2','3','4','5','6','7','8','9',   //   20byte
        '1','1','6','.','4','3','2','1','5','2',',','2','3','.','9','4','7','0','8',' ',' ',' ',	//      22byte
        0x85,0x0D
    };   //    1byte

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "读取事故分析数据");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "读取事故分析数据");
#endif

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    // 车牌号[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // 车牌号
    s1 += __CAR9SN_LEN__;

    switch( src[19] )
    {
    case '0':
    {
        //给发送缓冲区赋值，并发送出去-----------------------------*/
        *s1++ = 0x00;
        *s1++ = 0xdc;
        memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
        s1 += __CAR9SN_LEN__;
        *s1++ = '1';
        *s1++ = '1';
        *s1++ = '1';
        *s1++ = '1';
        memcpy( s1, m_carinfo.Driver_ID[0], __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
        s1 += __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__;
        *s1++ = '1';
        *s1++ = 0;
        *s1++ = 0x26;
        memcpy( s1, CarFault, sizeof( CarFault ) );
        s1 += sizeof( CarFault );
        *s1++ = 0x0F;
        memcpy( s1, CarChangLight, sizeof( CarChangLight ));
        s1 += sizeof( CarChangLight );
        *s1++ = 0x2c;
        memcpy( s1, DriveFlase, sizeof( DriveFlase ));
        s1 += sizeof( DriveFlase );
        *s1++ = 0;
        *s1++ = 0x56;
        memcpy( s1, TestBuf, 87);
        s1 += 87;
        break;
    }

    case '1':           // 成功/确认
    case '2':           // 失败
    case '3':           // 消息有误
    case '4':           // 不支持
    default:
        *s1++ = 0;
        *s1++ = 0;
        *s1++ = ' ';
        break;
    }

    return GTW_Packet( dsr, 0x805, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}



/*******************************************************************************
* Function Name  : Lamp_json_packet
* Description    : 变光时间和状态为json内容
******************************************************************************/
unsigned short Lamp_json_packet(unsigned char* dsr, Lamp_Def lamp)
{
    char* s1 = (char*)dsr;
    char buf[40] = { 0 };
    char buf1[40] = { 0 };

    // 变光时间
    *s1++ = '{';

    memset( buf1, 0, sizeof(buf1) );
    memcpy( buf1, lamp.ptime, sizeof(lamp.ptime) );
    sprintf( buf, "\"time\":\"%s\",", buf1 );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    //变光后灯状态
    sprintf( buf, "\"code\":\"%d\"", lamp.pstatus);
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    *s1++ = '}';

    return(s1 - (char*)dsr);
}



/*******************************************************************************
* Function Name  : Get_LampChang
* Description    : 收到读取变光数据命令
******************************************************************************/
unsigned short Get_LampChang(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    Lamp_Def lamp;
    unsigned char i;
    unsigned short  len;
    for(i=0; i<10; i++) {
        lamp.pstatus = (i % 3);
        sprintf( (char*)lamp.ptime, "20170421080808");
    }

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "读取变光数据指令");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "读取变光数据指令");
#endif

    // 处理应答回发数据
    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    // 车牌号[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // 车牌号
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;
    *s1++ = '[';

    len = Lamp_json_packet( s1, lamp );
    s1 += len;

    *s1++ = ']';

    return GTW_Packet( dsr, 0x807, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}


/*******************************************************************************
* Function Name  : AbnomalSpeed_json_packet
* Description    : 异常车速时间、状况和速度为json内容


******************************************************************************/
unsigned short AbnomalSpeed_json_packet(unsigned char* dsr, AbnomalSpeed pspeed)
{
    char* s1 = (char*)dsr;
    char buf[40] = { 0 };
    char buf1[40] = { 0 };

    // 异常车速时间
    *s1++ = '{';

    memset( buf1, 0, sizeof(buf1) );
    memcpy( buf1, pspeed.Nspeedtime, sizeof(pspeed.Nspeedtime) );
    sprintf( buf, "\"time\":\"%s\",", buf1 );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    //异常车速状态
    sprintf( buf, "\"code\":\"%d\",", pspeed.Nspeedtimestatus);
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    //异常车速数据
    sprintf( buf, "\"speed\":\"%d\"", pspeed.Nspeeddata);
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    *s1++ = '}';

    return(s1 - (char*)dsr);
}



/*******************************************************************************
* Function Name  : Get_AbnomalSpeed
* Description    : 收到读取异常车速数据
******************************************************************************/
unsigned short Get_AbnomalSpeed(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    AbnomalSpeed nuspeed;
    unsigned char i;
    unsigned short  len;
    for(i=0; i<10; i++) {
        nuspeed.Nspeedtimestatus = (i % 3);
        sprintf( (char*)nuspeed.Nspeedtime, "20170421080808");
        nuspeed.Nspeeddata = 200;
    }

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "读取异常车速数据指令");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "读取异常车速数据指令");
#endif
    // 处理应答回发数据
    s1 = dsr + HEAD_LENGTH;


    *s1++ = src[8];
    *s1++ = src[9];


    *s1++ = src[1];
    *s1++ = src[2];

    // 车牌号[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // 车牌号
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;
    *s1++ = '[';

    len = AbnomalSpeed_json_packet( s1, nuspeed );
    s1 += len;

    *s1++ = ']';

    return GTW_Packet( dsr, 0x808, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : belt_json_packet
* Description    : 安全带异常时间和状态为json内容
******************************************************************************/
unsigned short belt_json_packet(unsigned char* dsr, Safebelt pbelt)
{
    char* s1 = (char*)dsr;
    char buf[40] = { 0 };
    char buf1[40] = { 0 };

    // 变光时间
    *s1++ = '{';

    memset( buf1, 0, sizeof(buf1) );
    memcpy( buf1, pbelt.belttime, sizeof(pbelt.belttime) );
    sprintf( buf, "\"time\":\"%s\",", buf1 );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    //变光后灯状态
    sprintf( buf, "\"code\":\"%d\"", pbelt.beltstatus);
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    *s1++ = '}';

    return(s1 - (char*)dsr);
}

/*******************************************************************************
* Function Name  : Get_SafeBelt
* Description    : 收到读取安全带记录数据
******************************************************************************/
unsigned short Get_SafeBelt(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    Safebelt beltd;
    unsigned char i;
    unsigned short  len;
    for(i=0; i<10; i++) {
        beltd.beltstatus = (i % 3);
        sprintf( (char*)beltd.belttime, "20170421080808");
    }

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "读取安全带记录数据");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "读取安全带记录数据");
#endif

    // 处理应答回发数据
    s1 = dsr + HEAD_LENGTH;


    *s1++ = src[8];
    *s1++ = src[9];


    *s1++ = src[1];
    *s1++ = src[2];

    // 车牌号[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // 车牌号
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;
    *s1++ = '[';

    len = belt_json_packet( s1, beltd );
    s1 += len;

    *s1++ = ']';

    return GTW_Packet( dsr, 0x807, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}


/*******************************************************************************
* Function Name  : belt_json_packet
* Description    : 安全带异常时间和状态为json内容
******************************************************************************/
unsigned short obd_json_packet(unsigned char* dsr, OBDdata obdt)
{
    char* s1 = (char*)dsr;
    char buf[40] = { 0 };
    char buf1[40] = { 0 };

    // 时间
    *s1++ = '{';

    memset( buf1, 0, sizeof(buf1) );
    memcpy( buf1, obdt.obdttime, sizeof(obdt.obdttime) );
    sprintf( buf, "\"time\":\"%s\",", buf1 );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    //故障编码
    memset( buf1, 0, sizeof(buf1) );
    memcpy( buf1, obdt.errstatus, sizeof(obdt.errstatus) );
    sprintf( buf, "\"time\":\"%s\"", buf1 );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    *s1++ = '}';

    return(s1 - (char*)dsr);
}

/*******************************************************************************
* Function Name  : Get_SafeBelt
* Description    : 收到读取OBD故障数据
******************************************************************************/
unsigned short Get_OBDdata(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    OBDdata OBDd;
    unsigned char i;
    unsigned short  len;
    for(i=0; i<10; i++) {
        sprintf( (char*)OBDd.obdttime, "20170421080808");
        sprintf( (char*)OBDd.errstatus, "P1000");
    }

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "读取OBD故障记录数据");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "读取OBD故障记录数据");
#endif

    s1 = dsr + HEAD_LENGTH;


    *s1++ = src[8];
    *s1++ = src[9];


    *s1++ = src[1];
    *s1++ = src[2];

    // 车牌号[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // 车牌号
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;
    *s1++ = '[';

    len = obd_json_packet( s1, OBDd );
    s1 += len;

    *s1++ = ']';

    return GTW_Packet( dsr, 0x807, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}


/*******************************************************************************
* Function Name  : GTW_PoliceAPP
* Description    : 解析警务通APP消息
******************************************************************************/
extern unsigned short Get_IllegalContent(unsigned char* src, unsigned char* dsr);
u16 GTW_PoliceAPP(unsigned char* src, unsigned char* dsr)
{
    unsigned short send_pack_len = 0;
    unsigned short imsg;

#ifdef __DEBUG
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "警务通消息");
#endif

    // 警务通发送过来的数据
    if(memcmp(src+10, m_carinfo.Car_9SN, __CAR9SN_LEN__) != 0) {
        return 0;
    }
    else {
        imsg = MAKEWORD( src[1], src[2] );
        switch(imsg) {
        case 0x1:   // 读取指定罚单内容
            send_pack_len = Get_IllegalContent( src, dsr );
            break;

        case 0x2:   // 罚单信息提醒
            send_pack_len = Get_MatterTicket( src, dsr );
            break;

        case 0x3:   // 开电子罚单
            send_pack_len = Get_EleTicket( src, dsr );
            break;

        case 0x4:   // 警务通稽查
            send_pack_len = Get_PoliceCheck( src, dsr );
            break;

        case 0x5:   // 读取事故分析数据 -- 保持原来功能
            send_pack_len = Get_ReadAccident( src, dsr );
            break;

        case 0x6:   // 临检
            break;

        case 0x7:   // 读取变光数据
            send_pack_len = Get_LampChang( src, dsr );
            break;

        case 0x8:   // 读取异常车速数据
            send_pack_len = Get_AbnomalSpeed( src, dsr );
            break;

        case 0x9:   // 读取安全带记录数据
            send_pack_len = Get_SafeBelt( src, dsr );
            break;

        case 0x10: // 读取OBD故障
            send_pack_len = Get_OBDdata( src, dsr );
            break;
        }
    }

    return send_pack_len;
}
