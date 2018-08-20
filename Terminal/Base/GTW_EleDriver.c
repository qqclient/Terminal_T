/*******************************************************************************
* File Name          : gtw_eledriver.c
* Author             : csyxj@728920175@qq.com
* Version            : V0.1
* Date               : 2017-03-19
* Description        : GTW协议(V2.0)解析支持函数文件
*******************************************************************************/
#include "gtw_Head.h"

unsigned char  EleDB_Type = 0;      // 电子驾照功能状态
unsigned char  EleDB_Num  = 0;      // 电子驾照球号
unsigned char  EleDB_Phone[16] = { 0 };     // 电子驾照球电话号码


/*******************************************************************************
* Function Name  : Get_EleDriver_VerifyDriver
* Description    : 验证驾驶员
******************************************************************************/
unsigned short Get_EleDriver_VerifyDriver(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    unsigned char  result = 0;
    unsigned char  idNo[20];
    unsigned char  i;

    // 处理应答回发数据
    s1 = dsr + HEAD_LENGTH;

//    *s1++ = src[8];
//    *s1++ = src[9];

//    *s1++ = src[1];
//    *s1++ = src[2];

    // 驾照准驾类型(1B) + 电子驾照号(9B) + 驾照球号(1B)
    // memcpy( s1, &src[10], 11 );
    // s1 += 11;
//  memcpy( s1, &src[10], 10 );
//    s1 += 10;


    *s1++ = src[10];      //驾照准驾类型(1B)
    *s1++ = 0x01;   //驾照球号(1B)

    // 提取电子驾照 ==> idNo
    sprintf( (char*)idNo, "%02d", src[10] );// 驾照类型
    for( i=0; i<(__DRIVERID_LEN__ / 2); i++ ) {
        sprintf( (char*)idNo + 2 + i * 2, "%02x", src[i + 11] );
    }
    if( idNo[19] == 0xa ) { // 处理尾号=X的特例
        idNo[19] = 'X';
    }

    // 驾照球号
    EleDB_Num = src[20];

    // 驾照球目前功能状态
    EleDB_Type = src[22];

    // 电话号码
    for(i=0; i<(__PHONE_LENGTH__ / 2); i++ ) {
        sprintf( (char*)EleDB_Phone + i * 2, "%02x", src[i + 23] );
    }
    // 小写 ==> 大写
    for(i=0; i<__PHONE_LENGTH__; i++ ) {
        if(EleDB_Phone[i] > '9') {
            EleDB_Phone[i] -= 0x20;
        }
    }

    // 命令类型解析
    switch( src[21] ) {
    case 0x6:   // 申请六: 电子驾照球主动申请防盗验证，不需要带授权码，无优先级
    {
        //记录申请人信息，即将驾照号压入堆栈-------------------------*/
        Flag_ApplyResult = SET;                                       // 已收到申请
        PushApplyStack(&ApplyLicense.NoneApplyLic, idNo, 20); // 将申请人的驾照信息压入堆栈
        History_UseCar(idNo, 0);//保存用车记录

#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "申请六,防盗验证");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET,"申请六:电子驾照球主动申请防盗验证" );
#endif
        //根据当前智能终端状态 改变智能终端状态
        if(FlagTerminalStat == '2')	{			  // 当前智能终端所处的状态
            I2411E_ChangeCarStatus('3');		  // 更改终端状态为状态三
        }
        break;
    }

    case 0x7:   // 申请七: 电子驾照球申请成为驾驶员，不需要带授权码，时序最后的最优先, 优先级=2
    {
        //记录申请人信息，即将驾照号压入堆栈
        Flag_ApplyResult = SET; //已收到申请
        PushApplyStack(&ApplyLicense.SecApplyLic, idNo, 20);//将申请人的驾照信息压入堆栈
        History_UseCar(idNo, 0);//保存用车记录

#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "申请七,验证电子驾照");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET,"申请七:电子驾照球申请成为驾驶员");
#endif

        //根据当前智能终端状态 改变智能终端状态
        if((FlagTerminalStat == '2') || (FlagTerminalStat == '3')) {
            I2411E_ChangeCarStatus('4');    //更改终端状态为状态四
        }
        break;
    }

    case 0x8:   //申请八:临时禁用的电子驾照球连按三下按钮，强行申请成为驾驶员，不需要带授权码，时序最后的最优先, 优先级=2
    {
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "申请八,强制验证");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET,"申请八:临时禁用的电子驾照球强行申请为驾驶员");
#endif

        //记录申请人信息，即将驾照号压入堆栈
        Flag_ApplyResult = SET;// 已收到申请
        PushApplyStack(&ApplyLicense.SecApplyLic, idNo, 20);// 将申请人的驾照信息压入堆栈
        History_UseCar(idNo, 0);//保存用车记录

        //根据当前智能终端状态 改变智能终端状态
        if((FlagTerminalStat == '2') || (FlagTerminalStat == '3')) {
            I2411E_ChangeCarStatus('4');// 更改终端状态为状态四
        }
    }
    }

    // 验证结果


    // 回写驾照球功能状态位
    *s1++ = src[22];   // 写原来的功能状态


// 下发时间
    *s1++ = Byte2Bcd(Sys_CalendarTime.w_year/100); //年
    *s1++ = Byte2Bcd(Sys_CalendarTime.w_year%100);
    *s1++ = Byte2Bcd(Sys_CalendarTime.w_month);  //月
    *s1++ = Byte2Bcd(Sys_CalendarTime.w_day);   //日
//    *s1++ = Byte2Bcd(Sys_CalendarTime.hour);    //时
//    *s1++ = Byte2Bcd(Sys_CalendarTime.min);     //分
//    *s1++ = Byte2Bcd(Sys_CalendarTime.sec);     //秒

//    TimeToCharArray(Sys_CalendarTime, s1);
//    s1 += 14;

    *s1++ = result;
    return GTW_Packet( dsr, 0x801, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );  // 数据报文包
}

/*******************************************************************************
* Function Name  : Get_EleDriver_FollowCar
* Description    : 驾照随车通知
******************************************************************************/
unsigned short Get_EleDriver_FollowCar(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    unsigned char  idNo[20];
    unsigned char  i;

#ifdef __DEBUG
    //XFS_WriteBuffer( xfs_auto, "收到驾照随车通知" );
#endif

    // 处理应答回发数据
    s1 = dsr + HEAD_LENGTH;

//    *s1++ = src[8];
//    *s1++ = src[9];

//    *s1++ = src[1];
//    *s1++ = src[2];

    // 驾照准驾类型(1B) + 电子驾照号(9B) + 驾照球号(1B)
//    memcpy( s1, &src[10], 11 );
//    s1 += 11;
    *s1++ = src[10];
    *s1++ = src[20];

    // 提取电子驾照 ==> idNo
    sprintf( (char*)idNo, "%02d", src[10] );// 驾照类型
    for( i=0; i<(__DRIVERID_LEN__ / 2); i++ ) {
        sprintf( (char*)idNo + 2 + i * 2, "%02x", src[i + 11] );
    }
    if( idNo[19] == 0xa ) { // 处理尾号=X的特例
        idNo[19] = 'X';
    }
    // 检查收到的电子驾驶证号, 返回结果: 0=成功/确认;1=失败;2=消息有误;3=不支持
    if( memcmp( idNo+2, m_carinfo.Driver_ID[0]+2, __DRIVERID_LEN__ ) == 0 )
        *s1++ = 0;// 结果
    else {
        *s1++ = 1;
        History_UseCar(idNo, 255);//保存用车记录
    }

    return GTW_Packet( dsr, 0x802, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );  // 数据报文包
}

/*******************************************************************************
* Function Name  : GTW_DriverBallApp
* Description    : 解析电子驾照球消息
******************************************************************************/
unsigned short GTW_DriverBallApp(unsigned char* src, unsigned char* dsr)
{
    unsigned short imsg;
    unsigned short send_pack_len = 0;

    /*
    // 检测该条信息是否发给本车
    if( memcmp(src + HEAD_LENGTH, m_carinfo.Car_9SN, __CAR9SN_LEN__ ) != 0) {
    #ifdef __DEBUG
           XFS_WriteBuffer(xfs_auto, "车牌号不对");
        if(m_var.Flag_Debug_Enable == SET) {
            memset( buf, 0, sizeof(buf) );
            memcpy( buf, src + HEAD_LENGTH, __CAR9SN_LEN__ );
            Debug_Printf(SET,
    "车牌号(%s)[%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x]不对",
    buf, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8] );
        }
    #endif
           return 0; //不是发给本车的信息
       }
    */

    // 消息ID
    imsg = MAKEWORD( src[1], src[2] );

    switch(imsg) {
    case 0x1:   // 验证驾驶员
        send_pack_len = Get_EleDriver_VerifyDriver( src, dsr );
        break;

    case 0x2:   // 驾照随车通知
        send_pack_len = Get_EleDriver_FollowCar( src, dsr );
        break;

    default:
        break;
    }

    return send_pack_len;
}

