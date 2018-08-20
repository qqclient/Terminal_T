/******************** (C) COPYRIGHT soarsky ********************
* File Name          :
* Author             :
* Version            :
* Date               :
* Description        : Flash的特性是：写数据只能将1写为0，擦除数据就是写1
*                      因此如果想在以有数据的falsh上写入新的数据，则必须先擦除。
*                      还有Flash在擦除的时候必须整块擦除
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
* Description    : 将终端中的基本参数写到Flash地址
* Input          : None
* Output         : None
* Return         : None
* Remark         : 当结构变量m_carinfo中数据需要保存时调用
*******************************************************************************/
void MX25L128_CreatSaveTask(void)
{
    m_delaySave = 3000;
    m_flagSave = SET;
}


/*******************************************************************************
* Function Name  : MX25L128_SaveBase
* Description    : 保存基础数据
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
* Description    : 恢复默认值
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MX25L128_Default(void)
{
    int i = 0;

    if(m_carinfo.Info_crc16 != get_crc16( &m_carinfo.Car_Type, (unsigned char*)&m_carinfo.Info_crc16 - &m_carinfo.Car_Type )) {
        m_carinfo.Car_Type = 2;     // 小型汽车
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
                memset((char*)m_carinfo.Family_Phone[i], 'F', __PHONE_LENGTH__ );   // 填充空号码
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
        m_carinfo.Volume_Value       = 5;               // 音量
        m_carinfo.Value_HighBeam     = 0;               // 远光灯
        m_carinfo.Value_LowBeam      = 0;               // 近光灯
        m_carinfo.Value_Ignite       = 0;               // 点火
        m_carinfo.Value_Honk         = 0;               // 喇叭
        m_carinfo.Value_DriverBelt   = 0;               // 主安全带
        m_carinfo.Value_DeputyBelt   = 0;               // 副安全带1
        m_carinfo.Value_DeputyBelt2  = 0;               // 副安全带2
        m_carinfo.Value_DriverDoor   = 0;               // 门开关1
        m_carinfo.Value_SecondDoor   = 0;               // 门开关2
        m_carinfo.Flag_Simulation    = RESET;           // 按键仿真实车标志: 实车
        m_carinfo.Flag_LAMPInput     = RESET;           // IO口直接检测远近光
        m_carinfo.bt_packlen         = 155;             // 蓝牙数据包长度定义
    }

    m_carinfo.Flag_GSMSign            = RESET;
    m_carinfo.Count_IllegalPark       = 0;
    m_carinfo.Flag_Inspection_Channel = 0xff;

    // 可变参数
    if( m_carinfo.Param_crc16 != get_crc16( (unsigned char*)&m_carinfo.FD01_TimeOver, (unsigned char*)&m_carinfo.Param_crc16 - (unsigned char*)&m_carinfo.FD01_TimeOver ) ) {
        m_carinfo.FD01_TimeOver      = 15;              // 秒, 终端进入状态二后至提示防盗时间
        m_carinfo.FD02_TimeOver      = 90;              // 秒, 终端进入状态二后至触发防盗时间   
        m_carinfo.FD03_TimeOver      = 1*60;            // 秒, 触发防盗前上传车辆位置周期
        m_carinfo.FD04_TimeOver      = 15;              // 秒, 触发防盗后上传车辆位置周期
        m_carinfo.JZ01_TimeOver      = 30;              // 秒, 车辆开动后至提示验证驾驶员时间
        m_carinfo.JZ02_TimeOver      = 90;              // 秒, 车辆开动后至提示报无证驾驶时间 内场：10  外：90
        m_carinfo.JZ03_TimeOver      = 30;              // 秒, 自动登录驾照排队时间（超过此时间未通信的驾照移出队列）
        m_carinfo.JZ04_TimeOver      = 300;             // 秒, 从状态一进状态二后车辆不点火、无任何驾照发来申请，等待多长时间恢复状态一
        m_carinfo.JZ05_TimeOver      = 300;             // 秒, 状态三下无任何驾照发来申请，等待多长时间恢复状态一（未点火）／状态二（点火）
        m_carinfo.JZ06_TimeOver      = 600;             // 秒, 状态四下多长时间没有接收到驾照随车信息恢复状态一（停车熄火）／状态二（未熄火）
        m_carinfo.JZ07_TimeOver      = 20;              // 秒, 选定驾驶员后，多长时间接收不到此驾照的下一次申请，才去选择次优先级的申请
        m_carinfo.WZ01_CountOver     = 2;               // 次, 连续行车状态下，连续收到几次使用手机的信息（同一次使用手机）才记录违章
        m_carinfo.WZ02_TimeOver      = 30;              // 秒, 前后两个使用手机信号间隔多久认为是另一次使用手机，重新计次判断是否记录违章
        m_carinfo.WZ03_TimeOver      = 10;              // 秒, 车辆行驶后多久提示系安全带
        m_carinfo.WZ04_TimeOver      = 60;              // 秒, 车辆行驶后多久记录不系安全带违章 
        m_carinfo.WZ05_TimeOver      = 5;               // 秒, 收到对向来车变光信号后多久不变光记违章
        m_carinfo.WZ06_TimeOver      = 10;              // 秒, 收到禁用远光信号后多久不变光记违章
        sprintf((char*)m_carinfo.WZ07_Timer, "2:00");   // 24小时制, 大客车夜间禁行开始时间
        sprintf((char*)m_carinfo.WZ08_Timer, "5:00");   // 24小时制, 大客车夜间禁行结束时间
        sprintf((char*)m_carinfo.WZ09_Timer, "1:30");   // 24小时制, 大客车夜间禁行开始预警时间（在此时间之后，禁行开始时间之前，进入高速立即预警）
        m_carinfo.WZ10_TimeOver      = 600;             // 秒, 大客车夜间禁行预警周期（第一次提醒后，禁行开始时间之前，隔多久提醒一次）
        m_carinfo.WZ11_TimeOver      = 600;             // 秒, 大客车夜间禁行时段上高速后，多长时间记录违章
        m_carinfo.WZ13_TimeOver      = 4;               // 小时, 单次累计驾车多久提醒疲劳驾驶
        m_carinfo.WZ14_TimeOver      = 600;             // 秒, 疲劳驾驶提醒循环周期
        m_carinfo.WZ15_TimeOver      = 3000;            // 秒, 提醒疲劳驾驶后多久不休息或更换驾驶员记违章
        m_carinfo.WZ16_TimeOver      = 1200;            // 秒, 单次累计驾车进入疲劳驾驶后休息多久重新计时
        m_carinfo.SD01_TimeOver      = 15;              // 秒, 响应一寻稽查闪灯时间
        m_carinfo.SD02_TimeOver      = 60;              // 秒, 响应闪灯找车闪灯时间（开车门后关闭闪灯）
        m_carinfo.WT01_TimeOver      = 600;             // 秒, 离开被开罚单位置多远可自动撤销违停
        m_carinfo.WT02_DistOver      = 500;             // 米, 从开罚单开始允许撤消违停的时间长度
        m_carinfo.MoveCar_TimeOver   = 10*60;           // 秒[10*60], 请人移车电话及短信的保护时间
        m_carinfo.FollowCar_TimeOver = 10*60;           // 秒[10*60], 驾照随车超时时间
    }

    if(m_carinfo.Voice_crc16 != get_crc16( (unsigned char*)&m_carinfo.Y01_Addr, (unsigned char*)&m_carinfo.Voice_crc16 - (unsigned char*)&m_carinfo.Y01_Addr )) {
        m_carinfo.Y01_Addr = NULL;   // 您已进入禁用远光灯区域，请不要开启远光灯
        m_carinfo.Y02_Addr = NULL;   // 请您关闭远光灯，否则记录违章
        m_carinfo.Y03_Addr = NULL;   // 您因违规使用远光灯已被记录违章
        m_carinfo.Y04_Addr = NULL;   // 请您出示电子驾照
        m_carinfo.Y05_Addr = NULL;   // 请您出示电子驾照
        m_carinfo.Y06_Addr = NULL;   // 电子驾照验证完成
        m_carinfo.Y07_Addr = NULL;   // 您涉嫌无证驾驶，已记录违章
        m_carinfo.Y08_Addr = NULL;   // 请您系好安全带，否则记录违章
        m_carinfo.Y09_Addr = NULL;   // 您因未系安全带，已被记录违章
        m_carinfo.Y10_Addr = NULL;   // 请您正确使用安全带，否则记录违章
        m_carinfo.Y11_Addr = NULL;   // 您因违规使用安全带，已被记录违章
        m_carinfo.Y12_Addr = NULL;   // 您已连续驾驶超过XX小时XX分钟，请您在前方适当位置停车休息20分钟以上再继续驾驶。若连续驾驶超过4小时，将被记录违章。如需更换驾驶员，请验证新驾驶员电子驾照。
        m_carinfo.Y13_Addr = NULL;   // 您已连续驾驶超过XX小时XX分钟，请您在前方适当位置停车休息20分钟以上再继续驾驶。若连续驾驶超过4小时，将被记录违章。如需更换驾驶员，请验证新驾驶员电子驾照。
        m_carinfo.Y14_Addr = NULL;   // 您24小时内已累计驾驶超过7小时XX分钟，请您在前方适当位置停车休息或更换驾驶员。若累计驾驶超过8小时，将被记录违章。
        m_carinfo.Y15_Addr = NULL;   // 您因疲劳驾驶，已被记录违章
        m_carinfo.Y16_Addr = NULL;   // 请您凌晨2:00至5:00停运或更换驾驶员，否则记录违章
        m_carinfo.Y17_Addr = NULL;   // 请您允分休息后再驾车，否则记录违章?
        m_carinfo.Y18_Addr = NULL;   // 您在夜间进入三级以下公路，请尽快停车或离开此区域，否则记录违章
        m_carinfo.Y19_Addr = NULL;   // 您因违反大客车夜间行驶规定，已被记录违章
        m_carinfo.Y20_Addr = NULL;   //
        m_carinfo.Y21_Addr = NULL;   // 您已接近限行区域，请及时改道
        m_carinfo.Y22_Addr = NULL;   // 您因违反限行规定，已被记录违章
        m_carinfo.Y23_Addr = NULL;   // 当前限速XX，您已超速XX%，请控制车速，否则记录违章
        m_carinfo.Y24_Addr = NULL;   // 您因超速，已被记录违章
        m_carinfo.Y25_Addr = NULL;   // 当前已进入/离开禁鸣区域
        m_carinfo.Y26_Addr = NULL;   // 此区域禁鸣
        m_carinfo.Y27_Addr = NULL;   // 您因违反禁鸣规定，已被记录违章
        m_carinfo.Y28_Addr = NULL;   // 您的驾照等级太低，不能驾驶该车，否则将记录违章
        m_carinfo.Y29_Addr = NULL;   // 智能终端再根据要求进行自动语音播报
        m_carinfo.Y30_Addr = NULL;   // 祝您一路顺风
        m_carinfo.Y31_Addr = NULL;   // 请停止违章使用手机，否则记录违章
    }

    MX25L128_SaveBase(SET);
}

/*******************************************************************************
* Function Name  : MX25L128_Initialize
* Description    : 车辆相关信息初始化
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

    // 读取基本信息
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

        m_carinfo.FD03_TimeOver     = 60;               // 秒, 触发防盗前上传车辆位置周期
        m_carinfo.FD04_TimeOver     = 20;               // 秒, 触发防盗后上传车辆位置周期
        m_carinfo.MoveCar_TimeOver  = 10*60;            // 秒, 请人移车电话及短信的保护时间
        m_carinfo.FollowCar_TimeOver= 10*60;            // 秒, 驾照随车超时时间
        m_carinfo.JZ01_TimeOver     = 30;               // 秒, 车辆开动后至提示验证驾驶员时间
        m_carinfo.JZ02_TimeOver     = 90;               // 秒, 车辆开动后至提示报无证驾驶时间 内场：10  外：90
        m_carinfo.JZ04_TimeOver     = 0.5*60;           // 秒, 从状态一进状态二后车辆不点火、无任何驾照发来申请，等待多长时间恢复状态一
        m_carinfo.JZ05_TimeOver     = 0.5*60;           // 秒, 状态三下无任何驾照发来申请，等待多长时间恢复状态一（未点火）／状态二（点火）
        m_carinfo.JZ06_TimeOver     = 1*60;             // 秒, 状态四下多长时间没有接收到驾照随车信息恢复状态一（停车熄火）／状态二（未熄火）

        m_carinfo.Flag_Inspection_Channel = 0xff;

#endif

        if( m_carinfo.Car_Type > 16 || m_carinfo.Car_Type < 1 ) {
            m_carinfo.Car_Type = 2;     // 小型汽车
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
* Description    : 写不带定位信息、车牌号及驾驶证号的违章信息
* Input          : ill:需要写入Flash的违章信息
* Output         : None
* Return         : 写入的编号
*******************************************************************************/
unsigned int MX25L128_Illegal_Write(Illegal_Def* ill)
{
    ill->ewhemi = gpsx.ewhemi;                          // 东经/西经,E:东经;W:西经
    ill->longitude = (double)gpsx.longitude / 100000;   // 违章时经度
    ill->nshemi = gpsx.nshemi;                          // 北纬/南纬,N:北纬;S:南纬
    ill->latitude  = (double)gpsx.latitude / 100000;    // 违章时纬度

    return(MX25L128_Illegal_Write_Base(ill, SET));
}

/*******************************************************************************
* Function Name  : MX25L128_Illegal_Write
* Description    : 写不带车牌号及驾驶证号的违章信息
* Input          : ill:需要写入Flash的违章信息
* Output         : None
* Return         : 写入的编号
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
* Description    : 用车驾照记录
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

