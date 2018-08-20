/*******************************************************************************
* File Name          : gtw_userapp.c
* Author             : csyxj@728920175@qq.com
* Version            : V0.1
* Date               : 2017-03-19
* Description        : Mobile application support function
*******************************************************************************/
#include "gtw_Head.h"

extern volatile unsigned char DriverLicIndex;

extern unsigned char FlagTerminalStat;
extern unsigned char VerifyTelephone(unsigned char* TmpTelephone);

Update_Struct     update;
UpdateInfo_Struct updateInfo;

unsigned char  Phone_MoveCar[16];

/*******************************************************************************
* Function Name  : distance_json_packet
* Description    : ��װ�����Ϊjson����
******************************************************************************/
unsigned short distance_json_packet(unsigned char* dsr, unsigned char type, double distance)
{
    char buf[40] = { 0 };
    char* s1 = (char*)dsr;

    *s1++ = '{';

    sprintf( buf, "\"type\":\"%d\",", type );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    sprintf( buf, "\"km\":\"%.5f\"", (double)distance / 1000 );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    *s1++ = '}';

    return(s1 - (char*)dsr);
}


/*******************************************************************************
* Function Name  : illegal_json_packet
* Description    : ��װ��������Ϊjson����
******************************************************************************/
unsigned short illegal_json_packet(unsigned char* dsr, Illegal_Def ill)
{
    char buf[40] = { 0 };
    char buf1[40] = { 0 };
    char* s1 = (char*)dsr;

    *s1++ = '{';

    sprintf( buf, "\"id\":\"%d\",", ill.id );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    sprintf( buf, "\"ptype\":\"%02d\",", ill.ptype );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    memcpy( buf1, ill.carno, sizeof(ill.carno) );
    sprintf( buf, "\"carnum\":\"%s\",", buf1 );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    memset( buf1, 0, sizeof(buf1) );
    memcpy( buf1, ill.stime, sizeof(ill.stime) );
    sprintf( buf, "\"stime\":\"%s\",", buf1 );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    memset( buf1, 0, sizeof(buf1) );
    memcpy( buf1, ill.etime, sizeof(ill.etime) );
    sprintf( buf, "\"etime\":\"%s\",", buf1 );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    sprintf( buf, "\"dType\":\"%02d\",", ill.driver_type );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    memset( buf1, 0, sizeof(buf1) );
    memcpy( buf1, ill.driver_no, sizeof(ill.driver_no) );
    sprintf( buf, "\"idNo\":\"%s\",", buf1 );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    sprintf( buf, "\"lat\":\"%.7f\",", ill.longitude );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    sprintf( buf, "\"lon\":\"%.7f\",", ill.latitude );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    sprintf( buf, "\"ino\":\"%d\"", ill.ino );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    *s1++ = '}';

    return(s1 - (char*)dsr);
}

/*******************************************************************************
* Function Name  : Verify_Driver
* Description    : ��֤��ʻԱ
******************************************************************************/
unsigned short Verify_Driver(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    unsigned char  i;
    unsigned char buf[24] = { 0 };
    unsigned char buf2[24] = { 0 };

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    i = Asc2Byte( (char*)src + 20, 2 );
    if( i > m_carinfo.Driver_Type ) {
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "�������Ͳ������ʸ�");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "APP������ʻ֤����=%d,�ʸ񲻷���", i);
#endif
        *s1++ = 2;
    }
    else {
        switch( src[19] ) {
        case 1:	{
            if ( (src[40] == '1') && VerifyTelephone( src + 41 ) == 1 ) {
#ifdef __DEBUG
                //XFS_WriteBuffer(xfs_auto, "����һ,��֤�����");
                if(m_var.Flag_Debug_Enable == SET) {
                    memset( buf, 0, sizeof( buf ) );
                    memcpy( buf, src + 20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );

                    memset( buf2, 0, sizeof(buf) );
                    memcpy( buf2, src + 41, __PHONE_LENGTH__ );

                    Debug_Printf(SET, "����һ:��֤�����(�����=%s,����=%s)", buf2, buf);
                }
#endif
                Flag_ApplyResult = SET;
                PushApplyStack(&ApplyLicense.FouApplyLic, src+20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
                History_UseCar(src+20, 0);
                m_var.Apply1_TimeOver = m_carinfo.JZ03_TimeOver;

                if(FlagTerminalStat == '2') {
                    *s1++ = 3;
                    I2411E_ChangeCarStatus('3');
                }
                else if(FlagTerminalStat == '3') {
                    if(TerminalStatus == Status_Run) {
                        if(is_empty(&ApplyLicense.NoneApplyLic) != 1) {
                            if( m_var.Flag_ApplyDriver == SET ) {
                                if( memcmp( src+20, m_carinfo.Driver_ID[0], __DRIVERID_LEN__ ) == 0 ) {
                                    if( is_empty(&ApplyLicense.ThiApplyLic) != 1 ) {
                                        *s1++ = 0;
                                        m_var.Apply1_TimeOver = 0;
                                        I2411E_ChangeCarStatus('4');
                                    }
                                    else {
                                        *s1++ = 1;
                                    }
                                }
                                else {
                                    *s1++ = 1;
                                }
                            }
                            else {
                                *s1++ = 0;
                                m_var.Apply1_TimeOver = 0;
                                I2411E_ChangeCarStatus('4');
                            }
                        }
                        else {
                            *s1++ = 3;
                        }
                    }
                    else {
                        *s1++ = 3;
                    }
                }
                else if(FlagTerminalStat == '4') {
                    *s1++ = 0;
                    m_var.Apply1_TimeOver = 0;
                    if( DriverLicIndex > 10 )
                        DriverLicIndex = 10;
                }
            }
            else {
#ifdef __DEBUG
                XFS_WriteBuffer(xfs_auto, "����Ŵ���");
                if(m_var.Flag_Debug_Enable == SET) {
                    memset( buf, 0, sizeof(buf) );
                    memcpy( buf, src + 41, __PHONE_LENGTH__ );
                    Debug_Printf( SET, "����һ:�����(%s)����", buf );
                }
#endif
                *s1++ = 1;
            }
            break;
        }

        case 2:	{
            if ((src[40] == '0') &&
                    (memcmp(src+41, m_carinfo.Authorize_Code, __AUTHORIZE_LEN__) == 0)) {
#ifdef __DEBUG
                XFS_WriteBuffer(xfs_auto, "�����,��֤��Ȩ��");
                if(m_var.Flag_Debug_Enable == SET) {
                    memset( buf, 0, sizeof( buf ) );
                    memcpy( buf, src + 20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );

                    memset( buf2, 0, sizeof(buf) );
                    memcpy( buf2, src + 41, __PHONE_LENGTH__ );

                    Debug_Printf(SET, "�����:��֤�����(��Ȩ��=%s,����=%s)", buf2, buf);
                }
#endif
                Flag_ApplyResult = SET;
                PushApplyStack(&ApplyLicense.ThiApplyLic, src+20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
                History_UseCar(src+20, 0);//�����ó���¼
                m_var.Apply1_TimeOver = m_carinfo.JZ03_TimeOver;

                if(FlagTerminalStat == '2') {
                    *s1++ = 3;
                    I2411E_ChangeCarStatus('3');
                }
                else if(FlagTerminalStat == '3') {
                    if(TerminalStatus == Status_Run) {
                        if(is_empty(&ApplyLicense.NoneApplyLic) != 1) {
                            if( m_var.Flag_ApplyDriver == SET ) {
                                if( memcmp( src+20, m_carinfo.Driver_ID[0], __DRIVERID_LEN__ ) == 0 ) {
                                    *s1++ = 0;
                                    m_var.Apply1_TimeOver = 0;
                                    I2411E_ChangeCarStatus('4');
                                }
                                else {
                                    *s1++ = 1;
                                }
                            }
                            else {
                                *s1++ = 0;
                                m_var.Apply1_TimeOver = 0;
                                I2411E_ChangeCarStatus('4');
                            }
                        }
                        else {
                            *s1++ = 3;
                        }
                    }
                    else {
                        *s1++ = 3;
                    }
                }
                else if(FlagTerminalStat == '4') {
                    *s1++ = 0;
                    m_var.Apply1_TimeOver = 0;
                    if( DriverLicIndex > 10 )
                        DriverLicIndex = 10;
                }
            }
            else {
#ifdef __DEBUG
                XFS_WriteBuffer(xfs_auto, "�����,��Ȩ�����");
                if(m_var.Flag_Debug_Enable == SET) {
                    memset( buf, 0, sizeof(buf) );
                    memcpy( buf, src + 41, __AUTHORIZE_LEN__ );
                    Debug_Printf( SET, "�����:��Ȩ��(%s)����", buf );
                }
#endif
                *s1++ = 1;
            }
            break;
        }

        case 3: {
            if (( src[40] == '1' ) && VerifyTelephone(src + 41) == 1 ) {
#ifdef __DEBUG
                XFS_WriteBuffer(xfs_auto, "������,��֤�����");
                if(m_var.Flag_Debug_Enable == SET) {
                    memset( buf, 0, sizeof( buf ) );
                    memcpy( buf, src + 20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );

                    memset( buf2, 0, sizeof(buf) );
                    memcpy( buf2, src + 41, __PHONE_LENGTH__ );

                    Debug_Printf(SET, "������:��֤�����(�����=%s,����=%s)", buf2, buf);
                }
#endif
                Flag_ApplyResult = SET;
                PushApplyStack(&ApplyLicense.FirApplyLic, src+20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
                History_UseCar(src+20, 0);//�����ó���¼
                if( FlagTerminalStat != '1' ) {
                    *s1++ = 0;
                    m_var.Apply1_TimeOver = 0;
                    I2411E_ChangeCarStatus('4');
                    if( DriverLicIndex > 10 )
                        DriverLicIndex = 10;
//                    }
                }
                else {
                    *s1++ = 1;
                }
            }
            else {
#ifdef __DEBUG
                //XFS_WriteBuffer(xfs_auto, "������,����Ŵ���");
                if(m_var.Flag_Debug_Enable == SET) {
                    memset( buf, 0, sizeof(buf) );
                    memcpy( buf, src + 41, __PHONE_LENGTH__ );
                    Debug_Printf( SET, "������:�����(%s)����", buf );
                }
#endif
                *s1++ = 1;
            }
            break;
        }

        case 4: {
            if (( src[40] == '0' ) &&
                    (memcmp(src+41, m_carinfo.Authorize_Code, __AUTHORIZE_LEN__) == 0)) {
#ifdef __DEBUG
                //XFS_WriteBuffer(xfs_auto, "������,��֤��Ȩ��");
                if(m_var.Flag_Debug_Enable == SET) {
                    memset( buf, 0, sizeof( buf ) );
                    memcpy( buf, src + 20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );

                    memset( buf2, 0, sizeof(buf) );
                    memcpy( buf2, src + 41, __PHONE_LENGTH__ );

                    Debug_Printf(SET, "������:��֤��Ȩ��(��Ȩ��=%s,����=%s)", buf2, buf);
                }
#endif
                Flag_ApplyResult = SET;
                PushApplyStack(&ApplyLicense.FirApplyLic, src+20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
                History_UseCar(src+20, 0);//�����ó���¼
                if( FlagTerminalStat != '1' ) {
                    *s1++ = 0;
                    m_var.Apply1_TimeOver = 0;
                    I2411E_ChangeCarStatus('4');
                    if( DriverLicIndex > 10 )
                        DriverLicIndex = 10;
                }
            }
            else {
#ifdef __DEBUG
                XFS_WriteBuffer(xfs_auto, "������,��Ȩ�����");
                if(m_var.Flag_Debug_Enable == SET) {
                    memset( buf, 0, sizeof(buf) );
                    memcpy( buf, src + 41, __AUTHORIZE_LEN__ );
                    Debug_Printf( SET, "������:��Ȩ��(����=%s)����", buf );
                }
#endif
                *s1++ = 1;
            }
            break;
        }

        case 5: {
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "������,ǿ����֤");
            if(m_var.Flag_Debug_Enable == SET) {
                memset( buf, 0, sizeof( buf ) );
                memcpy( buf, src + 20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );

                memset( buf2, 0, sizeof(buf) );
                memcpy( buf2, src + 41, __PHONE_LENGTH__ );

                Debug_Printf(SET, "������:ǿ����֤(��֤��=%s,����=%s)", buf2, buf);
            }
#endif
            PushApplyStack(&ApplyLicense.FifApplyLic, src+20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
            History_UseCar(src+20, 0);//�����ó���¼
            *s1++ = 0;

            break;
        }

        default:
            *s1++ = 4;
            break;
        }
    }
    return GTW_Packet( dsr, 0x801, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_IllegalNumber
* Description    : ��ȡ��������
******************************************************************************/
unsigned short Get_IllegalNumber(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;

    s1 = dsr + HEAD_LENGTH;
    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;

    *s1++ = src[19];
    switch(src[19]) {
    case 1:
        *s1++ = HIBYTE(ViolenceUntreated);
        *s1++ = LOBYTE(ViolenceUntreated);
#ifdef __DEBUG
       XFS_WriteBuffer(xfs_auto, "��δ�ϴ�Υ����");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "δ�ϴ�Υ����=%d", ViolenceUntreated );
#endif
        break;
    case 2:
        *s1++ = HIBYTE(ViolenceTotal);
        *s1++ = LOBYTE(ViolenceTotal);
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "������Υ����");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "������Υ����=%d", ViolenceTotal );
#endif
        break;
    default:
        *s1++ = 0;
        *s1++ = 1;
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "δ֪��ȡΥ����");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "δ֪��ȡΥ����" );
#endif
        break;
    }
    return GTW_Packet( dsr, 0x802, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_IllegalContent
* Description    : 0x000f:��ȡָ����������
******************************************************************************/
unsigned short Get_IllegalContent(unsigned char* src, unsigned char* dsr)
{
#ifdef __DEBUG
    const unsigned char eTicket[3][13] = { "��δ�ϴ�����", "��ȡȫ������", "��ȡһ������" };
#endif
    unsigned int    i;
    unsigned char*  s1;
    Illegal_Def     illegal1;
    unsigned short  len;
    unsigned int    ill_start;
    unsigned int    ill_Addr;
    unsigned short  ill_size;
    unsigned int*   pill_upload;
    unsigned short  ill_ReadNum = 1;
    char   buf[100] = { 0 };

    ill_size  = (int)&illegal1.ino - (int)&illegal1.id + sizeof(illegal1.ino);
    ill_start = MAKEWORD( src[20], src[21] );

    s1 = dsr + HEAD_LENGTH;
    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;

    *s1++ = src[19];

    *s1++ = src[20];
    *s1++ = src[21];
    ill_ReadNum = (src[22] == 0 ? 1 : src[22] );
    if( ill_ReadNum > MAX_TICKET_NUMBER )
        ill_ReadNum = MAX_TICKET_NUMBER;

#ifdef __DEBUG
    //XFS_WriteBuffer( xfs_auto, (char*)eTicket[ src[19] - 1 ] );
    if(m_var.Flag_Debug_Enable == SET) {
        sprintf( buf, "%s:��ʼ=%d,����=%d", eTicket[ src[19] - 1 ], ill_start, ill_ReadNum );
        Debug_Printf( SET, buf );
    }
#endif

    *s1++ = '[';
    switch( src[19] ) {
    case 1: {
        if( ill_start <= ViolenceUntreated && ViolenceUntreated > 0 ) {
            if( ill_start + ill_ReadNum >= ViolenceUntreated ) {
                ill_ReadNum = ViolenceUntreated - ill_start;
            }
 
            pill_upload = (unsigned int*)malloc( ill_ReadNum * sizeof(int) );
            MX25L128_BufferRead((unsigned char*)pill_upload,
                                ADDR_ILLEGAL_UPLOAD + 8 + ( ViolenceUntreated - ill_ReadNum - ill_start ) * sizeof(int),
                                ill_ReadNum * sizeof(int) );

            for( i=ill_ReadNum; i>0; i-- ) {
                if( pill_upload[i-1] < (1+(0xffffff-0x300000)/ill_size) ) {
                    ill_Addr = ADDR_ILLEGAL_CONTECT + (pill_upload[i-1]-1) * ill_size;
                    MX25L128_BufferRead( (unsigned char*)&illegal1.id, ill_Addr, ill_size );
                    len = illegal_json_packet( s1, illegal1 );
                    s1 += len;
                    *s1++ = ',';
                }
            }
            if( *(s1-1) == ',' )
                s1 -= 1;
            free(pill_upload);
        }
        break;
    }
    case 2: {
        if( ill_start <= ViolenceTotal && ViolenceTotal > 0 ) {
            ill_Addr = ADDR_ILLEGAL_CONTECT + (ViolenceTotal - ill_start - 1) * ill_size;
            for( i=0; i<ill_ReadNum; i++ ) {
                MX25L128_BufferRead( (unsigned char*)&illegal1.id, ill_Addr, ill_size );
                ill_Addr -= ill_size;
                len = illegal_json_packet( s1, illegal1 );
                s1 += len;
                *s1++ = ',';
            }
            if( *(s1-1) == ',' )
                s1 -= 1;
        }
        break;
    }
    case 3: {
        if( ill_start <= ViolenceTotal && ViolenceTotal > 0 ) {
            ill_Addr = ADDR_ILLEGAL_CONTECT + (ViolenceTotal - ill_start - 1) * ill_size;
            MX25L128_BufferRead( (unsigned char*)&illegal1.id, ill_Addr, ill_size );
            len = illegal_json_packet( s1, illegal1 );
            s1 += len;
        }
        break;
    }
    }
    *s1++ = ']';
    ill_ReadNum = MAKEWORD( 0x8, src[2] );
    return GTW_Packet( dsr, ill_ReadNum, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_IllegalStamp
* Description    : 0x010:���δ�ϴ�Υ��
******************************************************************************/
unsigned short Get_IllegalStamp(unsigned char* src, unsigned char* dsr)
{
    i2b_Def         i2b;
    unsigned char*  s1;
    unsigned int    len;
    unsigned int    i;
    unsigned int    j;
    unsigned int*   pill_upload;
    unsigned int    ill_remove;

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    len = MAKEWORD(src[19], src[20]);
#ifdef __DEBUG
    XFS_WriteBuffer( xfs_auto, "���%d�����ϴ�Υ�¼�¼", len );
    if(m_var.Flag_Debug_Enable == SET) {
        Debug_Printf( SET, "���%d�����ϴ�Υ�¼�¼", len );
    }
#endif

    pill_upload = (unsigned int*)malloc(ViolenceUntreated * sizeof(int));
    MX25L128_BufferRead((unsigned char*)pill_upload,
                        ADDR_ILLEGAL_UPLOAD + 8,
                        ViolenceUntreated * sizeof(int) );
    ill_remove = 0;
    for(i=0; i<len; i++) {
        i2b.c[3] = src[i*4 + 21];
        i2b.c[2] = src[i*4 + 21];
        i2b.c[1] = src[i*4 + 21];
        i2b.c[0] = src[i*4 + 21];
        for(j=0; j<ViolenceUntreated; j++) {
            if( pill_upload[j] == i2b.x ) {
                pill_upload[j] = 0xffff;
                ill_remove++;
                break;
            }
        }
    }
    if( ill_remove > 0 ) {
        for( i=0; i<ViolenceUntreated; i++ ) {
            if( pill_upload[i] == 0xffff ) {
                for(j=i; j<ViolenceUntreated; j++) {
                    pill_upload[j] = pill_upload[j+1];
                }
                ViolenceUntreated--;
            }
        }

        MX25L128_Word_Write(ADDR_ILLEGAL_UPLOAD + 4, ViolenceUntreated);
        MX25L128_BufferWrite((unsigned char*)pill_upload,
                             ADDR_ILLEGAL_UPLOAD + 8,
                             ViolenceUntreated * sizeof(int));
    }
    free(pill_upload);

    *s1++ = 0;

    memcpy( s1, &src[9], len * 4 + 2 );
    s1 += len * 4 + 2;

    return GTW_Packet( dsr, 0x810, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_UpdatePhone
* Description    : 003:�����ֻ�����
******************************************************************************/
unsigned short Get_UpdatePhone(unsigned char* src, unsigned char* dsr)
{
    int i = 0;
    unsigned char* s1;
    unsigned char* s2;
    unsigned char  buf[24] = { 0 };

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    s2 = src + 37;

    if(memcmp( s2, m_carinfo.Authorize_Code, __AUTHORIZE_LEN__ ) == 0) {
#ifdef __DEBUG
        //XFS_WriteBuffer(xfs_auto, "�޸��ֻ���");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "��Ȩ����ȷ,���޸��ֻ���");
#endif
        s2 += __AUTHORIZE_LEN__;
        memset( m_carinfo.Family_Phone[0], 0, 5*sizeof(m_carinfo.Family_Phone[0]));
        for( i=0; i<5; i++ ) {
            memcpy( m_carinfo.Family_Phone[i], s2, __PHONE_LENGTH__ );
            s2 += __PHONE_LENGTH__;
        }
        MX25L128_CreatSaveTask();
        *s1++ = 0;
    }
    else {
#ifdef __DEBUG
        //XFS_WriteBuffer(xfs_auto, "��Ȩ�����,��ֹ�޸��ֻ���");
        if(m_var.Flag_Debug_Enable == SET) {
            memset( buf, 0, sizeof( buf ) );
            memcpy( buf, s2, __AUTHORIZE_LEN__ );
            Debug_Printf( SET, "��Ȩ��[%s]����,��ֹ�޸��ֻ���", buf );
        }
#endif
        *s1++ = 4;
    }
    return GTW_Packet( dsr, 0x803, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_AdjuctVolume
* Description    : 0x0005=�����ն�����
******************************************************************************/
unsigned short Get_AdjuctVolume(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    unsigned char  result = 0;

    if( memcmp( &src[19], m_carinfo.Authorize_Code, __AUTHORIZE_LEN__  ) == 0 ) {
        m_carinfo.Volume_Value = src[35];
        XFS_SetVolume( m_carinfo.Volume_Value );
        XFS_WriteBuffer( xfs_auto, "ף��һ·ƽ��" );
#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET) {
            Debug_Printf( SET, "��������=%d", m_carinfo.Volume_Value );
        }
#endif
        MX25L128_CreatSaveTask();
    }
    else {
        result = 4;
    }

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = result;
    return GTW_Packet( dsr, 0x805, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_PlayPhone
* Description    : �������ֻ�
******************************************************************************/
unsigned short Get_PlayPhone(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    unsigned char  st;

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "�յ��������ֻ�֪ͨ");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "�յ��������ֻ�֪ͨ");
#endif
    Flag_DrivePlayPhone = SET;

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;
    if( Status_Run == TerminalStatus ) {
        st = 1;
    }
    else {
        st = 0;
    }

    if( src[19] == 0 ) {
        *s1++ = st;
    }
    else if( src[19] == 5 ) {
        *s1++ = st;
    }
    return GTW_Packet( dsr, 0x806, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_ErrorNumber
* Description    : ��ȡ���ϱ��
******************************************************************************/
unsigned short Get_ErrorNumber(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
#ifdef __DEBUG
    //XFS_WriteBuffer(xfs_auto, "��ȡ���ϱ��");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "��ȡ���ϱ��");
#endif

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;

    *s1++ = 'D';
    *s1++ = '0';
    *s1++ = '0';
    *s1++ = '0';
    *s1++ = '8';
    return GTW_Packet( dsr, 0x808, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_TerminalData
* Description    : 0x9: ��ȡ�ն�����
******************************************************************************/
unsigned short Get_TerminalData(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;

    *s1++ = src[19];

    switch(src[19]) {
    case 1:     // ��ȡ��ʻ�쳣����
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "��ȡ��ʻ�쳣����");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "��ȡ�ն���ʻ�쳣����");
#endif
        *s1++ = 0;
        *s1++ = 0;
        break;
    case 2:     // ��ȡ����(�ó���¼)
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "��ȡ�ó���¼");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "��ȡ�ն��ó���¼");
#endif
        // ���ݳ���
        *s1++ = 0;
        *s1++ = 0;
        // �������ݸ�ʽ:
        // ����[4Byte]
        // ���Ӽ���[20byte]
        // ��Ȩ��[11byte]
        // ��ʼʱ��[14byte]
        // ����ʱ��[14byte]
        break;
    default:
        // ���ݳ���
        *s1++ = 0;
        *s1++ = 0;
        break;
    }
    return GTW_Packet( dsr, 0x809, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_MoveCar
* Description    : �����Ƴ�
******************************************************************************/
unsigned short Get_MoveCar(unsigned char* src, unsigned char* dsr)
{
    int i;
    unsigned char* s1;
#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "�յ��Ƴ�����");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "�յ��Ƴ�����");
#endif

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;

    for( i=0; i<__PHONE_LENGTH__; i++ ) {
        if( src[i+19] == 0xd || src[i+19] ==0xa || src[i+19] == 0 ) {
            break;
        }
        Phone_MoveCar[i] = src[i+19];
    }
    PhoneAlign( Phone_MoveCar, __PHONE_LENGTH__ );

    *s1++ = 0;

    if( GTW_MoveCar.Flag_Main == RESET ) {
        GTW_MoveCar.Flag_Main = SET;
        GTW_MoveCar.Flag_SMS = SET;
        GTW_MoveCar.Flag_Call = SET;
        GTW_MoveCar.lpszMsg = Phone_MoveCar;
        GTW_MoveCar.Sec_OverTime = Sys_TimeCount;
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_value, "�����Ƴ�%d�뱣����ʼ", m_carinfo.MoveCar_TimeOver );
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf( SET, "�����Ƴ�%d�뱣����ʼ", m_carinfo.MoveCar_TimeOver );
#endif
    }
    return GTW_Packet( dsr, 0x80a, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_UpdateClock
* Description    : 00B:�����ն�ʱ��
******************************************************************************/
unsigned short Get_UpdateClock(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
#ifdef __DEBUG
    //XFS_WriteBuffer(xfs_auto, "�����ն�ʱ��");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "�����ն�ʱ��");
#endif
    TimeSync( &Sys_CalendarTime, src + 19);

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;

    *s1++ = 0;
    return GTW_Packet( dsr, 0x80b, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}


/*******************************************************************************
* Function Name  : Get_LogoutCar
* Description    : 00C:�˳���ʻ״̬
******************************************************************************/
unsigned short Get_LogoutCar(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "�˳���ʻ״̬");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "�˳���ʻ״̬");
#endif

    Flag_ApplyResult = RESET;
    m_var.Flag_ApplyDriver = RESET;
    ApplyLicInit();

    if( TerminalStatus == Status_Park ) {
        I2411E_ChangeCarStatus('1');
    }
    else {
        I2411E_ChangeCarStatus('2');
    }

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;
    History_UseCar(src+17, 255);//�����ó���¼
    *s1++ = 0;

    History_UseCar( src, 0xff ); // 17.6.13 csYxj(728920175@qq.com)

    return GTW_Packet( dsr, 0x80c, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_FollowCar
* Description    : 0x000d:���Ӽ����泵֪ͨ
******************************************************************************/
unsigned short Get_FollowCar(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;

#ifdef __DEBUG
    char buf[24] = {0};
   // XFS_WriteBuffer( xfs_auto, "�����泵֪ͨ" );
    if( m_var.Flag_Debug_Enable == SET ) {
        memset( buf, 0, sizeof(buf) );
        memcpy( buf, src + 19, __DRIVERID_LEN__ );
        Debug_Printf( RESET, "�����泵֪ͨ,���պ�=%s", buf );
    }
#endif

    m_var.FollowCar_TimeOver = m_carinfo.FollowCar_TimeOver;

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;
    History_UseCar(src+17, 255);//�����ó���¼

    return GTW_Packet( dsr, 0x80d, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_FirmwareUpdateInfo
* Description    : 007:�����ն˹̼�
******************************************************************************/
unsigned short Get_FirmwareUpdateInfo(unsigned char* src, unsigned char* dsr)
{
    i2b_Def  i2b;
    unsigned char* s1;
    unsigned char  result = 0;
    unsigned char  ver_cmp = 1;
    unsigned char  buf[32] = {0};

    i2b.c[3] = src[19];
    i2b.c[2] = src[20];
    i2b.c[1] = src[21];
    i2b.c[0] = src[22];
    update.count_total = i2b.x;

    updateInfo.rev_check_sum = MAKEWORD(src[23], src[24]);

    memcpy( buf, &src[26], src[25]);

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "��ȡ������Ϣ");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "������Ϣ(recv[%s], self[%s])", buf, updateInfo.version);
#endif

    if(strcmp( (const char*)updateInfo.version, (const char*)buf ) == 0) {
        ver_cmp = 0;
        result = 6;
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "��ͬ�汾,��������");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "��ͬ�汾,��������");
#endif
    }

    update.ver_len = src[25];
    memset( update.version, 0, sizeof( update.version ) );
    memcpy( update.version, &src[26], src[25] );
    memcpy( update.use_date, &src[src[25] + 26], 14 );
    memcpy( update.task_id, &src[src[25] + 40], 10 );

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = result;

    if( ver_cmp == 1 ) {
        *s1++ = HIBYTE(RECV_PACKET_SIZE);
        *s1++ = LOBYTE(RECV_PACKET_SIZE);
    }
    else {
        *s1++ = 0;
        *s1++ = 0;
    }

    update.id_req = 0;
    *s1++ = HIBYTE(update.id_req);
    *s1++ = LOBYTE(update.id_req);

    *s1++ = update.ver_len;
    memcpy( s1, updateInfo.version, updateInfo.ver_len );
    s1 += updateInfo.ver_len;

    memcpy( s1, updateInfo.task_id, 10 );
    s1 += 10;

    memcpy( s1, m_carinfo.DTU_ID, 18 );
    s1 += 18;
    return GTW_Packet( dsr, 0x807, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_FirmwareUpdateData
* Description    : 0x000e:�����������ݰ�
******************************************************************************/
unsigned short Get_FirmwareUpdateData(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    unsigned short data_len;
    unsigned char  result = 0;
    update.pack_count = MAKEWORD( src[19], src[20] );
    update.id_ans = MAKEWORD( src[21], src[22] );
    if(update.id_ans != update.id_req) {
        result = 3;
    }
    else {
        data_len = MAKEWORD( src[4], src[5] ) - __CAR9SN_LEN__ - 2 - 2;
        memcpy(update.RxBuf + update.count_less1k, &src[23], data_len);
        update.count_less1k += data_len;
        if(update.count_less1k == 1024) {
            update.flag_write_flash = SET;
        }
        else if(update.count_less1k > 1024) {
            result = 3;
        }
        if( update.pack_count-1 == update.id_ans ) {
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_num, "�������,���յ�%d������", update.pack_count);
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "�������,���յ�%d������", update.pack_count);
#endif
            update.id_req = 0;
            result = 6;

            updateInfo.ver_len = update.ver_len;
            memset( updateInfo.version, 0, sizeof(updateInfo.version) );
            sprintf( (char*)updateInfo.version, "%s", update.version );
            memcpy( updateInfo.use_date, update.use_date, 14 );
            memcpy( updateInfo.task_id, update.task_id, 10 );
            updateInfo.flah_new = 1;
            update.flag_write_flash = SET;
        }
        else if( update.id_ans == 0 ) {
            updateInfo.count1k_packet = 0;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_num, "��ʼ�����������ݰ�");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "��ʼ�����������ݰ�");
#endif
        }

        if(update.flag_write_flash == SET) {
            update.flag_write_flash = RESET;
            update.addr_app = FLASH_BASE + 0x22000 + updateInfo.count1k_packet * PACKET_SIZE;
            updateInfo.count1k_packet++;
            STMFLASH_WriteAppBin(update.addr_app, update.RxBuf, update.count_less1k);
            update.count_less1k = 0;
        }
    }

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = result;

    update.id_req = update.id_ans+1;
    *s1++ = HIBYTE(update.id_req);
    *s1++ = LOBYTE(update.id_req);

    memcpy( s1, updateInfo.task_id, 10 );
    s1 += 10;

    memcpy( s1, m_carinfo.DTU_ID, 18 );
    s1 += 18;

    *s1++ = update.ver_len;
    memcpy( s1, update.version, update.ver_len );
    s1 += update.ver_len;
    return GTW_Packet( dsr, 0x80e, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_UpdateParameter
* Description    : 0x0011:���²���
******************************************************************************/
unsigned short Get_UpdateParameter(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    char* s2;
    char  key[16] = { 0 };
    char  value[16] = { 0 };
    unsigned char i = 0;
    unsigned char  result = 0;
    s1 = src + HEAD_LENGTH + 9;
    s1 = (unsigned char*)strstr( (const char*)s1, "[{\"K\":\"" );
    if( s1 != NULL ) {
        s1 += strlen("[{\"K\":\"");
        s2 = key;
        while( s1 != NULL ) {
            if( *s1 == 0xd || *s1 == 0x0a || *s1 == 0 || *s1 == '"' ) {
                s1 += 3;
                if( *s1 == 'V' || *s1 == 'v' ) {
                    s1 += 4;
                    s2 = value;
                    while( s1 != NULL ) {
                        if( *s1 == 0xd || *s1 == 0x0a || *s1 == 0 || *s1 == '"' ) {
                            break;
                        }
                        *s2++ = *s1++;
                    }
                }
                break;
            }
            *s2++ = *s1++;
        }
#ifdef __DEBUG
        //XFS_WriteBuffer( xfs_auto, "����%s����", key );
        if( m_var.Flag_Debug_Enable == SET ) {
            Debug_Printf( RESET, "���²���: %s=%s", key, value );
        }
#endif
        s2 = key;
        switch( *s2++ ) {
        case 'b':
        case 'B':
        {
            if( *s2 == 'L' || *s2 == 'l' ) {
                i = atoi( s2 + 1 );
                switch( i ) {
                case 1:   // �������ݰ�����
                    m_carinfo.bt_packlen = atoi(value);
                    break;
                }
            }
            break;
        }
        case 'f':
        case 'F':
        {
            if( *s2 == 'D' || *s2 == 'd' ) {
                i = atoi( s2 + 1 );
                switch( i ) {
                case 1:   // �ն˽���״̬��������ʾ����ʱ��
                    m_carinfo.FD01_TimeOver = atoi(value);
                    break;
                case 2:   // �ն˽���״̬��������������ʱ��
                    m_carinfo.FD02_TimeOver = atoi(value);
                    break;
                case 3:   // ��������ǰ�ϴ�����λ������
                    m_carinfo.FD03_TimeOver = atoi(value);
                    break;
                case 4:   // �����������ϴ�����λ������
                    m_carinfo.FD04_TimeOver = atoi(value);
                    break;
                default:
                    result = 3;
                    break;
                }
            }
            else {
                result = 3;
            }
            break;
        }
        case 'j':
        case 'J':
        {
            if( *s2 == 'Z' || *s2 == 'z' ) {
                i = atoi( s2 + 1 );
                switch( i ) {
                case 1:   // ��������������ʾ��֤��ʻԱʱ��
                    m_carinfo.JZ01_TimeOver = atoi(value);
                    break;
                case 2:   // ��������������ʾ����֤��ʻʱ��
                    m_carinfo.JZ02_TimeOver = atoi(value);
                    break;
                case 3:   // �Զ���¼�����Ŷ�ʱ�䣨������ʱ��δͨ�ŵļ����Ƴ����У�
                    m_carinfo.JZ03_TimeOver = atoi(value);
                    break;
                case 4:   // ��״̬һ��״̬��������������κμ��շ������룬�ȴ��೤ʱ��ָ�״̬һ
                    m_carinfo.JZ04_TimeOver = atoi(value);
                    break;
                case 5:   // ״̬�������κμ��շ������룬�ȴ��೤ʱ��ָ�״̬һ��δ��𣩣�״̬�������
                    m_carinfo.JZ05_TimeOver = atoi(value);
                    break;
                case 6:   // ״̬���¶೤ʱ��û�н��յ������泵��Ϣ�ָ�״̬һ��ͣ��Ϩ�𣩣�״̬����δϨ��
                    m_carinfo.JZ06_TimeOver = atoi(value);
                    break;
                case 7:   // ѡ����ʻԱ�󣬶೤ʱ����ղ����˼��յ���һ�����룬��ȥѡ������ȼ�������
                    m_carinfo.JZ07_TimeOver = atoi(value);
                    break;
                default:
                    result = 3;
                    break;
                }
            }
            else {
                result = 3;
            }
            break;
        }
        case 's':
        case 'S':
        {
            if( *s2 == 'D' || *s2 == 'd' ) {
                i = atoi( s2 + 1 );
                switch(*(s2+2)) {
                case '1':
                    m_carinfo.SD01_TimeOver = atoi(value);
                    break;
                case '2':
                    m_carinfo.SD02_TimeOver = atoi(value);
                    break;
                default:
                    result = 3;
                    break;
                }
            }
            else if( *s2 == 'C' || *s2 == 'c' ) {
                // �泵����
                i = atoi( s2 + 1 );
                switch(*(s2+2)) {
                case '1':   // �泵֪ͨ��ʱ����:{"K":"SC01";"V":"600"}
                    m_carinfo.FollowCar_TimeOver = atoi(value);
                    break;
                default:
                    result = 3;
                    break;
                }
            }
            else {
                result = 3;
            }
            break;
        }
        case 'w':
        case 'W':
        {
            i = atoi( s2 + 1 );
            if( *s2 == 'T' || *s2 == 't' ) {
                switch(i) {
                case 1: // �ӿ�������ʼ������Υͣ��ʱ�䳤��
                    m_carinfo.WT01_TimeOver = atoi(value);
                    break;
                case 2: // �뿪��������λ�ö�Զ���Զ�����Υͣ
                    m_carinfo.WT02_DistOver = atoi(value);
                    break;
                default:
                    result = 3;
                    break;
                }
            }
            else if(*s2 == 'Z' || *s2 == 'z' ) {
                switch(i) {
                case 1:  // �����г�״̬�£������յ�����ʹ���ֻ�����Ϣ��ͬһ��ʹ���ֻ����ż�¼Υ��
                    m_carinfo.WZ01_CountOver = atoi(value);
                    break;
                case 2:  // ǰ������ʹ���ֻ��źż�������Ϊ����һ��ʹ���ֻ������¼ƴ��ж��Ƿ��¼Υ��
                    m_carinfo.WZ02_TimeOver = atoi(value);
                    break;
                case 3:  // ������ʻ������ʾϵ��ȫ��
                    m_carinfo.WZ03_TimeOver = atoi(value);
                    break;
                case 4:  // ������ʻ���ü�¼��ϵ��ȫ��Υ��
                    m_carinfo.WZ04_TimeOver = atoi(value);
                    break;
                case 5:  // �յ�������������źź��ò�����Υ��
                    m_carinfo.WZ05_TimeOver = atoi(value);
                    break;
                case 6:  // �յ�����Զ���źź��ò�����Υ��
                    m_carinfo.WZ06_TimeOver = atoi(value);
                    break;
                case 7:  // ��ͳ�ҹ����п�ʼʱ��
                    sprintf( (char*)m_carinfo.WZ07_Timer, "%s", value );
                    break;
                case 8:  // ��ͳ�ҹ����н���ʱ��
                    sprintf( (char*)m_carinfo.WZ08_Timer, "%s", value );
                    break;
                case 9:  // ��ͳ�ҹ����п�ʼԤ��ʱ�䣨�ڴ�ʱ��֮�󣬽��п�ʼʱ��֮ǰ�������������Ԥ����
                    sprintf( (char*)m_carinfo.WZ09_Timer, "%s", value );
                    break;
                case 10: // ��ͳ�ҹ�����Ԥ�����ڣ���һ�����Ѻ󣬽��п�ʼʱ��֮ǰ�����������һ�Σ�
                    m_carinfo.WZ10_TimeOver = atoi(value);
                    break;
                case 11: // ��ͳ�ҹ�����ʱ���ϸ��ٺ󣬶೤ʱ���¼Υ��
                    m_carinfo.WZ11_TimeOver = atoi(value);
                    break;
                case 12: // 24Сʱ���ۼƼݳ��೤ʱ���������ն˷���Ϣ����ƣ�ͼ�ʻ
                    m_carinfo.WZ12_TimeOver = atoi(value);
                    break;
                case 13: // �����ۼƼݳ��������ƣ�ͼ�ʻ
                    m_carinfo.WZ13_TimeOver = atoi(value);
                    break;
                case 14: // ƣ�ͼ�ʻ����ѭ������
                    m_carinfo.WZ14_TimeOver = atoi(value);
                    break;
                case 15: // ����ƣ�ͼ�ʻ���ò���Ϣ�������ʻԱ��Υ��
                    m_carinfo.WZ15_TimeOver = atoi(value);
                    break;
                case 16: // �����ۼƼݳ�����ƣ�ͼ�ʻ����Ϣ������¼�ʱ
                    m_carinfo.WZ16_TimeOver = atoi(value);
                    break;
                default:
                    result = 3;
                    break;
                }
            }
            else {
                result = 3;
            }
            break;
        }
        case 'y':
        case 'Y':
        {
            i = atoi( s2 + 1 );
            if( *s2 == 'C' || *s2 == 'c' ) {
                switch(i) {
                case 1: // sec,�����Ƴ�����ʱ�䣺�ڸ�ʱ���ڲ��ٲ��ż�������
                    m_carinfo.MoveCar_TimeOver = atoi(value);
                    break;
                }
            }
            break;
        }
        default:
        {
            result = 3;
            break;
        }
        }
    }
    else {
        result = 3;
    }

    if( result == 0 ) {
        MX25L128_CreatSaveTask();
    }
    s1 = dsr + HEAD_LENGTH;
    *s1++ = src[8];
    *s1++ = src[9];
    *s1++ = src[1];
    *s1++ = src[2];
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = result;
    return GTW_Packet( dsr, 0x811, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_UpdateSpeech
* Description    : 0x0012:��������
******************************************************************************/
unsigned short Get_UpdateSpeech(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    char* s2;
    char  key[16] = { 0 };
    char  value[16] = { 0 };
    unsigned char i = 0;
    unsigned char  result = 0;
    s1 = src + HEAD_LENGTH + 9;
    s1 = (unsigned char*)strstr( (const char*)s1, "[{\"K\":\"" );
    if( s1 != NULL ) {
        s1 += strlen("[{\"K\":\"");
        s2 = key;
        while( s1 != NULL ) {
            if( *s1 == 0xd || *s1 == 0x0a || *s1 == 0 || *s1 == '"' ) {
                s1 += 3;
                if( *s1 == 'V' || *s1 == 'v' ) {
                    s1 += 3;
                    s2 = value;
                    while( s1 != NULL ) {
                        if( *s1 == 0xd || *s1 == 0x0a || *s1 == 0 || *s1 == '"' ) {
                            break;
                        }
                        *s2++ = *s1++;
                    }
                }
                break;
            }
            *s2++ = *s1++;
        }
        s2 = key;
        if( *s2 == 'Y' || *s2 == 'y') {
            i = atoi( s2 + 1 );
            switch(i) {
            case  1:    // ���ѽ������Զ��������벻Ҫ����Զ���
                break;
            case  2:    // �����ر�Զ��ƣ������¼Υ��
                break;
            case  3:    // ����Υ��ʹ��Զ����ѱ���¼Υ��
                break;
            case  4:    // ������ʾ���Ӽ���
                break;
            case  5:    // ������ʾ���Ӽ���
                break;
            case  6:    // ���Ӽ�����֤���
                break;
            case  7:    // ��������֤��ʻ���Ѽ�¼Υ��
                break;
            case  8:    // ����ϵ�ð�ȫ���������¼Υ��
                break;
            case  9:    // ����δϵ��ȫ�����ѱ���¼Υ��
                break;
            case 10:    // ������ȷʹ�ð�ȫ���������¼Υ��
                break;
            case 11:    // ����Υ��ʹ�ð�ȫ�����ѱ���¼Υ��
                break;
            case 12:    // ���Ѽ�ʻԱ��Ϣ���ظ����
                break;
            case 13:    // ���Ѽ�ʻԱ��Ϣ���ظ����
                break;
            case 14:    // ����������ʻ����XXСʱXX���ӣ�������ǰ���ʵ�λ��ͣ����Ϣ20���������ټ�����ʻ����������ʻ����4Сʱ��������¼Υ�¡����������ʻԱ������֤�¼�ʻԱ���Ӽ��ա�
                break;
            case 15:    // ����������ʻ����XXСʱXX���ӣ�������ǰ���ʵ�λ��ͣ����Ϣ20���������ټ�����ʻ����������ʻ����4Сʱ��������¼Υ�¡����������ʻԱ������֤�¼�ʻԱ���Ӽ��ա�
                break;
            case 16:    // ��24Сʱ�����ۼƼ�ʻ����7СʱXX���ӣ�������ǰ���ʵ�λ��ͣ����Ϣ�������ʻԱ�����ۼƼ�ʻ����8Сʱ��������¼Υ�¡�
                break;
            case 17:    // ����ƣ�ͼ�ʻ���ѱ���¼Υ��
                break;
            case 18:    // �����賿2:00��5:00ͣ�˻������ʻԱ�������¼Υ��
                break;
            case 19:    // �����賿2:00��5:00ͣ�˻������ʻԱ�������¼Υ��
                break;
            case 20:    // �����賿2:00��5:00ͣ�˻������ʻԱ�������¼Υ��
                break;
            case 21:    // �����ʷ���Ϣ���ټݳ��������¼Υ��?
                break;
            case 22:    // ����ҹ������������¹�·���뾡��ͣ�����뿪�����򣬷����¼Υ��
                break;
            case 23:    // ����Υ����ͳ�ҹ����ʻ�涨���ѱ���¼Υ��
                break;
            case 24:    //
                break;
            case 25:    // ���ѽӽ����������뼰ʱ�ĵ�
                break;
            case 26:    // ����Υ�����й涨���ѱ���¼Υ��
                break;
            case 27:    // ��ǰ����XX�����ѳ���XX%������Ƴ��٣������¼Υ��
                break;
            case 28:    // �����٣��ѱ���¼Υ��
                break;
            case 29:    // ��ǰ�ѽ���/�뿪��������
                break;
            case 30:    // ���������
                break;
            case 31:    // ����Υ�������涨���ѱ���¼Υ��
                break;
            case 32:    // ���ļ��յȼ�̫�ͣ����ܼ�ʻ�ó������򽫼�¼Υ��
                break;
            case 33:    // �����ն��ٸ���Ҫ������Զ���������
                break;
            case 34:    // ף��һ·˳��
                break;
            case 35:    // ��ֹͣΥ��ʹ���ֻ��������¼Υ��
                break;
            default:
                result = 3;
                break;
            }
        }
        else {
            result = 3;
        }
    }
    else {
        result = 3;
    }
    s1 = dsr + HEAD_LENGTH;
    *s1++ = src[8];
    *s1++ = src[9];
    *s1++ = src[1];
    *s1++ = src[2];
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = result;
    return GTW_Packet( dsr, 0x812, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_UpdateSpeech
* Description    : 0x0013:�黹������
******************************************************************************/
unsigned short Get_ReturnDrvBall(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "�黹������");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "�黹������");
#endif
    s1 = dsr + HEAD_LENGTH;
    *s1++ = src[8];
    *s1++ = src[9];
    *s1++ = src[1];
    *s1++ = src[2];
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;
    return GTW_Packet( dsr, 0x813, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_Distance
* Description    : ���������
******************************************************************************/
unsigned short Get_Distance(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    unsigned short len = 0;

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "��ȡ�����");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "��ȡ�����=%.5f", m_carinfo.distance );
#endif
    s1 = dsr + HEAD_LENGTH;
    *s1++ = src[8];
    *s1++ = src[9];
    *s1++ = src[1];
    *s1++ = src[2];
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;

    len = distance_json_packet( s1, 1, m_carinfo.distance );
    s1 += len;

    return GTW_Packet( dsr, 0x814, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : GTW_UserAPP
* Description    : �����û��ֻ�APP��Ϣ
******************************************************************************/
unsigned short GTW_UserAPP(unsigned char* src, unsigned char* dsr)
{
    unsigned short imsg;
    unsigned short send_pack_len = 0;
#ifdef __DEBUG
    unsigned char  buf[24] = {0};
#endif

    // ��ϢID
    imsg = MAKEWORD( src[1], src[2] );

    if( memcmp(src + HEAD_LENGTH, m_carinfo.Car_9SN, __CAR9SN_LEN__ ) != 0) {
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "���ƺŲ���");
        if(m_var.Flag_Debug_Enable == SET) {
            memset( buf, 0, sizeof(buf) );
            memcpy( buf, src + HEAD_LENGTH, __CAR9SN_LEN__ );
            Debug_Printf(SET,
                         "�����(%04x)�����ĳ��ƺ�(%s)[%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x]����",
                         imsg, buf,
                         buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8] );
        }
#endif
        return 0;
    }
#ifdef __DEBUG
		//Debug_Printf( SET, "�յ��û��ֻ�APP��Ϣ=%04x", m_var.Flag_Debug_Enable );
     if(m_var.Flag_Debug_Enable == SET){
				Debug_Printf( SET, "�յ��û��ֻ�APP��Ϣ=%04x", imsg );
		 }
        
#endif
    switch(imsg) {
    case 0x1:   // ��֤��ʻԱ
        send_pack_len = Verify_Driver( src, dsr );
        break;
    case 0x2:   // ��ȡ��������
        send_pack_len = Get_IllegalNumber( src, dsr );
        break;
    case 0x3:   // �޸��ֻ�����
        send_pack_len = Get_UpdatePhone( src, dsr );
        break;
    case 0x4:   // �ظ�������ݵĴ���״̬: 0x00=Υ������; 0x01=�쳣��������; 0x02=�ó���¼
        send_pack_len = Get_TerminalData( src, dsr );
        break;
    case 0x5:   // �����ն�����
        send_pack_len = Get_AdjuctVolume( src, dsr );
        break;
    case 0x6:   // �������ֻ�
        send_pack_len = Get_PlayPhone( src, dsr );
        break;
    case 0x7:   // �ն�������Ϣ
        send_pack_len = Get_FirmwareUpdateInfo( src, dsr );
        break;
    case 0x8:   // ��ȡ�ն˵Ĺ��ϴ���
        send_pack_len = Get_ErrorNumber( src, dsr );
        break;
    case 0x9:   // ��ȡ�ն���������: 0x01: ��ȡ��ʻ�쳣����; 0x02: ��ȡ����(�ó���¼)
        send_pack_len = Get_TerminalData( src, dsr );
        break;
    case 0xa:  // �����Ƴ�
        send_pack_len = Get_MoveCar( src, dsr );
        break;
    case 0xb:  // �����ն�ʱ��
        send_pack_len = Get_UpdateClock( src, dsr );
        break;
    case 0xc:  // �˳���ʻ״̬
        send_pack_len = Get_LogoutCar( src, dsr );
        break;
    case 0xd:  // ���Ӽ����泵֪ͨ
        send_pack_len = Get_FollowCar( src, dsr );
        break;
    case 0xe:  // �����������ݰ�
        send_pack_len = Get_FirmwareUpdateData( src, dsr );
        break;
    case 0xf:  // ��ȡָ����������
        send_pack_len = Get_IllegalContent( src, dsr );
        break;
    case 0x10: // ������ϴ�����
        send_pack_len = Get_IllegalStamp( src, dsr );
        break;
    case 0x11:  // ����ָ����Ų���
        send_pack_len = Get_UpdateParameter( src, dsr );
        break;
    case 0x12:  // ����ָ���������
        send_pack_len = Get_UpdateSpeech( src, dsr );
        break;
    case 0x13:  // �黹������
        send_pack_len = Get_ReturnDrvBall( src, dsr );
        break;
    case 0x14:
        send_pack_len = Get_Distance( src, dsr );
        break;
    default:
        break;
    }
    return send_pack_len;
}

