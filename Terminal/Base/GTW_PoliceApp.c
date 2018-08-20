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
* Description    : ���ź�������ʾ���ӷ���
******************************************************************************/
void eTicket_VoicePrompt(void)
{
    if(Flag_NeweTicket == SET &&
            Car_InputCheck.Value_DriverDoor == SET)
    {
        Flag_NeweTicket = RESET;
        XFS_WriteBuffer(xfs_auto, "����һ��Υ����Ϣ�������ֻ���ʱ�鿴����");
#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "����һ��Υ����Ϣ�������ֻ���ʱ�鿴����");
#endif
    }
}


/*******************************************************************************
* Function Name  : Get_MatterTicket
* Description    : ������Ϣ����
******************************************************************************/
unsigned short Get_MatterTicket(unsigned char* src, unsigned char* dsr)
{
    i2b_Def i2b;
    unsigned char* s1;
    unsigned short illTotal;

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "������Ϣ����");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "������Ϣ����");
#endif

    // ����Ӧ��ط�����
    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    // ���ƺ�[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // ���ƺ�
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;

    // ��ʻ֤��
    if((is_emptyApplyData(&ApplyLicense) == 0)) {
        memset(s1, '0', __DRIVERID_LEN__ );
    }
    else {
        memcpy(s1, &m_carinfo.Driver_ID[0][0] + __DRIVER_TYPE_LEN__, __DRIVERID_LEN__);
    }
    s1 += __DRIVERID_LEN__;

    illTotal = MX25L128_Word_read(ADDR_ILLEGAL_UPLOAD);     // ��ȡΥ������

    memcpy( Ill_ETicket.stime, src+21, 14 );                // ����PolicApp��Υ�¿�ʼʱ��
    memcpy( Ill_ETicket.etime, src+21, 14 );                // Υ�½���ʱ��

    Ill_ETicket.ewhemi = *(src + 35);                       // ����/����,E:����;W:����
    Ill_ETicket.longitude = Asc2Float((char*)src+36, 10);   // Υ��ʱ����
    Ill_ETicket.nshemi = *(src + 46);                       // ��γ/��γ,N:��γ;S:��γ
    Ill_ETicket.latitude = Asc2Float((char*)src+47, 10);    // Υ��ʱγ��

    if( Asc2Byte((char*)src+9, 2) == 0 )
        Ill_ETicket.ino = ino_IllegalPark;                  // Υ�´��� = Υͣ
    else
        Ill_ETicket.ino = Asc2Byte((char*)src+9, 2);

    i2b.x = MX25L128_Illegal_Write_Base(&Ill_ETicket, RESET);       // дΥ������

    *s1++ = i2b.c[3];
    *s1++ = i2b.c[2];
    *s1++ = i2b.c[1];
    *s1++ = i2b.c[0];

    *s1++ = HIBYTE(illTotal);
    *s1++ = LOBYTE(illTotal);

    Flag_NeweTicket = SET;      // ����������ʾ��־

    {
        // Υͣ���ӷ���,���Ͷ��ż�����绰
        GTW_IllegalPark.Flag_Main = SET;            // ���ڼ���Ƿ���ϳ�������
        GTW_IllegalPark.Flag_Call = SET;
        GTW_IllegalPark.Flag_SMS = SET;
        GTW_IllegalPark.lpszMsg = NULL;
        GTW_IllegalPark.Sec_OverTime = 10 * 60;   // ����Υͣ������10������,�����������뿪��������λ��500������ʱ������ȡ��Υͣ�źŸ��ֻ�
    }

    return GTW_Packet( dsr, 0x802, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}


/*******************************************************************************
* Function Name  : Get_EleTicket
* Description    : �յ����ӷ���
******************************************************************************/
unsigned short Get_EleTicket(unsigned char* src, unsigned char* dsr)
{
    i2b_Def i2b;
    unsigned char* s1;
    unsigned short illTotal;

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "�յ����ӷ���");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "�����ӷ���");
#endif

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    // ���ƺ�[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // ���ƺ�
    s1 += __CAR9SN_LEN__;

    *s1++ = 0;

    // ��ʻ֤��
    if( m_carinfo.Driver_ID[0][0] == 0x00 ) {
        memset(s1, '0', __DRIVERID_LEN__ );
    }
    else {
        memcpy(s1, &m_carinfo.Driver_ID[0][0] + __DRIVER_TYPE_LEN__, __DRIVERID_LEN__);
    }

    s1 += __DRIVERID_LEN__;

    illTotal = MX25L128_Word_read(ADDR_ILLEGAL_UPLOAD);//��ȡΥ������

    memcpy( Ill_ETicket.stime, src+21, 14 );                // ����PolicApp��Υ�¿�ʼʱ��
    memcpy( Ill_ETicket.etime, src+21, 14 );                // Υ�½���ʱ��

    Ill_ETicket.ewhemi = *(src + 35);                       // ����/����,E:����;W:����
    Ill_ETicket.longitude = Asc2Float((char*)src+36, 10);   // Υ��ʱ����
    Ill_ETicket.nshemi = *(src + 46);                       // ��γ/��γ,N:��γ;S:��γ
    Ill_ETicket.latitude = Asc2Float((char*)src+47, 10);    // Υ��ʱγ��

    Ill_ETicket.ino = ino_IllegalPark;                  // Υ�´��� = Υͣ

    i2b.x = MX25L128_Illegal_Write_Base(&Ill_ETicket, RESET);       // дΥ������

    *s1++ = i2b.c[3];
    *s1++ = i2b.c[2];
    *s1++ = i2b.c[1];
    *s1++ = i2b.c[0];

    *s1++ = HIBYTE(illTotal);
    *s1++ = LOBYTE(illTotal);

    Flag_NeweTicket = SET;

    {
        // Υͣ���ӷ���,���Ͷ��ż�����绰
        GTW_IllegalPark.Flag_Main = SET;            // ���ڼ���Ƿ���ϳ�������
        GTW_IllegalPark.Flag_Call = SET;
        GTW_IllegalPark.Flag_SMS = SET;
        GTW_IllegalPark.lpszMsg = NULL;
        GTW_IllegalPark.Sec_OverTime = 10 * 60;   // ����Υͣ������10������,�����������뿪��������λ��500������ʱ������ȡ��Υͣ�źŸ��ֻ�
    }

    return GTW_Packet( dsr, 0x803, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}


/*******************************************************************************
* Function Name  : Get_PoliceCheck
* Description    : �յ�����ͨ����
******************************************************************************/
unsigned short Get_PoliceCheck(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;

#ifdef __DEBUG
    //XFS_WriteBuffer(xfs_auto, "����ͨ����");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "����ͨ����");
#endif

    Flag_RevCopData = SET;

    // ����Ӧ��ط�����
    s1 = dsr + HEAD_LENGTH;


    *s1++ = src[8];
    *s1++ = src[9];


    *s1++ = src[1];
    *s1++ = src[2];

    // ���ƺ�[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // ���ƺ�
    s1 += __CAR9SN_LEN__;

    // ���[1Byte]: 0=�ɹ�/ȷ��;1=ʧ��;2=��Ϣ����;3=��֧��
    *s1++ = 0;

    return GTW_Packet( dsr, 0x804, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}


/*******************************************************************************
* Function Name  : Get_ReadAccident
* Description    : �յ���ȡ�¹ʷ�����������
******************************************************************************/
unsigned short Get_ReadAccident(unsigned char* src, unsigned char* dsr)
{
    unsigned char* s1;

    const unsigned char CarFault[38] = "20161212080808P000120161212202020P0002";
    const unsigned char CarChangLight[15] = "201612121010101";
    const unsigned char DriveFlase[48] = "20161212090909120110100090080070060050040005";
    const unsigned char TestBuf[87] = {
        0x00,0x00,0x00,0x01,    //4byte Υ�����ݵ�Ψһ��־��01��
        '1','1',0xE8,0x92,0x99,'A','1','2','3','4','5',// 11byte
        '2','0','1','6','1','2','1','2','1','4','1','4','1','4',//    14byte
        '2','0','1','6','1','2','1','2','1','4','1','4','3','0',//          14byte
        '1','1','1','2','3','4','5','6','7','8','9','1','2','3','4','5','6','7','8','9',   //   20byte
        '1','1','6','.','4','3','2','1','5','2',',','2','3','.','9','4','7','0','8',' ',' ',' ',	//      22byte
        0x85,0x0D
    };   //    1byte

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "��ȡ�¹ʷ�������");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "��ȡ�¹ʷ�������");
#endif

    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    // ���ƺ�[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // ���ƺ�
    s1 += __CAR9SN_LEN__;

    switch( src[19] )
    {
    case '0':
    {
        //�����ͻ�������ֵ�������ͳ�ȥ-----------------------------*/
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

    case '1':           // �ɹ�/ȷ��
    case '2':           // ʧ��
    case '3':           // ��Ϣ����
    case '4':           // ��֧��
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
* Description    : ���ʱ���״̬Ϊjson����
******************************************************************************/
unsigned short Lamp_json_packet(unsigned char* dsr, Lamp_Def lamp)
{
    char* s1 = (char*)dsr;
    char buf[40] = { 0 };
    char buf1[40] = { 0 };

    // ���ʱ��
    *s1++ = '{';

    memset( buf1, 0, sizeof(buf1) );
    memcpy( buf1, lamp.ptime, sizeof(lamp.ptime) );
    sprintf( buf, "\"time\":\"%s\",", buf1 );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    //�����״̬
    sprintf( buf, "\"code\":\"%d\"", lamp.pstatus);
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    *s1++ = '}';

    return(s1 - (char*)dsr);
}



/*******************************************************************************
* Function Name  : Get_LampChang
* Description    : �յ���ȡ�����������
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
    XFS_WriteBuffer(xfs_auto, "��ȡ�������ָ��");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "��ȡ�������ָ��");
#endif

    // ����Ӧ��ط�����
    s1 = dsr + HEAD_LENGTH;

    *s1++ = src[8];
    *s1++ = src[9];

    *s1++ = src[1];
    *s1++ = src[2];

    // ���ƺ�[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // ���ƺ�
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
* Description    : �쳣����ʱ�䡢״�����ٶ�Ϊjson����


******************************************************************************/
unsigned short AbnomalSpeed_json_packet(unsigned char* dsr, AbnomalSpeed pspeed)
{
    char* s1 = (char*)dsr;
    char buf[40] = { 0 };
    char buf1[40] = { 0 };

    // �쳣����ʱ��
    *s1++ = '{';

    memset( buf1, 0, sizeof(buf1) );
    memcpy( buf1, pspeed.Nspeedtime, sizeof(pspeed.Nspeedtime) );
    sprintf( buf, "\"time\":\"%s\",", buf1 );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    //�쳣����״̬
    sprintf( buf, "\"code\":\"%d\",", pspeed.Nspeedtimestatus);
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    //�쳣��������
    sprintf( buf, "\"speed\":\"%d\"", pspeed.Nspeeddata);
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    *s1++ = '}';

    return(s1 - (char*)dsr);
}



/*******************************************************************************
* Function Name  : Get_AbnomalSpeed
* Description    : �յ���ȡ�쳣��������
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
    XFS_WriteBuffer(xfs_auto, "��ȡ�쳣��������ָ��");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "��ȡ�쳣��������ָ��");
#endif
    // ����Ӧ��ط�����
    s1 = dsr + HEAD_LENGTH;


    *s1++ = src[8];
    *s1++ = src[9];


    *s1++ = src[1];
    *s1++ = src[2];

    // ���ƺ�[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // ���ƺ�
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
* Description    : ��ȫ���쳣ʱ���״̬Ϊjson����
******************************************************************************/
unsigned short belt_json_packet(unsigned char* dsr, Safebelt pbelt)
{
    char* s1 = (char*)dsr;
    char buf[40] = { 0 };
    char buf1[40] = { 0 };

    // ���ʱ��
    *s1++ = '{';

    memset( buf1, 0, sizeof(buf1) );
    memcpy( buf1, pbelt.belttime, sizeof(pbelt.belttime) );
    sprintf( buf, "\"time\":\"%s\",", buf1 );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    //�����״̬
    sprintf( buf, "\"code\":\"%d\"", pbelt.beltstatus);
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    *s1++ = '}';

    return(s1 - (char*)dsr);
}

/*******************************************************************************
* Function Name  : Get_SafeBelt
* Description    : �յ���ȡ��ȫ����¼����
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
    XFS_WriteBuffer(xfs_auto, "��ȡ��ȫ����¼����");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "��ȡ��ȫ����¼����");
#endif

    // ����Ӧ��ط�����
    s1 = dsr + HEAD_LENGTH;


    *s1++ = src[8];
    *s1++ = src[9];


    *s1++ = src[1];
    *s1++ = src[2];

    // ���ƺ�[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // ���ƺ�
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
* Description    : ��ȫ���쳣ʱ���״̬Ϊjson����
******************************************************************************/
unsigned short obd_json_packet(unsigned char* dsr, OBDdata obdt)
{
    char* s1 = (char*)dsr;
    char buf[40] = { 0 };
    char buf1[40] = { 0 };

    // ʱ��
    *s1++ = '{';

    memset( buf1, 0, sizeof(buf1) );
    memcpy( buf1, obdt.obdttime, sizeof(obdt.obdttime) );
    sprintf( buf, "\"time\":\"%s\",", buf1 );
    sprintf( s1, "%s", buf );
    s1 += strlen(buf);

    //���ϱ���
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
* Description    : �յ���ȡOBD��������
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
    XFS_WriteBuffer(xfs_auto, "��ȡOBD���ϼ�¼����");
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "��ȡOBD���ϼ�¼����");
#endif

    s1 = dsr + HEAD_LENGTH;


    *s1++ = src[8];
    *s1++ = src[9];


    *s1++ = src[1];
    *s1++ = src[2];

    // ���ƺ�[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // ���ƺ�
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
* Description    : ��������ͨAPP��Ϣ
******************************************************************************/
extern unsigned short Get_IllegalContent(unsigned char* src, unsigned char* dsr);
u16 GTW_PoliceAPP(unsigned char* src, unsigned char* dsr)
{
    unsigned short send_pack_len = 0;
    unsigned short imsg;

#ifdef __DEBUG
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "����ͨ��Ϣ");
#endif

    // ����ͨ���͹���������
    if(memcmp(src+10, m_carinfo.Car_9SN, __CAR9SN_LEN__) != 0) {
        return 0;
    }
    else {
        imsg = MAKEWORD( src[1], src[2] );
        switch(imsg) {
        case 0x1:   // ��ȡָ����������
            send_pack_len = Get_IllegalContent( src, dsr );
            break;

        case 0x2:   // ������Ϣ����
            send_pack_len = Get_MatterTicket( src, dsr );
            break;

        case 0x3:   // �����ӷ���
            send_pack_len = Get_EleTicket( src, dsr );
            break;

        case 0x4:   // ����ͨ����
            send_pack_len = Get_PoliceCheck( src, dsr );
            break;

        case 0x5:   // ��ȡ�¹ʷ������� -- ����ԭ������
            send_pack_len = Get_ReadAccident( src, dsr );
            break;

        case 0x6:   // �ټ�
            break;

        case 0x7:   // ��ȡ�������
            send_pack_len = Get_LampChang( src, dsr );
            break;

        case 0x8:   // ��ȡ�쳣��������
            send_pack_len = Get_AbnomalSpeed( src, dsr );
            break;

        case 0x9:   // ��ȡ��ȫ����¼����
            send_pack_len = Get_SafeBelt( src, dsr );
            break;

        case 0x10: // ��ȡOBD����
            send_pack_len = Get_OBDdata( src, dsr );
            break;
        }
    }

    return send_pack_len;
}
