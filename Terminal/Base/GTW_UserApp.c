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
* Description    : 封装里程数为json内容
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
* Description    : 封装单条罚单为json内容
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
* Description    : 验证驾驶员
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
        XFS_WriteBuffer(xfs_auto, "驾照类型不符合资格");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "APP发来驾驶证类型=%d,资格不符合", i);
#endif
        *s1++ = 2;
    }
    else {
        switch( src[19] ) {
        case 1:	{
            if ( (src[40] == '1') && VerifyTelephone( src + 41 ) == 1 ) {
#ifdef __DEBUG
                //XFS_WriteBuffer(xfs_auto, "申请一,验证亲情号");
                if(m_var.Flag_Debug_Enable == SET) {
                    memset( buf, 0, sizeof( buf ) );
                    memcpy( buf, src + 20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );

                    memset( buf2, 0, sizeof(buf) );
                    memcpy( buf2, src + 41, __PHONE_LENGTH__ );

                    Debug_Printf(SET, "申请一:验证亲情号(亲情号=%s,驾照=%s)", buf2, buf);
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
                XFS_WriteBuffer(xfs_auto, "亲情号错误");
                if(m_var.Flag_Debug_Enable == SET) {
                    memset( buf, 0, sizeof(buf) );
                    memcpy( buf, src + 41, __PHONE_LENGTH__ );
                    Debug_Printf( SET, "申请一:亲情号(%s)错误", buf );
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
                XFS_WriteBuffer(xfs_auto, "申请二,验证授权码");
                if(m_var.Flag_Debug_Enable == SET) {
                    memset( buf, 0, sizeof( buf ) );
                    memcpy( buf, src + 20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );

                    memset( buf2, 0, sizeof(buf) );
                    memcpy( buf2, src + 41, __PHONE_LENGTH__ );

                    Debug_Printf(SET, "申请二:验证亲情号(授权码=%s,驾照=%s)", buf2, buf);
                }
#endif
                Flag_ApplyResult = SET;
                PushApplyStack(&ApplyLicense.ThiApplyLic, src+20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
                History_UseCar(src+20, 0);//保存用车记录
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
                XFS_WriteBuffer(xfs_auto, "申请二,授权码错误");
                if(m_var.Flag_Debug_Enable == SET) {
                    memset( buf, 0, sizeof(buf) );
                    memcpy( buf, src + 41, __AUTHORIZE_LEN__ );
                    Debug_Printf( SET, "申请二:授权码(%s)错误", buf );
                }
#endif
                *s1++ = 1;
            }
            break;
        }

        case 3: {
            if (( src[40] == '1' ) && VerifyTelephone(src + 41) == 1 ) {
#ifdef __DEBUG
                XFS_WriteBuffer(xfs_auto, "申请三,验证亲情号");
                if(m_var.Flag_Debug_Enable == SET) {
                    memset( buf, 0, sizeof( buf ) );
                    memcpy( buf, src + 20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );

                    memset( buf2, 0, sizeof(buf) );
                    memcpy( buf2, src + 41, __PHONE_LENGTH__ );

                    Debug_Printf(SET, "申请三:验证亲情号(亲情号=%s,驾照=%s)", buf2, buf);
                }
#endif
                Flag_ApplyResult = SET;
                PushApplyStack(&ApplyLicense.FirApplyLic, src+20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
                History_UseCar(src+20, 0);//保存用车记录
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
                //XFS_WriteBuffer(xfs_auto, "申请三,亲情号错误");
                if(m_var.Flag_Debug_Enable == SET) {
                    memset( buf, 0, sizeof(buf) );
                    memcpy( buf, src + 41, __PHONE_LENGTH__ );
                    Debug_Printf( SET, "申请三:亲情号(%s)错误", buf );
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
                //XFS_WriteBuffer(xfs_auto, "申请四,验证授权码");
                if(m_var.Flag_Debug_Enable == SET) {
                    memset( buf, 0, sizeof( buf ) );
                    memcpy( buf, src + 20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );

                    memset( buf2, 0, sizeof(buf) );
                    memcpy( buf2, src + 41, __PHONE_LENGTH__ );

                    Debug_Printf(SET, "申请四:验证授权码(授权码=%s,驾照=%s)", buf2, buf);
                }
#endif
                Flag_ApplyResult = SET;
                PushApplyStack(&ApplyLicense.FirApplyLic, src+20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
                History_UseCar(src+20, 0);//保存用车记录
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
                XFS_WriteBuffer(xfs_auto, "申请四,授权码错误");
                if(m_var.Flag_Debug_Enable == SET) {
                    memset( buf, 0, sizeof(buf) );
                    memcpy( buf, src + 41, __AUTHORIZE_LEN__ );
                    Debug_Printf( SET, "申请四:授权码(驾照=%s)错误", buf );
                }
#endif
                *s1++ = 1;
            }
            break;
        }

        case 5: {
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "申请五,强制验证");
            if(m_var.Flag_Debug_Enable == SET) {
                memset( buf, 0, sizeof( buf ) );
                memcpy( buf, src + 20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );

                memset( buf2, 0, sizeof(buf) );
                memcpy( buf2, src + 41, __PHONE_LENGTH__ );

                Debug_Printf(SET, "申请五:强制验证(验证码=%s,驾照=%s)", buf2, buf);
            }
#endif
            PushApplyStack(&ApplyLicense.FifApplyLic, src+20, __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ );
            History_UseCar(src+20, 0);//保存用车记录
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
* Description    : 读取罚单条数
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
       XFS_WriteBuffer(xfs_auto, "读未上传违章数");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "未上传违章数=%d", ViolenceUntreated );
#endif
        break;
    case 2:
        *s1++ = HIBYTE(ViolenceTotal);
        *s1++ = LOBYTE(ViolenceTotal);
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "读所有违章数");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "读所有违章数=%d", ViolenceTotal );
#endif
        break;
    default:
        *s1++ = 0;
        *s1++ = 1;
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "未知读取违章数");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "未知读取违章数" );
#endif
        break;
    }
    return GTW_Packet( dsr, 0x802, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_IllegalContent
* Description    : 0x000f:读取指定罚单内容
******************************************************************************/
unsigned short Get_IllegalContent(unsigned char* src, unsigned char* dsr)
{
#ifdef __DEBUG
    const unsigned char eTicket[3][13] = { "读未上传罚单", "读取全部罚单", "读取一条罚单" };
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
        sprintf( buf, "%s:开始=%d,条数=%d", eTicket[ src[19] - 1 ], ill_start, ill_ReadNum );
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
* Description    : 0x010:标记未上传违章
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
    XFS_WriteBuffer( xfs_auto, "标记%d条已上传违章记录", len );
    if(m_var.Flag_Debug_Enable == SET) {
        Debug_Printf( SET, "标记%d条已上传违章记录", len );
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
* Description    : 003:更新手机号码
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
        //XFS_WriteBuffer(xfs_auto, "修改手机号");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "授权码正确,可修改手机号");
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
        //XFS_WriteBuffer(xfs_auto, "授权码错误,禁止修改手机号");
        if(m_var.Flag_Debug_Enable == SET) {
            memset( buf, 0, sizeof( buf ) );
            memcpy( buf, s2, __AUTHORIZE_LEN__ );
            Debug_Printf( SET, "授权码[%s]错误,禁止修改手机号", buf );
        }
#endif
        *s1++ = 4;
    }
    return GTW_Packet( dsr, 0x803, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_AdjuctVolume
* Description    : 0x0005=设置终端音量
******************************************************************************/
unsigned short Get_AdjuctVolume(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    unsigned char  result = 0;

    if( memcmp( &src[19], m_carinfo.Authorize_Code, __AUTHORIZE_LEN__  ) == 0 ) {
        m_carinfo.Volume_Value = src[35];
        XFS_SetVolume( m_carinfo.Volume_Value );
        XFS_WriteBuffer( xfs_auto, "祝您一路平安" );
#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET) {
            Debug_Printf( SET, "设置音量=%d", m_carinfo.Volume_Value );
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
* Description    : 开车玩手机
******************************************************************************/
unsigned short Get_PlayPhone(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    unsigned char  st;

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "收到开车玩手机通知");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "收到开车玩手机通知");
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
* Description    : 读取故障编号
******************************************************************************/
unsigned short Get_ErrorNumber(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
#ifdef __DEBUG
    //XFS_WriteBuffer(xfs_auto, "读取故障编号");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "读取故障编号");
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
* Description    : 0x9: 读取终端数据
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
    case 1:     // 读取行驶异常数据
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "读取行驶异常数据");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "读取终端行驶异常数据");
#endif
        *s1++ = 0;
        *s1++ = 0;
        break;
    case 2:     // 读取车况(用车记录)
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "读取用车记录");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "读取终端用车记录");
#endif
        // 数据长度
        *s1++ = 0;
        *s1++ = 0;
        // 数据内容格式:
        // 编码[4Byte]
        // 电子驾照[20byte]
        // 授权码[11byte]
        // 开始时间[14byte]
        // 结束时间[14byte]
        break;
    default:
        // 数据长度
        *s1++ = 0;
        *s1++ = 0;
        break;
    }
    return GTW_Packet( dsr, 0x809, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_MoveCar
* Description    : 请人移车
******************************************************************************/
unsigned short Get_MoveCar(unsigned char* src, unsigned char* dsr)
{
    int i;
    unsigned char* s1;
#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "收到移车请求");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "收到移车请求");
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
        XFS_WriteBuffer(xfs_value, "请人移车%d秒保护开始", m_carinfo.MoveCar_TimeOver );
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf( SET, "请人移车%d秒保护开始", m_carinfo.MoveCar_TimeOver );
#endif
    }
    return GTW_Packet( dsr, 0x80a, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_UpdateClock
* Description    : 00B:更新终端时间
******************************************************************************/
unsigned short Get_UpdateClock(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
#ifdef __DEBUG
    //XFS_WriteBuffer(xfs_auto, "更新终端时间");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "更新终端时间");
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
* Description    : 00C:退出驾驶状态
******************************************************************************/
unsigned short Get_LogoutCar(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "退出驾驶状态");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "退出驾驶状态");
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
    History_UseCar(src+17, 255);//保存用车记录
    *s1++ = 0;

    History_UseCar( src, 0xff ); // 17.6.13 csYxj(728920175@qq.com)

    return GTW_Packet( dsr, 0x80c, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_FollowCar
* Description    : 0x000d:电子驾照随车通知
******************************************************************************/
unsigned short Get_FollowCar(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;

#ifdef __DEBUG
    char buf[24] = {0};
   // XFS_WriteBuffer( xfs_auto, "驾照随车通知" );
    if( m_var.Flag_Debug_Enable == SET ) {
        memset( buf, 0, sizeof(buf) );
        memcpy( buf, src + 19, __DRIVERID_LEN__ );
        Debug_Printf( RESET, "驾照随车通知,驾照号=%s", buf );
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
    History_UseCar(src+17, 255);//保存用车记录

    return GTW_Packet( dsr, 0x80d, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}

/*******************************************************************************
* Function Name  : Get_FirmwareUpdateInfo
* Description    : 007:更新终端固件
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
    XFS_WriteBuffer(xfs_auto, "获取升级信息");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "升级信息(recv[%s], self[%s])", buf, updateInfo.version);
#endif

    if(strcmp( (const char*)updateInfo.version, (const char*)buf ) == 0) {
        ver_cmp = 0;
        result = 6;
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "相同版本,不用升级");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "相同版本,不用升级");
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
* Description    : 0x000e:传送升级数据包
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
            XFS_WriteBuffer(xfs_num, "接收完成,共收到%d包数据", update.pack_count);
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "接收完成,共收到%d包数据", update.pack_count);
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
            XFS_WriteBuffer(xfs_num, "开始接收升级数据包");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "开始接收升级数据包");
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
* Description    : 0x0011:更新参数
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
        //XFS_WriteBuffer( xfs_auto, "更新%s参数", key );
        if( m_var.Flag_Debug_Enable == SET ) {
            Debug_Printf( RESET, "更新参数: %s=%s", key, value );
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
                case 1:   // 蓝牙数据包长度
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
                case 1:   // 终端进入状态二后至提示防盗时间
                    m_carinfo.FD01_TimeOver = atoi(value);
                    break;
                case 2:   // 终端进入状态二后至触发防盗时间
                    m_carinfo.FD02_TimeOver = atoi(value);
                    break;
                case 3:   // 触发防盗前上传车辆位置周期
                    m_carinfo.FD03_TimeOver = atoi(value);
                    break;
                case 4:   // 触发防盗后上传车辆位置周期
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
                case 1:   // 车辆开动后至提示验证驾驶员时间
                    m_carinfo.JZ01_TimeOver = atoi(value);
                    break;
                case 2:   // 车辆开动后至提示报无证驾驶时间
                    m_carinfo.JZ02_TimeOver = atoi(value);
                    break;
                case 3:   // 自动登录驾照排队时间（超过此时间未通信的驾照移出队列）
                    m_carinfo.JZ03_TimeOver = atoi(value);
                    break;
                case 4:   // 从状态一进状态二后车辆不点火、无任何驾照发来申请，等待多长时间恢复状态一
                    m_carinfo.JZ04_TimeOver = atoi(value);
                    break;
                case 5:   // 状态三下无任何驾照发来申请，等待多长时间恢复状态一（未点火）／状态二（点火）
                    m_carinfo.JZ05_TimeOver = atoi(value);
                    break;
                case 6:   // 状态四下多长时间没有接收到驾照随车信息恢复状态一（停车熄火）／状态二（未熄火）
                    m_carinfo.JZ06_TimeOver = atoi(value);
                    break;
                case 7:   // 选定驾驶员后，多长时间接收不到此驾照的下一次申请，才去选择次优先级的申请
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
                // 随车参数
                i = atoi( s2 + 1 );
                switch(*(s2+2)) {
                case '1':   // 随车通知超时参数:{"K":"SC01";"V":"600"}
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
                case 1: // 从开罚单开始允许撤消违停的时间长度
                    m_carinfo.WT01_TimeOver = atoi(value);
                    break;
                case 2: // 离开被开罚单位置多远可自动撤销违停
                    m_carinfo.WT02_DistOver = atoi(value);
                    break;
                default:
                    result = 3;
                    break;
                }
            }
            else if(*s2 == 'Z' || *s2 == 'z' ) {
                switch(i) {
                case 1:  // 连续行车状态下，连续收到几次使用手机的信息（同一次使用手机）才记录违章
                    m_carinfo.WZ01_CountOver = atoi(value);
                    break;
                case 2:  // 前后两个使用手机信号间隔多久认为是另一次使用手机，重新计次判断是否记录违章
                    m_carinfo.WZ02_TimeOver = atoi(value);
                    break;
                case 3:  // 车辆行驶后多久提示系安全带
                    m_carinfo.WZ03_TimeOver = atoi(value);
                    break;
                case 4:  // 车辆行驶后多久记录不系安全带违章
                    m_carinfo.WZ04_TimeOver = atoi(value);
                    break;
                case 5:  // 收到对向来车变光信号后多久不变光记违章
                    m_carinfo.WZ05_TimeOver = atoi(value);
                    break;
                case 6:  // 收到禁用远光信号后多久不变光记违章
                    m_carinfo.WZ06_TimeOver = atoi(value);
                    break;
                case 7:  // 大客车夜间禁行开始时间
                    sprintf( (char*)m_carinfo.WZ07_Timer, "%s", value );
                    break;
                case 8:  // 大客车夜间禁行结束时间
                    sprintf( (char*)m_carinfo.WZ08_Timer, "%s", value );
                    break;
                case 9:  // 大客车夜间禁行开始预警时间（在此时间之后，禁行开始时间之前，进入高速立即预警）
                    sprintf( (char*)m_carinfo.WZ09_Timer, "%s", value );
                    break;
                case 10: // 大客车夜间禁行预警周期（第一次提醒后，禁行开始时间之前，隔多久提醒一次）
                    m_carinfo.WZ10_TimeOver = atoi(value);
                    break;
                case 11: // 大客车夜间禁行时段上高速后，多长时间记录违章
                    m_carinfo.WZ11_TimeOver = atoi(value);
                    break;
                case 12: // 24小时内累计驾车多长时间以上向终端发信息提醒疲劳驾驶
                    m_carinfo.WZ12_TimeOver = atoi(value);
                    break;
                case 13: // 单次累计驾车多久提醒疲劳驾驶
                    m_carinfo.WZ13_TimeOver = atoi(value);
                    break;
                case 14: // 疲劳驾驶提醒循环周期
                    m_carinfo.WZ14_TimeOver = atoi(value);
                    break;
                case 15: // 提醒疲劳驾驶后多久不休息或更换驾驶员记违章
                    m_carinfo.WZ15_TimeOver = atoi(value);
                    break;
                case 16: // 单次累计驾车进入疲劳驾驶后休息多久重新计时
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
                case 1: // sec,请人移车保护时间：在该时间内不再拨号及发短信
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
* Description    : 0x0012:更新语音
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
            case  1:    // 您已进入禁用远光灯区域，请不要开启远光灯
                break;
            case  2:    // 请您关闭远光灯，否则记录违章
                break;
            case  3:    // 您因违规使用远光灯已被记录违章
                break;
            case  4:    // 请您出示电子驾照
                break;
            case  5:    // 请您出示电子驾照
                break;
            case  6:    // 电子驾照验证完成
                break;
            case  7:    // 您涉嫌无证驾驶，已记录违章
                break;
            case  8:    // 请您系好安全带，否则记录违章
                break;
            case  9:    // 您因未系安全带，已被记录违章
                break;
            case 10:    // 请您正确使用安全带，否则记录违章
                break;
            case 11:    // 您因违规使用安全带，已被记录违章
                break;
            case 12:    // 提醒驾驶员休息，重复多次
                break;
            case 13:    // 提醒驾驶员休息，重复多次
                break;
            case 14:    // 您已连续驾驶超过XX小时XX分钟，请您在前方适当位置停车休息20分钟以上再继续驾驶。若连续驾驶超过4小时，将被记录违章。如需更换驾驶员，请验证新驾驶员电子驾照。
                break;
            case 15:    // 您已连续驾驶超过XX小时XX分钟，请您在前方适当位置停车休息20分钟以上再继续驾驶。若连续驾驶超过4小时，将被记录违章。如需更换驾驶员，请验证新驾驶员电子驾照。
                break;
            case 16:    // 您24小时内已累计驾驶超过7小时XX分钟，请您在前方适当位置停车休息或更换驾驶员。若累计驾驶超过8小时，将被记录违章。
                break;
            case 17:    // 您因疲劳驾驶，已被记录违章
                break;
            case 18:    // 请您凌晨2:00至5:00停运或更换驾驶员，否则记录违章
                break;
            case 19:    // 请您凌晨2:00至5:00停运或更换驾驶员，否则记录违章
                break;
            case 20:    // 请您凌晨2:00至5:00停运或更换驾驶员，否则记录违章
                break;
            case 21:    // 请您允分休息后再驾车，否则记录违章?
                break;
            case 22:    // 您在夜间进入三级以下公路，请尽快停车或离开此区域，否则记录违章
                break;
            case 23:    // 您因违反大客车夜间行驶规定，已被记录违章
                break;
            case 24:    //
                break;
            case 25:    // 您已接近限行区域，请及时改道
                break;
            case 26:    // 您因违反限行规定，已被记录违章
                break;
            case 27:    // 当前限速XX，您已超速XX%，请控制车速，否则记录违章
                break;
            case 28:    // 您因超速，已被记录违章
                break;
            case 29:    // 当前已进入/离开禁鸣区域
                break;
            case 30:    // 此区域禁鸣
                break;
            case 31:    // 您因违反禁鸣规定，已被记录违章
                break;
            case 32:    // 您的驾照等级太低，不能驾驶该车，否则将记录违章
                break;
            case 33:    // 智能终端再根据要求进行自动语音播报
                break;
            case 34:    // 祝您一路顺风
                break;
            case 35:    // 请停止违章使用手机，否则记录违章
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
* Description    : 0x0013:归还驾照球
******************************************************************************/
unsigned short Get_ReturnDrvBall(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "归还驾照球");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "归还驾照球");
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
* Description    : 返回里程数
******************************************************************************/
unsigned short Get_Distance(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;
    unsigned short len = 0;

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "读取里程数");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "读取里程数=%.5f", m_carinfo.distance );
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
* Description    : 解析用户手机APP消息
******************************************************************************/
unsigned short GTW_UserAPP(unsigned char* src, unsigned char* dsr)
{
    unsigned short imsg;
    unsigned short send_pack_len = 0;
#ifdef __DEBUG
    unsigned char  buf[24] = {0};
#endif

    // 消息ID
    imsg = MAKEWORD( src[1], src[2] );

    if( memcmp(src + HEAD_LENGTH, m_carinfo.Car_9SN, __CAR9SN_LEN__ ) != 0) {
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "车牌号不对");
        if(m_var.Flag_Debug_Enable == SET) {
            memset( buf, 0, sizeof(buf) );
            memcpy( buf, src + HEAD_LENGTH, __CAR9SN_LEN__ );
            Debug_Printf(SET,
                         "伴君行(%04x)操作的车牌号(%s)[%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x,%02x]不对",
                         imsg, buf,
                         buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8] );
        }
#endif
        return 0;
    }
#ifdef __DEBUG
		//Debug_Printf( SET, "收到用户手机APP消息=%04x", m_var.Flag_Debug_Enable );
     if(m_var.Flag_Debug_Enable == SET){
				Debug_Printf( SET, "收到用户手机APP消息=%04x", imsg );
		 }
        
#endif
    switch(imsg) {
    case 0x1:   // 验证驾驶员
        send_pack_len = Verify_Driver( src, dsr );
        break;
    case 0x2:   // 读取罚单数量
        send_pack_len = Get_IllegalNumber( src, dsr );
        break;
    case 0x3:   // 修改手机号码
        send_pack_len = Get_UpdatePhone( src, dsr );
        break;
    case 0x4:   // 回复相关数据的处理状态: 0x00=违章数据; 0x01=异常车速数据; 0x02=用车记录
        send_pack_len = Get_TerminalData( src, dsr );
        break;
    case 0x5:   // 设置终端音量
        send_pack_len = Get_AdjuctVolume( src, dsr );
        break;
    case 0x6:   // 开车玩手机
        send_pack_len = Get_PlayPhone( src, dsr );
        break;
    case 0x7:   // 终端升级信息
        send_pack_len = Get_FirmwareUpdateInfo( src, dsr );
        break;
    case 0x8:   // 读取终端的故障代码
        send_pack_len = Get_ErrorNumber( src, dsr );
        break;
    case 0x9:   // 读取终端数据命令: 0x01: 读取行驶异常数据; 0x02: 读取车况(用车记录)
        send_pack_len = Get_TerminalData( src, dsr );
        break;
    case 0xa:  // 请人移车
        send_pack_len = Get_MoveCar( src, dsr );
        break;
    case 0xb:  // 更新终端时间
        send_pack_len = Get_UpdateClock( src, dsr );
        break;
    case 0xc:  // 退出驾驶状态
        send_pack_len = Get_LogoutCar( src, dsr );
        break;
    case 0xd:  // 电子驾照随车通知
        send_pack_len = Get_FollowCar( src, dsr );
        break;
    case 0xe:  // 传送升级数据包
        send_pack_len = Get_FirmwareUpdateData( src, dsr );
        break;
    case 0xf:  // 读取指定罚单内容
        send_pack_len = Get_IllegalContent( src, dsr );
        break;
    case 0x10: // 标记已上传罚单
        send_pack_len = Get_IllegalStamp( src, dsr );
        break;
    case 0x11:  // 更新指定编号参数
        send_pack_len = Get_UpdateParameter( src, dsr );
        break;
    case 0x12:  // 更新指定编号语音
        send_pack_len = Get_UpdateSpeech( src, dsr );
        break;
    case 0x13:  // 归还驾照球
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

