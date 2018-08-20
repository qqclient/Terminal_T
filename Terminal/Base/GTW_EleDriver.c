/*******************************************************************************
* File Name          : gtw_eledriver.c
* Author             : csyxj@728920175@qq.com
* Version            : V0.1
* Date               : 2017-03-19
* Description        : GTWЭ��(V2.0)����֧�ֺ����ļ�
*******************************************************************************/
#include "gtw_Head.h"

unsigned char  EleDB_Type = 0;      // ���Ӽ��չ���״̬
unsigned char  EleDB_Num  = 0;      // ���Ӽ������
unsigned char  EleDB_Phone[16] = { 0 };     // ���Ӽ�����绰����


/*******************************************************************************
* Function Name  : Get_EleDriver_VerifyDriver
* Description    : ��֤��ʻԱ
******************************************************************************/
unsigned short Get_EleDriver_VerifyDriver(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    unsigned char  result = 0;
    unsigned char  idNo[20];
    unsigned char  i;

    // ����Ӧ��ط�����
    s1 = dsr + HEAD_LENGTH;

//    *s1++ = src[8];
//    *s1++ = src[9];

//    *s1++ = src[1];
//    *s1++ = src[2];

    // ����׼������(1B) + ���Ӽ��պ�(9B) + �������(1B)
    // memcpy( s1, &src[10], 11 );
    // s1 += 11;
//  memcpy( s1, &src[10], 10 );
//    s1 += 10;


    *s1++ = src[10];      //����׼������(1B)
    *s1++ = 0x01;   //�������(1B)

    // ��ȡ���Ӽ��� ==> idNo
    sprintf( (char*)idNo, "%02d", src[10] );// ��������
    for( i=0; i<(__DRIVERID_LEN__ / 2); i++ ) {
        sprintf( (char*)idNo + 2 + i * 2, "%02x", src[i + 11] );
    }
    if( idNo[19] == 0xa ) { // ����β��=X������
        idNo[19] = 'X';
    }

    // �������
    EleDB_Num = src[20];

    // ������Ŀǰ����״̬
    EleDB_Type = src[22];

    // �绰����
    for(i=0; i<(__PHONE_LENGTH__ / 2); i++ ) {
        sprintf( (char*)EleDB_Phone + i * 2, "%02x", src[i + 23] );
    }
    // Сд ==> ��д
    for(i=0; i<__PHONE_LENGTH__; i++ ) {
        if(EleDB_Phone[i] > '9') {
            EleDB_Phone[i] -= 0x20;
        }
    }

    // �������ͽ���
    switch( src[21] ) {
    case 0x6:   // ������: ���Ӽ������������������֤������Ҫ����Ȩ�룬�����ȼ�
    {
        //��¼��������Ϣ���������պ�ѹ���ջ-------------------------*/
        Flag_ApplyResult = SET;                                       // ���յ�����
        PushApplyStack(&ApplyLicense.NoneApplyLic, idNo, 20); // �������˵ļ�����Ϣѹ���ջ
        History_UseCar(idNo, 0);//�����ó���¼

#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "������,������֤");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET,"������:���Ӽ������������������֤" );
#endif
        //���ݵ�ǰ�����ն�״̬ �ı������ն�״̬
        if(FlagTerminalStat == '2')	{			  // ��ǰ�����ն�������״̬
            I2411E_ChangeCarStatus('3');		  // �����ն�״̬Ϊ״̬��
        }
        break;
    }

    case 0x7:   // ������: ���Ӽ����������Ϊ��ʻԱ������Ҫ����Ȩ�룬ʱ������������, ���ȼ�=2
    {
        //��¼��������Ϣ���������պ�ѹ���ջ
        Flag_ApplyResult = SET; //���յ�����
        PushApplyStack(&ApplyLicense.SecApplyLic, idNo, 20);//�������˵ļ�����Ϣѹ���ջ
        History_UseCar(idNo, 0);//�����ó���¼

#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "������,��֤���Ӽ���");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET,"������:���Ӽ����������Ϊ��ʻԱ");
#endif

        //���ݵ�ǰ�����ն�״̬ �ı������ն�״̬
        if((FlagTerminalStat == '2') || (FlagTerminalStat == '3')) {
            I2411E_ChangeCarStatus('4');    //�����ն�״̬Ϊ״̬��
        }
        break;
    }

    case 0x8:   //�����:��ʱ���õĵ��Ӽ������������°�ť��ǿ�������Ϊ��ʻԱ������Ҫ����Ȩ�룬ʱ������������, ���ȼ�=2
    {
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "�����,ǿ����֤");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET,"�����:��ʱ���õĵ��Ӽ�����ǿ������Ϊ��ʻԱ");
#endif

        //��¼��������Ϣ���������պ�ѹ���ջ
        Flag_ApplyResult = SET;// ���յ�����
        PushApplyStack(&ApplyLicense.SecApplyLic, idNo, 20);// �������˵ļ�����Ϣѹ���ջ
        History_UseCar(idNo, 0);//�����ó���¼

        //���ݵ�ǰ�����ն�״̬ �ı������ն�״̬
        if((FlagTerminalStat == '2') || (FlagTerminalStat == '3')) {
            I2411E_ChangeCarStatus('4');// �����ն�״̬Ϊ״̬��
        }
    }
    }

    // ��֤���


    // ��д��������״̬λ
    *s1++ = src[22];   // дԭ���Ĺ���״̬


// �·�ʱ��
    *s1++ = Byte2Bcd(Sys_CalendarTime.w_year/100); //��
    *s1++ = Byte2Bcd(Sys_CalendarTime.w_year%100);
    *s1++ = Byte2Bcd(Sys_CalendarTime.w_month);  //��
    *s1++ = Byte2Bcd(Sys_CalendarTime.w_day);   //��
//    *s1++ = Byte2Bcd(Sys_CalendarTime.hour);    //ʱ
//    *s1++ = Byte2Bcd(Sys_CalendarTime.min);     //��
//    *s1++ = Byte2Bcd(Sys_CalendarTime.sec);     //��

//    TimeToCharArray(Sys_CalendarTime, s1);
//    s1 += 14;

    *s1++ = result;
    return GTW_Packet( dsr, 0x801, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );  // ���ݱ��İ�
}

/*******************************************************************************
* Function Name  : Get_EleDriver_FollowCar
* Description    : �����泵֪ͨ
******************************************************************************/
unsigned short Get_EleDriver_FollowCar(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    unsigned char  idNo[20];
    unsigned char  i;

#ifdef __DEBUG
    //XFS_WriteBuffer( xfs_auto, "�յ������泵֪ͨ" );
#endif

    // ����Ӧ��ط�����
    s1 = dsr + HEAD_LENGTH;

//    *s1++ = src[8];
//    *s1++ = src[9];

//    *s1++ = src[1];
//    *s1++ = src[2];

    // ����׼������(1B) + ���Ӽ��պ�(9B) + �������(1B)
//    memcpy( s1, &src[10], 11 );
//    s1 += 11;
    *s1++ = src[10];
    *s1++ = src[20];

    // ��ȡ���Ӽ��� ==> idNo
    sprintf( (char*)idNo, "%02d", src[10] );// ��������
    for( i=0; i<(__DRIVERID_LEN__ / 2); i++ ) {
        sprintf( (char*)idNo + 2 + i * 2, "%02x", src[i + 11] );
    }
    if( idNo[19] == 0xa ) { // ����β��=X������
        idNo[19] = 'X';
    }
    // ����յ��ĵ��Ӽ�ʻ֤��, ���ؽ��: 0=�ɹ�/ȷ��;1=ʧ��;2=��Ϣ����;3=��֧��
    if( memcmp( idNo+2, m_carinfo.Driver_ID[0]+2, __DRIVERID_LEN__ ) == 0 )
        *s1++ = 0;// ���
    else {
        *s1++ = 1;
        History_UseCar(idNo, 255);//�����ó���¼
    }

    return GTW_Packet( dsr, 0x802, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );  // ���ݱ��İ�
}

/*******************************************************************************
* Function Name  : GTW_DriverBallApp
* Description    : �������Ӽ�������Ϣ
******************************************************************************/
unsigned short GTW_DriverBallApp(unsigned char* src, unsigned char* dsr)
{
    unsigned short imsg;
    unsigned short send_pack_len = 0;

    /*
    // ��������Ϣ�Ƿ񷢸�����
    if( memcmp(src + HEAD_LENGTH, m_carinfo.Car_9SN, __CAR9SN_LEN__ ) != 0) {
    #ifdef __DEBUG
           XFS_WriteBuffer(xfs_auto, "���ƺŲ���");
        if(m_var.Flag_Debug_Enable == SET) {
            memset( buf, 0, sizeof(buf) );
            memcpy( buf, src + HEAD_LENGTH, __CAR9SN_LEN__ );
            Debug_Printf(SET,
    "���ƺ�(%s)[%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x]����",
    buf, buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8] );
        }
    #endif
           return 0; //���Ƿ�����������Ϣ
       }
    */

    // ��ϢID
    imsg = MAKEWORD( src[1], src[2] );

    switch(imsg) {
    case 0x1:   // ��֤��ʻԱ
        send_pack_len = Get_EleDriver_VerifyDriver( src, dsr );
        break;

    case 0x2:   // �����泵֪ͨ
        send_pack_len = Get_EleDriver_FollowCar( src, dsr );
        break;

    default:
        break;
    }

    return send_pack_len;
}

