/******************** (C) COPYRIGHT soarsky ************************************
* File Name          : gtw_type.h
* Author             : CSYXJ
* Version            : V0.1
* Date               : 2017-03-19
* Description        : ���̱������ṹ�������ȶ���
*******************************************************************************/

#ifndef __GTW_TYPE_H
#define __GTW_TYPE_H

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int    DWORD;

#define HEAD_LENGTH 10

//���������ʱʱ��
#define KEY_COUNT   200  //10
#define KEY_COUNT_KEY   2000  //1000 *100us = 100ms

#define MAKEWORD(a, b)      ((WORD)(((BYTE)(b)) | ((WORD)((BYTE)(a))) << 8))
#define MAKELONG(a, b)      ((LONG)(((WORD)(b)) | ((DWORD)((WORD)(a))) << 16))

#define LOWORD(l)           ((WORD)(l))
#define HIWORD(l)           ((WORD)(((DWORD)(l) >> 16) & 0xFFFF))

#define LOBYTE(w)           ((BYTE)(w))
#define HIBYTE(w)           ((BYTE)(((WORD)(w) >> 8) & 0xFF))

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#define SPEED_MIN    5

extern unsigned int Sys_RunTime;        // ��¼���ն�������ʱ��


// int�������ֽ����黥ת����
typedef union {
    unsigned char c[4];
    unsigned int  x;
} i2b_Def;

// float�������ֽ����黥ת����
typedef union {
    unsigned char c[4];// 0 1 2 3
    float  x;   //0123
} f2b_Def;

// short�������ֽ����黥ת����
typedef union {
    unsigned char  c[2];
    unsigned short x;
} w2b_Def;


////////////////////////////////////////////////////////////////////////////////////
// [[ Flash��ŵ�ַ����
#define ADDR_CARINFO                0x000000                // �����������ݴ�ŵ�ַ
#define ADDR_VOICE                  0x001000                // �����������ݴ�ŵ�ַ
#define ADDR_BROADCAST              0x002000                // ��̬�ܿ���Ϣ��ŵ�ַ
#define ADDR_PWR_RECORD             0x008000                // �����ն˵�Դ��¼��ŵ�ַ
#define ADDR_USE_RECORD             0x010000                // ����ʹ�ü�¼��ŵ�ַ
#define ADDR_SPEED_RECORD           0x110000                // �������뼱���ټ�¼��ŵ�ַ
#define ADDR_CAR_MEET               0x2F0000                // �ᳵʱΥ�¼�¼��ŵ�ַ
#define ADDR_ILLEGAL_UPLOAD         0x300000                // Υ��δ�ϴ���ſ�ʼ��ַ
#define ADDR_ILLEGAL_CONTECT        0x301000                // Υ�¼�¼��ſ�ʼ��ַ
#define ADDR_DriVer_RECORD          0x306000                // ��������֤����ǳ���¼��ſ�ʼ��ַ
// Flash��ŵ�ַ���� ]]
////////////////////////////////////////////////////////////////////////////////////


typedef struct {
    unsigned char  Flag_ApplyDriver;
    unsigned short FollowCar_TimeOver;
    unsigned short Apply1_TimeOver;
    unsigned char  Flag_DriverPark;
    unsigned char  Flag_InitLAMPidNo;                       // ��ʼ�����ģ�鳵��: RESET=��д����; SET=д����
    unsigned char  Flag_Debug_Enable;                       // �Ƿ����������Ϣ���: RESET=�����; SET=���
    unsigned short gtw_recv_timeout;
    unsigned char  Flag_Init_I2411E;
    unsigned char  Flag_Init_RF433;
    unsigned char  Flag_Init_M35;
    unsigned char  FLag_UpdateClock;
    unsigned char  Flag_InitDone_M35;
    unsigned short UpdateClock_TimeOver;
    unsigned int   IdClear_TimeOver;
    unsigned char  Flag_InputStudy;
    unsigned int   UseCar_Count;
	  unsigned int   ReadBeamTimer;    //��������ʱ�������ܼ��Զ����
} Base_Def;
extern Base_Def m_var;

////////////////////////////////////////////////////////////////////////////////////
// [[ �����������ݽṹ
// ���ȶ���
#define __DRIVER_TYPE_LEN__         2                       // �������ͳ���
#define __DRIVERID_LEN__            18                      // ���ճ��� = ��������(2B) + ����(18B)
#define __PASSWORD_LEN__            6                       // ��ѯ���볤��
#define __CAR5SN_LEN__              5                       // ѹ�����ӳ�����Ϣ����
#define __CAR8SN_LEN__              8                       // GBK����ӳ�����Ϣ����
#define __CAR9SN_LEN__              9                       // UTF-8����ӳ�����Ϣ����
#define __AUTHORIZE_LEN__           16                      // ��Ȩ�볤��
#define __DTU_ID_LEN__              18                      // �ն˱�ų���
#define __SERVERNAME_LEN__          24                      // ���������Ƴ���
#define __PHONE_LENGTH__            16                      // �ֻ�������볤��

typedef struct {
    unsigned char  Flag_head;                               // �ṹ���忪ʼ��� - ���뱣֤Ϊ��һ�ֽ�

    unsigned char  Car_Type;
    unsigned char  Driver_Type;
    unsigned char  Car_5SN[ __CAR5SN_LEN__ + 1 ];           // ѹ�����ӳ�����Ϣ
    unsigned char  Car_8SN[ __CAR8SN_LEN__ + 1 ];           // GBK����ӳ�����Ϣ
    unsigned char  Car_9SN[ __CAR9SN_LEN__ + 1 ];           // UTF-8����ӳ�����Ϣ
    unsigned char  Family_Phone[5][ __PHONE_LENGTH__ + 1 ]; // �ֻ��������
    unsigned char  Driver_ID[5][ __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ + 1 ];    // ���պ��� = ��������(2B) + ����(18B)
    unsigned char  Password[ __PASSWORD_LEN__ + 1 ];        // ��ѯ����
    unsigned char  Authorize_Code[ __AUTHORIZE_LEN__ + 1 ]; // ��Ȩ��

    unsigned char  DTU_ID[ __DTU_ID_LEN__ + 1 ];            // �豸id 18Byte
    unsigned char  DTU_Type;                                // �ն�����: MsgSource_Typedef��������

    unsigned char  NetAddrType;                             // ��������ַ����: 0=ip��ַ, 1=������ַ
    unsigned int   NetPort;                                 // �������˿�
    unsigned char  ServerName[ __SERVERNAME_LEN__ + 1 ];    // ��������ַ

    unsigned short Info_crc16;


    // ����ɼ�ѧϰ���Ľӿ�״̬[�ر�״̬]
    unsigned char  Volume_Value;                            // ����
    unsigned char  Value_HighBeam;                          // Զ���  //==SET��ʾ��,==RESET��ʾ�ر�
    unsigned char  Value_LowBeam;                           // �����
    unsigned char  Value_Ignite;                            // ���
    unsigned char  Value_Honk;                              // ����
    unsigned char  Value_DriverBelt;                        // ����ȫ��
    unsigned char  Value_DeputyBelt;                        // ����ȫ��1
    unsigned char  Value_DeputyBelt2;                       // ����ȫ��2
    unsigned char  Value_DriverDoor;                        // �ſ���1
    unsigned char  Value_SecondDoor;                        // �ſ���2

    unsigned char  Flag_Simulation;                         // ��������ʵ����־
    unsigned char  Flag_LAMPInput;                          // Զ�����ⷽʽ: 0=IOֱ�Ӽ��; 1=RF433MHzģ����
    unsigned short bt_packlen;                              // �������ݰ����ȶ���

    unsigned short Input_crc16;


    unsigned char  Flag_GSMSign;                            // GSM���źŲ���: 0=
    unsigned char  Count_IllegalPark;                       // Υͣ����,ÿ��ǰ2�οɳ���
    unsigned char  Flag_Inspection_Channel;                 // 0xff: wifi; 0x00: BlueTooth


    // �ɱ����
    unsigned short FD01_TimeOver;                           // ��, �ն˽���״̬��������ʾ����ʱ��
    unsigned short FD02_TimeOver;                           // ��, �ն˽���״̬��������������ʱ��
    unsigned short FD03_TimeOver;                           // ��, ��������ǰ�ϴ�����λ������
    unsigned short FD04_TimeOver;                           // ��, �����������ϴ�����λ������
    unsigned short JZ01_TimeOver;                           // ��, ��������������ʾ��֤��ʻԱʱ��
    unsigned short JZ02_TimeOver;                           // ��, ��������������ʾ����֤��ʻʱ��
    unsigned short JZ03_TimeOver;                           // ��, �Զ���¼�����Ŷ�ʱ�䣨������ʱ��δͨ�ŵļ����Ƴ����У�
    unsigned short JZ04_TimeOver;                           // ��, ��״̬һ��״̬��������������κμ��շ������룬�ȴ��೤ʱ��ָ�״̬һ
    unsigned short JZ05_TimeOver;                           // ��, ״̬�������κμ��շ������룬�ȴ��೤ʱ��ָ�״̬һ��δ��𣩣�״̬�������
    unsigned short JZ06_TimeOver;                           // ��, ״̬���¶೤ʱ��û�н��յ������泵��Ϣ�ָ�״̬һ��ͣ��Ϩ�𣩣�״̬����δϨ��
    unsigned short JZ07_TimeOver;                           // ��, ѡ����ʻԱ�󣬶೤ʱ����ղ����˼��յ���һ�����룬��ȥѡ������ȼ�������
    unsigned short WZ01_CountOver;                          // ��, �����г�״̬�£������յ�����ʹ���ֻ�����Ϣ��ͬһ��ʹ���ֻ����ż�¼Υ��
    unsigned short WZ02_TimeOver;                           // ��, ǰ������ʹ���ֻ��źż�������Ϊ����һ��ʹ���ֻ������¼ƴ��ж��Ƿ��¼Υ��
    unsigned short WZ03_TimeOver;                           // ��, ������ʻ��ú���ʾϵ��ȫ��
    unsigned short WZ04_TimeOver;                           // ��, ������ʻ��ú��¼��ϵ��ȫ��Υ��
    unsigned short WZ05_TimeOver;                           // ��, �յ�������������źź��ò�����Υ��
    unsigned short WZ06_TimeOver;                           // ��, �յ�����Զ���źź��ò�����Υ��
    unsigned char  WZ07_Timer[6];                           // 24Сʱ��, ��ͳ�ҹ����п�ʼʱ��
    unsigned char  WZ08_Timer[6];                           // 24Сʱ��, ��ͳ�ҹ����н���ʱ��
    unsigned char  WZ09_Timer[6];                           // 24Сʱ��, ��ͳ�ҹ����п�ʼԤ��ʱ�䣨�ڴ�ʱ��֮�󣬽��п�ʼʱ��֮ǰ�������������Ԥ����
    unsigned short WZ10_TimeOver;                           // ��, ��ͳ�ҹ�����Ԥ�����ڣ���һ�����Ѻ󣬽��п�ʼʱ��֮ǰ�����������һ�Σ�
    unsigned short WZ11_TimeOver;                           // ��, ��ͳ�ҹ�����ʱ���ϸ��ٺ󣬶೤ʱ���¼Υ��
    unsigned short WZ12_TimeOver;                           // ��, 24Сʱ���ۼƼݳ��೤ʱ���������ն˷���Ϣ����ƣ�ͼ�ʻ
    unsigned short WZ13_TimeOver;                           // ��, �����ۼƼݳ��������ƣ�ͼ�ʻ
    unsigned short WZ14_TimeOver;                           // ��, ƣ�ͼ�ʻ����ѭ������
    unsigned short WZ15_TimeOver;                           // ��, ����ƣ�ͼ�ʻ���ò���Ϣ�������ʻԱ��Υ��
    unsigned short WZ16_TimeOver;                           // ��, �����ۼƼݳ�����ƣ�ͼ�ʻ����Ϣ������¼�ʱ
    unsigned short SD01_TimeOver;                           // ��[15], ��ӦһѰ��������ʱ��
    unsigned short SD02_TimeOver;                           // ��, ��Ӧ�����ҳ�����ʱ�䣨�����ź�ر����ƣ�
    unsigned short WT01_TimeOver;                           // ��[10*60], �ӿ�������ʼ������Υͣ��ʱ�䳤��
    unsigned short WT02_DistOver;                           // ��[500], �뿪��������λ�ö�Զ���Զ�����Υͣ
    unsigned short MoveCar_TimeOver;                        // ��[20*60], �յ��Ƴ������绰�����ű���ʱ��
    unsigned short FollowCar_TimeOver;                      // ��[10*60], �����泵��ʱʱ��

    unsigned short Param_crc16;


    // ����������ڵ�ַ
    unsigned char* Y01_Addr;                                // ���ѽ������Զ��������벻Ҫ����Զ���
    unsigned char* Y02_Addr;                                // �����ر�Զ��ƣ������¼Υ��
    unsigned char* Y03_Addr;                                // ����Υ��ʹ��Զ����ѱ���¼Υ��
    unsigned char* Y04_Addr;                                // ������ʾ���Ӽ���
    unsigned char* Y05_Addr;                                // ������ʾ���Ӽ���
    unsigned char* Y06_Addr;                                // ���Ӽ�����֤���
    unsigned char* Y07_Addr;                                // ��������֤��ʻ���Ѽ�¼Υ��
    unsigned char* Y08_Addr;                                // ����ϵ�ð�ȫ���������¼Υ��
    unsigned char* Y09_Addr;                                // ����δϵ��ȫ�����ѱ���¼Υ��
    unsigned char* Y10_Addr;                                // ������ȷʹ�ð�ȫ���������¼Υ��
    unsigned char* Y11_Addr;                                // ����Υ��ʹ�ð�ȫ�����ѱ���¼Υ��
    unsigned char* Y12_Addr;                                // ����������ʻ����XXСʱXX���ӣ�������ǰ���ʵ�λ��ͣ����Ϣ20���������ټ�����ʻ����������ʻ����4Сʱ��������¼Υ�¡����������ʻԱ������֤�¼�ʻԱ���Ӽ��ա�
    unsigned char* Y13_Addr;                                // ����������ʻ����XXСʱXX���ӣ�������ǰ���ʵ�λ��ͣ����Ϣ20���������ټ�����ʻ����������ʻ����4Сʱ��������¼Υ�¡����������ʻԱ������֤�¼�ʻԱ���Ӽ��ա�
    unsigned char* Y14_Addr;                                // ��24Сʱ�����ۼƼ�ʻ����7СʱXX���ӣ�������ǰ���ʵ�λ��ͣ����Ϣ�������ʻԱ�����ۼƼ�ʻ����8Сʱ��������¼Υ�¡�
    unsigned char* Y15_Addr;                                // ����ƣ�ͼ�ʻ���ѱ���¼Υ��
    unsigned char* Y16_Addr;                                // �����賿2:00��5:00ͣ�˻������ʻԱ�������¼Υ��
    unsigned char* Y17_Addr;                                // �����ʷ���Ϣ���ټݳ��������¼Υ��?
    unsigned char* Y18_Addr;                                // ����ҹ������������¹�·���뾡��ͣ�����뿪�����򣬷����¼Υ��
    unsigned char* Y19_Addr;                                // ����Υ����ͳ�ҹ����ʻ�涨���ѱ���¼Υ��
    unsigned char* Y20_Addr;                                //
    unsigned char* Y21_Addr;                                // ���ѽӽ����������뼰ʱ�ĵ�
    unsigned char* Y22_Addr;                                // ����Υ�����й涨���ѱ���¼Υ��
    unsigned char* Y23_Addr;                                // ��ǰ����XX�����ѳ���XX%������Ƴ��٣������¼Υ��
    unsigned char* Y24_Addr;                                // �����٣��ѱ���¼Υ��
    unsigned char* Y25_Addr;                                // ��ǰ�ѽ���/�뿪��������
    unsigned char* Y26_Addr;                                // ���������
    unsigned char* Y27_Addr;                                // ����Υ�������涨���ѱ���¼Υ��
    unsigned char* Y28_Addr;                                // ���ļ��յȼ�̫�ͣ����ܼ�ʻ�ó������򽫼�¼Υ��
    unsigned char* Y29_Addr;                                // �����ն��ٸ���Ҫ������Զ���������
    unsigned char* Y30_Addr;                                // ף��һ·˳��
    unsigned char* Y31_Addr;                                // ��ֹͣΥ��ʹ���ֻ��������¼Υ��

    unsigned short Voice_crc16;


    double         distance;                                // �����ͳ��
    unsigned int   distance_longitude;                      // ���������ʱ��gpsλ��
    unsigned int   distance_latitude;                       // ���������ʱ��gpsλ��


    unsigned short crc16;                                   // ��������У���

    unsigned char  Flag_end;                                // �ṹ���������� - ���뱣֤Ϊ���2�ֽ�

} CarInfo_Def;
extern CarInfo_Def m_carinfo;
// �����������ݽṹ ]]
////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////
// [[ Υ�����ݽṹ
typedef struct {
    unsigned int  id;
    unsigned char ptype;
    unsigned char carno[__CAR9SN_LEN__];
    unsigned char stime[14];
    unsigned char etime[14];
    unsigned char driver_type;
    unsigned char driver_no[18];
    float         longitude;
    unsigned char ewhemi;
    float         latitude;
    unsigned char nshemi;
    unsigned char ino;
} Illegal_Def;

extern Illegal_Def Ill_ETicket;
// Υ�����ݽṹ ]]
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// [[ ������ݽṹ
typedef struct {
    unsigned char ptime[14];
    unsigned char pstatus;
} Lamp_Def;
// ������ݽṹ ]]
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// [[ �쳣�������ݽṹ
typedef struct {
    unsigned char  Nspeedtime[14];
    unsigned char  Nspeedtimestatus;
    unsigned char  Nspeeddata;
} AbnomalSpeed;
// �쳣�������ݽṹ ]]
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// [[ ������ݽṹ
typedef struct {
    unsigned char belttime[14];
    unsigned char beltstatus;
} Safebelt;
// ������ݽṹ ]]
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// [[ OBDģ�����ݽṹ
typedef struct {
    unsigned char obdttime[14];
    unsigned char errstatus[5];
} OBDdata;
// OBDģ�����ݽṹ ]]
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// [[ ��������
#define  UPDATE_INFO_ADDR   0x08001F00
#define  MAX_TICKET_NUMBER  10
#define  PACKET_SIZE        1024
#define  RECV_PACKET_SIZE   128

typedef struct {
    unsigned char  RxBuf[PACKET_SIZE];
    unsigned int   count_total;
    unsigned short count_less1k;
    unsigned int   addr_app;
    unsigned short flag_write_flash;
    unsigned short pack_count;
    unsigned short id_req;
    unsigned short id_ans;
    unsigned char  version[20];
    unsigned char  ver_len;
    unsigned char  use_date[14];
    unsigned char  task_id[10];
    unsigned short cal_check_sum;
} Update_Struct;
extern Update_Struct update;

typedef struct {
    unsigned short flah_new;
    unsigned short count1k_packet;
    unsigned short rev_check_sum;
    unsigned char  version[20];
    unsigned char  ver_len;
    unsigned char  use_date[14];
    unsigned char  task_id[10];
    unsigned short crc16;
} UpdateInfo_Struct;
extern UpdateInfo_Struct updateInfo;
// �������� ]]
////////////////////////////////////////////////////////////////////////////////////

typedef enum {
    xfs_auto    = '0',
    xfs_num     = '1',
    xfs_value   = '2',
} XFS_nType;


typedef enum {
    day_time    = 1,
    night_time  = 2,
} DayOrNight;


// �豸����
typedef enum {
    CarTerminal     = 1,                    // ���������ն�
    DataCollector   = 2,                    // ���ݲɼ���
    PolicePhone     = 3,                    // ����ͨ
    UserPhone       = 4,                    // �����ֻ�
    EleDriverBall   = 5,                    // ���Ӽ�����
    PoliceServer    = 6,                    // ������
    CarTerManager   = 7,                    // �����ն˹�����
    OBD_Module      = 8,                    // OBDģ��
    LightModule     = 9,                    // Զ����Ƽ��ģ��
    SetupApp        = 10                    // ��װApp
} MsgSource_Typedef;

// �������Ͷ���
typedef enum {
    CarType01 = 1,                          // 01�й�糵
    CarType02 = 2,                          // 02�޹�糵
    CarType03 = 3,                          // 03��ʽ���л�е��
    CarType04 = 4,                          // 04���Ħ�г�
    CarType05 = 5,                          // 05��ͨ����Ħ�г�
    CarType06 = 6,                          // 06��ͨ����Ħ�г�
    CarType07 = 7,                          // 07�м���ר��С���Զ����ؿ�����
    CarType08 = 8,                          // 08��������
    CarType09 = 9,                          // 09�����ػ�����
    CarType10 = 10,                         // 10С���Զ�������
    CarType11 = 11,                         // 11С������
    CarType12 = 12,                         // 12���ͻ���
    CarType13 = 13,                         // 13���Ϳͳ�
    CarType14 = 14,                         // 14���й�����
    CarType15 = 15,                         // 15ǣ����
    CarType16 = 16                          // 16���Ϳͳ�
} CarTypeDef;


// Υ������
typedef enum {
    IllegalUnknow       = 0,                // δ֪
    ino_IllegalPark     = 1,                // Υ��ͣ��
    ino_NoDriveLic      = 2,                // ��֤��ʻ
    IllegalUseFarLight  = 3,                // ����Զ��
    DriveNoSeatBelt     = 4,                // ûϵ��ȫ��
    IllegalUseSeatBelt  = 5,                // Υ��ʹ�ð�ȫ��
    OverSpeed           = 6,                // ����
    FatigueDrive        = 7,                // ƣ�ͼ�ʻ
    BusIllegalRun       = 8,                // ��ͳ�ҹ��Υ������
    IllegalRun          = 9,                // Υ�����й涨
    IllegalUseTrumpet   = 10,               // Υ��ʹ������
    DrivePlayPhone_enum = 11,               // �������ֻ�
    Acceleratione_enum  = 12,               // ��ͣ����
} ViolenceType_Typedef;


// �����ն˵�״̬����
typedef enum {
    Status_Park = 0,
    Status_ReadyRunOrPark = 1,
    Status_Run = 2,
} TerminalStatus_Typedef;
extern TerminalStatus_Typedef TerminalStatus;


// ��Ϣͷ ���ݽṹ
typedef struct {
    unsigned short MsgID;                   // ��ϢID
    unsigned char  MsgProperty;             // ��Ϣ����
    unsigned short MsgLen;                  // ��Ϣ����
    unsigned short MsgSource;               // ���ݵ���Դ�豸
    unsigned short MsgNum;                  // ��Ϣ��ˮ��

    // ��Ϣ�������зְ��������������2����Ϣ��
    unsigned short MsgPacketCount;          // ��Ϣ�ܰ���
    unsigned short MsgPacketId;             // �����

    unsigned short MsgCheck;                // У����
} MsgHead_Struct;


//���ݲɼ��ǻ�������ı�־��Ϣ
typedef struct {
    unsigned char MsgWhistle;
    unsigned char MsgHighBeams;
    unsigned char MsgCity;
    unsigned char MsgHighway;
    unsigned char MsgWirelessSignal;
    unsigned char MsgRestAreas;
} Msg_Sign_DataCollector_Struct;
extern Msg_Sign_DataCollector_Struct DataCollectorMsg;


//���������������־��Ϣ
typedef struct {
    unsigned char Restrict_Direction;
    unsigned char Max_Speed;
    unsigned char Vehicle_Type;
} Direction_Speed_Group_Struct;
extern Direction_Speed_Group_Struct Dir_SGroup;


////////////////////////////////////////////////////////////////////////////////////
// �ó���¼�ṹ�嶨��
typedef struct {
    unsigned char Time[14];
    unsigned char idType[__DRIVER_TYPE_LEN__];
    unsigned char idNo[__DRIVERID_LEN__];
    unsigned char type;
} UserCarDef;
////////////////////////////////////////////////////////////////////////////////////


#endif
