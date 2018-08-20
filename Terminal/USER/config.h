#ifndef __CONFIG_H__
#define __CONFIG_H__





typedef enum
{
    BreakTrafficRule_step1 = 0,
    BreakTrafficRule_step2 = 1,
    BreakTrafficRule_step3 = 2,
    BreakTrafficRule_step4 = 3,
    BreakTrafficRule_step5 = 4
} BTR_Status_Typedef; //Υ����ͨΥ��  Break Traffic Rule


/*---�豸ID���ݽṹ --------------------------------------------*/
typedef enum
{
    CarTerminal = 1,                        //���������ն�
    DataCollector = 2,                      //���ݲɼ���
    PolicePhone = 0x03,                     //����ͨ
    UserPhone = 4,                          //�����ֻ�
    EleDriverBall = 5,                      //���Ӽ�����
    PoliceServer = 6,                       //������
    CarTerManager = 7,                      //�����ն˹�����
    OBD_Module = 8                          //OBDģ��
} MsgSource_Typedef;

/* ---��Ϣͷ ���ݽṹ --------------------------------------------*/
typedef struct
{
    unsigned short int MsgID;               //��ϢID
    unsigned short int MsgLen;              //��Ϣ����
    MsgSource_Typedef  MsgSource;           //��ϢԴͷ
    unsigned short int MsgNum;              //��Ϣ��ˮ��
    unsigned short int MsgCheck;            //У����
    //unsigned char    MsgEncryption;       //��Ϣ���ܡ��ְ�
} MsgHead_Struct;

/* ����������Ϣ���ݽṹ --------------------------------------------*/
typedef struct
{
    unsigned char  Car5SN[5];	            //ѹ����ĳ��Ʊ���
    unsigned char  CarType[2];              //��������
    unsigned char  Car9SN[9];               //���ƺ�
    unsigned char  LicenceType[2];          //��������
    unsigned char  LicenceID[18];           //���պ���
    unsigned char  FamilyTelephone[55];		//�������
//	unsigned char InquireCipher[4];			//��ѯ����
    unsigned char  AuthCode[11];            //��Ȩ��
    unsigned char  VolumeValue[1];          //����
} CarInformation_Struct;

/* ����Υ����Ϣ���ݽṹ --------------------------------------------*/
typedef struct
{
    unsigned int  ViolenceSN;               //���
    unsigned char PlateNum[11];             //���ƺ���
    unsigned char StartTime[14];            //��ʼʱ��
    unsigned char EndTime[14];              //����ʱ��
    unsigned char DriveLic[20];             //Υ�¼��պ�
    unsigned char Location[22];             //Υ��λ����Ϣ
    unsigned char ViolenceType[1];          //Υ������
    unsigned char ViolenceGrade[1];         //Υ�µȼ�
} ViolenceInfo_Struct;

/*--Υ������������--------------------------------------------*/
typedef enum
{
    NoDriveLic = '0',                       //��֤��ʻ
    IllegalUseFarLight = '1',               //����Զ��
    DriveNoSeatBelt = '2',                  //������ϵ��ȫ��
    IllegalUseSeatBelt = '3',               //Υ��ʹ�ð�ȫ��
    OverSpeed = '4',                        //����
    FatigueDrive = '5',                     //ƣ�ͼ�ʻ
    BusIllegalRun = '6',                    //��ͳ�ҹ��Υ������
    IllegalRun = '7',                       //Υ�����й涨
    IllegalUseTrumpet = '8',                //Υ��ʹ������
    ElectroniccTicket = '9',                //���ӷ���
    DrivePlayPhone = 'a',                   //�������ֻ�
} ViolenceType_Typedef;

/*--Υ�µȼ�������--------------------------------------------*/
typedef enum
{
    BreakRule = '0',//Υ��
    BreakLaw = '1',//Υ��
} ViolationGrade_Typedef;

/*--������ص�������������������--------------------------------------------*/
typedef enum
{
    CarSNCode = 0,				//�����ĳ��ƺ�
    CarNumType = 1,				//��������
    CarNum =2,				    //��������
    Telephone = 3,				//�ֻ�����
    DriveNumType = 4,			//��������
    DriveNum = 5,				//���պ���
    PassWord = 6,			    //��ѯ����
    AuthCodeNum = 7,		    //��Ȩ��
    VolumeValue =8,             //����
} CarDataType_Typedef;          //Υ����ͨΥ��

/*--���Ƶı����ʽ--------------------------------------------*/
typedef enum
{
    GBK = 0,//GBK����
    UTF_8 = 1//UTF-8����
} CarEncodeType_Typedef; //���ƺű�������

/*--�����ն˵�״̬-------------------------------------------*/
typedef enum
{
    Status_Park = 0,			//�복����״̬
    Status_ReadyRunOrPark = 1,//ͣ��״̬
    Status_Run = 2,				//��ʻ״̬
} TerminalStatus_Typedef; //���ƺű�������

#endif /* __NMEA_CONFIG_H__ */
