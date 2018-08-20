/******************** (C) COPYRIGHT soarsky ************************************
* File Name          : main.c
* Author             :
* Version            :
* Date               :
* Description        :
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"

/***************���غ�������*************/
static void Detect_VehicleStatus(void);
void SystemUpdate(void);
static void ViolationDetect(void);
/***************ȫ�ֱ�������*************/
Base_Def m_var;
unsigned int Sys_RunTime = 0;       // ��¼���ն�������ʱ��
extern volatile unsigned short Park_TimeOver;        // ������ʱ�л�״̬

TerminalStatus_Typedef TerminalStatus = Status_Park;//����״̬

extern unsigned short Get_UpdateParameter(unsigned char* src, unsigned char* dsr);
extern UpdateInfo_Struct updateInfo;


#ifdef __USE_UCOS__

/////////////////////////////////////////////////////////////////////////////////
// uc/os֧�ֲ��� - ��������ջ����
#define STARTUP_TASK_PRIO 4                     // task priority
#define STARTUP_TASK_STK_SIZE 80                // task stack size

OS_TCB taskStartTCB;
CPU_STK taskStartStk[STARTUP_TASK_STK_SIZE];    // ��������ĳ���ռ�

static void TaskStart(void);


#define MAIN_TASK_PRIO 5                        // task priority
#define MAIN_TASK_STK_SIZE 200                  // task stack size

OS_TCB  taskMainTCB;
CPU_STK taskMainStk[MAIN_TASK_STK_SIZE];        // ��������ĳ���ռ�

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
* Description   :ϵͳ��ʱ��ʱ������
*******************************************************************************/
void SysTickInit(void)
{
    SysTick_Config( SystemCoreClock / 1000 ); 
}

/*******************************************************************************
* Function Name  : main
* Description    : �������
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
* Description   : ��������
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
* Description    : �������
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

    // ���õ�ǰ�汾��
    STMFLASH_Read( 0x08001F00, &updateInfo.flah_new, sizeof(UpdateInfo_Struct)/2 );//��ȡ����

    memset( updateInfo.version, 0, sizeof(updateInfo.version) );
    sprintf( (char*)updateInfo.version, "V%s", __ver_id );
    updateInfo.ver_len = strlen( (char*)updateInfo.version );

#ifdef __USE_USB__
    MyUSB_Init();
#endif

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto,
                    "����,Ӳ��%s,�̼�%s(%04d-%02d-%02d)",
                    __pcb_ver_id,
                    __ver_id,
                    __VER_YEAR,
                    __VER_MONTH,
                    __VER_DAY );
    if( m_var.Flag_Debug_Enable == SET )
        Debug_Printf(SET,
                     "����,Ӳ��%s,�̼�%s(%04d-%02d-%02d),%dMHz\r\n���ӳ��ƺ�:%s,��������:%02d\r\nΥ������:%d,δ�ϴ�Υ����:%d",
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
        XFS_WriteBuffer(xfs_auto, "δд����");
    }
//    SmartcardTest();
#endif

    ApplyLicInit(); // ��֤��ʻԱ�Ķ�ջ��ʼ��

    Sys_RunTime = Sys_TimeCount;

    Study_CarStatus();

    // GetLockCode(); // ��ȡMCU��96Bitȫ��ID��
    m_flagSave = RESET;
    m_delaySave = 0;

    while(1) {

#ifdef __USE_UCOS__
        OSTimeDly(  (OS_TICK    )10,
                    (OS_OPT     )OS_OPT_TIME_DLY,
                    (OS_ERR    *)&err);
#endif

        Si4432_CheckerAnswer();// ���յ��ɼ��ǵĻ����źź�,�����ӳ����Ϸ����ɼ���

        Si4432_RxBufferProcess();//SI4432Э�����

        XFS_Process();//����⵽���ȿ���ʱ,�ر�����,�����������

        eTicket_VoicePrompt();//���ӷ���������ʾ

        MX25L128_SaveBase(RESET);//����Ƿ��б�������

        I2411E_Process();//��ʼ��I2411Eģ�飬����ʼ��һ��

        M35_Process();//��ʼ��M35ģ�飬����ʼ��һ��

        GPS_RxBufferProcess();//GPS���ݽ���

        Read_CarStatus(RESET);//IO��⳵��״̬

        Detect_VehicleStatus();//��⳵��״̬

        ViolationDetect();//���Υ��

        PreventSteal();//������֤

        I2411E_Inspection();//���鹦��

        IWDG_ReloadCounter();//ι��

        SystemUpdate();//����Ƿ��и���

        //Si4432_SendSnToLightMode(); // ���ô��ģ�鳵��  ��ʱ����

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
* Description    : ��⳵��״̬
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
            //XFS_WriteBuffer(xfs_auto, "׼��");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "��ת��:׼���г�����׼���복״̬...");
#endif
        }
				
				else if( FlagTerminalStat != 0x31 && Park_TimeOver == 0){
					
				   if( Car_InputCheck.Value_DriverDoor == RESET ){  //���Ų�ִ��  20171017����  ��� ���� �ٹ��ź����Ƶ�����
					 
							TerminalStatus = Status_Park;
						  Park_TimeOver = 20;// 10 * 60;  ��//3 * 60
						}
					
				}
        break;
			}

    case Status_ReadyRunOrPark: {
        if( GetSpeed() >= SPEED_MIN ) {
            TerminalStatus = Status_Run;
#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "��ʻ");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "��ת��:��ʻ״̬...");
#endif
        }
        else if((Car_InputCheck.Value_DriverDoor == RESET) && (Car_InputCheck.Value_Ignite == RESET)) {  //���ż�Ϩ�� 
            TerminalStatus = Status_Park;
					  // Debug_Printf(SET, "���ż�Ϩ��");
            Park_TimeOver = 20;// 10 * 60;  ��//3 * 60

#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "����");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "��ת��:����״̬...");
#endif
        }
        break;
    }

    case Status_Run: {
        Park_TimeOver = 0;
        if( GetSpeed() < SPEED_MIN ) {
            TerminalStatus = Status_ReadyRunOrPark;
#ifdef __DEBUG
            //XFS_WriteBuffer(xfs_auto, "׼��");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "��ת��:׼���г�����׼���복״̬...");
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
* Description    : ʵ��
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
* Description   :ϵͳʱ��� ��ʼ��
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
* Description   :��ʱ��� ����ֵ
* Input         :�����ļ���ֵ
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
