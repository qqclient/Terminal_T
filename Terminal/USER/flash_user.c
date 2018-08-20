/******************** (C) COPYRIGHT soarsky ********************
* File Name          :
* Author             :
* Version            :
* Date               :
* Description        : Flash�������ǣ�д����ֻ�ܽ�1дΪ0���������ݾ���д1
*                      �����������������ݵ�falsh��д���µ����ݣ�������Ȳ�����
*                      ����Flash�ڲ�����ʱ������������
*******************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"

unsigned char m_flagSave;
unsigned short m_delaySave;

extern unsigned char I2411E_TxBuffer[2048];

unsigned int ViolenceTotal = 0;
unsigned int ViolenceUntreated = 0;

CarInfo_Def  m_carinfo;


/*******************************************************************************
* Function Name  : MX25L128_SaveTask
* Description    : ���ն��еĻ�������д��Flash��ַ
* Input          : None
* Output         : None
* Return         : None
* Remark         : ���ṹ����m_carinfo��������Ҫ����ʱ����
*******************************************************************************/
void MX25L128_CreatSaveTask(void)
{
    m_delaySave = 3000;
    m_flagSave = SET;
}


/*******************************************************************************
* Function Name  : MX25L128_SaveBase
* Description    : �����������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MX25L128_SaveBase( unsigned char nDelay )
{
    if( (m_flagSave == SET && m_delaySave == 0) || nDelay == SET ) {
        nDelay = RESET;
        m_delaySave = 0;
        m_flagSave = RESET;

        m_carinfo.Info_crc16  = get_crc16( &m_carinfo.Car_Type, (unsigned char*)&m_carinfo.Info_crc16 - &m_carinfo.Car_Type );
        m_carinfo.Param_crc16 = get_crc16( (unsigned char*)&m_carinfo.FD01_TimeOver, (unsigned char*)&m_carinfo.Param_crc16 - (unsigned char*)&m_carinfo.FD01_TimeOver );
        m_carinfo.Input_crc16 = get_crc16( &m_carinfo.Volume_Value, (unsigned char*)&m_carinfo.Input_crc16 - &m_carinfo.Volume_Value );
        m_carinfo.Voice_crc16 = get_crc16( (unsigned char*)&m_carinfo.Y01_Addr, (unsigned char*)&m_carinfo.Voice_crc16 - (unsigned char*)&m_carinfo.Y01_Addr );

        m_carinfo.crc16 = get_crc16( &m_carinfo.Flag_head, (unsigned char*)&m_carinfo.crc16 - &m_carinfo.Flag_head );
        MX25L128_Sector_Erase( ADDR_CARINFO );
        MX25L128_BufferWrite( &m_carinfo.Flag_head, ADDR_CARINFO, (int)&m_carinfo.Flag_end - (int)&m_carinfo.Flag_head);
    }
}

/*******************************************************************************
* Function Name  : MX25L128_Default
* Description    : �ָ�Ĭ��ֵ
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MX25L128_Default(void)
{
    int i = 0;

    if(m_carinfo.Info_crc16 != get_crc16( &m_carinfo.Car_Type, (unsigned char*)&m_carinfo.Info_crc16 - &m_carinfo.Car_Type )) {
        m_carinfo.Car_Type = 2;     // С������
        m_carinfo.Driver_Type = 6;  // C1

        if( m_carinfo.Car_5SN[0] == 0xff &&
                m_carinfo.Car_8SN[0] == 0xff &&
                m_carinfo.Car_9SN[0] == 0xff ) {
            memset((char*)m_carinfo.Car_5SN, 0, sizeof(m_carinfo.Car_5SN));
            memset((char*)m_carinfo.Car_8SN, 0, sizeof(m_carinfo.Car_8SN));
            memset((char*)m_carinfo.Car_9SN, 0, sizeof(m_carinfo.Car_9SN));
        }

        for(i=0; i<5; i++) {
            if( m_carinfo.Family_Phone[i][0] == 0xff ) {
                memset((char*)m_carinfo.Family_Phone[i], 0, sizeof(m_carinfo.Family_Phone[i]));
                memset((char*)m_carinfo.Family_Phone[i], 'F', __PHONE_LENGTH__ );   // ���պ���
            }
            if( m_carinfo.Driver_ID[i][0] == 0xff ) {
                memset((char*)m_carinfo.Driver_ID[i], 0, sizeof(m_carinfo.Driver_ID[i]));
            }
        }

        sprintf((char*)m_carinfo.Password, "123456");
        sprintf((char*)m_carinfo.Authorize_Code, "1123581321345589");

        if( m_carinfo.DTU_ID[0] == 0xff ) {
            memset((char*)m_carinfo.DTU_ID, 0, sizeof(m_carinfo.DTU_ID));
            sprintf((char*)m_carinfo.DTU_ID, "%08x",
                    (*(volatile unsigned int*)(0x1ffff7e8)) ^
                    (*(volatile unsigned int*)(0x1ffff7ec)) ^
                    (*(volatile unsigned int*)(0x1ffff7f0)));
        }
        m_carinfo.DTU_Type = CarTerminal;

        m_carinfo.NetAddrType        = 0;
        m_carinfo.NetPort            = 8001;

        memset(m_carinfo.ServerName, 0, sizeof(m_carinfo.ServerName));
        m_carinfo.ServerName[0]      = 112;
        m_carinfo.ServerName[1]      = 74;
        m_carinfo.ServerName[2]      = 173;
        m_carinfo.ServerName[3]      = 0;
    }

    if( m_carinfo.Input_crc16 != get_crc16( &m_carinfo.Volume_Value, (unsigned char*)&m_carinfo.Input_crc16 - &m_carinfo.Volume_Value ) ) {
        m_carinfo.Volume_Value       = 5;               // ����
        m_carinfo.Value_HighBeam     = 0;               // Զ���
        m_carinfo.Value_LowBeam      = 0;               // �����
        m_carinfo.Value_Ignite       = 0;               // ���
        m_carinfo.Value_Honk         = 0;               // ����
        m_carinfo.Value_DriverBelt   = 0;               // ����ȫ��
        m_carinfo.Value_DeputyBelt   = 0;               // ����ȫ��1
        m_carinfo.Value_DeputyBelt2  = 0;               // ����ȫ��2
        m_carinfo.Value_DriverDoor   = 0;               // �ſ���1
        m_carinfo.Value_SecondDoor   = 0;               // �ſ���2
        m_carinfo.Flag_Simulation    = RESET;           // ��������ʵ����־: ʵ��
        m_carinfo.Flag_LAMPInput     = RESET;           // IO��ֱ�Ӽ��Զ����
        m_carinfo.bt_packlen         = 155;             // �������ݰ����ȶ���
    }

    m_carinfo.Flag_GSMSign            = RESET;
    m_carinfo.Count_IllegalPark       = 0;
    m_carinfo.Flag_Inspection_Channel = 0xff;

    // �ɱ����
    if( m_carinfo.Param_crc16 != get_crc16( (unsigned char*)&m_carinfo.FD01_TimeOver, (unsigned char*)&m_carinfo.Param_crc16 - (unsigned char*)&m_carinfo.FD01_TimeOver ) ) {
        m_carinfo.FD01_TimeOver      = 15;              // ��, �ն˽���״̬��������ʾ����ʱ��
        m_carinfo.FD02_TimeOver      = 90;              // ��, �ն˽���״̬��������������ʱ��   
        m_carinfo.FD03_TimeOver      = 1*60;            // ��, ��������ǰ�ϴ�����λ������
        m_carinfo.FD04_TimeOver      = 15;              // ��, �����������ϴ�����λ������
        m_carinfo.JZ01_TimeOver      = 30;              // ��, ��������������ʾ��֤��ʻԱʱ��
        m_carinfo.JZ02_TimeOver      = 90;              // ��, ��������������ʾ����֤��ʻʱ�� �ڳ���10  �⣺90
        m_carinfo.JZ03_TimeOver      = 30;              // ��, �Զ���¼�����Ŷ�ʱ�䣨������ʱ��δͨ�ŵļ����Ƴ����У�
        m_carinfo.JZ04_TimeOver      = 300;             // ��, ��״̬һ��״̬��������������κμ��շ������룬�ȴ��೤ʱ��ָ�״̬һ
        m_carinfo.JZ05_TimeOver      = 300;             // ��, ״̬�������κμ��շ������룬�ȴ��೤ʱ��ָ�״̬һ��δ��𣩣�״̬�������
        m_carinfo.JZ06_TimeOver      = 600;             // ��, ״̬���¶೤ʱ��û�н��յ������泵��Ϣ�ָ�״̬һ��ͣ��Ϩ�𣩣�״̬����δϨ��
        m_carinfo.JZ07_TimeOver      = 20;              // ��, ѡ����ʻԱ�󣬶೤ʱ����ղ����˼��յ���һ�����룬��ȥѡ������ȼ�������
        m_carinfo.WZ01_CountOver     = 2;               // ��, �����г�״̬�£������յ�����ʹ���ֻ�����Ϣ��ͬһ��ʹ���ֻ����ż�¼Υ��
        m_carinfo.WZ02_TimeOver      = 30;              // ��, ǰ������ʹ���ֻ��źż�������Ϊ����һ��ʹ���ֻ������¼ƴ��ж��Ƿ��¼Υ��
        m_carinfo.WZ03_TimeOver      = 10;              // ��, ������ʻ������ʾϵ��ȫ��
        m_carinfo.WZ04_TimeOver      = 60;              // ��, ������ʻ���ü�¼��ϵ��ȫ��Υ�� 
        m_carinfo.WZ05_TimeOver      = 5;               // ��, �յ�������������źź��ò�����Υ��
        m_carinfo.WZ06_TimeOver      = 10;              // ��, �յ�����Զ���źź��ò�����Υ��
        sprintf((char*)m_carinfo.WZ07_Timer, "2:00");   // 24Сʱ��, ��ͳ�ҹ����п�ʼʱ��
        sprintf((char*)m_carinfo.WZ08_Timer, "5:00");   // 24Сʱ��, ��ͳ�ҹ����н���ʱ��
        sprintf((char*)m_carinfo.WZ09_Timer, "1:30");   // 24Сʱ��, ��ͳ�ҹ����п�ʼԤ��ʱ�䣨�ڴ�ʱ��֮�󣬽��п�ʼʱ��֮ǰ�������������Ԥ����
        m_carinfo.WZ10_TimeOver      = 600;             // ��, ��ͳ�ҹ�����Ԥ�����ڣ���һ�����Ѻ󣬽��п�ʼʱ��֮ǰ�����������һ�Σ�
        m_carinfo.WZ11_TimeOver      = 600;             // ��, ��ͳ�ҹ�����ʱ���ϸ��ٺ󣬶೤ʱ���¼Υ��
        m_carinfo.WZ13_TimeOver      = 4;               // Сʱ, �����ۼƼݳ��������ƣ�ͼ�ʻ
        m_carinfo.WZ14_TimeOver      = 600;             // ��, ƣ�ͼ�ʻ����ѭ������
        m_carinfo.WZ15_TimeOver      = 3000;            // ��, ����ƣ�ͼ�ʻ���ò���Ϣ�������ʻԱ��Υ��
        m_carinfo.WZ16_TimeOver      = 1200;            // ��, �����ۼƼݳ�����ƣ�ͼ�ʻ����Ϣ������¼�ʱ
        m_carinfo.SD01_TimeOver      = 15;              // ��, ��ӦһѰ��������ʱ��
        m_carinfo.SD02_TimeOver      = 60;              // ��, ��Ӧ�����ҳ�����ʱ�䣨�����ź�ر����ƣ�
        m_carinfo.WT01_TimeOver      = 600;             // ��, �뿪��������λ�ö�Զ���Զ�����Υͣ
        m_carinfo.WT02_DistOver      = 500;             // ��, �ӿ�������ʼ������Υͣ��ʱ�䳤��
        m_carinfo.MoveCar_TimeOver   = 10*60;           // ��[10*60], �����Ƴ��绰�����ŵı���ʱ��
        m_carinfo.FollowCar_TimeOver = 10*60;           // ��[10*60], �����泵��ʱʱ��
    }

    if(m_carinfo.Voice_crc16 != get_crc16( (unsigned char*)&m_carinfo.Y01_Addr, (unsigned char*)&m_carinfo.Voice_crc16 - (unsigned char*)&m_carinfo.Y01_Addr )) {
        m_carinfo.Y01_Addr = NULL;   // ���ѽ������Զ��������벻Ҫ����Զ���
        m_carinfo.Y02_Addr = NULL;   // �����ر�Զ��ƣ������¼Υ��
        m_carinfo.Y03_Addr = NULL;   // ����Υ��ʹ��Զ����ѱ���¼Υ��
        m_carinfo.Y04_Addr = NULL;   // ������ʾ���Ӽ���
        m_carinfo.Y05_Addr = NULL;   // ������ʾ���Ӽ���
        m_carinfo.Y06_Addr = NULL;   // ���Ӽ�����֤���
        m_carinfo.Y07_Addr = NULL;   // ��������֤��ʻ���Ѽ�¼Υ��
        m_carinfo.Y08_Addr = NULL;   // ����ϵ�ð�ȫ���������¼Υ��
        m_carinfo.Y09_Addr = NULL;   // ����δϵ��ȫ�����ѱ���¼Υ��
        m_carinfo.Y10_Addr = NULL;   // ������ȷʹ�ð�ȫ���������¼Υ��
        m_carinfo.Y11_Addr = NULL;   // ����Υ��ʹ�ð�ȫ�����ѱ���¼Υ��
        m_carinfo.Y12_Addr = NULL;   // ����������ʻ����XXСʱXX���ӣ�������ǰ���ʵ�λ��ͣ����Ϣ20���������ټ�����ʻ����������ʻ����4Сʱ��������¼Υ�¡����������ʻԱ������֤�¼�ʻԱ���Ӽ��ա�
        m_carinfo.Y13_Addr = NULL;   // ����������ʻ����XXСʱXX���ӣ�������ǰ���ʵ�λ��ͣ����Ϣ20���������ټ�����ʻ����������ʻ����4Сʱ��������¼Υ�¡����������ʻԱ������֤�¼�ʻԱ���Ӽ��ա�
        m_carinfo.Y14_Addr = NULL;   // ��24Сʱ�����ۼƼ�ʻ����7СʱXX���ӣ�������ǰ���ʵ�λ��ͣ����Ϣ�������ʻԱ�����ۼƼ�ʻ����8Сʱ��������¼Υ�¡�
        m_carinfo.Y15_Addr = NULL;   // ����ƣ�ͼ�ʻ���ѱ���¼Υ��
        m_carinfo.Y16_Addr = NULL;   // �����賿2:00��5:00ͣ�˻������ʻԱ�������¼Υ��
        m_carinfo.Y17_Addr = NULL;   // �����ʷ���Ϣ���ټݳ��������¼Υ��?
        m_carinfo.Y18_Addr = NULL;   // ����ҹ������������¹�·���뾡��ͣ�����뿪�����򣬷����¼Υ��
        m_carinfo.Y19_Addr = NULL;   // ����Υ����ͳ�ҹ����ʻ�涨���ѱ���¼Υ��
        m_carinfo.Y20_Addr = NULL;   //
        m_carinfo.Y21_Addr = NULL;   // ���ѽӽ����������뼰ʱ�ĵ�
        m_carinfo.Y22_Addr = NULL;   // ����Υ�����й涨���ѱ���¼Υ��
        m_carinfo.Y23_Addr = NULL;   // ��ǰ����XX�����ѳ���XX%������Ƴ��٣������¼Υ��
        m_carinfo.Y24_Addr = NULL;   // �����٣��ѱ���¼Υ��
        m_carinfo.Y25_Addr = NULL;   // ��ǰ�ѽ���/�뿪��������
        m_carinfo.Y26_Addr = NULL;   // ���������
        m_carinfo.Y27_Addr = NULL;   // ����Υ�������涨���ѱ���¼Υ��
        m_carinfo.Y28_Addr = NULL;   // ���ļ��յȼ�̫�ͣ����ܼ�ʻ�ó������򽫼�¼Υ��
        m_carinfo.Y29_Addr = NULL;   // �����ն��ٸ���Ҫ������Զ���������
        m_carinfo.Y30_Addr = NULL;   // ף��һ·˳��
        m_carinfo.Y31_Addr = NULL;   // ��ֹͣΥ��ʹ���ֻ��������¼Υ��
    }

    MX25L128_SaveBase(SET);
}

/*******************************************************************************
* Function Name  : MX25L128_Initialize
* Description    : ���������Ϣ��ʼ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MX25L128_Initialize(void)
{
    unsigned short i;
//#ifdef __DEBUG
//    unsigned char  buf[200];
//    char* s1;
//#endif

    ViolenceTotal = MX25L128_Word_read(ADDR_ILLEGAL_UPLOAD);
    ViolenceUntreated = MX25L128_Word_read(ADDR_ILLEGAL_UPLOAD+4);
    if(ViolenceTotal == 0xFFFFFFFF || ViolenceUntreated == 0xFFFFFFFF) {
        ViolenceTotal = 0;
        ViolenceUntreated = 0;
        MX25L128_Sector_Erase(ADDR_ILLEGAL_CONTECT);
        MX25L128_Sector_Erase(ADDR_ILLEGAL_UPLOAD);
        MX25L128_Word_Write(ADDR_ILLEGAL_UPLOAD, 0);
        MX25L128_Word_Write(ADDR_ILLEGAL_UPLOAD+4, 0);
    }

    // ��ȡ������Ϣ
    MX25L128_BufferRead(&m_carinfo.Flag_head, ADDR_CARINFO, (int)&m_carinfo.Flag_end - (int)&m_carinfo.Flag_head);

    i = get_crc16( &m_carinfo.Flag_head, (int)&m_carinfo.crc16 - (int)&m_carinfo.Flag_head );
    if(i != m_carinfo.crc16) {
        MX25L128_Default();
    }
    else {
#ifdef __DEBUG
        m_carinfo.NetAddrType       = 0;
        m_carinfo.NetPort           = 8001;

        m_carinfo.ServerName[0]     = 112;
        m_carinfo.ServerName[1]     = 74;
        m_carinfo.ServerName[2]     = 173;
        m_carinfo.ServerName[3]     = 0;

        m_carinfo.FD03_TimeOver     = 60;               // ��, ��������ǰ�ϴ�����λ������
        m_carinfo.FD04_TimeOver     = 20;               // ��, �����������ϴ�����λ������
        m_carinfo.MoveCar_TimeOver  = 10*60;            // ��, �����Ƴ��绰�����ŵı���ʱ��
        m_carinfo.FollowCar_TimeOver= 10*60;            // ��, �����泵��ʱʱ��
        m_carinfo.JZ01_TimeOver     = 30;               // ��, ��������������ʾ��֤��ʻԱʱ��
        m_carinfo.JZ02_TimeOver     = 90;               // ��, ��������������ʾ����֤��ʻʱ�� �ڳ���10  �⣺90
        m_carinfo.JZ04_TimeOver     = 0.5*60;           // ��, ��״̬һ��״̬��������������κμ��շ������룬�ȴ��೤ʱ��ָ�״̬һ
        m_carinfo.JZ05_TimeOver     = 0.5*60;           // ��, ״̬�������κμ��շ������룬�ȴ��೤ʱ��ָ�״̬һ��δ��𣩣�״̬�������
        m_carinfo.JZ06_TimeOver     = 1*60;             // ��, ״̬���¶೤ʱ��û�н��յ������泵��Ϣ�ָ�״̬һ��ͣ��Ϩ�𣩣�״̬����δϨ��

        m_carinfo.Flag_Inspection_Channel = 0xff;

#endif

        if( m_carinfo.Car_Type > 16 || m_carinfo.Car_Type < 1 ) {
            m_carinfo.Car_Type = 2;     // С������
            m_carinfo.Driver_Type = 6;  // C1
            MX25L128_CreatSaveTask();
        }

        if( m_carinfo.bt_packlen == 0xffff ) {
            m_carinfo.bt_packlen = 155;
            MX25L128_CreatSaveTask();
        }

        if(m_flagSave == SET ) {
            MX25L128_SaveBase(SET);
        }
    }

    m_var.Flag_Init_I2411E       = 0xff;
    m_var.Flag_Init_M35          = 0xff;
    m_var.Flag_Init_RF433        = 0xff;
    m_carinfo.DTU_Type = CarTerminal;
}

/*******************************************************************************
* Function Name  : MX25L128_Illegal_Write
* Description    : д������λ��Ϣ�����ƺż���ʻ֤�ŵ�Υ����Ϣ
* Input          : ill:��Ҫд��Flash��Υ����Ϣ
* Output         : None
* Return         : д��ı��
*******************************************************************************/
unsigned int MX25L128_Illegal_Write(Illegal_Def* ill)
{
    ill->ewhemi = gpsx.ewhemi;                          // ����/����,E:����;W:����
    ill->longitude = (double)gpsx.longitude / 100000;   // Υ��ʱ����
    ill->nshemi = gpsx.nshemi;                          // ��γ/��γ,N:��γ;S:��γ
    ill->latitude  = (double)gpsx.latitude / 100000;    // Υ��ʱγ��

    return(MX25L128_Illegal_Write_Base(ill, SET));
}

/*******************************************************************************
* Function Name  : MX25L128_Illegal_Write
* Description    : д�������ƺż���ʻ֤�ŵ�Υ����Ϣ
* Input          : ill:��Ҫд��Flash��Υ����Ϣ
* Output         : None
* Return         : д��ı��
*******************************************************************************/
unsigned int MX25L128_Illegal_Write_Base(Illegal_Def* ill, unsigned char flag)
{
    unsigned int    Start_Addr;
    unsigned int*   pIll_Num;

    ViolenceTotal++;
    if( flag == SET ) {
        ViolenceUntreated++;
    }

    pIll_Num = (unsigned int*)malloc( ViolenceUntreated * sizeof(int) );
    MX25L128_BufferRead((unsigned char*)pIll_Num, ADDR_ILLEGAL_UPLOAD + 8, ViolenceUntreated * sizeof(int) );
    MX25L128_Sector_Erase(ADDR_ILLEGAL_UPLOAD);

    Start_Addr = ADDR_ILLEGAL_CONTECT + (ViolenceTotal-1) * ((int)&ill->ino - (int)&ill->id + sizeof(ill->ino));
    ill->id = ViolenceTotal;
    ill->ptype = m_carinfo.Car_Type;
    memcpy(ill->carno, m_carinfo.Car_9SN, __CAR9SN_LEN__);

    if( Flag_ApplyResult == SET || ill->ino == ino_IllegalPark ) {
        ill->driver_type = Asc2Byte( (char*)m_carinfo.Driver_ID[0], __DRIVER_TYPE_LEN__ );
        memcpy(ill->driver_no, &m_carinfo.Driver_ID[0][0] + __DRIVER_TYPE_LEN__, __DRIVERID_LEN__ );
    }
    else {
        ill->driver_type = 0;
        memset( ill->driver_no, '0', __DRIVERID_LEN__ );
    }

    MX25L128_BufferWrite( (unsigned char*)&ill->id, Start_Addr, (int)&ill->ino - (int)&ill->id + sizeof(ill->ino) );

    MX25L128_Word_Write(ADDR_ILLEGAL_UPLOAD, ViolenceTotal);
    MX25L128_Word_Write(ADDR_ILLEGAL_UPLOAD+4, ViolenceUntreated);

    if( flag == SET ) {
        *(pIll_Num + ViolenceUntreated - 1) = ViolenceTotal;
        MX25L128_BufferWrite((unsigned char*)pIll_Num, ADDR_ILLEGAL_UPLOAD + 8, ViolenceUntreated * sizeof(int));
    }

    free(pIll_Num);

    return(ill->id);
}

/*******************************************************************************
* Function Name  : History_UseCar
* Description    : �ó����ռ�¼
* type: 0=login; 255=logout  csYxj(728920175@qq.com)
******************************************************************************/
void History_UseCar(unsigned char* src, unsigned char type)
{
    unsigned int   write_addr;
    unsigned short uc_size = 0;
    UserCarDef     uc;

    uc_size = (int)&uc.type - (int)&uc.Time[0] + sizeof(uc.type);
    m_var.UseCar_Count = MX25L128_Word_read( ADDR_USE_RECORD );
    if( m_var.UseCar_Count > ((0x100000 / uc_size) + 1) )
        m_var.UseCar_Count = 0;

    write_addr = ADDR_USE_RECORD + sizeof(m_var.UseCar_Count) + m_var.UseCar_Count * uc_size;

    TimeToCharArray( Sys_CalendarTime, &uc.Time[0] );

    memcpy((char*)uc.idType, src, __DRIVER_TYPE_LEN__ );
    memcpy((char*)uc.idNo, src+__DRIVER_TYPE_LEN__, __DRIVERID_LEN__ );
    uc.type = type;

    MX25L128_BufferWrite( &uc.Time[0], write_addr, uc_size );

    m_var.UseCar_Count++;
    MX25L128_Word_Write( ADDR_USE_RECORD, m_var.UseCar_Count );
}

