#ifndef __CONFIG_H__
#define __CONFIG_H__





typedef enum
{
    BreakTrafficRule_step1 = 0,
    BreakTrafficRule_step2 = 1,
    BreakTrafficRule_step3 = 2,
    BreakTrafficRule_step4 = 3,
    BreakTrafficRule_step5 = 4
} BTR_Status_Typedef; //违反交通违章  Break Traffic Rule


/*---设备ID数据结构 --------------------------------------------*/
typedef enum
{
    CarTerminal = 1,                        //车载智能终端
    DataCollector = 2,                      //数据采集仪
    PolicePhone = 0x03,                     //警务通
    UserPhone = 4,                          //车主手机
    EleDriverBall = 5,                      //电子驾照球
    PoliceServer = 6,                       //服务器
    CarTerManager = 7,                      //智能终端管理工具
    OBD_Module = 8                          //OBD模块
} MsgSource_Typedef;

/* ---消息头 数据结构 --------------------------------------------*/
typedef struct
{
    unsigned short int MsgID;               //消息ID
    unsigned short int MsgLen;              //消息长度
    MsgSource_Typedef  MsgSource;           //消息源头
    unsigned short int MsgNum;              //消息流水号
    unsigned short int MsgCheck;            //校验码
    //unsigned char    MsgEncryption;       //消息加密、分包
} MsgHead_Struct;

/* 车辆基本信息数据结构 --------------------------------------------*/
typedef struct
{
    unsigned char  Car5SN[5];	            //压缩后的车牌编码
    unsigned char  CarType[2];              //车辆类型
    unsigned char  Car9SN[9];               //车牌号
    unsigned char  LicenceType[2];          //驾照类型
    unsigned char  LicenceID[18];           //驾照号码
    unsigned char  FamilyTelephone[55];		//亲情号码
//	unsigned char InquireCipher[4];			//查询密码
    unsigned char  AuthCode[11];            //授权码
    unsigned char  VolumeValue[1];          //音量
} CarInformation_Struct;

/* 车辆违章信息数据结构 --------------------------------------------*/
typedef struct
{
    unsigned int  ViolenceSN;               //编号
    unsigned char PlateNum[11];             //车牌号码
    unsigned char StartTime[14];            //开始时间
    unsigned char EndTime[14];              //结束时间
    unsigned char DriveLic[20];             //违章驾照号
    unsigned char Location[22];             //违章位置信息
    unsigned char ViolenceType[1];          //违章类型
    unsigned char ViolenceGrade[1];         //违章等级
} ViolenceInfo_Struct;

/*--违章类型联合体--------------------------------------------*/
typedef enum
{
    NoDriveLic = '0',                       //无证驾驶
    IllegalUseFarLight = '1',               //滥用远光
    DriveNoSeatBelt = '2',                  //开车不系安全带
    IllegalUseSeatBelt = '3',               //违规使用安全带
    OverSpeed = '4',                        //超速
    FatigueDrive = '5',                     //疲劳驾驶
    BusIllegalRun = '6',                    //大客车夜间违章运行
    IllegalRun = '7',                       //违反限行规定
    IllegalUseTrumpet = '8',                //违规使用喇叭
    ElectroniccTicket = '9',                //电子罚单
    DrivePlayPhone = 'a',                   //开车玩手机
} ViolenceType_Typedef;

/*--违章等级联合体--------------------------------------------*/
typedef enum
{
    BreakRule = '0',//违章
    BreakLaw = '1',//违法
} ViolationGrade_Typedef;

/*--车辆相关的特征数据类型联合体--------------------------------------------*/
typedef enum
{
    CarSNCode = 0,				//编码后的车牌号
    CarNumType = 1,				//车辆类型
    CarNum =2,				    //车辆号码
    Telephone = 3,				//手机号码
    DriveNumType = 4,			//驾照类型
    DriveNum = 5,				//驾照号码
    PassWord = 6,			    //查询密码
    AuthCodeNum = 7,		    //授权码
    VolumeValue =8,             //音量
} CarDataType_Typedef;          //违反交通违章

/*--车牌的编码格式--------------------------------------------*/
typedef enum
{
    GBK = 0,//GBK编码
    UTF_8 = 1//UTF-8编码
} CarEncodeType_Typedef; //车牌号编码类型

/*--智能终端的状态-------------------------------------------*/
typedef enum
{
    Status_Park = 0,			//离车泊车状态
    Status_ReadyRunOrPark = 1,//停车状态
    Status_Run = 2,				//行驶状态
} TerminalStatus_Typedef; //车牌号编码类型

#endif /* __NMEA_CONFIG_H__ */
