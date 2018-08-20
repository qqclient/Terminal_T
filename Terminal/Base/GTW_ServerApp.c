/*******************************************************************************
* File Name          : gtw_serverapp.c
* Author             : csyxj@728920175@qq.com
* Version            : V0.1
* Date               : 2017-03-19
* Description        : Background server support function
*******************************************************************************/
#include "gtw_Head.h"

unsigned short GPRS_LocationTime = 0;           // ��ʱֵ(ms): �ϱ���λ��Ϣ => ��̨������

Illegal_Process_Def GTW_BurglarAlarm;           // ��������
Illegal_Process_Def GTW_IllegalPark;            // Υͣ
Illegal_Process_Def GTW_MoveCar;                // �Ƴ�

Illegal_Def Ill_ETicket;

unsigned char  Flag_IllegalParkCancel = RESET;  // ���볷��Υͣ��־

unsigned char M35_TxBuf[256] = { 0 };

M35_SendDef m_gprs;


/*******************************************************************************
* Function Name  : GTW_ServerAPP
* Description    : ������������Ϣ
******************************************************************************/
unsigned short GTW_ServerAPP(unsigned char* src, unsigned char* dsr)
{
    return(0);
}


/*******************************************************************************
* Function Name  : M35_Process
* Description    : ����M35ģ��Ĵ�绰,������,����GPRS����ҵ��
*******************************************************************************/
extern unsigned char Flag_Setup_SMS;    // ��װ������Ϣ
void M35_Process(void)
{
    unsigned char buf[32] = { 0 };
    static unsigned short Alarm_Count = 0;

    // ��������г�ʼ��
    M35_Initialize();

    // ��������
    if( M35_GetStatus() == SET ) {    // ��ǰSIM������

        if( Flag_Setup_SMS == SET ) { // ���Ͱ�װ��Ϣ => ��̨������
            GPRS_CmdNow = SendTcp_Start;
            Flag_Setup_SMS = RESET;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "��װ����");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "��װ����" );
#endif

            m_gprs.count = M35_SendSMS( M35_TxBuf,
                                        4,
                                        (unsigned char*)"FFFFFFFFFFFFFFFF",
                                        m_carinfo.Family_Phone[0] );//�����ݱ���
            m_gprs.lpmsg = M35_TxBuf;
        }
        else if( GTW_BurglarAlarm.Flag_SMS == SET ) {           // ������������
            GPRS_CmdNow = SendTcp_Start;
            GTW_BurglarAlarm.Flag_SMS = RESET;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "������������");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "������������" );
#endif

            m_gprs.count = M35_SendSMS( M35_TxBuf,
                                        1,
                                        (unsigned char*)"FFFFFFFFFFFFFFFF",
                                        m_carinfo.Family_Phone[0] );//�����ݱ���
            m_gprs.lpmsg = M35_TxBuf;
        }
        else if( GTW_IllegalPark.Flag_SMS == SET ) {     // Υͣ���� => ��̨������
            GPRS_CmdNow = SendTcp_Start;
            GTW_IllegalPark.Flag_SMS = RESET;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "Υͣ����");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "Υͣ����" );
#endif

            m_gprs.count = M35_SendSMS( M35_TxBuf,
                                        2,
                                        (unsigned char*)"FFFFFFFFFFFFFFFF",
                                        m_carinfo.Family_Phone[0] );//�����ݱ���
            m_gprs.lpmsg = M35_TxBuf;
        }
        else if( GTW_MoveCar.Flag_SMS == SET ) {         // �����Ƴ����� => ��̨������
            GPRS_CmdNow = SendTcp_Start;
            GTW_MoveCar.Flag_SMS = RESET;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "�Ƴ�����");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "�Ƴ�����" );
#endif

            m_gprs.count = M35_SendSMS( M35_TxBuf,
                                        3,
                                        GTW_MoveCar.lpszMsg,
                                        m_carinfo.Family_Phone[0] );   //�����ݱ���
            m_gprs.lpmsg = M35_TxBuf;
        }
        else if( Flag_IllegalParkCancel == SET ) {      // ���ͳ���Υͣ => ��̨������
            GPRS_CmdNow = SendTcp_Start;
            Flag_IllegalParkCancel = RESET;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "����Υͣ����");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "����Υͣ" );
#endif

            m_gprs.count = M35_SendIllegalPark( M35_TxBuf );    //�����ݱ���
            m_gprs.lpmsg = M35_TxBuf;
        }
        else if( GPRS_LocationTime == 0 ) {             // ��ʱ���Ͷ�λ��Ϣ=>��̨������
            // [[ 17.6.9 CsYxj Modify
            if(GTW_BurglarAlarm.Flag_Main == SET) {
                GPRS_LocationTime = m_carinfo.FD04_TimeOver;    // (Ĭ��30��)��������������ʱ����λ����Ϣ
                if( Alarm_Count == 0 ) {
                    Alarm_Count = 2;
#ifdef __DEBUG
                   // XFS_WriteBuffer( xfs_num, "��λ����=%d", (int)gpsx.longitude / 100000 );
                    GetLocation( buf );
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf( SET, "��λ����=%s", buf );
#endif
                    // λ����Ϣ => ��̨������
                    GPRS_CmdNow = SendTcp_Start;
                    m_gprs.count = M35_SendCarLocation( M35_TxBuf, '3' );//�����ݱ��� λ����Ϣ => ��̨������
                    m_gprs.lpmsg = M35_TxBuf;
                }
                else {
                    if( gpsx.gpssta == 1 || gpsx.gpssta == 2 ) {    // δ��λʱ�����Ͷ�λ����: 2017-6-1 CSYXJ(728920175@qq.com)
#ifdef __DEBUG
                       // XFS_WriteBuffer( xfs_num, "��λ����=%d", (int)gpsx.longitude / 100000 );
                        GetLocation( buf );
                        if(m_var.Flag_Debug_Enable == SET)
                            Debug_Printf( SET, "��λ����=%s", buf );
#endif
                        // λ����Ϣ => ��̨������
                        GPRS_CmdNow = SendTcp_Start;
                        //�����ݱ��� λ����Ϣ => ��̨������
                        m_gprs.count = M35_SendCarLocation( M35_TxBuf,
                                                            m_var.Flag_ApplyDriver == SET ? '1' : '2' );
                        m_gprs.lpmsg = M35_TxBuf;
                    }
                }
            }
            else {
                Alarm_Count = 0;
                GPRS_LocationTime = m_carinfo.FD03_TimeOver;    // (10����)������������ǰ��ʱ����λ����Ϣ
                if( gpsx.gpssta == 1 || gpsx.gpssta == 2 ) {    // δ��λʱ�����Ͷ�λ����: 2017-6-1 CSYXJ(728920175@qq.com)
#ifdef __DEBUG
                  //  XFS_WriteBuffer( xfs_num, "��λ����=%d", (int)gpsx.longitude / 100000 );
                    GetLocation( buf );
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf( SET, "��λ����=%s", buf );
#endif
                    // λ����Ϣ => ��̨������
                    GPRS_CmdNow = SendTcp_Start;
                    m_gprs.count = M35_SendCarLocation( M35_TxBuf, '0' );//�����ݱ��� λ����Ϣ => ��̨������
                    m_gprs.lpmsg = M35_TxBuf;
                }
            }
        }
        // ���绰��������
        else if( GTW_BurglarAlarm.Flag_Call == SET ) {  // ���������绰���г���
            GTW_BurglarAlarm.Flag_Call = RESET;
            GPRS_CmdNow = Call_Start;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "���������绰");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "���������绰" );
#endif
        }
        else if( GTW_IllegalPark.Flag_Call == SET ) {   // Υͣ�绰���г���
            GTW_IllegalPark.Flag_Call = RESET;
            GPRS_CmdNow = Call_Start;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "Υͣ�绰");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "Υͣ�绰" );
#endif
        }
        else if( GTW_MoveCar.Flag_Call == SET ) {       // �����Ƴ��绰���г���
            GTW_MoveCar.Flag_Call = RESET;
            GPRS_CmdNow = Call_Start;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "�����Ƴ��绰");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "�����Ƴ��绰" );
#endif
        }
        else if( M35_TimeCount == 0 ) {                 // ����ź�
            GPRS_CmdNow = CheckSignal1;
        }
    }

    if( GTW_MoveCar.Flag_Main == SET ) {
        if( Sys_TimeCount - GTW_MoveCar.Sec_OverTime > m_carinfo.MoveCar_TimeOver ) {
            // �Ƴ�����ʱ�䵽��
            GTW_MoveCar.Flag_Main = RESET;  // ����Ƴ�����־, ʵ��ָ��ʱ���ڲ�����Ӧ����Ϣ
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_value, "�����Ƴ�%d�뱣������", m_carinfo.MoveCar_TimeOver );
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "�����Ƴ�%d�뱣������", m_carinfo.MoveCar_TimeOver );
#endif
        }
    }
}


/*******************************************************************************
* Function Name  : M35_SendCarStatus
* Description    : ͨ��GPRS������״̬���͵���̨������
******************************************************************************/
unsigned short M35_SendCarStatus(unsigned char* dsr, unsigned char Car_InputCheck,unsigned char Additon)
{
    unsigned char* ptr = dsr + HEAD_LENGTH;

    if(GPRS_CmdNow < SendTcp_Start || GPRS_CmdNow > SendTcp_Finished) {
        return 0;
    }
    memcpy( ptr, m_carinfo.Car_9SN, __CAR9SN_LEN__ );   // ���ó��ƺ�[9Byte]:
    ptr += __CAR9SN_LEN__;

    *ptr++ = Car_InputCheck;
    *ptr++ = Additon;
    return GTW_Packet( dsr, 1, dsr + HEAD_LENGTH, ptr-dsr-HEAD_LENGTH, m_carinfo.DTU_Type );
}


/*******************************************************************************
* Function Name  : M35_SendCarLocation
* Description    : �ύ������λ�� => ��̨������
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
    TimeToCharArray(Sys_CalendarTime, s1 );        //14λ��ǰʱ��ֵ
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
* Description    : ����ȡ��Υͣ�������� => ��̨������
******************************************************************************/
unsigned short M35_SendIllegalPark(unsigned char* dsr)
{
    unsigned char* s1;
    unsigned short k = 0;

    if(GPRS_CmdNow < SendTcp_Start || GPRS_CmdNow > SendTcp_Finished) {
        return 0;
    }

    s1 = dsr + HEAD_LENGTH;

    sprintf( (char*)s1, "%02d", m_carinfo.Car_Type );       // ��������[2Byte]: 0=����, 1=����, 2=����, 3=����
    s1 += 2;

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );        // ���ƺ�[9Byte]:
    s1 += __CAR9SN_LEN__;

    memcpy( s1, m_carinfo.Driver_ID[0] + 2, __DRIVERID_LEN__ ); // �����˼�ʻ֤��[18Byte]
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
* Description    : ������������Ϣ������̨������ͳһ����
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

    sprintf( (char*)s1, "%02d", m_carinfo.Car_Type );           // ��������[2Byte]: 0=����, 1=����, 2=����, 3=����
    s1 += 2;

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );            // ���ƺ�[9Byte]:
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

