/******************** (C) COPYRIGHT soarsky ************************************
* File Name          : main.c
* Author             :
* Version            :
* Date               :
* Description        :
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"

/***************本地函数声明*************/
static void Detect_VehicleStatus(void);
void SystemUpdate(void);
static void ViolationDetect(void);
/***************全局变量声明*************/
Base_Def m_var;
unsigned int Sys_RunTime = 0;       // 记录下终端启动的时间
extern volatile unsigned short Park_TimeOver;        // 泊车后超时切换状态

TerminalStatus_Typedef TerminalStatus = Status_Park;//车辆状态

extern unsigned short Get_UpdateParameter(unsigned char* src, unsigned char* dsr);
extern UpdateInfo_Struct updateInfo;


#ifdef __USE_UCOS__

/////////////////////////////////////////////////////////////////////////////////
// uc/os支持部分 - 启动任务栈定义
#define STARTUP_TASK_PRIO 4                     // task priority
#define STARTUP_TASK_STK_SIZE 80                // task stack size

OS_TCB taskStartTCB;
CPU_STK taskStartStk[STARTUP_TASK_STK_SIZE];    // 启动任务的程序空间

static void TaskStart(void);


#define MAIN_TASK_PRIO 5                        // task priority
#define MAIN_TASK_STK_SIZE 200                  // task stack size

OS_TCB  taskMainTCB;
CPU_STK taskMainStk[MAIN_TASK_STK_SIZE];        // 启动任务的程序空间

static void TaskMain(void);


/////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
* Function Name  : Delay
* Description    : Inserts a delay time.
*******************************************************************************/
void Delay(volatile CPU_INT32U nCount)
{
    for(; nCount != 0; nCount--);
}

/*******************************************************************************
* Function Name :void SysTickInit(void)
* Description   :系统定时器时间配置
*******************************************************************************/
void SysTickInit(void)
{
    SysTick_Config( SystemCoreClock / 1000 ); 
}

/*******************************************************************************
* Function Name  : main
* Description    : 程序入口
*******************************************************************************/
int main(void)
{
    OS_ERR err;

    NVIC_Configuration();
    SysTickInit();
    CPU_Init();
    OSInit(         (OS_ERR    *)&err);
    OSTaskCreate(   (OS_TCB    *)&taskStartTCB,
                    (CPU_CHAR  *)"Task Start",
                    (OS_TASK_PTR)TaskStart,
                    (void      *)0,
                    (OS_PRIO    ) STARTUP_TASK_PRIO,
                    (CPU_STK   *)&taskStartStk[0],
                    (CPU_STK_SIZE)STARTUP_TASK_STK_SIZE / 10,
                    (CPU_STK_SIZE)STARTUP_TASK_STK_SIZE,
                    (OS_MSG_QTY )0,
                    (OS_TICK    )0,
                    (void      *)0,
                    (OS_OPT     )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                    (OS_ERR    *)&err);

    OSStart(        (OS_ERR    *)&err);
    return 0;
}


/*******************************************************************************
* Function Name : TaskStart
* Description   : 任务启动
* Input         :
* Output        :
* Other         :
* Date          :
*******************************************************************************/
static void TaskStart(void)
{
    OS_ERR  err;

    OSTaskCreate(   (OS_TCB    *)&taskMainTCB,
                    (CPU_CHAR  *)"Task Main",
                    (OS_TASK_PTR)TaskMain,
                    (void      *)0,
                    (OS_PRIO    ) MAIN_TASK_PRIO,
                    (CPU_STK   *)&taskStartStk[0],
                    (CPU_STK_SIZE)MAIN_TASK_STK_SIZE / 10,
                    (CPU_STK_SIZE)MAIN_TASK_STK_SIZE,
                    (OS_MSG_QTY )0,
                    (OS_TICK    )0,
                    (void      *)0,
                    (OS_OPT     )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                    (OS_ERR    *)&err);

    OSTaskDel(      (OS_TCB    *)&taskStartTCB,
                    (OS_ERR    *)&err);
}

#endif 


/*******************************************************************************
* Function Name  : main
* Description    : 程序入口
*******************************************************************************/
#ifdef __USE_UCOS__
static void TaskMain(void)
{
    OS_ERR  err;

#else

int main(void)
{
    NVIC_Configuration(); 
#endif

    m_var.Flag_Debug_Enable = SET;

    MCU_Initialize();

    // 设置当前版本号
    STMFLASH_Read( 0x08001F00, &updateInfo.flah_new, sizeof(UpdateInfo_Struct)/2 );//读取参数

    memset( updateInfo.version, 0, sizeof(updateInfo.version) );
    sprintf( (char*)updateInfo.version, "V%s", __ver_id );
    updateInfo.ver_len = strlen( (char*)updateInfo.version );

#ifdef __USE_USB__
    MyUSB_Init();
#endif

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto,
                    "开机,硬件%s,固件%s(%04d-%02d-%02d)",
                    __pcb_ver_id,
                    __ver_id,
                    __VER_YEAR,
                    __VER_MONTH,
                    __VER_DAY );
    if( m_var.Flag_Debug_Enable == SET )
        Debug_Printf(SET,
                     "开机,硬件%s,固件%s(%04d-%02d-%02d),%dMHz\r\n电子车牌号:%s,车辆类型:%02d\r\n违章总数:%d,未上传违章数:%d",
                     __pcb_ver_id,
                     __ver_id,
                     __VER_YEAR,
                     __VER_MONTH,
                     __VER_DAY,
                     SystemCoreClock / 1000000,
                     m_carinfo.Car_8SN,
                     m_carinfo.Car_Type,
                     ViolenceTotal,
                     ViolenceUntreated );

    if( strlen( (char*)m_carinfo.Car_5SN ) == 0 ) {
        XFS_WriteBuffer(xfs_auto, "未写车牌");
    }
//    SmartcardTest();
#endif

    ApplyLicInit(); // 验证驾驶员的堆栈初始化

    Sys_RunTime = Sys_TimeCount;

    Study_CarStatus();

    // GetLockCode(); // 获取MCU的96Bit全球ID号
    m_flagSave = RESET;
    m_delaySave = 0;

    while(1) {

#ifdef __USE_UCOS__
        OSTimeDly(  (OS_TICK    )10,
                    (OS_OPT     )OS_OPT_TIME_DLY,
                    (OS_ERR    *)&err);
#endif

        Si4432_CheckerAnswer();// 接收到采集仪的稽查信号后,将电子车牌上发给采集仪

        Si4432_RxBufferProcess();//SI4432协议解析

        XFS_Process();//当检测到喇叭空闲时,关闭喇叭,解决噪音问题

        eTicket_VoicePrompt();//电子罚单语音提示

        MX25L128_SaveBase(RESET);//检查是否有保存任务

        I2411E_Process();//初始化I2411E模块，仅初始化一次

        M35_Process();//初始化M35模块，仅初始化一次

        GPS_RxBufferProcess();//GPS数据解析

        Read_CarStatus(RESET);//IO检测车辆状态

        Detect_VehicleStatus();//检测车辆状态

        ViolationDetect();//检测违章

        PreventSteal();//防盗验证

        I2411E_Inspection();//稽查功能

        IWDG_ReloadCounter();//喂狗

        SystemUpdate();//检测是否有更新

        //Si4432_SendSnToLightMode(); // 设置大灯模块车牌  暂时不用

        // MPU_Process();
    }
}

/*******************************************************************************
* Function Name  : SystemUpdate
* Description    :
*******************************************************************************/
void SystemUpdate(void)
{
    if(updateInfo.flah_new == 1) {
        updateInfo.crc16 = get_crc16((unsigned char*)&updateInfo.flah_new, (int)&updateInfo.crc16 - (int)&updateInfo.flah_new );

        FLASH_Unlock();
        FLASH_ErasePage(UPDATE_INFO_ADDR);
        STMFLASH_Write_NoCheck(UPDATE_INFO_ADDR, &updateInfo.flah_new, sizeof(UpdateInfo_Struct)/2);
        FLASH_Lock();

        updateInfo.flah_new = 0;
        SystemReset();
    }
}

/*******************************************************************************
* Function Name  : Detect_VehicleStatus
* Description    : 检测车辆状态
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void Detect_VehicleStatus(void)
{
    switch(TerminalStatus) {
    case Status_Park: {
        if(Car_InputCheck.Value_Ignite == SET) {
            Park_TimeOver = 0;
            TerminalStatus = Status_ReadyRunOrPark;
#ifdef __DEBUG
            //XFS_WriteBuffer(xfs_auto, "准备");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "跳转到:准备行车或者准备离车状态...");
#endif
        }
				
				else if( FlagTerminalStat != 0x31 && Park_TimeOver == 0){
					
				   if( Car_InputCheck.Value_DriverDoor == RESET ){  //关门才执行  20171017增加  解决 开门 再关门后不闪灯的问题
					 
							TerminalStatus = Status_Park;
						  Park_TimeOver = 20;// 10 * 60;  　//3 * 60
						}
					
				}
        break;
			}

    case Status_ReadyRunOrPark: {
        if( GetSpeed() >= SPEED_MIN ) {
            TerminalStatus = Status_Run;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "行驶");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "跳转到:行驶状态...");
#endif
        }
        else if((Car_InputCheck.Value_DriverDoor == RESET) && (Car_InputCheck.Value_Ignite == RESET)) {  //关门及熄火 
            TerminalStatus = Status_Park;
					  // Debug_Printf(SET, "关门及熄火");
            Park_TimeOver = 20;// 10 * 60;  　//3 * 60

#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "泊车");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "跳转到:泊车状态...");
#endif
        }
        break;
    }

    case Status_Run: {
        Park_TimeOver = 0;
        if( GetSpeed() < SPEED_MIN ) {
            TerminalStatus = Status_ReadyRunOrPark;
#ifdef __DEBUG
            //XFS_WriteBuffer(xfs_auto, "准备");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "跳转到:准备行车或者准备离车状态...");
#endif
        }
        break;
    }

    default:
        break;
    }
}



/*******************************************************************************
* Function Name  : Violation_Detect
* Description    : 实车
*******************************************************************************/
static void ViolationDetect(void)
{
    DriverVerify();
    Si4432_CheckLight();
    FarLightInArea();
    FarLightInMeet();
    DrivePlayPhone();
    SafeBelt();
    FatiguDriving();
    BusRunAtNight();
    VehicleLimitRun();
    LimitSpeed();
    LimitTrumpet();
    IllegalPark();
    Input_Study();
}

/*******************************************************************************
* Function Name :void SystemConfig(void)
* Description   :系统时间戳 初始化
* Input         :
* Output        :
* Other         :
*******************************************************************************/
#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
void  CPU_TS_TmrInit (void)
{
}
#endif

/*******************************************************************************
* Function Name :void SystemConfig(void)
* Description   :读时间戳 计数值
* Input         :读到的计数值
* Output        :
* Other         :
*******************************************************************************/
#if (CPU_CFG_TS_TMR_EN == DEF_ENABLED)
unsigned int CPU_TS_TmrRd (void)
{
    return (SysTick->VAL);
}
#endif


/****************************************END OF FILE**********************************/
