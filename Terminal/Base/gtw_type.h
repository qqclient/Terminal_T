/******************** (C) COPYRIGHT soarsky ************************************
* File Name          : gtw_type.h
* Author             : CSYXJ
* Version            : V0.1
* Date               : 2017-03-19
* Description        : 工程变量、结构、常量等定义
*******************************************************************************/

#ifndef __GTW_TYPE_H
#define __GTW_TYPE_H

typedef unsigned char   BYTE;
typedef unsigned short  WORD;
typedef unsigned int    DWORD;

#define HEAD_LENGTH 10

//按键检测延时时间
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

extern unsigned int Sys_RunTime;        // 记录下终端启动的时间


// int数据与字节数组互转联合
typedef union {
    unsigned char c[4];
    unsigned int  x;
} i2b_Def;

// float数据与字节数组互转联合
typedef union {
    unsigned char c[4];// 0 1 2 3
    float  x;   //0123
} f2b_Def;

// short数据与字节数组互转联合
typedef union {
    unsigned char  c[2];
    unsigned short x;
} w2b_Def;


////////////////////////////////////////////////////////////////////////////////////
// [[ Flash存放地址分配
#define ADDR_CARINFO                0x000000                // 基本运行数据存放地址
#define ADDR_VOICE                  0x001000                // 语音播报内容存放地址
#define ADDR_BROADCAST              0x002000                // 动态管控信息存放地址
#define ADDR_PWR_RECORD             0x008000                // 智能终端电源记录存放地址
#define ADDR_USE_RECORD             0x010000                // 车辆使用记录存放地址
#define ADDR_SPEED_RECORD           0x110000                // 急加速与急减速记录存放地址
#define ADDR_CAR_MEET               0x2F0000                // 会车时违章记录存放地址
#define ADDR_ILLEGAL_UPLOAD         0x300000                // 违章未上传存放开始地址
#define ADDR_ILLEGAL_CONTECT        0x301000                // 违章记录存放开始地址
#define ADDR_DriVer_RECORD          0x306000                // 驾照球认证登入登出记录存放开始地址
// Flash存放地址分配 ]]
////////////////////////////////////////////////////////////////////////////////////


typedef struct {
    unsigned char  Flag_ApplyDriver;
    unsigned short FollowCar_TimeOver;
    unsigned short Apply1_TimeOver;
    unsigned char  Flag_DriverPark;
    unsigned char  Flag_InitLAMPidNo;                       // 初始化大灯模块车牌: RESET=不写车牌; SET=写车牌
    unsigned char  Flag_Debug_Enable;                       // 是否允许调试信息输出: RESET=不输出; SET=输出
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
	  unsigned int   ReadBeamTimer;    //当点火后延时二秒后才能检测远近灯
} Base_Def;
extern Base_Def m_var;

////////////////////////////////////////////////////////////////////////////////////
// [[ 基本运行数据结构
// 长度定义
#define __DRIVER_TYPE_LEN__         2                       // 驾照类型长度
#define __DRIVERID_LEN__            18                      // 驾照长度 = 驾照类型(2B) + 号码(18B)
#define __PASSWORD_LEN__            6                       // 查询密码长度
#define __CAR5SN_LEN__              5                       // 压缩电子车牌信息长度
#define __CAR8SN_LEN__              8                       // GBK码电子车牌信息长度
#define __CAR9SN_LEN__              9                       // UTF-8码电子车牌信息长度
#define __AUTHORIZE_LEN__           16                      // 授权码长度
#define __DTU_ID_LEN__              18                      // 终端编号长度
#define __SERVERNAME_LEN__          24                      // 服务器名称长度
#define __PHONE_LENGTH__            16                      // 手机亲情号码长度

typedef struct {
    unsigned char  Flag_head;                               // 结构定义开始标记 - 必须保证为第一字节

    unsigned char  Car_Type;
    unsigned char  Driver_Type;
    unsigned char  Car_5SN[ __CAR5SN_LEN__ + 1 ];           // 压缩电子车牌信息
    unsigned char  Car_8SN[ __CAR8SN_LEN__ + 1 ];           // GBK码电子车牌信息
    unsigned char  Car_9SN[ __CAR9SN_LEN__ + 1 ];           // UTF-8码电子车牌信息
    unsigned char  Family_Phone[5][ __PHONE_LENGTH__ + 1 ]; // 手机亲情号码
    unsigned char  Driver_ID[5][ __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__ + 1 ];    // 驾照号码 = 驾照类型(2B) + 号码(18B)
    unsigned char  Password[ __PASSWORD_LEN__ + 1 ];        // 查询密码
    unsigned char  Authorize_Code[ __AUTHORIZE_LEN__ + 1 ]; // 授权码

    unsigned char  DTU_ID[ __DTU_ID_LEN__ + 1 ];            // 设备id 18Byte
    unsigned char  DTU_Type;                                // 终端类型: MsgSource_Typedef中所定义

    unsigned char  NetAddrType;                             // 服务器地址类型: 0=ip地址, 1=域名地址
    unsigned int   NetPort;                                 // 服务器端口
    unsigned char  ServerName[ __SERVERNAME_LEN__ + 1 ];    // 服务器地址

    unsigned short Info_crc16;


    // 输入采集学习到的接口状态[关闭状态]
    unsigned char  Volume_Value;                            // 音量
    unsigned char  Value_HighBeam;                          // 远光灯  //==SET表示打开,==RESET表示关闭
    unsigned char  Value_LowBeam;                           // 近光灯
    unsigned char  Value_Ignite;                            // 点火
    unsigned char  Value_Honk;                              // 喇叭
    unsigned char  Value_DriverBelt;                        // 主安全带
    unsigned char  Value_DeputyBelt;                        // 副安全带1
    unsigned char  Value_DeputyBelt2;                       // 副安全带2
    unsigned char  Value_DriverDoor;                        // 门开关1
    unsigned char  Value_SecondDoor;                        // 门开关2

    unsigned char  Flag_Simulation;                         // 按键仿真实车标志
    unsigned char  Flag_LAMPInput;                          // 远近光检测方式: 0=IO直接检测; 1=RF433MHz模块检测
    unsigned short bt_packlen;                              // 蓝牙数据包长度定义

    unsigned short Input_crc16;


    unsigned char  Flag_GSMSign;                            // GSM卡信号播报: 0=
    unsigned char  Count_IllegalPark;                       // 违停次数,每月前2次可撤销
    unsigned char  Flag_Inspection_Channel;                 // 0xff: wifi; 0x00: BlueTooth


    // 可变参数
    unsigned short FD01_TimeOver;                           // 秒, 终端进入状态二后至提示防盗时间
    unsigned short FD02_TimeOver;                           // 秒, 终端进入状态二后至触发防盗时间
    unsigned short FD03_TimeOver;                           // 秒, 触发防盗前上传车辆位置周期
    unsigned short FD04_TimeOver;                           // 秒, 触发防盗后上传车辆位置周期
    unsigned short JZ01_TimeOver;                           // 秒, 车辆开动后至提示验证驾驶员时间
    unsigned short JZ02_TimeOver;                           // 秒, 车辆开动后至提示报无证驾驶时间
    unsigned short JZ03_TimeOver;                           // 秒, 自动登录驾照排队时间（超过此时间未通信的驾照移出队列）
    unsigned short JZ04_TimeOver;                           // 秒, 从状态一进状态二后车辆不点火、无任何驾照发来申请，等待多长时间恢复状态一
    unsigned short JZ05_TimeOver;                           // 秒, 状态三下无任何驾照发来申请，等待多长时间恢复状态一（未点火）／状态二（点火）
    unsigned short JZ06_TimeOver;                           // 秒, 状态四下多长时间没有接收到驾照随车信息恢复状态一（停车熄火）／状态二（未熄火）
    unsigned short JZ07_TimeOver;                           // 秒, 选定驾驶员后，多长时间接收不到此驾照的下一次申请，才去选择次优先级的申请
    unsigned short WZ01_CountOver;                          // 次, 连续行车状态下，连续收到几次使用手机的信息（同一次使用手机）才记录违章
    unsigned short WZ02_TimeOver;                           // 秒, 前后两个使用手机信号间隔多久认为是另一次使用手机，重新计次判断是否记录违章
    unsigned short WZ03_TimeOver;                           // 秒, 车辆行驶多久后提示系安全带
    unsigned short WZ04_TimeOver;                           // 秒, 车辆行驶多久后记录不系安全带违章
    unsigned short WZ05_TimeOver;                           // 秒, 收到对向来车变光信号后多久不变光记违章
    unsigned short WZ06_TimeOver;                           // 秒, 收到禁用远光信号后多久不变光记违章
    unsigned char  WZ07_Timer[6];                           // 24小时制, 大客车夜间禁行开始时间
    unsigned char  WZ08_Timer[6];                           // 24小时制, 大客车夜间禁行结束时间
    unsigned char  WZ09_Timer[6];                           // 24小时制, 大客车夜间禁行开始预警时间（在此时间之后，禁行开始时间之前，进入高速立即预警）
    unsigned short WZ10_TimeOver;                           // 秒, 大客车夜间禁行预警周期（第一次提醒后，禁行开始时间之前，隔多久提醒一次）
    unsigned short WZ11_TimeOver;                           // 秒, 大客车夜间禁行时段上高速后，多长时间记录违章
    unsigned short WZ12_TimeOver;                           // 秒, 24小时内累计驾车多长时间以上向终端发信息提醒疲劳驾驶
    unsigned short WZ13_TimeOver;                           // 秒, 单次累计驾车多久提醒疲劳驾驶
    unsigned short WZ14_TimeOver;                           // 秒, 疲劳驾驶提醒循环周期
    unsigned short WZ15_TimeOver;                           // 秒, 提醒疲劳驾驶后多久不休息或更换驾驶员记违章
    unsigned short WZ16_TimeOver;                           // 秒, 单次累计驾车进入疲劳驾驶后休息多久重新计时
    unsigned short SD01_TimeOver;                           // 秒[15], 响应一寻稽查闪灯时间
    unsigned short SD02_TimeOver;                           // 秒, 响应闪灯找车闪灯时间（开车门后关闭闪灯）
    unsigned short WT01_TimeOver;                           // 秒[10*60], 从开罚单开始允许撤消违停的时间长度
    unsigned short WT02_DistOver;                           // 米[500], 离开被开罚单位置多远可自动撤销违停
    unsigned short MoveCar_TimeOver;                        // 秒[20*60], 收到移车请求后电话及短信保护时间
    unsigned short FollowCar_TimeOver;                      // 秒[10*60], 驾照随车超时时间

    unsigned short Param_crc16;


    // 语音播报入口地址
    unsigned char* Y01_Addr;                                // 您已进入禁用远光灯区域，请不要开启远光灯
    unsigned char* Y02_Addr;                                // 请您关闭远光灯，否则记录违章
    unsigned char* Y03_Addr;                                // 您因违规使用远光灯已被记录违章
    unsigned char* Y04_Addr;                                // 请您出示电子驾照
    unsigned char* Y05_Addr;                                // 请您出示电子驾照
    unsigned char* Y06_Addr;                                // 电子驾照验证完成
    unsigned char* Y07_Addr;                                // 您涉嫌无证驾驶，已记录违章
    unsigned char* Y08_Addr;                                // 请您系好安全带，否则记录违章
    unsigned char* Y09_Addr;                                // 您因未系安全带，已被记录违章
    unsigned char* Y10_Addr;                                // 请您正确使用安全带，否则记录违章
    unsigned char* Y11_Addr;                                // 您因违规使用安全带，已被记录违章
    unsigned char* Y12_Addr;                                // 您已连续驾驶超过XX小时XX分钟，请您在前方适当位置停车休息20分钟以上再继续驾驶。若连续驾驶超过4小时，将被记录违章。如需更换驾驶员，请验证新驾驶员电子驾照。
    unsigned char* Y13_Addr;                                // 您已连续驾驶超过XX小时XX分钟，请您在前方适当位置停车休息20分钟以上再继续驾驶。若连续驾驶超过4小时，将被记录违章。如需更换驾驶员，请验证新驾驶员电子驾照。
    unsigned char* Y14_Addr;                                // 您24小时内已累计驾驶超过7小时XX分钟，请您在前方适当位置停车休息或更换驾驶员。若累计驾驶超过8小时，将被记录违章。
    unsigned char* Y15_Addr;                                // 您因疲劳驾驶，已被记录违章
    unsigned char* Y16_Addr;                                // 请您凌晨2:00至5:00停运或更换驾驶员，否则记录违章
    unsigned char* Y17_Addr;                                // 请您允分休息后再驾车，否则记录违章?
    unsigned char* Y18_Addr;                                // 您在夜间进入三级以下公路，请尽快停车或离开此区域，否则记录违章
    unsigned char* Y19_Addr;                                // 您因违反大客车夜间行驶规定，已被记录违章
    unsigned char* Y20_Addr;                                //
    unsigned char* Y21_Addr;                                // 您已接近限行区域，请及时改道
    unsigned char* Y22_Addr;                                // 您因违反限行规定，已被记录违章
    unsigned char* Y23_Addr;                                // 当前限速XX，您已超速XX%，请控制车速，否则记录违章
    unsigned char* Y24_Addr;                                // 您因超速，已被记录违章
    unsigned char* Y25_Addr;                                // 当前已进入/离开禁鸣区域
    unsigned char* Y26_Addr;                                // 此区域禁鸣
    unsigned char* Y27_Addr;                                // 您因违反禁鸣规定，已被记录违章
    unsigned char* Y28_Addr;                                // 您的驾照等级太低，不能驾驶该车，否则将记录违章
    unsigned char* Y29_Addr;                                // 智能终端再根据要求进行自动语音播报
    unsigned char* Y30_Addr;                                // 祝您一路顺风
    unsigned char* Y31_Addr;                                // 请停止违章使用手机，否则记录违章

    unsigned short Voice_crc16;


    double         distance;                                // 总里程统计
    unsigned int   distance_longitude;                      // 里程数保存时的gps位置
    unsigned int   distance_latitude;                       // 里程数保存时的gps位置


    unsigned short crc16;                                   // 保存数据校验和

    unsigned char  Flag_end;                                // 结构定义结束标记 - 必须保证为最后2字节

} CarInfo_Def;
extern CarInfo_Def m_carinfo;
// 基本运行数据结构 ]]
////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////
// [[ 违章数据结构
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
// 违章数据结构 ]]
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// [[ 变光数据结构
typedef struct {
    unsigned char ptime[14];
    unsigned char pstatus;
} Lamp_Def;
// 变光数据结构 ]]
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// [[ 异常车速数据结构
typedef struct {
    unsigned char  Nspeedtime[14];
    unsigned char  Nspeedtimestatus;
    unsigned char  Nspeeddata;
} AbnomalSpeed;
// 异常车速数据结构 ]]
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// [[ 变光数据结构
typedef struct {
    unsigned char belttime[14];
    unsigned char beltstatus;
} Safebelt;
// 变光数据结构 ]]
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// [[ OBD模块数据结构
typedef struct {
    unsigned char obdttime[14];
    unsigned char errstatus[5];
} OBDdata;
// OBD模块数据结构 ]]
////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////
// [[ 升级程序
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
// 升级程序 ]]
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


// 设备类型
typedef enum {
    CarTerminal     = 1,                    // 车载智能终端
    DataCollector   = 2,                    // 数据采集仪
    PolicePhone     = 3,                    // 警务通
    UserPhone       = 4,                    // 车主手机
    EleDriverBall   = 5,                    // 电子驾照球
    PoliceServer    = 6,                    // 服务器
    CarTerManager   = 7,                    // 智能终端管理工具
    OBD_Module      = 8,                    // OBD模块
    LightModule     = 9,                    // 远近光灯检测模块
    SetupApp        = 10                    // 安装App
} MsgSource_Typedef;

// 车辆类型定义
typedef enum {
    CarType01 = 1,                          // 01有轨电车
    CarType02 = 2,                          // 02无轨电车
    CarType03 = 3,                          // 03轮式自行机械车
    CarType04 = 4,                          // 04轻便摩托车
    CarType05 = 5,                          // 05普通二轮摩托车
    CarType06 = 6,                          // 06普通三轮摩托车
    CarType07 = 7,                          // 07残疾人专用小型自动挡载客汽车
    CarType08 = 8,                          // 08三轮汽车
    CarType09 = 9,                          // 09低速载货汽车
    CarType10 = 10,                         // 10小型自动挡汽车
    CarType11 = 11,                         // 11小型汽车
    CarType12 = 12,                         // 12大型货车
    CarType13 = 13,                         // 13中型客车
    CarType14 = 14,                         // 14城市公交车
    CarType15 = 15,                         // 15牵引车
    CarType16 = 16                          // 16大型客车
} CarTypeDef;


// 违章类型
typedef enum {
    IllegalUnknow       = 0,                // 未知
    ino_IllegalPark     = 1,                // 违章停车
    ino_NoDriveLic      = 2,                // 无证驾驶
    IllegalUseFarLight  = 3,                // 滥用远光
    DriveNoSeatBelt     = 4,                // 没系安全带
    IllegalUseSeatBelt  = 5,                // 违规使用安全带
    OverSpeed           = 6,                // 超速
    FatigueDrive        = 7,                // 疲劳驾驶
    BusIllegalRun       = 8,                // 大客车夜间违章运行
    IllegalRun          = 9,                // 违反限行规定
    IllegalUseTrumpet   = 10,               // 违规使用喇叭
    DrivePlayPhone_enum = 11,               // 开车玩手机
    Acceleratione_enum  = 12,               // 急停急走
} ViolenceType_Typedef;


// 智能终端的状态类型
typedef enum {
    Status_Park = 0,
    Status_ReadyRunOrPark = 1,
    Status_Run = 2,
} TerminalStatus_Typedef;
extern TerminalStatus_Typedef TerminalStatus;


// 消息头 数据结构
typedef struct {
    unsigned short MsgID;                   // 消息ID
    unsigned char  MsgProperty;             // 消息属性
    unsigned short MsgLen;                  // 消息长度
    unsigned short MsgSource;               // 数据的来源设备
    unsigned short MsgNum;                  // 消息流水号

    // 消息属性中有分包标记则就有下面的2条信息域
    unsigned short MsgPacketCount;          // 消息总包数
    unsigned short MsgPacketId;             // 包序号

    unsigned short MsgCheck;                // 校验码
} MsgHead_Struct;


//数据采集仪稽查命令的标志信息
typedef struct {
    unsigned char MsgWhistle;
    unsigned char MsgHighBeams;
    unsigned char MsgCity;
    unsigned char MsgHighway;
    unsigned char MsgWirelessSignal;
    unsigned char MsgRestAreas;
} Msg_Sign_DataCollector_Struct;
extern Msg_Sign_DataCollector_Struct DataCollectorMsg;


//限速限行命令组标志信息
typedef struct {
    unsigned char Restrict_Direction;
    unsigned char Max_Speed;
    unsigned char Vehicle_Type;
} Direction_Speed_Group_Struct;
extern Direction_Speed_Group_Struct Dir_SGroup;


////////////////////////////////////////////////////////////////////////////////////
// 用车记录结构体定义
typedef struct {
    unsigned char Time[14];
    unsigned char idType[__DRIVER_TYPE_LEN__];
    unsigned char idNo[__DRIVERID_LEN__];
    unsigned char type;
} UserCarDef;
////////////////////////////////////////////////////////////////////////////////////


#endif
