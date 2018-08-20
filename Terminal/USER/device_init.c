/******************** (C) COPYRIGHT soarsky ********************
* File Name          : device_init.c
* Author             :
* Version            : V0.1
* Date               :
* Description        : 该文件实现STM32的外设初始化
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "device_init.h"


/* Private functions ---------------------------------------------------------*/
void RCC_Config (void);     //时钟配置

void USART2_Config(void);   //串口2配置
void USART3_Config(void);   //串口3配置
void UART4_Config(void);    //串口4配置
void UART5_Config(void);    //串口5配置
void MX25L128_GPIO_Config(void);//Flash存储器配置
void SI4432_GPIO_Config(void);//RF无线配置
void GPIO_Input_Config(void);//GPIO配置
void GPIO_Output_Config(void);//GPIO配置
void NVIC_Config(void);     //中断优先级配置
void TIM2_Config(void);     //定时器2配置
void TIM3_Config(void);     //定时器1配置
void TIM4_Config(void);

unsigned char RTC_Config(void);      //日历配置
void PVD_Config(void);

/*******************************************************************************
* Function Name  : MCU_Initialize
* Description    : stm32各接口配置，初始化，
******************************************************************************/
void MCU_Initialize(void)
{
    RCC_Config();               // 时钟配置

#ifndef HARDWARE_VER2
    USART1_Config();            // 串口1主要用于下载调试
#else
    XFS_Config();
#endif

    USART2_Config();            // 串口2与GSM模块相连

    USART3_Config();            // 串口3与WIFI模块相连

    UART4_Config();             // 串口4相关配置

    UART5_Config();             // 串口5相关配置

    TIM2_Config();              // 定时器2配置为1s中断方式

    TIM3_Config();              // 定时器3配置为1ms中断方式

    TIM4_Config();              // 定时器4配置为10ms中断方式

    MX25L128_GPIO_Config();     // Flash存储芯片引脚配置

    MX25L126_Initialize();      // Flash初始化

    SI4432_GPIO_Config();       // RF无线模块引脚配置

    Si4432_Initialize();        // RF无线模块寄存器初始化

    GPIO_Output_Config();       // 配置输出端口
    GPIO_Input_Config();        // 配置输入端口

    PVD_Config();               // 掉电检测配置

    RTC_Config();               // 日历配置

    XFS_Initialize();           // XFS5152语音模块初始化

//    IWDG_Config();              // 独立看门狗配置
}

/*******************************************************************************
* Function Name  : RCC_Config
* Description    : 时钟初始化:系统时钟64M,SysTick定时器配置为1ms中断
*                :APB2=64M,APB1=32M
******************************************************************************/
void RCC_Config (void)
{
    //SysTick_Config(72000);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP |
                           RCC_APB1Periph_USART2 | RCC_APB1Periph_USART3 |
                           RCC_APB1Periph_UART4 | RCC_APB1Periph_UART5 |
                           RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM3 |
                           RCC_APB1Periph_TIM4 | RCC_APB1Periph_SPI3, ENABLE);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |
                           RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE |
                           RCC_APB2Periph_USART1, ENABLE);

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
}


/*******************************************************************************
* 函数名  : RTC_Config
* 描述    : 实时时钟配置,初始化RTC时钟,同时检测时钟是否工作正常,BKP->DR1用于保存是否第一次配置的设置
* 输入    : 无
* 输出    : 无
* 返回值  : 返回0:正常  其他:错误代码
*******************************************************************************/
unsigned char RTC_Config(void)
{
    //检查是不是第一次配置时钟
    u8 temp = 0;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//使能PWR和BKP外设时钟
    PWR_BackupAccessCmd(ENABLE);	//使能后备寄存器访问
    if (BKP_ReadBackupRegister(BKP_DR1) != 0x5050)		//从指定的后备寄存器中读出数据:读出了与写入的指定数据不相乎
    {
        BKP_DeInit();	//复位备份区域
        RCC_LSICmd(ENABLE);//开启内部低速时钟LSI
        while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET && temp < 250)	//检查指定的RCC标志位设置与否,等待低速晶振就绪
        {
            temp++;
            Delay_ms(10);
        }

//        RCC_LSEConfig(RCC_LSE_ON);	//设置外部低速晶振(LSE),使用外设低速晶振
//        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && temp<250)	//检查指定的RCC标志位设置与否,等待低速晶振就绪
//        {
//            temp++;
//            Delay_ms(10);
//        }

        if(temp >= 250)
        {
            return 1; //初始化时钟失败,晶振有问题
        }
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟
//        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//设置RTC时钟(RTCCLK),选择LSE作为RTC时钟
        RCC_RTCCLKCmd(ENABLE);	//使能RTC时钟
        RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
        RTC_WaitForSynchro();		//等待RTC寄存器同步
        RTC_ITConfig(RTC_IT_SEC, ENABLE);		//使能RTC秒中断
        RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
        RTC_EnterConfigMode();/// 允许配置
        RTC_SetPrescaler(40000); //设置RTC预分频的值
//        RTC_SetPrescaler(32767); //设置RTC预分频的值
        RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
        RTC_SetTime(2017,03,16,11,37,00); //设置时间
        RTC_ExitConfigMode(); //退出配置模式
        BKP_WriteBackupRegister(BKP_DR1, 0x5050);	//向指定的后备寄存器中写入用户程序数据
    }
    else//系统继续计时
    {
//    RTC_WaitForSynchro();	//等待最近一次对RTC寄存器的写操作完成
        RTC_ITConfig(RTC_IT_SEC, ENABLE);	//使能RTC秒中断
        RTC_WaitForLastTask();	//等待最近一次对RTC寄存器的写操作完成
    }
    RTC_GetTime();//更新时间
    return 0; //ok
}


/*******************************************************************************
* Function Name  : PVD_Config
* Description    : 掉电检测初始化,需要与EXTI_Line16相关联
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void PVD_Config(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR, ENABLE);
    PWR_PVDLevelConfig(PWR_PVDLevel_2V9);
    EXTI_ClearITPendingBit(EXTI_Line16);
    EXTI_InitStructure.EXTI_Line = EXTI_Line16;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;//掉电检测
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    PWR_PVDCmd(ENABLE);
}


#ifdef SPI3_I2411E
/*******************************************************************************
* Function Name  : SPI3_I2411E_Config
* Description    : SPI3配置
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
static void SPI3_I2411E_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    SPI_InitTypeDef   SPI_InitStructure;

    /*片选引脚*/
    GPIO_InitStructure.GPIO_Pin = I2411E_CS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(I2411E_CS_PORT, &GPIO_InitStructure);

    /*使能引脚*/
    GPIO_InitStructure.GPIO_Pin = I2411E_EN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(I2411E_EN_PORT, &GPIO_InitStructure);

    /* 初始化SCK、MISO、MOSI引脚 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5);

    /* 初始化配置STM32 SPI3 */
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	//SPI设置为双线双向全双工
    SPI_InitStructure.SPI_Mode=SPI_Mode_Master;							//设置为主SPI
    SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;						//SPI发送接收8位帧结构
    SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;							//时钟悬空低
    SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;							//数据捕获于第1个时钟沿
    SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;								//NSS由外部管脚管理
    SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;	//波特率预分频值为2
    SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;					//数据传输从MSB位开始
    SPI_InitStructure.SPI_CRCPolynomial=7;								//CRC多项式为7
    SPI_Init(SPI3,&SPI_InitStructure);									//根据SPI_InitStruct中指定的参数初始化外设SPI2寄存器
    SPI_Cmd(SPI3, ENABLE);	//STM32使能SPI3
}
#endif



/*******************************************************************************
* Function Name  : GPIO_Output_Config
* Description    :
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void GPIO_Output_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /*右转灯*/
    GPIO_InitStructure.GPIO_Pin = LED_R;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_R_PORT, &GPIO_InitStructure);

    /*左转灯*/
    GPIO_InitStructure.GPIO_Pin = LED_L;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_L_PORT, &GPIO_InitStructure);

    /*备用输出*/
    GPIO_InitStructure.GPIO_Pin = O3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(O3_PORT,&GPIO_InitStructure);

    /*指示灯*/
    GPIO_InitStructure.GPIO_Pin = LIGHT;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LIGHT_PORT, &GPIO_InitStructure);


//  GPIO_SetBits(LED_R_PORT,LED_R);
//  GPIO_SetBits(LED_L_PORT,LED_L);
//  GPIO_SetBits(O3_PORT,O3);
//  GPIO_SetBits(LIGHT_PORT,LIGHT);       // 灭
//
//  GPIO_ResetBits(LED_R_PORT,LED_R);
//  GPIO_ResetBits(LED_L_PORT,LED_L);
//  GPIO_ResetBits(O3_PORT,O3);
    GPIO_ResetBits(LIGHT_PORT,LIGHT);     // 亮
}


/*******************************************************************************
* Function Name  : GPIO_Input_Config
* Description    :
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void GPIO_Input_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /*用于检测驾驶员一侧的门------------------*/
    GPIO_InitStructure.GPIO_Pin = DRIVER_DOOR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DRIVER_DOOR_PORT, &GPIO_InitStructure);

    /*用于检测副驾驶员一侧的门------------------*/
    GPIO_InitStructure.GPIO_Pin = SECOND_DOOR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(SECOND_DOOR_PORT, &GPIO_InitStructure);

    /*点火检测引脚----------------------------*/
    GPIO_InitStructure.GPIO_Pin = IGNITE;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(IGNITE_PORT, &GPIO_InitStructure);

    /*远光灯检测引脚----------------------------*/
    GPIO_InitStructure.GPIO_Pin = HIGH_BEAM;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(HIGH_BEAM_PORT, &GPIO_InitStructure);

    /*近光灯检测引脚----------------------------*/
    GPIO_InitStructure.GPIO_Pin = LOW_BEAM;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(LOW_BEAM_PORT, &GPIO_InitStructure);

    /*安全带检测引脚---------------------------*/
    GPIO_InitStructure.GPIO_Pin = DRIVER_BELT;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DRIVER_BELT_PORT, &GPIO_InitStructure);

    /*副安全带检测引脚1--------------------------*/
    GPIO_InitStructure.GPIO_Pin = DEPUTY_BELT;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DEPUTY_BELT_PORT, &GPIO_InitStructure);

    /*副安全带检测引脚2--------------------------*/
    GPIO_InitStructure.GPIO_Pin = DEPUTY_BELT2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DEPUTY_BELT2_PORT, &GPIO_InitStructure);

    /*喇叭检测引脚------------------------------*/
    GPIO_InitStructure.GPIO_Pin = HONK;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(HONK_PORT, &GPIO_InitStructure);

    /*IC10检测引脚------------------------------*/
    GPIO_InitStructure.GPIO_Pin = IC1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(IC1_PORT, &GPIO_InitStructure);

    /*KEY检测引脚------------------------------*/
    GPIO_InitStructure.GPIO_Pin = KEY;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_PORT, &GPIO_InitStructure);

    /*自擦除引脚检测*/
    GPIO_InitStructure.GPIO_Pin = C_KEY;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(C_KEY_PORT, &GPIO_InitStructure);
}


/*******************************************************************************
* Function Name  : MX25L128_GPIO_Config
* Description    : 用于初始化MX25L128存储芯片的引脚，实用模拟SPI方式操作该芯片
                   该芯片的引脚与SW下载口服用，调试该芯片是需要拔掉SW下载线。
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void MX25L128_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /*MX25L128  数据存储芯片的数据输入引脚*/
    GPIO_InitStructure.GPIO_Pin = MX25_DO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MX25_DO_PORT, &GPIO_InitStructure);

    /*MX25L128  数据存储芯片的写保护引脚*/
    GPIO_InitStructure.GPIO_Pin = MX25_WP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MX25_WP_PORT, &GPIO_InitStructure);

    /*MX25L128  数据存储芯片的数据输入引脚*/
    GPIO_InitStructure.GPIO_Pin = MX25_DI;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MX25_DI_PORT, &GPIO_InitStructure);

    /*MX25L128  数据存储芯片的时钟输入引脚*/
    GPIO_InitStructure.GPIO_Pin = MX25_CLK;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MX25_CLK_PORT, &GPIO_InitStructure);

    /*MX25L128  数据存储芯片的片选引脚*/
    GPIO_InitStructure.GPIO_Pin = MX25_CS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MX25_CS_PORT, &GPIO_InitStructure);
    GPIO_WriteBit(MX25_CS_PORT,MX25_CS,Bit_SET);//该引脚为片选脚，默认拉高使能，等使用的时候再拉低
}


/*******************************************************************************
* Function Name  : SI4432_GPIO_Config
* Description    : 用于初始化RF模块的引脚
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void SI4432_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /*si4432  RF无线模块的SDN引脚*/
    GPIO_InitStructure.GPIO_Pin = Si4432_SDN;		//(主备共用)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Si4432_SDN_PORT, &GPIO_InitStructure);
    /*si4432  RF无线模块的IRQ引脚*/
    GPIO_InitStructure.GPIO_Pin = Si4432_IRQ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Si4432_IRQ_PORT, &GPIO_InitStructure);
    /*si4432  RF无线模块的SEL引脚*/
    GPIO_InitStructure.GPIO_Pin = Si4432_SEL;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Si4432_SEL_PORT, &GPIO_InitStructure);
    /*si4432  RF无线模块的SCK引脚*/
    GPIO_InitStructure.GPIO_Pin = Si4432_SCK;		//(主备共用)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Si4432_SCK_PORT, &GPIO_InitStructure);
    /*si4432  RF无线模块的SDI引脚*/
    GPIO_InitStructure.GPIO_Pin = Si4432_SDI;		//(主备共用)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Si4432_SDI_PORT, &GPIO_InitStructure);
    /*si4432  RF无线模块的SDO引脚*/
    GPIO_InitStructure.GPIO_Pin = Si4432_SDO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Si4432_SDO_PORT, &GPIO_InitStructure);
}

#ifdef HARDWARE_VER2
/*******************************************************************************
* Function Name  : XFS_Config
* Description    :
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void XFS_Config(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = XFS_BAUD;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &USART_InitStructure);
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);
}
#endif



/*******************************************************************************
* Function Name  : USART1_Config
* Description    :
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void USART1_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // 打开GPIO和USART部件的时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    /* USART1 GPIO config */
    /* Configure USART1 Tx (PA.09) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART1 Rx (PA.10) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART1 mode config */
    USART_InitStructure.USART_BaudRate = DEBUG_BAUD;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(DEBUG_UART, &USART_InitStructure);

    USART_ITConfig(DEBUG_UART, USART_IT_RXNE, ENABLE);
    USART_ITConfig(DEBUG_UART, USART_IT_IDLE, ENABLE);

    USART_Cmd(DEBUG_UART, ENABLE);
}


/*******************************************************************************
* Function Name  : USART2_Config
* Description    : 串口2配置
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void USART2_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // 打开GPIO和USART部件的时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);

    /* XFS5152 RST GPIO config */
    GPIO_InitStructure.GPIO_Pin = XFS5152_RST;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(XFS5152_RST_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = AMP_OPEN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(AMP_OPEN_PORT, &GPIO_InitStructure);

    /* USART2 GPIO config */
    /* Configure USART2 Tx (PA.2) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /* Configure USART2 Rx (PA.3) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* USART2 mode config */
#ifdef HARDWARE_VER2
    USART_InitStructure.USART_BaudRate = GPS_BAUD;
#else
    USART_InitStructure.USART_BaudRate = XFS_BAUD;
#endif
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    USART_Cmd(USART2, ENABLE);
}

/*******************************************************************************
* Function Name  : USART3_Config
* Description    : 串口3配置
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void USART3_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* config USART3 clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    /* USART3 GPIO config */
    /* Configure USART3 Tx (PB.10) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Configure USART3 Rx (PB.11) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* USART3 mode config */
    USART_InitStructure.USART_BaudRate = I2411E_BAUD;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART3, &USART_InitStructure);
    USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);
    USART_Cmd(USART3, ENABLE);
}



/*******************************************************************************
* Function Name  : UART4_Config
* Description    : 串口4配置
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void UART4_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // 打开GPIO和USART部件的时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

    /*M35 电源使能引脚*/
    GPIO_InitStructure.GPIO_Pin = M35_EN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(M35_EN_PORT, &GPIO_InitStructure);

    //GSM芯片硬件复位引脚
    GPIO_InitStructure.GPIO_Pin = M35_PWR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(M35_PWR_PORT, &GPIO_InitStructure);

    //备用引脚
    GPIO_InitStructure.GPIO_Pin = M35_RTS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(M35_RTS_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(M35_RTS_PORT,M35_RTS);//默认接到GND

    //GSM状态读取引脚
    GPIO_InitStructure.GPIO_Pin = M35_STATUS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(M35_STATUS_PORT, &GPIO_InitStructure);

    //备用引脚
    GPIO_InitStructure.GPIO_Pin = M35_DTR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(M35_DTR_PORT,&GPIO_InitStructure);

    /* USART4 GPIO config */
    /* Configure USART4 Tx (PC.10) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Configure USART4 Rx (PC.11) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* USART4 mode config */
#ifdef HARDWARE_VER2
    USART_InitStructure.USART_BaudRate = I2411E_BAUD;
#else
    USART_InitStructure.USART_BaudRate = M35_BAUD;
#endif
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART4, &USART_InitStructure);

    USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
    USART_ITConfig(UART4, USART_IT_IDLE, ENABLE);

    USART_Cmd(UART4, ENABLE);
}

/*******************************************************************************
* Function Name  : UART5_Config
* Description    : 串口5配置
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void UART5_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // 打开GPIO和USART部件的时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);

    /*GPS 电源使能引脚*/
    GPIO_InitStructure.GPIO_Pin = GPS_ON;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPS_ON_PORT, &GPIO_InitStructure);
    GPIO_SetBits(GPS_ON_PORT,GPS_ON);

    /* USART5 GPIO config */
    /* Configure USART5 Tx (PC.12) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* Configure USART5 Rx (PD.2) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* USART5 mode config */
#ifdef HARDWARE_VER2
    USART_InitStructure.USART_BaudRate = M35_BAUD;
#else
    USART_InitStructure.USART_BaudRate = GPS_BAUD;
#endif
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(UART5, &USART_InitStructure);

    USART_ITConfig(UART5, USART_IT_RXNE, ENABLE);
    USART_ITConfig(UART5, USART_IT_IDLE, ENABLE);

    USART_Cmd(UART5, ENABLE);
}


/*******************************************************************************
* Function Name  : NVIC_Configuration
* Description    : 中断优先级配置
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );//优先级设置  全为抢占式优先级
    // NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    /* Enable the USARTy Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* Enable the USARTy Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* Enable the USARTy Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* Enable the USARTy Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* Enable the DMA1_Channel6 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = UART5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* Enable the RTC Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 6;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* Enable the TIM2 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* Enable the TIM3 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 8;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* Enable the TIM4 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 9;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    /* Enable the PVD Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 10;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


/*******************************************************************************
* Function Name  : TIM2_Config
* Description    : 定时器2配置为1s产生中断
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void TIM2_Config(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);    // PCLK1经过2倍频后作为TIM2的时钟源等于64MHz

    TIM_DeInit(TIM2);
    TIM_TimeBaseStructure.TIM_Period = 10000;               // 自动重装载寄存器周期的值(计数值)：1sec = 1000 * 10 * 100us

    // 累计 TIM_Period个频率后产生一个更新或者中断
    TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 10000) - 1; // 时钟预分频数: 100us计时
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // 采样分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);                   // 清除溢出中断标志
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);                                  // 开启时钟
}


/*******************************************************************************
* Function Name  : TIM3_Config
* Description    : 定时器3配置为1ms产生中断
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void TIM3_Config(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_DeInit(TIM3);
    TIM_TimeBaseStructure.TIM_Period = 1000;                // 自动重装载寄存器周期的值(计数值)：1ms = 1000 * 1us

    // 累计 TIM_Period个频率后产生一个更新或者中断
    TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1; // 时钟预分频数: 1us计时
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // 采样分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ClearFlag(TIM3, TIM_FLAG_Update);                   // 清除溢出中断标志
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE);                                  // 开启时钟
}


/*******************************************************************************
* Function Name  : TIM4_Config
* Description    : 通用定时器中断初始化  10ms中断
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void TIM4_Config(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);    // 时钟使能//TIM4时钟使能

    //定时器TIM3初始化
    TIM_TimeBaseStructure.TIM_Period        = 1000;         // 自动重装载寄存器周期的值(计数值)：10ms = 1000 * 10us
    TIM_TimeBaseStructure.TIM_Prescaler     = (SystemCoreClock / 100000) - 1; // 时钟预分频数: 10us计时
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // 设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;   // TIM向上计数模式
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);         // 根据指定的参数初始化TIMx的时间基数单位

    TIM_ClearFlag(TIM4, TIM_FLAG_Update);                   // 清除溢出中断标志
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE );             // 使能指定的TIM4中断,允许更新中断
    TIM_Cmd(TIM4, ENABLE);                                  // 开启时钟
}

/*******************************************************************************
* Function Name  : IWDG_Config
* Description    : 独立看门狗初始化
                   喂狗时间 = ((4 * 2^分频值) * 装载值) / 时钟 = (4*2^4)*625/40khz = 1000ms
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void IWDG_Config(void)
{
    /*独立看门狗由内部专用40khz时钟驱动。*/
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);/*使能对寄存器IWDG_PR和IWDG_RLR寄存器的写操作*/
    IWDG_SetPrescaler(IWDG_Prescaler_64);//设置IWDG预分频值：取值范围0~7，低3位有效
    IWDG_SetReload(1250);//设置IWDG重装载值：取值范围0~2047，低11位有效
    IWDG_ReloadCounter();//按照IWDG重载寄存器的值重装载IWDG计数器
    IWDG_Enable();//使能IWDG  Time = (64/(40*10^3))*1250 =2s
}




/******************* (C) COPYRIGHT 2012 WildFire Team *****END OF FILE************/

