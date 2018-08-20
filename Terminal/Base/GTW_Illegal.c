/******************** (C) COPYRIGHT soarsky ************************************
* File Name          : gtw_illegal.c
* Author             : csyxj@728920175@qq.com
* Version            : V0.1
* Date               : 2017-05-19
* Description        : Violation detection support function
*******************************************************************************/

#include "gtw_Head.h"

volatile unsigned char  DriverLicIndex = 0;          // 验证电子驾照状态字
volatile unsigned char  Flag_LimitSpeed = RESET;     // 接收到大客车限行信号
volatile unsigned char  Flag_LimitRun = RESET;       // 接收到限行信号
volatile unsigned char  Flag_ChangeLight = RESET;    // 接收到变光信号
volatile unsigned char  Flag_DrivePlayPhone = RESET; // 接收到开车玩手机信号
volatile unsigned char  Flag_NoDriverLic = RESET;    // 无证驾驶标志
volatile unsigned char  Flag_ApplyResult = RESET;    // 收到驾照申请标记

/*******************************************************************************
* Function Name  : SafeBelt
* Description    : 安全带检测
*******************************************************************************/
void SafeBelt(void)
{
    static Illegal_Def             SafeBeltViolence;         //用于保存安全带违章信息
    static volatile unsigned char  Flag_SafeBeltStatus = 0;
    static volatile unsigned int   SafeBelt_TimeOver = 0;
    unsigned int  latitude;             // 纬度 分扩大100000倍,实际要除以100000
    unsigned int  longitude;            // 经度 分扩大100000倍,实际要除以100000

    switch(Flag_SafeBeltStatus) {
    case 0: {
        if( Sys_TimeCount - Sys_RunTime > 30 ) {    // 开机30秒内不检测
            if( Car_InputCheck.Value_DriverBelt == RESET &&     // 未系安全带
                    TerminalStatus == Status_Run )              // 车辆在行驶状态
            {
                Flag_SafeBeltStatus = 1;
                SafeBelt_TimeOver = Sys_TimeCount;              // 记录时间

                SafeBeltViolence.ino = DriveNoSeatBelt;         // 违章类型
                TimeToCharArray(Sys_CalendarTime, SafeBeltViolence.stime);//记录违章开始时间
            }
        }
        break;
    }

    case 1: {
        if( Car_InputCheck.Value_DriverBelt == SET ||
                TerminalStatus != Status_Run ) {

            Flag_SafeBeltStatus = 0;
        }
        else {
            if( Sys_TimeCount - SafeBelt_TimeOver > m_carinfo.WZ03_TimeOver ) {
                XFS_WriteBuffer(xfs_auto, "请您系好安全带,否则记录违章");
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "请您系好安全带,否则记录违章");
#endif
                SafeBelt_TimeOver = Sys_TimeCount;          // 记录时间
                Flag_SafeBeltStatus = 2;
            }
        }
        break;
    }

    case 2: {
        if( TerminalStatus != Status_Run ||                 // 非行驶状态
                Car_InputCheck.Value_DriverBelt == SET )    // 系好安全带
        {
            Flag_SafeBeltStatus = 0;
            break;
        }

        if( GetSpeed() >= SPEED_MIN ) {
            // 行驶状态
            if(gpsx.gpssta == 1 || gpsx.gpssta == 2) {  // GPS定位成功
                latitude = gpsx.latitude;       // 记录开始位置
                longitude = gpsx.longitude;
            }
            Flag_SafeBeltStatus = 3;
						
        }
        break;
    }

    case 3: {
        if( TerminalStatus != Status_Run ||                 // 非行驶状态
                Car_InputCheck.Value_DriverBelt == SET )    // 系好安全带
        {
            Flag_SafeBeltStatus = 0;
            break;
        }

        if( (m_carinfo.Flag_Simulation & 0x01) == SET ) {
            if( Sys_TimeCount - SafeBelt_TimeOver > m_carinfo.WZ04_TimeOver ) {//m_carinfo.WZ04_TimeOver  //仿真内场修改 采用10 外场恢复使用m_carinfo.WZ04_TimeOver
                // 按键 门开关2 一直未按下去超时 1分钟进记录安全带违章
                Flag_SafeBeltStatus = 4;
						
            }
            else if( Car_InputCheck.Value_SecondDoor == SET ) {
                // 按键 门开关2 模拟行驶离开500米记录安全带违章
                Flag_SafeBeltStatus = 4;
            }
        }
        else {
            if( Sys_TimeCount - SafeBelt_TimeOver > m_carinfo.WZ04_TimeOver ) {
                // 车辆行驶达到记录不系安全带违章的时间
                Flag_SafeBeltStatus = 4;
            }
            else {
                if(gpsx.gpssta == 1 || gpsx.gpssta == 2) {
                    // GPS定位成功
                    if((Sys_TimeCount - SafeBelt_TimeOver) % 5 == 0) {
                        //5s计算一次距离
                        if(GetDistance(latitude, longitude, gpsx.latitude, gpsx.longitude) >= 500) {
                            //距离大于500米, 记录安全带违章
                            Flag_SafeBeltStatus = 4;
                        }
                    }
                }
            }
        }
        break;
    }

    case 4: {
        XFS_WriteBuffer(xfs_auto, "您因未系安全带,已被记录违章");
#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "您因未系安全带,已被记录违章");
#endif
        SafeBeltViolence.ino = DriveNoSeatBelt;          //违章类型
        TimeToCharArray(Sys_CalendarTime,SafeBeltViolence.etime);//记录违章结束时间
        MX25L128_Illegal_Write(&SafeBeltViolence);             //将违章数据写入Flash中
        Flag_SafeBeltStatus = 5;
    }
    break;

    case 5: {
        if( Car_InputCheck.Value_DriverBelt == SET ||
                TerminalStatus == Status_Park ||
                m_var.Flag_ApplyDriver == RESET        ) {
            Flag_SafeBeltStatus = 6;  //外场 0 内场 6 只检测一下安全带演示要求
        }
        break;
    }

    default :
        Flag_SafeBeltStatus = 6; //外场 0 内场 6 只检测一下安全带演示要求
        break;
    }
}


/*******************************************************************************
* Function Name  : LimitFarLightInArea
* Description    : 在禁用远光区域远光灯检测
*******************************************************************************/
void FarLightInArea(void)
{
    static Illegal_Def             FarLightViolence1;         //用于保存滥用远光违章信息
    static volatile unsigned char  FarLightStatus = 0;
    static volatile unsigned int   FarLightWarnTime1 = 0;

    switch(FarLightStatus) {
    case 0:
        if(DataCollectorMsg.MsgHighBeams == SET &&    //禁用远光区域
                IsDayOrNight() == night_time )//夜间
        {
            XFS_WriteBuffer(xfs_auto, "您已进入禁用远光区域,请不要开启远光灯");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "您已进入禁用远光区域,请不要开启远光灯");
#endif
            FarLightViolence1.ino = IllegalUseFarLight;       //违规使用远光灯
            TimeToCharArray(Sys_CalendarTime,FarLightViolence1.stime);//记录违章开始时间
            FarLightStatus++;
        }
        break;

    case 1:
        if( DataCollectorMsg.MsgHighBeams == RESET && //非禁用远光区域
                IsDayOrNight() == day_time )    //非夜间
        {
            FarLightStatus = 0;
            break;
        }
        if(Car_InputCheck.Value_HighBeam == SET ) //开启远光灯
        {
            XFS_WriteBuffer(xfs_auto, "请您关闭远光灯,否则记录违章");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "请您关闭远光灯,否则记录违章");
#endif
            FarLightWarnTime1 = Sys_TimeCount;      //记录下报警的时间
            FarLightStatus++;
        }
        break;

    case 2:
        if(Sys_TimeCount - FarLightWarnTime1 >= 15)
        {
            FarLightStatus++;
        }
        break;

    case 3:
        if( DataCollectorMsg.MsgHighBeams != SET || //非禁用远光区域
                IsDayOrNight() == day_time )     //非夜间
        {
            FarLightStatus = 0;
            break;
        }

        if(Car_InputCheck.Value_HighBeam == SET )//开启远光
        {

            XFS_WriteBuffer(xfs_auto, "您因违规使用远光灯,已被记录违章");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "您因违规使用远光灯,已被记录违章");
#endif
            TimeToCharArray(Sys_CalendarTime,FarLightViolence1.etime);//记录违章结束时间
            MX25L128_Illegal_Write(&FarLightViolence1);//将违章数据写入Flash中
            FarLightStatus++;
        }
        break;

    case 4:
    {
        if(Car_InputCheck.Value_HighBeam == RESET || //关闭远光
                DataCollectorMsg.MsgHighBeams != SET ||         //非禁用远光区域
                IsDayOrNight() == 0 )                //非夜间
        {
            if(DataCollectorMsg.MsgHighBeams != SET || //非禁用远光区域
                    IsDayOrNight() == 0 )   //非夜间
            {
                FarLightStatus = 0;
            }
            else
            {
                FarLightStatus = 1;
            }
        }
        break;
    }

    default :
        FarLightStatus = 0;
        break;
    }
}


/*******************************************************************************
* Function Name  : LimitFarLightInMeet
* Description    : 会车时远光灯检测
*******************************************************************************/
void FarLightInMeet(void)
{
    static Illegal_Def             FarLightViolence2;         //用于保存滥用远光违章信息
    static volatile unsigned char  FarLightMeetStatus = 0;
    static volatile unsigned int   FarLightWarnTime2 = 0;

    switch(FarLightMeetStatus) {
    case 0: {
        if( Flag_ChangeLight == SET ) {
            if( Car_InputCheck.Value_HighBeam == SET ) {
                // 远灯打开情况下收到变光要求
                XFS_WriteBuffer(xfs_auto, "对向来车,请使用近光灯");
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "对向来车,请使用近光灯");
#endif
                FarLightViolence2.ino = IllegalUseFarLight;                  // 违规使用远光灯
                TimeToCharArray(Sys_CalendarTime, FarLightViolence2.stime);  // 记录违章开始时间
                FarLightWarnTime2 = Sys_TimeCount;   // 提醒时间
                FarLightMeetStatus = 1;
            }
            else {
                Flag_ChangeLight = RESET;
                FarLightMeetStatus = 3;
                FarLightWarnTime2 = Sys_TimeCount;   // 提醒时间
            }
        }
        break;
    }

    case 1: {
        if( Sys_TimeCount - FarLightWarnTime2 >= m_carinfo.WZ05_TimeOver ) {
            FarLightMeetStatus = 2; // 连续8sec未关远灯, Goto记违章
        }
        else {
            if( Car_InputCheck.Value_HighBeam == RESET) {
                // 远灯关闭
                Flag_ChangeLight = RESET;
                FarLightMeetStatus = 0;
            }
        }
        break;
    }

    case 2: {
        XFS_WriteBuffer(xfs_auto, "您因违规使用远光灯,已被记录违章");
#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "您因违规使用远光灯,已被记录违章");
#endif
        FarLightViolence2.ino = IllegalUseFarLight;          // 违规使用远光灯
        TimeToCharArray(Sys_CalendarTime, FarLightViolence2.etime);     // 记录违章结束时间
        MX25L128_Illegal_Write(&FarLightViolence2);           // 将违章数据写入Flash中

        Flag_ChangeLight = RESET;
        FarLightMeetStatus = 0;
        break;
    }

    case 3:
        if( Sys_TimeCount - FarLightWarnTime2 >= m_carinfo.WZ05_TimeOver ) {
            Flag_ChangeLight = RESET;
            FarLightMeetStatus = 0;
        }
        else {
            if( Car_InputCheck.Value_HighBeam == SET ) {
                XFS_WriteBuffer(xfs_auto, "对向来车,请使用近光灯");
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "对向来车,请使用近光灯");
#endif
                FarLightViolence2.ino = IllegalUseFarLight;                  // 违规使用远光灯
                TimeToCharArray(Sys_CalendarTime, FarLightViolence2.stime);  // 记录违章开始时间
                FarLightWarnTime2 = Sys_TimeCount;
                FarLightMeetStatus = 1; // 连续8sec未关远灯, Goto记违章
            }
        }
        break;

    default :
        break;
    }
}


/*******************************************************************************
* Function Name  : DriverVerify
* Description    : 验证电子驾照、语音提示、记录电子驾照
******************************************************************************/
void DriverVerify(void)
{
    static Illegal_Def             NoDriverLicViolence;         // 用于保存无证驾驶违章信息
    static volatile unsigned int   NoDriverLicTime = 0;         // 无证驾驶时间
    static volatile unsigned int   TmpTime = 0;
    static volatile unsigned int   IgniteTime = 0;

    if( m_var.Flag_ApplyDriver == SET && FlagTerminalStat == '4' && DriverLicIndex < 10 ) {
        DriverLicIndex = 10;
        m_var.Flag_ApplyDriver = SET;
        m_var.Apply1_TimeOver = 0;
        m_var.FollowCar_TimeOver = m_carinfo.FollowCar_TimeOver;  // 随车超时: 10分钟
    }

    switch(DriverLicIndex) {
    case 0: {
        if( Car_InputCheck.Value_Ignite == SET ) {  // 车辆点火
            TmpTime = Sys_TimeCount;
            DriverLicIndex++;
        }
        break;
    }

    case 1: {
        if( is_emptyApplyData(&ApplyLicense) == 1 && FlagTerminalStat != '3' ) {
            DriverLicIndex = 10;
        }
        else {
            if(Car_InputCheck.Value_Ignite == RESET) {
                ApplyLicInit();
                DriverLicIndex = 0;
            }
            if(Sys_TimeCount - TmpTime >= m_carinfo.JZ01_TimeOver ) {
                XFS_WriteBuffer(xfs_auto, "请确认驾驶员,否则记录无证驾驶");
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                     Debug_Printf(SET, "%d=请确认驾驶员,否则记录无证驾驶, TerminalStatus=%d", DriverLicIndex,TerminalStatus);
#endif
                NoDriverLicViolence.ino = ino_NoDriveLic;//违章类型
                TimeToCharArray(Sys_CalendarTime, NoDriverLicViolence.stime);//违章开始时间

                DriverLicIndex++;
            }
        }
        break;
    }

    case 2: {
        if( is_emptyApplyData(&ApplyLicense) == 1 && FlagTerminalStat != '3' ) {
            DriverLicIndex = 10;
        }
        else {
            if( TerminalStatus == Status_Run ) {
                TmpTime = Sys_TimeCount;
                DriverLicIndex++;
            }
            else {
                if( Car_InputCheck.Value_Ignite == RESET ) {
                    ApplyLicInit();
                    DriverLicIndex = 0;
                }
            }
        }
        break;
    }

    case 3: {
        if( is_emptyApplyData(&ApplyLicense) == 1 && FlagTerminalStat != '3' ) {
            DriverLicIndex = 10;
        }
        else {
            if( Sys_TimeCount - TmpTime >= m_carinfo.JZ01_TimeOver ) { //外场演示恢复
                XFS_WriteBuffer(xfs_auto, "请确认驾驶员,否则记录无证驾驶");
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "%d=请确认驾驶员,否则记录无证驾驶, TerminalStatus=%d", DriverLicIndex,TerminalStatus);
#endif
                DriverLicIndex++;
            }
			      DriverLicIndex++;  //外场演示注释
        }
        break;
    }

    case 4: {
        if( is_emptyApplyData(&ApplyLicense) == 1 && FlagTerminalStat != '3' ) {
            DriverLicIndex = 10;
        }
        else {
            if( TerminalStatus == Status_Run ) {
                if( Sys_TimeCount - TmpTime >= m_carinfo.JZ02_TimeOver ) { //5 *60 ) {
                    XFS_WriteBuffer(xfs_auto, "您涉嫌无证驾驶,已被记录违章");
#ifdef __DEBUG
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf(SET, "您涉嫌无证驾驶,已被记录违章");
#endif
                    Flag_NoDriverLic = SET;
                    DriverLicIndex++;
                    TimeToCharArray(Sys_CalendarTime, NoDriverLicViolence.etime);//违章结束时间
                    MX25L128_Illegal_Write(&NoDriverLicViolence); //将违章数据写入Flash中
                }
            }
            else {
                if( Car_InputCheck.Value_Ignite == RESET ) {
                    ApplyLicInit();
                    DriverLicIndex = 0;
                }
            }
        }
        break;
    }

    case 5: {
        if( Car_InputCheck.Value_Ignite == SET ) {
            IgniteTime = Sys_TimeCount;

            if( is_emptyApplyData(&ApplyLicense) == 1 && FlagTerminalStat != '3' ) {
                DriverLicIndex = 10;
            }
            else {
                if( Sys_TimeCount - TmpTime >= 2*60 ) {
                    TmpTime = Sys_TimeCount;

                    XFS_WriteBuffer(xfs_auto, "请确认驾驶员,停止无证驾驶");
#ifdef __DEBUG
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf(SET, "%d=请确认驾驶员,停止无证驾驶", DriverLicIndex);
#endif
                }
            }
        }
        else {
            if( Sys_TimeCount - IgniteTime > 8 ) {
                ApplyLicInit();
                DriverLicIndex = 0;
            }
        }
        break;
    }

    case 10: {
        if( Car_InputCheck.Value_Ignite == SET ) {
            Flag_NoDriverLic = RESET;
            m_var.Flag_ApplyDriver = SET;
            GetApplyData(&ApplyLicense, m_carinfo.Driver_ID[0]);
            XFS_WriteBuffer( xfs_num, "尾号为%s的驾照已登录为驾驶员", &m_carinfo.Driver_ID[0][__DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ - 4] );
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET) {
                Debug_Printf( SET, "尾号为%s的驾照已登录为驾驶员", &m_carinfo.Driver_ID[0][__DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ - 4] );
            }
#endif
            DriverLicIndex++;
        }
        else {
            DriverLicIndex = 0;
        }
        break;
    }

    case 11: {
        if( TerminalStatus == Status_Park || m_var.Flag_ApplyDriver == RESET ) {
            TmpTime = Sys_TimeCount;
            ApplyLicInit();
            DriverLicIndex = 0;
        }
        break;
    }

    default :
        DriverLicIndex = 0;
        break;
    }
}


/*******************************************************************************
* Function Name  : void PreventSteal(void)
* Description    : 防盗验证并报警
******************************************************************************/
void PreventSteal(void)
{
    static u8 PreventSteal_Index = 0;//索引值
    static volatile unsigned int OpenDoorTime = 0;//开门时间

    switch (PreventSteal_Index) {
    case 0: {
        if( Car_InputCheck.Value_DriverDoor == SET &&
                Flag_ApplyResult == RESET ) {
            OpenDoorTime = Sys_TimeCount;
            PreventSteal_Index = 1;
        }
        break;
    }

    case 1: {
        if( Flag_ApplyResult == SET ) {
            XFS_WriteBuffer(xfs_auto, "防盗验证通过");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "防盗验证通过");
#endif
            PreventSteal_Index = 3;
        }
        else if((Sys_TimeCount - OpenDoorTime) >= m_carinfo.FD01_TimeOver ) {
            //XFS_WriteBuffer(xfs_auto, "请进行防盗验证");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "请进行防盗验证");
#endif

            PreventSteal_Index = 2;
        }
        break;
    }

    case 2: {
        if(Flag_ApplyResult == SET)	{
            //XFS_WriteBuffer(xfs_auto, "防盗验证通过");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "防盗验证通过");
#endif
            PreventSteal_Index = 3;
        }
        else if ((Sys_TimeCount - OpenDoorTime) >= m_carinfo.FD02_TimeOver ) {
            if( Flag_ApplyResult != SET ) {
                GTW_BurglarAlarm.Flag_Main = SET;   // 设置防盗报警标志
                GTW_BurglarAlarm.Flag_Call = SET;   //
                GTW_BurglarAlarm.Flag_SMS  = SET;   //
#ifdef __DEBUG
                //XFS_WriteBuffer(xfs_auto, m_var.Flag_InitDone_M35 == SET ? "已报警" : "触发防盗警报" );
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, m_var.Flag_InitDone_M35 == SET ? "已报警" : "触发防盗警报" );
#endif
            }
            PreventSteal_Index = 3;
        }
        break;
    }

    case 3: {
        if(TerminalStatus == Status_Park &&
                (Sys_TimeCount - OpenDoorTime) >= 10 * 60)
        {
            PreventSteal_Index = 0;
            Flag_ApplyResult = RESET;
        }
        break;
    }

    default:
        break;
    }
}


/*******************************************************************************
* Function Name  : DrivePlayPhone
* Description    : 开车玩手机,
*******************************************************************************/
void DrivePlayPhone(void)
{
    static Illegal_Def             DrivePlayPhoneViolence;     // 开车玩手机
    static volatile unsigned int   Time_DrivePlayPhoneWarn = 0;
    static volatile unsigned char  DrivePlayPhoneStatus;
    static volatile unsigned char  DrivePlayPhoneCount = 0;    // 开车玩手机记录到的次数
    switch(DrivePlayPhoneStatus) {
    case 0: {
        if(TerminalStatus != Status_Run) {
            Flag_DrivePlayPhone = RESET;
            DrivePlayPhoneStatus = 0;
        }
        else {
            if(Flag_DrivePlayPhone == SET ) {
                Flag_DrivePlayPhone = RESET;

                XFS_WriteBuffer(xfs_auto, "请您停止违章使用手机,否则将被记录违章");// 进行语音提醒
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "请您停止违章使用手机,否则将被记录违章");
#endif
                DrivePlayPhoneCount++;  // 次数加一次
                TimeToCharArray(Sys_CalendarTime, DrivePlayPhoneViolence.stime); // 记录违章开始时间
                Time_DrivePlayPhoneWarn = Sys_TimeCount;// 记录提醒时间
                DrivePlayPhoneStatus++; // 跳转到下一步
            }
        }
        break;
    }

    case 1: {
        if(Sys_TimeCount - Time_DrivePlayPhoneWarn > 10) {
            DrivePlayPhoneStatus++;//跳转到下一步
        }
    }
    case 2: {
        if((Sys_TimeCount - Time_DrivePlayPhoneWarn > (5*60)) || //检测到时间大于5分钟
                TerminalStatus != Status_Run)                    //未行驶状态
        {
            Flag_DrivePlayPhone = RESET;
            DrivePlayPhoneStatus = 0;
            break;
        }

        if(Flag_DrivePlayPhone == SET ) { //开车玩手机
            Flag_DrivePlayPhone = RESET;
            XFS_WriteBuffer(xfs_auto, "您因违章使用手机,已被记录违章");//进行语音提醒
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "您因违章使用手机,已被记录违章");
#endif
            TimeToCharArray(Sys_CalendarTime,DrivePlayPhoneViolence.stime); //记录违章开始时间
            TimeToCharArray(Sys_CalendarTime,DrivePlayPhoneViolence.etime); //记录违章结束时间
            DrivePlayPhoneViolence.ino = DrivePlayPhone_enum;               //违章类型
            MX25L128_Illegal_Write(&DrivePlayPhoneViolence);                 //将违章数据写入Flash中
            DrivePlayPhoneStatus = 0;
        }
        break;
    }
    default :
        break;
    }
}


/*******************************************************************************
* Function Name  : FatiguDriving
* Description    : 疲劳驾驶检测
*******************************************************************************/
void FatiguDriving(void)
{
    static Illegal_Def             FatiguDriveViolence;
    static volatile unsigned char  FatiguDrivingStatus;
    static volatile unsigned int   Time_CarRun = 0;
    static volatile unsigned char  TimeType = day_time;
    static volatile unsigned int   Time_CarStop = 0;
    static volatile unsigned int   Time_Warn;
    switch(FatiguDrivingStatus)
    {
    case 0:
        if(TerminalStatus == Status_Run) { //车辆行驶
            Time_CarRun = Sys_TimeCount;//记录行驶时间
            TimeToCharArray(Sys_CalendarTime,FatiguDriveViolence.stime);//记录违章开始时间
            if(Sys_CalendarTime.hour>= 7 && Sys_CalendarTime.hour<19) {
                TimeType = day_time;
            }
            else {
                TimeType = night_time;
            }
            FatiguDrivingStatus ++;
        }
        break;
    case 1:
        if(TerminalStatus != Status_Run) { //车辆停止
            Time_CarStop = Sys_TimeCount; //记录停止时间
            FatiguDrivingStatus++;
            break;
        }

        if(TimeType == day_time) {
            if(Sys_TimeCount - Time_CarRun >= 3.5*60*60) { //3.5小时
                Time_Warn = Sys_TimeCount;
                FatiguDrivingStatus = 3;
                break;
            }
        }
        else {
            if(Sys_TimeCount - Time_CarRun >= 1.5*60*60) { //1.5小时
                Time_Warn = Sys_TimeCount;
                FatiguDrivingStatus = 3;
                break;
            }
        }
        break;
    case 2:
        if(TerminalStatus == Status_Run) { //车辆行驶
            Time_CarRun = Sys_TimeCount; //记录行驶时间
            TimeToCharArray(Sys_CalendarTime,FatiguDriveViolence.stime);//记录违章开始时间
            FatiguDrivingStatus = 1;
            break;
        }
        if(Sys_TimeCount - Time_CarStop >= (20 * 60)) { //20分钟后
            FatiguDrivingStatus  = 0;//重新记录行驶时间
            break;
        }
        break;
    case 3:
        XFS_WriteBuffer(xfs_auto, "您已连续驾驶超过3小时30分钟，请您在前方适当位置停车休息20分钟以上再继续驾驶。若连续驾驶超过4小时，将被记录违章。如需更换驾驶员，请验证新驾驶员电子驾照");
        Time_Warn = Sys_TimeCount;
        FatiguDrivingStatus++;
        break;
    case 4:
        if(TerminalStatus != Status_Run) { //车辆停止

            break;
        }
        if(Sys_TimeCount - Time_Warn >= (15*60)) { //15分钟后
            FatiguDrivingStatus++;
        }
        break;
    case 5:
        XFS_WriteBuffer(xfs_auto, "您已连续驾驶超过3小时45分钟，请您在前方适当位置停车休息20分钟以上再继续驾驶。若连续驾驶超过4小时，将被记录违章。如需更换驾驶员，请验证新驾驶员电子驾照");
        Time_Warn = Sys_TimeCount;
        FatiguDrivingStatus++;
        break;
    case 6:
        if(TerminalStatus != Status_Run) { //车辆停止

            break;
        }
        if(Sys_TimeCount - Time_CarRun > (4*60*60)) {
            FatiguDrivingStatus = 0;
            break;
        }
        if(Sys_TimeCount - Time_Warn >= (10*60)) { //10分钟后
            FatiguDrivingStatus++;
        }
        break;
    case 7:
        XFS_WriteBuffer(xfs_auto, "您已连续驾驶超过3小时55分钟，请您在前方适当位置停车休息20分钟以上再继续驾驶。若连续驾驶超过4小时，将被记录违章。如需更换驾驶员，请验证新驾驶员电子驾照");
        Time_Warn = Sys_TimeCount;
        FatiguDrivingStatus++;
        break;
    case 8:
        if(TerminalStatus != Status_Run) { //车辆停止

            break;
        }
        if(Sys_TimeCount - Time_CarRun > (4*60*60)) {
            FatiguDrivingStatus = 0;
            break;
        }
        if(Sys_TimeCount - Time_Warn >= (15*60)) { //15分钟后
            FatiguDrivingStatus++;
        }
        break;
    case 9:
        XFS_WriteBuffer(xfs_auto, "您因疲劳驾驶，已被记录违章");
        FatiguDriveViolence.ino = DriveNoSeatBelt;          //违章类型
        FatiguDrivingStatus++;
        break;
    case 10:
        if(TerminalStatus == Status_Park) {
            FatiguDrivingStatus++;
        }
        break;
    case 11:
        if(TerminalStatus != Status_Park) {
            FatiguDrivingStatus = 10;
        }
        if(Sys_TimeCount - Time_CarStop > (20 * 60)) {
            TimeToCharArray(Sys_CalendarTime,FatiguDriveViolence.stime);//违章结束时间
            MX25L128_Illegal_Write(&FatiguDriveViolence);               //将违章数据写入Flash中
            FatiguDrivingStatus = 0;
        }
        break;
    default :
        break;
    }
}
/*******************************************************************************
* Function Name  : BusRunAtNight
* Description    : 大客车夜间限行
*******************************************************************************/
void BusRunAtNight(void)
{
    static Illegal_Def             LimitBusRunViolence;      //用于保存大客车违章信息
    static volatile unsigned int   Time_LimitRunWarn1 = 0;
    static volatile unsigned char  Flag_LimitRunStatus;
    static volatile unsigned char  Flag_BelowThirdRoad = SET;//标志是否在三级以下公路

    if(Sys_CalendarTime.hour == 01 &&        //1点钟
            (Sys_CalendarTime.min == 30 || Sys_CalendarTime.min == 40 || Sys_CalendarTime.min == 50) && //30分,40分,50分
            Sys_CalendarTime.sec == 0 &&     //0秒
            TerminalStatus == Status_Run &&  //行车状态
            Flag_BelowThirdRoad == SET     //在3级以下公路
      )              //语音模块空闲
    {
        XFS_WriteBuffer(xfs_auto, "请您凌晨2点至5点停运或更换驾驶员,否则记录违章");
#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "请您凌晨2点至5点停运或更换驾驶员,否则记录违章");
#endif
    }


    switch(Flag_LimitRunStatus)
    {
    case 0:
        if(Flag_LimitRun == SET &&             //限行信号
                TerminalStatus == Status_Run &&//行车状态
                Flag_BelowThirdRoad == SET &&  //在3级以下公路
                Sys_CalendarTime.hour >= 2 && Sys_CalendarTime.hour <= 5 //夜间2点至5点
          )            //语音模块空闲
        {
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "您在夜间进入三级以下公路,请尽快停车或者离开此区域,否则记录违章");
#endif
            XFS_WriteBuffer(xfs_auto, "您在夜间进入三级以下公路,请尽快停车或者离开此区域,否则记录违章");
            Time_LimitRunWarn1 = Sys_TimeCount;
            Flag_LimitRunStatus++;
        }
        break;
    case 1:
        if(Sys_TimeCount - Time_LimitRunWarn1 >= (20*60))//20分钟后
        {
            Flag_LimitRunStatus++;
        }
        break;

    case 2:
        if(Sys_CalendarTime.hour > 5 ||        //5点以后
                Flag_BelowThirdRoad == RESET ||//不在3级以下公路
                TerminalStatus != Status_Run)  //非行驶
        {
            Flag_LimitRunStatus = 0;
            break;
        }

        if( Flag_BelowThirdRoad == SET )  //在3级以下公路
        {
            XFS_WriteBuffer(xfs_auto, "您因违反大客车夜间行驶规定,已被记录违章");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "您因违反大客车夜间行驶规定,已被记录违章");
#endif
            LimitBusRunViolence.ino = BusIllegalRun;//记录违章类型
            TimeToCharArray(Sys_CalendarTime,LimitBusRunViolence.stime);//记录违章开始时间

            Flag_LimitRunStatus++;
        }
        break;
    case 3:
        if(TerminalStatus == Status_Park)
        {
            Flag_LimitRunStatus++;
            Time_LimitRunWarn1 = Sys_TimeCount;//记下当前时间
            break;
        }
        if((Sys_CalendarTime.hour > 5 &&  Flag_BelowThirdRoad == RESET) ||//5点以后//不在3级以下公路
                IsDayOrNight() == night_time)//白天
        {
            TimeToCharArray(Sys_CalendarTime,LimitBusRunViolence.etime);//记录违章结束时间
            MX25L128_Illegal_Write(&LimitBusRunViolence);//将违章数据写入Flash中
            Flag_LimitRunStatus = 0;
            break;
        }
        break;
    case 4:
        if(Sys_CalendarTime.hour > 5 &&  //5点以后
                Flag_BelowThirdRoad == RESET )//不在3级以下公路
        {
            TimeToCharArray(Sys_CalendarTime,LimitBusRunViolence.etime);//记录违章结束时间
            MX25L128_Illegal_Write(&LimitBusRunViolence);//将违章数据写入Flash中
            Flag_LimitRunStatus = 0;
            break;
        }
        if(Sys_TimeCount - Time_LimitRunWarn1 >= (30 * 60))//30分钟以后
        {
            Flag_LimitRunStatus++;
        }
    case 5:
        TimeToCharArray(Sys_CalendarTime,LimitBusRunViolence.etime);//记录违章结束时间
        MX25L128_Illegal_Write(&LimitBusRunViolence);            //将违章数据写入Flash中
        Flag_LimitRunStatus = 0;
        break;
    default :
        Flag_LimitRunStatus = 0;
        break;
    }
}


/*******************************************************************************
* Function Name  : VehicleLimitRun
* Description    : 车辆限行
*******************************************************************************/
void VehicleLimitRun(void)
{
    static Illegal_Def             LimitRunViolence;         //用于保存禁用喇叭违章信息
    static volatile unsigned char  Flag_LimitRunStatue = 0;
    static volatile unsigned int   Time_LimitRunWarn2 = 0;
    static volatile unsigned char  Flag_CloseToLimitArea = 0;
    static volatile unsigned int   latitude;
    static volatile unsigned int   longitude;

    switch (Flag_LimitRunStatue)
    {
    case 0:
        if(Flag_CloseToLimitArea == SET &&   //接近限行区域
                TerminalStatus == Status_Run)//车辆行驶
        {
            XFS_WriteBuffer(xfs_auto, "您已接近限行区域,请及时改道");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "您已接近限行区域,请及时改道");
#endif
            Flag_LimitRunStatue++;
        }
        break;
    case 1:
        if(Flag_LimitRun == RESET ||     //非限行区域
                TerminalStatus != Status_Run)//非行车状态
        {
            Flag_LimitRunStatue = 0;
            break;
        }

        if(Flag_LimitRun == SET &&            //限行区域
                TerminalStatus == Status_Run )//行车状态
        {
            XFS_WriteBuffer(xfs_auto, "您已进入限行区域,请尽快离开");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "您已进入限行区域,请尽快离开");
#endif

            Time_LimitRunWarn2 = Sys_TimeCount;//记录提醒时间
            TimeToCharArray(Sys_CalendarTime,LimitRunViolence.stime);//记录违章开始时间
            if(gpsx.gpssta == 1 || gpsx.gpssta == 2) { //GPS定位成功

                latitude = gpsx.latitude;
                longitude = gpsx.longitude;
            }
            Flag_LimitRunStatue++;

        }
        break;
    case 2:
        if(Flag_LimitRun == RESET &&            //限行区域
                TerminalStatus != Status_Run ) { //行车状态

            break;
        }

        if(Sys_TimeCount - Time_LimitRunWarn2 > (5*60)) {
            Flag_LimitRunStatue++;
            break;
        }

        if(gpsx.gpssta == 1 || gpsx.gpssta == 2) { //GPS定位成功
            if(GetDistance(latitude,longitude,gpsx.latitude,gpsx.longitude) >= 1000)//距离大于1000米
            {
                Flag_LimitRunStatue++;
            }
        }
        break;
    case 3:
        XFS_WriteBuffer(xfs_auto, "您因违反限行规定,已被记录违章");
#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "您因违反限行规定,已被记录违章");
#endif
        LimitRunViolence.ino = IllegalRun;               //记录违章类型
        TimeToCharArray(Sys_CalendarTime,LimitRunViolence.etime);  //记录违章结束时间
        MX25L128_Illegal_Write(&LimitRunViolence);         //将违章数据写入Flash中
        Flag_LimitRunStatue ++;
        break;
    case 4:
        Flag_LimitRunStatue = 0;
        break;
    default :
        Flag_LimitRunStatue = 0;
        break;
    }
}
/*******************************************************************************
* Function Name  : LimitSpeed
* Description    : 限速
*******************************************************************************/
void LimitSpeed(void)
{
    static Illegal_Def             SpeedViolence;            //用于保存限速违章信息
    static volatile unsigned int   SpeedWarnTime = 0;
    static volatile unsigned char  Flag_SpeedStatue = 0;

    switch (Flag_SpeedStatue)
    {
    case 0:
        if(Flag_LimitSpeed == SET) {
            if(gpsx.gpssta == 1 || gpsx.gpssta == 2) {//GPS定位成功
                if(GetSpeed() > (Dir_SGroup.Max_Speed * 11 / 10 )) { //超出10%
                    Flag_SpeedStatue ++;
                }
            }
        }
        break;
    case 1:
        XFS_WriteBuffer(xfs_auto, "当前限速XX,您已超过10%,请控制车速,否则记录违章");
#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "当前限速XX,您已超过百分之十,请控制车速,否则记录违章");
#endif
        TimeToCharArray(Sys_CalendarTime,SpeedViolence.stime);//违章开始时间
        SpeedWarnTime = Sys_TimeCount;
        Flag_SpeedStatue++;
        break;
    case 2:
        if(gpsx.gpssta == 1 || gpsx.gpssta == 2) {
            if(GetSpeed() <= (Dir_SGroup.Max_Speed * 11 / 10)) {
                Flag_SpeedStatue = 0;
                break;
            }
        }

        if(Flag_LimitSpeed == RESET) {
            Flag_SpeedStatue = 0;
            break;
        }

        if(Sys_TimeCount - SpeedWarnTime > 60 )//30秒
        {
            XFS_WriteBuffer(xfs_auto, "您因超速,已被记录违章");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "您因超速,已被记录违章");
#endif
            SpeedWarnTime = Sys_TimeCount;
            SpeedViolence.ino = OverSpeed;//超速
            Flag_SpeedStatue++;
        }
        break;
    case 3:
        if(Flag_LimitSpeed == SET) {
            if(gpsx.gpssta == 1 || gpsx.gpssta == 2) {//GPS定位成功
                if(GetSpeed() > (Dir_SGroup.Max_Speed * 11 / 10 )) { //超出10%
                    Flag_SpeedStatue ++;
                    break;
                }
            }
        }
        if(Flag_LimitSpeed == RESET) {
            Flag_SpeedStatue = 0;
            break;
        }
        if(Sys_TimeCount - SpeedWarnTime >= (5 * 60)) {
            TimeToCharArray(Sys_CalendarTime,SpeedViolence.etime);//违章结束时间
            MX25L128_Illegal_Write(&SpeedViolence);//将违章数据写入Flash中
            Flag_SpeedStatue = 0;
        }
        break;
    default :
        Flag_SpeedStatue = 0;
        break;
    }
}

/*******************************************************************************
* Function Name  : LimitTrumpet
* Description    : 禁用喇叭检测
*******************************************************************************/
void LimitTrumpet(void)
{
    static Illegal_Def             TrumpetViolence;          //用于保存禁用喇叭违章信息
    static volatile unsigned char  Flag_TrumpetStatue = 0;
    static volatile unsigned int   Time_FirstTrumpet = 0;
    static volatile unsigned int   TempTime = 0;

    switch (Flag_TrumpetStatue)
    {
    case 0:
        if(DataCollectorMsg.MsgWhistle == SET  )        //禁鸣区
        {
            XFS_WriteBuffer(xfs_auto, "当前已进入禁鸣区域");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "当前已进入禁鸣区域");
#endif
            Flag_TrumpetStatue++;
        }
        break;
    case 1:
        if(DataCollectorMsg.MsgWhistle == RESET) //非禁鸣区
        {
            XFS_WriteBuffer(xfs_auto, "当前已离开禁鸣区域");
            Flag_TrumpetStatue = 0;
            break;
        }
        if( Car_InputCheck.Value_Honk == SET )//当第一次按喇叭时进行语音提醒
        {
            XFS_WriteBuffer(xfs_auto, "此区域禁鸣");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "此区域禁鸣");
#endif
            Time_FirstTrumpet = Sys_TimeCount;//记录第一次使用喇叭的时间
            Flag_TrumpetStatue++;
        }
        break;
    case 2:
        if(Sys_TimeCount - Time_FirstTrumpet >= 10) { //第二次按喇叭时,进行语音提示,并记录违章
            Flag_TrumpetStatue++;
        }
        break;
    case 3:
        if(DataCollectorMsg.MsgWhistle == RESET)
        {
            XFS_WriteBuffer(xfs_auto, "当前已离开禁鸣区域");
            Flag_TrumpetStatue = 0;
            break;
        }
        if( Car_InputCheck.Value_Honk == SET )
        {
            XFS_WriteBuffer(xfs_auto, "您因违反禁鸣规定,已被记录违章");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "您因违反禁鸣规定,已被记录违章");
#endif
            TrumpetViolence.ino = IllegalUseTrumpet;
            TimeToCharArray(Sys_CalendarTime, TrumpetViolence.stime);
            TimeToCharArray(Sys_CalendarTime, TrumpetViolence.etime);
            MX25L128_Illegal_Write(&TrumpetViolence);
            TempTime = Sys_TimeCount;
            Flag_TrumpetStatue ++;
        }
        break;
    case 4:
        if(Sys_TimeCount - TempTime > 60) { //1分钟后再次检测
            Flag_TrumpetStatue = 0;
        }
        break;
    default :
        Flag_TrumpetStatue = 0;
        break;
    }
}
/*******************************************************************************
* Function Name  : IllegalPark
* Description    : 检测违停
*******************************************************************************/
void IllegalPark(void)
{
    static unsigned char  IllegalParkStatus;
    static unsigned int   ill_latitude;      //纬度 分扩大100000倍,实际要除以100000
    static unsigned int   ill_longitude;     //经度 分扩大100000倍,实际要除以100000
    static unsigned int   gps_ok_time;

    if( m_carinfo.Count_IllegalPark > 2 || GTW_IllegalPark.Flag_Main == RESET ) {
        GTW_IllegalPark.Flag_Main = RESET;
        GTW_IllegalPark.Sec_OverTime = 0;
        IllegalParkStatus = 0;
        return;
    }

    switch(IllegalParkStatus) {
    case 0:
        if(TerminalStatus == Status_Run) {
            IllegalParkStatus++;//跳到下一个状态
        }
        break;

    case 1:
        if(gpsx.gpssta == 1 || gpsx.gpssta == 2)
        {
            ill_latitude = gpsx.latitude;
            ill_longitude = gpsx.longitude;
            gps_ok_time = Sys_TimeCount;
            if(TerminalStatus == Status_Run) { //车辆行驶
                IllegalParkStatus++;//跳到下一个状态
            }
        }
        break;

    case 2:
        if((Sys_TimeCount - gps_ok_time) % 5 == 0) { // 5s计算一次距离
            if(GetDistance( ill_latitude, ill_longitude, gpsx.latitude, gpsx.longitude ) >= 500) { //距离大于500米
                XFS_WriteBuffer(xfs_auto, "您已离开五百米,可申请撤销违停记录");
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "您已离开五百米,可申请撤销违停记录");
#endif
                IllegalParkStatus++;
            }
        }
        break;

    case 3:
        if( m_carinfo.Count_IllegalPark < 2 ) {
            Flag_IllegalParkCancel = SET;       // 设置撤销违停发送标志
            m_carinfo.Count_IllegalPark++;
            MX25L128_CreatSaveTask();           // 保存违停记录
        }

    default:
        IllegalParkStatus = 0;
        break;

    }
}

/*******************************************************************************
* Function Name  : MakeViolation
* Description    : 制造违章数据
*******************************************************************************/
void MakeViolation(unsigned short count)
{
    unsigned short i;
    Illegal_Def makeill;

    makeill.ewhemi = 'E';                               // 东经/西经,E:东经;W:西经
    makeill.longitude = (double)112.34567;              // 违章时经度
    makeill.nshemi = 'N';                               // 北纬/南纬,N:北纬;S:南纬
    makeill.latitude  = (double)28.12345;               // 违章时纬度

    for(i=0; i<count; i++) {
        TimeToCharArray(Sys_CalendarTime, makeill.stime);       // 记录违章开始时间
        TimeToCharArray(Sys_CalendarTime, makeill.etime);       // 记录违章结束时间
        makeill.ino = (i % 12);                                 // 违章类型
        MX25L128_Illegal_Write_Base( &makeill, SET);            // 将违章数据写入Flash中
    }
}
/****************************************END OF FILE**********************************/
