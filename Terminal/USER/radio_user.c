/******************** (C) COPYRIGHT soarsky ********************
* File Name          : radio.c
* Author             :
* Version            :
* Date               :
* Description        : ����ģ��ʹ�õ���si4432оƬ
            ----------------------------------------------
*���ݸ�ʽ��|Preamble|SYNC|HEADER ADDR|PK LENGTH|DATA|CRC |
            ----------------------------------------------

*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"

#define SI4432_DATARATE_128K
#ifdef SI4432_DATARATE_128K
#define TIMESLOT	3
#endif
#ifdef SI4432_DATARATE_100K
#define TIMESLOT	3
#endif
#ifdef SI4432_DATARATE_38K4
#define TIMESLOT	8
#endif
#ifdef SI4432_DATARATE_9K6
#define TIMESLOT	30
#endif


/* Global variables ---------------------------------------------------------*/
volatile unsigned char  Si4432_SendTimeOut;
volatile unsigned char  Fla_ReadTicketCmd = RESET;
volatile unsigned char  Flag_InspectCmd = RESET;
volatile unsigned char  Flag_SendEPN = ERROR;
volatile unsigned char  CheckerAnswer_index = 0;
volatile unsigned int   CheckerAnswer_timer;//Ӧ����ʱ
volatile unsigned short Si4432_TimeSendChangeLight = 0;
volatile unsigned short Si4432_TimeSendSn = 0;

unsigned short  S_Timer[8];  //�����������ʱʱ��

/***************���غ궨��*************/

/***************���غ�������*************/
static void Si4432_Send_EPlateNum(unsigned char * src);
//static void Si4432_RxBufferProcess(void);

/***************���ر�������*************/
static unsigned char CheckerAnswer_CMDno;       // �����������Ļ����������
i2b_Def CheckerAnswer_ID;                       // �ɼ���ID����λ

Msg_Sign_DataCollector_Struct DataCollectorMsg; // ���ݲɼ��ǻ�������ı�־��Ϣ
Msg_Sign_DataCollector_Struct GTW02_BoardCast_Msg;
unsigned char RF433_MsgMask = 0;
unsigned char RF433_MsgData = 0;


Direction_Speed_Group_Struct Dir_SGroup;

/*******************************************************************************
* Function Name  : Si4432_Process
* Description    : Si4432���߽�������,�������ݴ�����.ֻ���ڳ���������ʻ״̬ʱ�ŵ��øú���
******************************************************************************/
void Si4432_CheckLight(void)
{
    static u8  SendChangeLight_Step = 0;
    static u32 Send_StartTime = 0;

    if( Flag_ChangeLight == SET ) {  // �յ�����źŲ��÷�����ź�
        SendChangeLight_Step = 0;
	
        return;
    }

    switch(SendChangeLight_Step) {
		
    case 0:
        if(Car_InputCheck.Value_HighBeam == SET) { //Զ����Ǵ�
            Si4432_TimeSendChangeLight = 200;
            SendChangeLight_Step = 1;
        }
        break;

    case 1:
        if(Si4432_TimeSendChangeLight == 0) {
            if(Car_InputCheck.Value_HighBeam == SET) { //Զ��ƴ�
                SendChangeLight_Step = 0;
            }
            else if(Car_InputCheck.Value_HighBeam == RESET) { //Զ��ƹر�
                Send_StartTime = Sys_TimeCount;
#ifdef __DEBUG
                XFS_WriteBuffer(xfs_auto, "���ͱ��");
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf( SET, "���ͱ��");
#endif
                SendChangeLight_Step = 2;
            }
        }
        break;

    case 2:
        Si4432_SendChangeLight(Si4432_TxBuf);//���ͱ���ź�
		Debug_Printf( SET, "���ͱ���ź� %d", Si4432_TimeSendChangeLight);
		if( CheckerAnswer_CMDno > 7 )//���ͱ��ȴ�ʱ���������������������� 2017-9-21
			CheckerAnswer_CMDno = 7;
        //Si4432_TimeSendChangeLight = S_Timer[CheckerAnswer_CMDno] + rand() % 10;  //ÿ�η��ͱ���źŵļ��ʱ��
		Si4432_TimeSendChangeLight = 400;   //400ms �������һ��
        SendChangeLight_Step = 3;
        break;

    case 3:
        if(Sys_TimeCount - Send_StartTime > 4) { //4S   ����źų�����������
            SendChangeLight_Step = 4;
        }
        else {
            if(Si4432_TimeSendChangeLight == 0) {
                SendChangeLight_Step = 2;
            }
        }
		
        break;

    case 4:
        if(Si4432_TimeSendChangeLight == 0) {
            SendChangeLight_Step = 0;
        }
		//Debug_Printf( SET, "���ͱ���ź� %d", Si4432_TimeSendChangeLight);
        break;

    default:
        SendChangeLight_Step = 0;
        break;
    }
}


/*******************************************************************************
* Function Name  : Si4432_Send_EPlateNum
* Description    : ���͵��ӳ��ƺ���
******************************************************************************/
void Si4432_SendBuffer(unsigned char* src, unsigned char len)
{
    u8 i;
    Si4432_SetTxMode();//����Ϊ����ģʽ

    // ����Ҫ������������뻺����
    Si4432_WriteRegister(0x3e, len); // ������
    rst_Si4432_SEL;
    Si4432_WriteByte(0x7f | 0x80);//д��Ĵ�����ַ
    for(i = 0; i< len; i++) {
        Si4432_WriteByte(src[i]);
    }
    set_Si4432_SEL;

    Si4432_WaitTxFinished();//�ȴ������������
}


/*******************************************************************************
* Function Name  : Si4432_Send_EPlateNum
* Description    : ���͵��ӳ��ƺ���
******************************************************************************/
static void Si4432_Send_EPlateNum(unsigned char* src)
{
    unsigned char* s1 = src;
    unsigned short crc16;

    unsigned char msg_word;//��Ϣ��                         //�ϴ��ɼ�����Ϣ
    //0x00: ����
    //0x01: ��Υ��Ϊ�ϴ�
    //0x02: ��֤��ʻ
    //0x03: ��֤��Ӫ
    //0x04: ��������
    //0x05: �ڳ�
    *s1++ = 0x7f;
    memcpy( s1, m_carinfo.Car_5SN, __CAR5SN_LEN__ );
    s1 += __CAR5SN_LEN__;

    *s1++ = m_carinfo.Car_Type;
    *s1++ = msg_word;

    *s1++ = HIBYTE( crc16 );
    *s1++ = LOBYTE( crc16 );

    Si4432_SendBuffer(src, s1 - src);
}


/*******************************************************************************
* Function Name  : Get_Illegal_Untreated
* Description    : ��� �� �Ѿ��ϴ���Υ��  ��   δ�ϴ�Υ���б�  ��  ɾ��
******************************************************************************/
void  Get_Illegal_Untreated(unsigned int ill_num )
{
    unsigned short  i;
    unsigned short  j;
    unsigned int*   pill_upload;
    unsigned char   ill_remove;

    pill_upload = (unsigned int*)malloc(ViolenceUntreated * sizeof(int)); // ��δ�ϴ�������¼���
    MX25L128_BufferRead((unsigned char*)pill_upload, ADDR_ILLEGAL_UPLOAD + 8, ViolenceUntreated * sizeof(int) );

    ill_remove = 0;

    // ���ñ���Ƿ����б���
    for(j=0; j<ViolenceUntreated; j++) {
        if( pill_upload[j] == ill_num) {// �ҵ�����ɾ���ı��
            pill_upload[j] = 0xffff; // �����Ƴ����
            ill_remove++;// ɾ��������1
            break;
        }
    }

    if( ill_remove > 0 ) {
        for( i=0; i<ViolenceUntreated; i++ ) {
            if( pill_upload[i] == 0xffff ) {	        // ���߱���˵ļ�¼���
                for(j=i; j<ViolenceUntreated; j++) {	// ��ǰ�ƶ�һ����¼
                    pill_upload[j] = pill_upload[j+1];
                }
                ViolenceUntreated--;
            }
        }

        //�޸ı�Ǻ��δ�ϴ�Υ����
        MX25L128_Word_Write(ADDR_ILLEGAL_UPLOAD + 4, ViolenceUntreated);

        //����д����ϴ�Υ�¼�¼���
        MX25L128_BufferWrite((unsigned char*)pill_upload, ADDR_ILLEGAL_UPLOAD + 8, ViolenceUntreated * sizeof(int));
    }

    free(pill_upload);
}


/*******************************************************************************
* Function Name  : Si4432_Send_IllegalNum
* Description    : ����Υ������
******************************************************************************/
void Si4432_Send_IllegalNum(unsigned char* src)
{
    unsigned char* s1 = src;
    unsigned short crc16;
    unsigned short msg_id=0x0802;//��Ϣid
    Illegal_Def     illegal1;
    unsigned int    ill_Addr;
    volatile unsigned int*   pill_upload;
    unsigned char   ill_size;
    f2b_Def rev_ill_longi;
    f2b_Def rev_ill_lati;
    i2b_Def rev_ill_id;

    ill_size = (int)&illegal1.ino - (int)&illegal1.id + sizeof(illegal1.ino);
    pill_upload = (unsigned int*)malloc(  sizeof(int) );
    MX25L128_BufferRead((unsigned char*)pill_upload,
                        ADDR_ILLEGAL_UPLOAD + 8 + ( ViolenceUntreated - 1  ) * sizeof(int),
                        sizeof(int) );

    if( pill_upload[0] < (1+(0xffffff-0x300000)/ill_size) ) {
        ill_Addr = ADDR_ILLEGAL_CONTECT + ( pill_upload[0]-1) * ill_size;
        MX25L128_BufferRead( (unsigned char*)&illegal1.id, ill_Addr, ill_size );    // ������

        *s1++ = 0x7f;
        *s1++ = HIBYTE(msg_id);           //��Ϣid
        *s1++ = LOBYTE(msg_id);

        memcpy( s1, m_carinfo.Car_5SN, __CAR5SN_LEN__ ); //���ƺ�
        s1 += __CAR5SN_LEN__;

        *s1++ =  0x02;             //�������� 0x02�����ͷ�������

        rev_ill_id.x = (u32)(illegal1.id);
        *s1++ =  rev_ill_id.c[3];     
        *s1++ =  rev_ill_id.c[2];     
        *s1++ =  rev_ill_id.c[1];     
        *s1++ =  rev_ill_id.c[0];     
        *s1++ =  illegal1.ptype;      

        memcpy(s1, illegal1.carno, __CAR9SN_LEN__);
        s1 += __CAR9SN_LEN__;

        Str2BcdTime(illegal1.stime,s1,14);
        s1 += 7;

        Str2BcdTime(illegal1.etime,s1,14); 
        s1 += 7;

        *s1++ =  illegal1.driver_type;   //׼�ݳ���

        Str2BcdDriverNo(illegal1.driver_no, s1);
        s1 += 9;

        rev_ill_longi.x = illegal1.longitude;
        *s1++ = rev_ill_longi.c[0];
        *s1++ = rev_ill_longi.c[1];
        *s1++ = rev_ill_longi.c[2];
        *s1++ = rev_ill_longi.c[3];
        *s1++ =  illegal1.ewhemi;

        rev_ill_lati.x = illegal1.latitude;
        *s1++ = rev_ill_lati.c[0];
        *s1++ = rev_ill_lati.c[1];
        *s1++ = rev_ill_lati.c[2];
        *s1++ = rev_ill_lati.c[3];
        *s1++ =  illegal1.nshemi;

        *s1++ =  illegal1.ino;          //Υ�´���

        *s1++ = HIBYTE( crc16 );
        *s1++ = LOBYTE( crc16 );        //У����

        Si4432_SendBuffer(src, s1 - src);
    }
}


/*******************************************************************************
* Function Name  : Si4432_SendSnToLightMode
* Description    : ���ͳ���
******************************************************************************/
void Si4432_SendSnToLightMode(void)
{
    static u32 last_time = 0;
    unsigned char send_len;
    static unsigned char send_sn_sta = 0;

    if(last_time == 0 ) {
        last_time = Sys_TimeCount;
        return;
    }

    switch(send_sn_sta) {
    case 0:
        if( m_var.Flag_InitLAMPidNo == SET ) {
            m_var.Flag_InitLAMPidNo = RESET;
            last_time = Sys_TimeCount;//��ǰ����ʱ��
            send_sn_sta = 1;
        }
        break;
    case 1:
        send_len = Get_SendCarNo(Si4432_TxBuf);
        Si4432_SendBuffer(Si4432_TxBuf, send_len);
        Si4432_TimeSendSn = 50;
        send_sn_sta = 2;
        break;
    case 2:
        if(Sys_TimeCount - last_time > 2) {
            last_time = Sys_TimeCount;//��ǰ����ʱ��
            send_sn_sta = 3;
        }
        else if(Si4432_TimeSendSn == 0) {
            send_sn_sta = 1;
        }
        break;
    case 3:
        if(Sys_TimeCount - last_time > 5) {
            send_sn_sta = 0;
        }
        break;
    default:
        send_sn_sta = 0;
        break;
    }
}


/*******************************************************************************
* Function Name  : Si4432_SendChangeLight
* Description    : ���ͱ���ź�
******************************************************************************/
void Si4432_SendChangeLight(unsigned char* src)
{
    unsigned char pack_len;
    unsigned char *s = src;

    s += HEAD_LENGTH;

    *s++ = 1;
    *s++ = gpsx.direction;

    GetLocation( s ); 
    s += 24;

    pack_len = GTW_Packet( src, 1, src+HEAD_LENGTH, s - src - HEAD_LENGTH, m_carinfo.DTU_Type );

    Si4432_SendBuffer(src, pack_len);
}


/*******************************************************************************
* Function Name  : Si4432_RxBufferProcess
* Description    : ��������ı�־��Ϣ����
******************************************************************************/
void Msg_Set_Sign_DataColl(u8 *src)
{
    GTW02_BoardCast_Msg.MsgWhistle    = ReadDataBit(*src, GPIO_Pin_7);        // D0: ��������0=��Ч;1=��Ч
    GTW02_BoardCast_Msg.MsgHighBeams  = ReadDataBit(*src, GPIO_Pin_6);        // D1: Զ������0=��Ч;1=��Ч
    GTW02_BoardCast_Msg.MsgCity       = ReadDataBit(*src, GPIO_Pin_5);        // D2: ��������0=��Ч;1=��Ч
    GTW02_BoardCast_Msg.MsgHighway    = ReadDataBit(*src, GPIO_Pin_4);        // D3: ��������0=��Ч;1=��Ч
    GTW02_BoardCast_Msg.MsgWirelessSignal = ReadDataBit(*src, GPIO_Pin_3);    // D4: �����ź�����0=��Ч;1=��Ч
    GTW02_BoardCast_Msg.MsgRestAreas  = ReadDataBit(*src, GPIO_Pin_2);        // D5: ����������0=��Ч;1=��Ч

    if( GTW02_BoardCast_Msg.MsgWhistle == SET ) {
        DataCollectorMsg.MsgWhistle     = ReadDataBit(*(src+1), GPIO_Pin_7);            // 0=��������;1=��ֹ����
    }
    if( GTW02_BoardCast_Msg.MsgHighBeams == SET ) {
        DataCollectorMsg.MsgHighBeams   = ReadDataBit(*(src+1), GPIO_Pin_6);            // 0=����Զ��;1=��ֹԶ��
    }
    if( GTW02_BoardCast_Msg.MsgCity == SET ) {
        DataCollectorMsg.MsgCity        = ReadDataBit(*(src+1), GPIO_Pin_5);            // 0=������;1=����
    }
    if( GTW02_BoardCast_Msg.MsgHighway == SET ) {
        DataCollectorMsg.MsgHighway     = ReadDataBit(*(src+1), GPIO_Pin_4);    	    // 0=�Ǹ���;1=����
    }
    if( GTW02_BoardCast_Msg.MsgWirelessSignal == SET ) {
        DataCollectorMsg.MsgWirelessSignal = ReadDataBit(*(src+1), GPIO_Pin_3);         // 0=���������ź�;1=���������ź�
    }
    if( GTW02_BoardCast_Msg.MsgRestAreas == SET ) {
        DataCollectorMsg.MsgRestAreas   = ReadDataBit(*(src+1), GPIO_Pin_2);            // 0=�Ƿ�����; 1=������
    }
#ifdef __DEBUG
    if(m_var.Flag_Debug_Enable == SET && m_var.Flag_Init_RF433 == 0 )
        Debug_Printf(SET,
                     "\t�㲥=[%02x, %02x]\r\n\t����%s[%s]\tԶ��%s[%s]\r\n\t����%s[%s]\t����%s[%s]\r\n\t����%s[%s]\t������%s[%s]",
                     *src, *(src+1),
                     DataCollectorMsg.MsgWhistle == SET ? "��ֹ" : "����",     GTW02_BoardCast_Msg.MsgWhistle == SET ? "��Ч" : "��Ч",
                     DataCollectorMsg.MsgHighBeams == SET ? "��ֹ" : "����",   GTW02_BoardCast_Msg.MsgHighBeams == SET ? "��Ч" : "��Ч",
                     DataCollectorMsg.MsgCity == SET ? "��ֹ" : "����",        GTW02_BoardCast_Msg.MsgCity == SET ? "��Ч" : "��Ч",
                     DataCollectorMsg.MsgHighway == SET ? "��ֹ" : "����",     GTW02_BoardCast_Msg.MsgHighway == SET ? "��Ч" : "��Ч",
                     DataCollectorMsg.MsgWirelessSignal == SET ? "��ֹ" : "����", GTW02_BoardCast_Msg.MsgWirelessSignal == SET ? "��Ч" : "��Ч",
                     DataCollectorMsg.MsgRestAreas == SET ? "��ֹ" : "����",   GTW02_BoardCast_Msg.MsgRestAreas == SET ? "��Ч" : "��Ч");
#endif
}


/*******************************************************************************
* Function Name  : Si4432_RxBufferProcess
* Description    : �����յ������ݽ���Э�����
******************************************************************************/
void Si4432_RxBufferProcess(void)
{
#define RF433_HEAD_LEN 9
    unsigned char  i;
    unsigned char  rev_len;
    unsigned char  speed_group;
    unsigned short pack_len = 64;
    unsigned short iMsg = 0;
    unsigned short send_count;
    i2b_Def        rev_ill_num;
    i2b_Def        rev_speed_group;
    i2b_Def        TerCarType;      // CarTypeDef;�������Ͷ����е�  ����ʵ�ʳ���

    rev_len = Si4432_ReadBuffer();

    if(rev_len >  0 ) {
        if( Si4432_RxBuf[0] == 0x7E) {  //Э��
#ifdef __DEBUG
     
        Debug_Printf( SET, "�յ�SI432�ź�" );
#endif
            send_count = GTW_Analysis(Si4432_RxBuf, rev_len, Si4432_TxBuf, &pack_len);  //���ݽ��
            if( send_count > 0 ) {
                for( i=0; i<send_count / pack_len; i++ ) {
                    Si4432_SendBuffer( Si4432_TxBuf, pack_len );
                }
                if( send_count % pack_len > 0 ) {
                    Si4432_SendBuffer( Si4432_TxBuf, send_count % pack_len );
                }
            }
        }
        else if (Si4432_RxBuf[0] == 0x7F ) {
            if( (Si4432_RxBuf[1] ^ Si4432_RxBuf[2]) == 0xFF) {
                CheckerAnswer_CMDno = Si4432_RxBuf[1]; 

                Msg_Set_Sign_DataColl(&Si4432_RxBuf[3]);

                memcpy( rev_speed_group.c, Si4432_RxBuf+5, sizeof(rev_speed_group.c) ); // 17.6.7 csYxj(728920175@qq.com)

                for(i = 0; i<10; i++) {
                    if( memcmp(m_carinfo.Car_5SN, Si4432_RxBuf + RF433_HEAD_LEN + __CAR5SN_LEN__ * i, __CAR5SN_LEN__) == 0 ) {
                        CheckerAnswer_index = 4;
                        CheckerAnswer_ID.x = rev_speed_group.x;
                    }
                    if( (RF433_HEAD_LEN + __CAR5SN_LEN__ * i) >= rev_len )
                        break;
                }

                if( CheckerAnswer_index != 4 && rev_speed_group.x != CheckerAnswer_ID.x && rev_speed_group.x != 0 ) {
                    CheckerAnswer_CMDno = Si4432_RxBuf[1];
                    Flag_InspectCmd = SET;
#ifdef __DEBUG
                    // XFS_WriteBuffer(xfs_auto, "�յ��ɼ��Ǳ��");
                    if(m_var.Flag_Debug_Enable == SET && m_var.Flag_Init_RF433 == 0 )
                        Debug_Printf(SET,
                                     "�ɼ��Ǳ��(%02x%02x%02x%02x),�㲥(%02x,%02x)",
                                     rev_speed_group.c[0],
                                     rev_speed_group.c[1],
                                     rev_speed_group.c[2],
                                     rev_speed_group.c[3],
                                     RF433_MsgMask,
                                     RF433_MsgData);
#endif
                }
            }
            else {
                iMsg = MAKEWORD(Si4432_RxBuf[1], Si4432_RxBuf[2]);
                switch( iMsg ) {
                case 1: {
                    Msg_Set_Sign_DataColl(&Si4432_RxBuf[3]);

                    speed_group = Si4432_RxBuf[5];

                    for(i = 0; i < speed_group; i++) {
                        rev_speed_group.c[3] = (Si4432_RxBuf[i*6+6]);
                        rev_speed_group.c[2] = (Si4432_RxBuf[i*6+6+1]);
                        rev_speed_group.c[1] = (Si4432_RxBuf[i*6+6+2]);
                        rev_speed_group.c[0] = (Si4432_RxBuf[i*6+6+3]);

                        if( ReadData32Bit( rev_speed_group.x, (1 << (TerCarType.x - 1)) ) == SET) {
                            Flag_LimitSpeed = 1;
                            Dir_SGroup.Restrict_Direction = (Si4432_RxBuf[i*6+6+4]);    //��������
                            Dir_SGroup.Max_Speed = (Si4432_RxBuf[i*6+6+5]);             //��߳�������
                        }
                        else if( ReadData32Bit( rev_speed_group.x, (1 << (TerCarType.x - 1)) ) == RESET) {
                            Flag_LimitSpeed = 0;
                        }
                    }
                    break;
                }
                case 2: {  
                    if(Si4432_RxBuf[8] == 0x01)
                    {
                        if(ViolenceUntreated > 0)    //Υ����������0�����͵��ӳ���
                        {
                            Si4432_Send_EPlateNum(Si4432_RxBuf);//���͵��ӳ���
#ifdef __DEBUG
                            XFS_WriteBuffer(xfs_auto, "���͵��ӳ���");
#endif
                        }
                    }
                    else if(Si4432_RxBuf[8] == 0x02) //0x02��ȷ������Υ������
                    {
                        if(memcmp(&Si4432_RxBuf[3], m_carinfo.Car_5SN, __CAR5SN_LEN__) == 0) {
                            rev_ill_num.c[3] = Si4432_RxBuf[9];
                            rev_ill_num.c[2] = Si4432_RxBuf[10];
                            rev_ill_num.c[1] = Si4432_RxBuf[11];
                            rev_ill_num.c[0] = Si4432_RxBuf[12];
                            if( rev_ill_num.x > 0) {
                                Get_Illegal_Untreated(rev_ill_num.x); //��� �� �Ѿ��ϴ���Υ��  ��   δ�ϴ�Υ���б�  ��  ɾ��
                            }

                            if(ViolenceUntreated > 0) {
                                Si4432_Send_IllegalNum( Si4432_TxBuf);       //����Υ������
                            }
                        }
                    }
                    break;
                }
                }
            }
        }
        memset(Si4432_RxBuf, 0, sizeof(Si4432_RxBuf));
    }
}


/*******************************************************************************
* Function Name  : Si4432_CheckerAnswer
* Description    :
******************************************************************************/
extern unsigned int Sys_RunTime;
void Si4432_CheckerAnswer(void)
{
    switch(CheckerAnswer_index) {
    case 0: {
        if(Flag_InspectCmd == SET && CheckerAnswer_timer == 0) {
            Flag_InspectCmd = RESET;
            CheckerAnswer_CMDno = (CheckerAnswer_CMDno % 8);
            CheckerAnswer_timer = S_Timer[CheckerAnswer_CMDno] + rand() % 10;
            CheckerAnswer_index = 2;
        }
        else {
            Flag_InspectCmd = RESET;
        }
        break;
    }

    case 1: {
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "%d΢����ͳ���", CheckerAnswer_timer);
        Debug_Printf(SET, "%d΢����ͳ���", CheckerAnswer_timer);
#endif
        CheckerAnswer_index = 2;
        break;
    }

    case 2: {
        if(CheckerAnswer_timer == 0) {
            Si4432_Send_EPlateNum(Si4432_RxBuf);//���͵��ӳ���
            CheckerAnswer_index = 0;
        }
        break;
    }

    case 3: {
        if(CheckerAnswer_timer == 0) {
            CheckerAnswer_index = 0;//��ת��Default
        }
        break;
    }

    case 4: {
        CheckerAnswer_timer = 100000;//����10S����ʱ��
        CheckerAnswer_index = 0;//��ת��Default
        break;
    }

    default:
        break;
    }
}

/*******************************************************************************
* Function Name  : Si4432_SendTimer
* Description    : ȡ����Ӧ��λ�����֣���������ʱ��
******************************************************************************/
void Si4432_SendTimer(unsigned char* TmpSN)
{
    int i;
    unsigned int TmpSNData;

    TmpSNData = TmpSN[1];		//ֻȡ���ŵĵ�4�ֽ�
    TmpSNData <<= 8;
    TmpSNData += TmpSN[2];
    TmpSNData <<= 8;
    TmpSNData += TmpSN[3];
    TmpSNData <<= 8;
    TmpSNData += TmpSN[4];
    S_Timer[0] = (unsigned char)(TmpSNData%10);
    S_Timer[1] = (unsigned char)((TmpSNData%100)/10);
    S_Timer[2] = (unsigned char)((TmpSNData%1000)/100);
    S_Timer[3] = (unsigned char)((TmpSNData%10000)/1000);
    S_Timer[4] = (unsigned char)((TmpSNData%100000)/10000);
    S_Timer[5] = (unsigned char)((TmpSNData%1000000)/100000);
    S_Timer[6] = (unsigned char)((TmpSNData%10000000)/1000000);
    S_Timer[7] = (unsigned char)((TmpSNData%100000000)/10000000);

    for(i=0; i<8; i++) {
        S_Timer[i] = S_Timer[i] * 35;
    }
}

