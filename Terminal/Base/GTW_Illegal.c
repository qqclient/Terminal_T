/******************** (C) COPYRIGHT soarsky ************************************
* File Name          : gtw_illegal.c
* Author             : csyxj@728920175@qq.com
* Version            : V0.1
* Date               : 2017-05-19
* Description        : Violation detection support function
*******************************************************************************/

#include "gtw_Head.h"

volatile unsigned char  DriverLicIndex = 0;          // ��֤���Ӽ���״̬��
volatile unsigned char  Flag_LimitSpeed = RESET;     // ���յ���ͳ������ź�
volatile unsigned char  Flag_LimitRun = RESET;       // ���յ������ź�
volatile unsigned char  Flag_ChangeLight = RESET;    // ���յ�����ź�
volatile unsigned char  Flag_DrivePlayPhone = RESET; // ���յ��������ֻ��ź�
volatile unsigned char  Flag_NoDriverLic = RESET;    // ��֤��ʻ��־
volatile unsigned char  Flag_ApplyResult = RESET;    // �յ�����������

/*******************************************************************************
* Function Name  : SafeBelt
* Description    : ��ȫ�����
*******************************************************************************/
void SafeBelt(void)
{
    static Illegal_Def             SafeBeltViolence;         //���ڱ��氲ȫ��Υ����Ϣ
    static volatile unsigned char  Flag_SafeBeltStatus = 0;
    static volatile unsigned int   SafeBelt_TimeOver = 0;
    unsigned int  latitude;             // γ�� ������100000��,ʵ��Ҫ����100000
    unsigned int  longitude;            // ���� ������100000��,ʵ��Ҫ����100000

    switch(Flag_SafeBeltStatus) {
    case 0: {
        if( Sys_TimeCount - Sys_RunTime > 30 ) {    // ����30���ڲ����
            if( Car_InputCheck.Value_DriverBelt == RESET &&     // δϵ��ȫ��
                    TerminalStatus == Status_Run )              // ��������ʻ״̬
            {
                Flag_SafeBeltStatus = 1;
                SafeBelt_TimeOver = Sys_TimeCount;              // ��¼ʱ��

                SafeBeltViolence.ino = DriveNoSeatBelt;         // Υ������
                TimeToCharArray(Sys_CalendarTime, SafeBeltViolence.stime);//��¼Υ�¿�ʼʱ��
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
                XFS_WriteBuffer(xfs_auto, "����ϵ�ð�ȫ��,�����¼Υ��");
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "����ϵ�ð�ȫ��,�����¼Υ��");
#endif
                SafeBelt_TimeOver = Sys_TimeCount;          // ��¼ʱ��
                Flag_SafeBeltStatus = 2;
            }
        }
        break;
    }

    case 2: {
        if( TerminalStatus != Status_Run ||                 // ����ʻ״̬
                Car_InputCheck.Value_DriverBelt == SET )    // ϵ�ð�ȫ��
        {
            Flag_SafeBeltStatus = 0;
            break;
        }

        if( GetSpeed() >= SPEED_MIN ) {
            // ��ʻ״̬
            if(gpsx.gpssta == 1 || gpsx.gpssta == 2) {  // GPS��λ�ɹ�
                latitude = gpsx.latitude;       // ��¼��ʼλ��
                longitude = gpsx.longitude;
            }
            Flag_SafeBeltStatus = 3;
						
        }
        break;
    }

    case 3: {
        if( TerminalStatus != Status_Run ||                 // ����ʻ״̬
                Car_InputCheck.Value_DriverBelt == SET )    // ϵ�ð�ȫ��
        {
            Flag_SafeBeltStatus = 0;
            break;
        }

        if( (m_carinfo.Flag_Simulation & 0x01) == SET ) {
            if( Sys_TimeCount - SafeBelt_TimeOver > m_carinfo.WZ04_TimeOver ) {//m_carinfo.WZ04_TimeOver  //�����ڳ��޸� ����10 �ⳡ�ָ�ʹ��m_carinfo.WZ04_TimeOver
                // ���� �ſ���2 һֱδ����ȥ��ʱ 1���ӽ���¼��ȫ��Υ��
                Flag_SafeBeltStatus = 4;
						
            }
            else if( Car_InputCheck.Value_SecondDoor == SET ) {
                // ���� �ſ���2 ģ����ʻ�뿪500�׼�¼��ȫ��Υ��
                Flag_SafeBeltStatus = 4;
            }
        }
        else {
            if( Sys_TimeCount - SafeBelt_TimeOver > m_carinfo.WZ04_TimeOver ) {
                // ������ʻ�ﵽ��¼��ϵ��ȫ��Υ�µ�ʱ��
                Flag_SafeBeltStatus = 4;
            }
            else {
                if(gpsx.gpssta == 1 || gpsx.gpssta == 2) {
                    // GPS��λ�ɹ�
                    if((Sys_TimeCount - SafeBelt_TimeOver) % 5 == 0) {
                        //5s����һ�ξ���
                        if(GetDistance(latitude, longitude, gpsx.latitude, gpsx.longitude) >= 500) {
                            //�������500��, ��¼��ȫ��Υ��
                            Flag_SafeBeltStatus = 4;
                        }
                    }
                }
            }
        }
        break;
    }

    case 4: {
        XFS_WriteBuffer(xfs_auto, "����δϵ��ȫ��,�ѱ���¼Υ��");
#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "����δϵ��ȫ��,�ѱ���¼Υ��");
#endif
        SafeBeltViolence.ino = DriveNoSeatBelt;          //Υ������
        TimeToCharArray(Sys_CalendarTime,SafeBeltViolence.etime);//��¼Υ�½���ʱ��
        MX25L128_Illegal_Write(&SafeBeltViolence);             //��Υ������д��Flash��
        Flag_SafeBeltStatus = 5;
    }
    break;

    case 5: {
        if( Car_InputCheck.Value_DriverBelt == SET ||
                TerminalStatus == Status_Park ||
                m_var.Flag_ApplyDriver == RESET        ) {
            Flag_SafeBeltStatus = 6;  //�ⳡ 0 �ڳ� 6 ֻ���һ�°�ȫ����ʾҪ��
        }
        break;
    }

    default :
        Flag_SafeBeltStatus = 6; //�ⳡ 0 �ڳ� 6 ֻ���һ�°�ȫ����ʾҪ��
        break;
    }
}


/*******************************************************************************
* Function Name  : LimitFarLightInArea
* Description    : �ڽ���Զ������Զ��Ƽ��
*******************************************************************************/
void FarLightInArea(void)
{
    static Illegal_Def             FarLightViolence1;         //���ڱ�������Զ��Υ����Ϣ
    static volatile unsigned char  FarLightStatus = 0;
    static volatile unsigned int   FarLightWarnTime1 = 0;

    switch(FarLightStatus) {
    case 0:
        if(DataCollectorMsg.MsgHighBeams == SET &&    //����Զ������
                IsDayOrNight() == night_time )//ҹ��
        {
            XFS_WriteBuffer(xfs_auto, "���ѽ������Զ������,�벻Ҫ����Զ���");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "���ѽ������Զ������,�벻Ҫ����Զ���");
#endif
            FarLightViolence1.ino = IllegalUseFarLight;       //Υ��ʹ��Զ���
            TimeToCharArray(Sys_CalendarTime,FarLightViolence1.stime);//��¼Υ�¿�ʼʱ��
            FarLightStatus++;
        }
        break;

    case 1:
        if( DataCollectorMsg.MsgHighBeams == RESET && //�ǽ���Զ������
                IsDayOrNight() == day_time )    //��ҹ��
        {
            FarLightStatus = 0;
            break;
        }
        if(Car_InputCheck.Value_HighBeam == SET ) //����Զ���
        {
            XFS_WriteBuffer(xfs_auto, "�����ر�Զ���,�����¼Υ��");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "�����ر�Զ���,�����¼Υ��");
#endif
            FarLightWarnTime1 = Sys_TimeCount;      //��¼�±�����ʱ��
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
        if( DataCollectorMsg.MsgHighBeams != SET || //�ǽ���Զ������
                IsDayOrNight() == day_time )     //��ҹ��
        {
            FarLightStatus = 0;
            break;
        }

        if(Car_InputCheck.Value_HighBeam == SET )//����Զ��
        {

            XFS_WriteBuffer(xfs_auto, "����Υ��ʹ��Զ���,�ѱ���¼Υ��");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "����Υ��ʹ��Զ���,�ѱ���¼Υ��");
#endif
            TimeToCharArray(Sys_CalendarTime,FarLightViolence1.etime);//��¼Υ�½���ʱ��
            MX25L128_Illegal_Write(&FarLightViolence1);//��Υ������д��Flash��
            FarLightStatus++;
        }
        break;

    case 4:
    {
        if(Car_InputCheck.Value_HighBeam == RESET || //�ر�Զ��
                DataCollectorMsg.MsgHighBeams != SET ||         //�ǽ���Զ������
                IsDayOrNight() == 0 )                //��ҹ��
        {
            if(DataCollectorMsg.MsgHighBeams != SET || //�ǽ���Զ������
                    IsDayOrNight() == 0 )   //��ҹ��
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
* Description    : �ᳵʱԶ��Ƽ��
*******************************************************************************/
void FarLightInMeet(void)
{
    static Illegal_Def             FarLightViolence2;         //���ڱ�������Զ��Υ����Ϣ
    static volatile unsigned char  FarLightMeetStatus = 0;
    static volatile unsigned int   FarLightWarnTime2 = 0;

    switch(FarLightMeetStatus) {
    case 0: {
        if( Flag_ChangeLight == SET ) {
            if( Car_InputCheck.Value_HighBeam == SET ) {
                // Զ�ƴ�������յ����Ҫ��
                XFS_WriteBuffer(xfs_auto, "��������,��ʹ�ý����");
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "��������,��ʹ�ý����");
#endif
                FarLightViolence2.ino = IllegalUseFarLight;                  // Υ��ʹ��Զ���
                TimeToCharArray(Sys_CalendarTime, FarLightViolence2.stime);  // ��¼Υ�¿�ʼʱ��
                FarLightWarnTime2 = Sys_TimeCount;   // ����ʱ��
                FarLightMeetStatus = 1;
            }
            else {
                Flag_ChangeLight = RESET;
                FarLightMeetStatus = 3;
                FarLightWarnTime2 = Sys_TimeCount;   // ����ʱ��
            }
        }
        break;
    }

    case 1: {
        if( Sys_TimeCount - FarLightWarnTime2 >= m_carinfo.WZ05_TimeOver ) {
            FarLightMeetStatus = 2; // ����8secδ��Զ��, Goto��Υ��
        }
        else {
            if( Car_InputCheck.Value_HighBeam == RESET) {
                // Զ�ƹر�
                Flag_ChangeLight = RESET;
                FarLightMeetStatus = 0;
            }
        }
        break;
    }

    case 2: {
        XFS_WriteBuffer(xfs_auto, "����Υ��ʹ��Զ���,�ѱ���¼Υ��");
#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "����Υ��ʹ��Զ���,�ѱ���¼Υ��");
#endif
        FarLightViolence2.ino = IllegalUseFarLight;          // Υ��ʹ��Զ���
        TimeToCharArray(Sys_CalendarTime, FarLightViolence2.etime);     // ��¼Υ�½���ʱ��
        MX25L128_Illegal_Write(&FarLightViolence2);           // ��Υ������д��Flash��

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
                XFS_WriteBuffer(xfs_auto, "��������,��ʹ�ý����");
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "��������,��ʹ�ý����");
#endif
                FarLightViolence2.ino = IllegalUseFarLight;                  // Υ��ʹ��Զ���
                TimeToCharArray(Sys_CalendarTime, FarLightViolence2.stime);  // ��¼Υ�¿�ʼʱ��
                FarLightWarnTime2 = Sys_TimeCount;
                FarLightMeetStatus = 1; // ����8secδ��Զ��, Goto��Υ��
            }
        }
        break;

    default :
        break;
    }
}


/*******************************************************************************
* Function Name  : DriverVerify
* Description    : ��֤���Ӽ��ա�������ʾ����¼���Ӽ���
******************************************************************************/
void DriverVerify(void)
{
    static Illegal_Def             NoDriverLicViolence;         // ���ڱ�����֤��ʻΥ����Ϣ
    static volatile unsigned int   NoDriverLicTime = 0;         // ��֤��ʻʱ��
    static volatile unsigned int   TmpTime = 0;
    static volatile unsigned int   IgniteTime = 0;

    if( m_var.Flag_ApplyDriver == SET && FlagTerminalStat == '4' && DriverLicIndex < 10 ) {
        DriverLicIndex = 10;
        m_var.Flag_ApplyDriver = SET;
        m_var.Apply1_TimeOver = 0;
        m_var.FollowCar_TimeOver = m_carinfo.FollowCar_TimeOver;  // �泵��ʱ: 10����
    }

    switch(DriverLicIndex) {
    case 0: {
        if( Car_InputCheck.Value_Ignite == SET ) {  // �������
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
                XFS_WriteBuffer(xfs_auto, "��ȷ�ϼ�ʻԱ,�����¼��֤��ʻ");
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                     Debug_Printf(SET, "%d=��ȷ�ϼ�ʻԱ,�����¼��֤��ʻ, TerminalStatus=%d", DriverLicIndex,TerminalStatus);
#endif
                NoDriverLicViolence.ino = ino_NoDriveLic;//Υ������
                TimeToCharArray(Sys_CalendarTime, NoDriverLicViolence.stime);//Υ�¿�ʼʱ��

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
            if( Sys_TimeCount - TmpTime >= m_carinfo.JZ01_TimeOver ) { //�ⳡ��ʾ�ָ�
                XFS_WriteBuffer(xfs_auto, "��ȷ�ϼ�ʻԱ,�����¼��֤��ʻ");
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "%d=��ȷ�ϼ�ʻԱ,�����¼��֤��ʻ, TerminalStatus=%d", DriverLicIndex,TerminalStatus);
#endif
                DriverLicIndex++;
            }
			      DriverLicIndex++;  //�ⳡ��ʾע��
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
                    XFS_WriteBuffer(xfs_auto, "��������֤��ʻ,�ѱ���¼Υ��");
#ifdef __DEBUG
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf(SET, "��������֤��ʻ,�ѱ���¼Υ��");
#endif
                    Flag_NoDriverLic = SET;
                    DriverLicIndex++;
                    TimeToCharArray(Sys_CalendarTime, NoDriverLicViolence.etime);//Υ�½���ʱ��
                    MX25L128_Illegal_Write(&NoDriverLicViolence); //��Υ������д��Flash��
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

                    XFS_WriteBuffer(xfs_auto, "��ȷ�ϼ�ʻԱ,ֹͣ��֤��ʻ");
#ifdef __DEBUG
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf(SET, "%d=��ȷ�ϼ�ʻԱ,ֹͣ��֤��ʻ", DriverLicIndex);
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
            XFS_WriteBuffer( xfs_num, "β��Ϊ%s�ļ����ѵ�¼Ϊ��ʻԱ", &m_carinfo.Driver_ID[0][__DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ - 4] );
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET) {
                Debug_Printf( SET, "β��Ϊ%s�ļ����ѵ�¼Ϊ��ʻԱ", &m_carinfo.Driver_ID[0][__DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ - 4] );
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
* Description    : ������֤������
******************************************************************************/
void PreventSteal(void)
{
    static u8 PreventSteal_Index = 0;//����ֵ
    static volatile unsigned int OpenDoorTime = 0;//����ʱ��

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
            XFS_WriteBuffer(xfs_auto, "������֤ͨ��");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "������֤ͨ��");
#endif
            PreventSteal_Index = 3;
        }
        else if((Sys_TimeCount - OpenDoorTime) >= m_carinfo.FD01_TimeOver ) {
            //XFS_WriteBuffer(xfs_auto, "����з�����֤");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "����з�����֤");
#endif

            PreventSteal_Index = 2;
        }
        break;
    }

    case 2: {
        if(Flag_ApplyResult == SET)	{
            //XFS_WriteBuffer(xfs_auto, "������֤ͨ��");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "������֤ͨ��");
#endif
            PreventSteal_Index = 3;
        }
        else if ((Sys_TimeCount - OpenDoorTime) >= m_carinfo.FD02_TimeOver ) {
            if( Flag_ApplyResult != SET ) {
                GTW_BurglarAlarm.Flag_Main = SET;   // ���÷���������־
                GTW_BurglarAlarm.Flag_Call = SET;   //
                GTW_BurglarAlarm.Flag_SMS  = SET;   //
#ifdef __DEBUG
                //XFS_WriteBuffer(xfs_auto, m_var.Flag_InitDone_M35 == SET ? "�ѱ���" : "������������" );
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, m_var.Flag_InitDone_M35 == SET ? "�ѱ���" : "������������" );
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
* Description    : �������ֻ�,
*******************************************************************************/
void DrivePlayPhone(void)
{
    static Illegal_Def             DrivePlayPhoneViolence;     // �������ֻ�
    static volatile unsigned int   Time_DrivePlayPhoneWarn = 0;
    static volatile unsigned char  DrivePlayPhoneStatus;
    static volatile unsigned char  DrivePlayPhoneCount = 0;    // �������ֻ���¼���Ĵ���
    switch(DrivePlayPhoneStatus) {
    case 0: {
        if(TerminalStatus != Status_Run) {
            Flag_DrivePlayPhone = RESET;
            DrivePlayPhoneStatus = 0;
        }
        else {
            if(Flag_DrivePlayPhone == SET ) {
                Flag_DrivePlayPhone = RESET;

                XFS_WriteBuffer(xfs_auto, "����ֹͣΥ��ʹ���ֻ�,���򽫱���¼Υ��");// ������������
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "����ֹͣΥ��ʹ���ֻ�,���򽫱���¼Υ��");
#endif
                DrivePlayPhoneCount++;  // ������һ��
                TimeToCharArray(Sys_CalendarTime, DrivePlayPhoneViolence.stime); // ��¼Υ�¿�ʼʱ��
                Time_DrivePlayPhoneWarn = Sys_TimeCount;// ��¼����ʱ��
                DrivePlayPhoneStatus++; // ��ת����һ��
            }
        }
        break;
    }

    case 1: {
        if(Sys_TimeCount - Time_DrivePlayPhoneWarn > 10) {
            DrivePlayPhoneStatus++;//��ת����һ��
        }
    }
    case 2: {
        if((Sys_TimeCount - Time_DrivePlayPhoneWarn > (5*60)) || //��⵽ʱ�����5����
                TerminalStatus != Status_Run)                    //δ��ʻ״̬
        {
            Flag_DrivePlayPhone = RESET;
            DrivePlayPhoneStatus = 0;
            break;
        }

        if(Flag_DrivePlayPhone == SET ) { //�������ֻ�
            Flag_DrivePlayPhone = RESET;
            XFS_WriteBuffer(xfs_auto, "����Υ��ʹ���ֻ�,�ѱ���¼Υ��");//������������
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "����Υ��ʹ���ֻ�,�ѱ���¼Υ��");
#endif
            TimeToCharArray(Sys_CalendarTime,DrivePlayPhoneViolence.stime); //��¼Υ�¿�ʼʱ��
            TimeToCharArray(Sys_CalendarTime,DrivePlayPhoneViolence.etime); //��¼Υ�½���ʱ��
            DrivePlayPhoneViolence.ino = DrivePlayPhone_enum;               //Υ������
            MX25L128_Illegal_Write(&DrivePlayPhoneViolence);                 //��Υ������д��Flash��
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
* Description    : ƣ�ͼ�ʻ���
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
        if(TerminalStatus == Status_Run) { //������ʻ
            Time_CarRun = Sys_TimeCount;//��¼��ʻʱ��
            TimeToCharArray(Sys_CalendarTime,FatiguDriveViolence.stime);//��¼Υ�¿�ʼʱ��
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
        if(TerminalStatus != Status_Run) { //����ֹͣ
            Time_CarStop = Sys_TimeCount; //��¼ֹͣʱ��
            FatiguDrivingStatus++;
            break;
        }

        if(TimeType == day_time) {
            if(Sys_TimeCount - Time_CarRun >= 3.5*60*60) { //3.5Сʱ
                Time_Warn = Sys_TimeCount;
                FatiguDrivingStatus = 3;
                break;
            }
        }
        else {
            if(Sys_TimeCount - Time_CarRun >= 1.5*60*60) { //1.5Сʱ
                Time_Warn = Sys_TimeCount;
                FatiguDrivingStatus = 3;
                break;
            }
        }
        break;
    case 2:
        if(TerminalStatus == Status_Run) { //������ʻ
            Time_CarRun = Sys_TimeCount; //��¼��ʻʱ��
            TimeToCharArray(Sys_CalendarTime,FatiguDriveViolence.stime);//��¼Υ�¿�ʼʱ��
            FatiguDrivingStatus = 1;
            break;
        }
        if(Sys_TimeCount - Time_CarStop >= (20 * 60)) { //20���Ӻ�
            FatiguDrivingStatus  = 0;//���¼�¼��ʻʱ��
            break;
        }
        break;
    case 3:
        XFS_WriteBuffer(xfs_auto, "����������ʻ����3Сʱ30���ӣ�������ǰ���ʵ�λ��ͣ����Ϣ20���������ټ�����ʻ����������ʻ����4Сʱ��������¼Υ�¡����������ʻԱ������֤�¼�ʻԱ���Ӽ���");
        Time_Warn = Sys_TimeCount;
        FatiguDrivingStatus++;
        break;
    case 4:
        if(TerminalStatus != Status_Run) { //����ֹͣ

            break;
        }
        if(Sys_TimeCount - Time_Warn >= (15*60)) { //15���Ӻ�
            FatiguDrivingStatus++;
        }
        break;
    case 5:
        XFS_WriteBuffer(xfs_auto, "����������ʻ����3Сʱ45���ӣ�������ǰ���ʵ�λ��ͣ����Ϣ20���������ټ�����ʻ����������ʻ����4Сʱ��������¼Υ�¡����������ʻԱ������֤�¼�ʻԱ���Ӽ���");
        Time_Warn = Sys_TimeCount;
        FatiguDrivingStatus++;
        break;
    case 6:
        if(TerminalStatus != Status_Run) { //����ֹͣ

            break;
        }
        if(Sys_TimeCount - Time_CarRun > (4*60*60)) {
            FatiguDrivingStatus = 0;
            break;
        }
        if(Sys_TimeCount - Time_Warn >= (10*60)) { //10���Ӻ�
            FatiguDrivingStatus++;
        }
        break;
    case 7:
        XFS_WriteBuffer(xfs_auto, "����������ʻ����3Сʱ55���ӣ�������ǰ���ʵ�λ��ͣ����Ϣ20���������ټ�����ʻ����������ʻ����4Сʱ��������¼Υ�¡����������ʻԱ������֤�¼�ʻԱ���Ӽ���");
        Time_Warn = Sys_TimeCount;
        FatiguDrivingStatus++;
        break;
    case 8:
        if(TerminalStatus != Status_Run) { //����ֹͣ

            break;
        }
        if(Sys_TimeCount - Time_CarRun > (4*60*60)) {
            FatiguDrivingStatus = 0;
            break;
        }
        if(Sys_TimeCount - Time_Warn >= (15*60)) { //15���Ӻ�
            FatiguDrivingStatus++;
        }
        break;
    case 9:
        XFS_WriteBuffer(xfs_auto, "����ƣ�ͼ�ʻ���ѱ���¼Υ��");
        FatiguDriveViolence.ino = DriveNoSeatBelt;          //Υ������
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
            TimeToCharArray(Sys_CalendarTime,FatiguDriveViolence.stime);//Υ�½���ʱ��
            MX25L128_Illegal_Write(&FatiguDriveViolence);               //��Υ������д��Flash��
            FatiguDrivingStatus = 0;
        }
        break;
    default :
        break;
    }
}
/*******************************************************************************
* Function Name  : BusRunAtNight
* Description    : ��ͳ�ҹ������
*******************************************************************************/
void BusRunAtNight(void)
{
    static Illegal_Def             LimitBusRunViolence;      //���ڱ����ͳ�Υ����Ϣ
    static volatile unsigned int   Time_LimitRunWarn1 = 0;
    static volatile unsigned char  Flag_LimitRunStatus;
    static volatile unsigned char  Flag_BelowThirdRoad = SET;//��־�Ƿ����������¹�·

    if(Sys_CalendarTime.hour == 01 &&        //1����
            (Sys_CalendarTime.min == 30 || Sys_CalendarTime.min == 40 || Sys_CalendarTime.min == 50) && //30��,40��,50��
            Sys_CalendarTime.sec == 0 &&     //0��
            TerminalStatus == Status_Run &&  //�г�״̬
            Flag_BelowThirdRoad == SET     //��3�����¹�·
      )              //����ģ�����
    {
        XFS_WriteBuffer(xfs_auto, "�����賿2����5��ͣ�˻������ʻԱ,�����¼Υ��");
#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "�����賿2����5��ͣ�˻������ʻԱ,�����¼Υ��");
#endif
    }


    switch(Flag_LimitRunStatus)
    {
    case 0:
        if(Flag_LimitRun == SET &&             //�����ź�
                TerminalStatus == Status_Run &&//�г�״̬
                Flag_BelowThirdRoad == SET &&  //��3�����¹�·
                Sys_CalendarTime.hour >= 2 && Sys_CalendarTime.hour <= 5 //ҹ��2����5��
          )            //����ģ�����
        {
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "����ҹ������������¹�·,�뾡��ͣ�������뿪������,�����¼Υ��");
#endif
            XFS_WriteBuffer(xfs_auto, "����ҹ������������¹�·,�뾡��ͣ�������뿪������,�����¼Υ��");
            Time_LimitRunWarn1 = Sys_TimeCount;
            Flag_LimitRunStatus++;
        }
        break;
    case 1:
        if(Sys_TimeCount - Time_LimitRunWarn1 >= (20*60))//20���Ӻ�
        {
            Flag_LimitRunStatus++;
        }
        break;

    case 2:
        if(Sys_CalendarTime.hour > 5 ||        //5���Ժ�
                Flag_BelowThirdRoad == RESET ||//����3�����¹�·
                TerminalStatus != Status_Run)  //����ʻ
        {
            Flag_LimitRunStatus = 0;
            break;
        }

        if( Flag_BelowThirdRoad == SET )  //��3�����¹�·
        {
            XFS_WriteBuffer(xfs_auto, "����Υ����ͳ�ҹ����ʻ�涨,�ѱ���¼Υ��");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "����Υ����ͳ�ҹ����ʻ�涨,�ѱ���¼Υ��");
#endif
            LimitBusRunViolence.ino = BusIllegalRun;//��¼Υ������
            TimeToCharArray(Sys_CalendarTime,LimitBusRunViolence.stime);//��¼Υ�¿�ʼʱ��

            Flag_LimitRunStatus++;
        }
        break;
    case 3:
        if(TerminalStatus == Status_Park)
        {
            Flag_LimitRunStatus++;
            Time_LimitRunWarn1 = Sys_TimeCount;//���µ�ǰʱ��
            break;
        }
        if((Sys_CalendarTime.hour > 5 &&  Flag_BelowThirdRoad == RESET) ||//5���Ժ�//����3�����¹�·
                IsDayOrNight() == night_time)//����
        {
            TimeToCharArray(Sys_CalendarTime,LimitBusRunViolence.etime);//��¼Υ�½���ʱ��
            MX25L128_Illegal_Write(&LimitBusRunViolence);//��Υ������д��Flash��
            Flag_LimitRunStatus = 0;
            break;
        }
        break;
    case 4:
        if(Sys_CalendarTime.hour > 5 &&  //5���Ժ�
                Flag_BelowThirdRoad == RESET )//����3�����¹�·
        {
            TimeToCharArray(Sys_CalendarTime,LimitBusRunViolence.etime);//��¼Υ�½���ʱ��
            MX25L128_Illegal_Write(&LimitBusRunViolence);//��Υ������д��Flash��
            Flag_LimitRunStatus = 0;
            break;
        }
        if(Sys_TimeCount - Time_LimitRunWarn1 >= (30 * 60))//30�����Ժ�
        {
            Flag_LimitRunStatus++;
        }
    case 5:
        TimeToCharArray(Sys_CalendarTime,LimitBusRunViolence.etime);//��¼Υ�½���ʱ��
        MX25L128_Illegal_Write(&LimitBusRunViolence);            //��Υ������д��Flash��
        Flag_LimitRunStatus = 0;
        break;
    default :
        Flag_LimitRunStatus = 0;
        break;
    }
}


/*******************************************************************************
* Function Name  : VehicleLimitRun
* Description    : ��������
*******************************************************************************/
void VehicleLimitRun(void)
{
    static Illegal_Def             LimitRunViolence;         //���ڱ����������Υ����Ϣ
    static volatile unsigned char  Flag_LimitRunStatue = 0;
    static volatile unsigned int   Time_LimitRunWarn2 = 0;
    static volatile unsigned char  Flag_CloseToLimitArea = 0;
    static volatile unsigned int   latitude;
    static volatile unsigned int   longitude;

    switch (Flag_LimitRunStatue)
    {
    case 0:
        if(Flag_CloseToLimitArea == SET &&   //�ӽ���������
                TerminalStatus == Status_Run)//������ʻ
        {
            XFS_WriteBuffer(xfs_auto, "���ѽӽ���������,�뼰ʱ�ĵ�");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "���ѽӽ���������,�뼰ʱ�ĵ�");
#endif
            Flag_LimitRunStatue++;
        }
        break;
    case 1:
        if(Flag_LimitRun == RESET ||     //����������
                TerminalStatus != Status_Run)//���г�״̬
        {
            Flag_LimitRunStatue = 0;
            break;
        }

        if(Flag_LimitRun == SET &&            //��������
                TerminalStatus == Status_Run )//�г�״̬
        {
            XFS_WriteBuffer(xfs_auto, "���ѽ�����������,�뾡���뿪");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "���ѽ�����������,�뾡���뿪");
#endif

            Time_LimitRunWarn2 = Sys_TimeCount;//��¼����ʱ��
            TimeToCharArray(Sys_CalendarTime,LimitRunViolence.stime);//��¼Υ�¿�ʼʱ��
            if(gpsx.gpssta == 1 || gpsx.gpssta == 2) { //GPS��λ�ɹ�

                latitude = gpsx.latitude;
                longitude = gpsx.longitude;
            }
            Flag_LimitRunStatue++;

        }
        break;
    case 2:
        if(Flag_LimitRun == RESET &&            //��������
                TerminalStatus != Status_Run ) { //�г�״̬

            break;
        }

        if(Sys_TimeCount - Time_LimitRunWarn2 > (5*60)) {
            Flag_LimitRunStatue++;
            break;
        }

        if(gpsx.gpssta == 1 || gpsx.gpssta == 2) { //GPS��λ�ɹ�
            if(GetDistance(latitude,longitude,gpsx.latitude,gpsx.longitude) >= 1000)//�������1000��
            {
                Flag_LimitRunStatue++;
            }
        }
        break;
    case 3:
        XFS_WriteBuffer(xfs_auto, "����Υ�����й涨,�ѱ���¼Υ��");
#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "����Υ�����й涨,�ѱ���¼Υ��");
#endif
        LimitRunViolence.ino = IllegalRun;               //��¼Υ������
        TimeToCharArray(Sys_CalendarTime,LimitRunViolence.etime);  //��¼Υ�½���ʱ��
        MX25L128_Illegal_Write(&LimitRunViolence);         //��Υ������д��Flash��
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
* Description    : ����
*******************************************************************************/
void LimitSpeed(void)
{
    static Illegal_Def             SpeedViolence;            //���ڱ�������Υ����Ϣ
    static volatile unsigned int   SpeedWarnTime = 0;
    static volatile unsigned char  Flag_SpeedStatue = 0;

    switch (Flag_SpeedStatue)
    {
    case 0:
        if(Flag_LimitSpeed == SET) {
            if(gpsx.gpssta == 1 || gpsx.gpssta == 2) {//GPS��λ�ɹ�
                if(GetSpeed() > (Dir_SGroup.Max_Speed * 11 / 10 )) { //����10%
                    Flag_SpeedStatue ++;
                }
            }
        }
        break;
    case 1:
        XFS_WriteBuffer(xfs_auto, "��ǰ����XX,���ѳ���10%,����Ƴ���,�����¼Υ��");
#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "��ǰ����XX,���ѳ����ٷ�֮ʮ,����Ƴ���,�����¼Υ��");
#endif
        TimeToCharArray(Sys_CalendarTime,SpeedViolence.stime);//Υ�¿�ʼʱ��
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

        if(Sys_TimeCount - SpeedWarnTime > 60 )//30��
        {
            XFS_WriteBuffer(xfs_auto, "������,�ѱ���¼Υ��");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "������,�ѱ���¼Υ��");
#endif
            SpeedWarnTime = Sys_TimeCount;
            SpeedViolence.ino = OverSpeed;//����
            Flag_SpeedStatue++;
        }
        break;
    case 3:
        if(Flag_LimitSpeed == SET) {
            if(gpsx.gpssta == 1 || gpsx.gpssta == 2) {//GPS��λ�ɹ�
                if(GetSpeed() > (Dir_SGroup.Max_Speed * 11 / 10 )) { //����10%
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
            TimeToCharArray(Sys_CalendarTime,SpeedViolence.etime);//Υ�½���ʱ��
            MX25L128_Illegal_Write(&SpeedViolence);//��Υ������д��Flash��
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
* Description    : �������ȼ��
*******************************************************************************/
void LimitTrumpet(void)
{
    static Illegal_Def             TrumpetViolence;          //���ڱ����������Υ����Ϣ
    static volatile unsigned char  Flag_TrumpetStatue = 0;
    static volatile unsigned int   Time_FirstTrumpet = 0;
    static volatile unsigned int   TempTime = 0;

    switch (Flag_TrumpetStatue)
    {
    case 0:
        if(DataCollectorMsg.MsgWhistle == SET  )        //������
        {
            XFS_WriteBuffer(xfs_auto, "��ǰ�ѽ����������");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "��ǰ�ѽ����������");
#endif
            Flag_TrumpetStatue++;
        }
        break;
    case 1:
        if(DataCollectorMsg.MsgWhistle == RESET) //�ǽ�����
        {
            XFS_WriteBuffer(xfs_auto, "��ǰ���뿪��������");
            Flag_TrumpetStatue = 0;
            break;
        }
        if( Car_InputCheck.Value_Honk == SET )//����һ�ΰ�����ʱ������������
        {
            XFS_WriteBuffer(xfs_auto, "���������");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "���������");
#endif
            Time_FirstTrumpet = Sys_TimeCount;//��¼��һ��ʹ�����ȵ�ʱ��
            Flag_TrumpetStatue++;
        }
        break;
    case 2:
        if(Sys_TimeCount - Time_FirstTrumpet >= 10) { //�ڶ��ΰ�����ʱ,����������ʾ,����¼Υ��
            Flag_TrumpetStatue++;
        }
        break;
    case 3:
        if(DataCollectorMsg.MsgWhistle == RESET)
        {
            XFS_WriteBuffer(xfs_auto, "��ǰ���뿪��������");
            Flag_TrumpetStatue = 0;
            break;
        }
        if( Car_InputCheck.Value_Honk == SET )
        {
            XFS_WriteBuffer(xfs_auto, "����Υ�������涨,�ѱ���¼Υ��");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "����Υ�������涨,�ѱ���¼Υ��");
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
        if(Sys_TimeCount - TempTime > 60) { //1���Ӻ��ٴμ��
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
* Description    : ���Υͣ
*******************************************************************************/
void IllegalPark(void)
{
    static unsigned char  IllegalParkStatus;
    static unsigned int   ill_latitude;      //γ�� ������100000��,ʵ��Ҫ����100000
    static unsigned int   ill_longitude;     //���� ������100000��,ʵ��Ҫ����100000
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
            IllegalParkStatus++;//������һ��״̬
        }
        break;

    case 1:
        if(gpsx.gpssta == 1 || gpsx.gpssta == 2)
        {
            ill_latitude = gpsx.latitude;
            ill_longitude = gpsx.longitude;
            gps_ok_time = Sys_TimeCount;
            if(TerminalStatus == Status_Run) { //������ʻ
                IllegalParkStatus++;//������һ��״̬
            }
        }
        break;

    case 2:
        if((Sys_TimeCount - gps_ok_time) % 5 == 0) { // 5s����һ�ξ���
            if(GetDistance( ill_latitude, ill_longitude, gpsx.latitude, gpsx.longitude ) >= 500) { //�������500��
                XFS_WriteBuffer(xfs_auto, "�����뿪�����,�����볷��Υͣ��¼");
#ifdef __DEBUG
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, "�����뿪�����,�����볷��Υͣ��¼");
#endif
                IllegalParkStatus++;
            }
        }
        break;

    case 3:
        if( m_carinfo.Count_IllegalPark < 2 ) {
            Flag_IllegalParkCancel = SET;       // ���ó���Υͣ���ͱ�־
            m_carinfo.Count_IllegalPark++;
            MX25L128_CreatSaveTask();           // ����Υͣ��¼
        }

    default:
        IllegalParkStatus = 0;
        break;

    }
}

/*******************************************************************************
* Function Name  : MakeViolation
* Description    : ����Υ������
*******************************************************************************/
void MakeViolation(unsigned short count)
{
    unsigned short i;
    Illegal_Def makeill;

    makeill.ewhemi = 'E';                               // ����/����,E:����;W:����
    makeill.longitude = (double)112.34567;              // Υ��ʱ����
    makeill.nshemi = 'N';                               // ��γ/��γ,N:��γ;S:��γ
    makeill.latitude  = (double)28.12345;               // Υ��ʱγ��

    for(i=0; i<count; i++) {
        TimeToCharArray(Sys_CalendarTime, makeill.stime);       // ��¼Υ�¿�ʼʱ��
        TimeToCharArray(Sys_CalendarTime, makeill.etime);       // ��¼Υ�½���ʱ��
        makeill.ino = (i % 12);                                 // Υ������
        MX25L128_Illegal_Write_Base( &makeill, SET);            // ��Υ������д��Flash��
    }
}
/****************************************END OF FILE**********************************/
