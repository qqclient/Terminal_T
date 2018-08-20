/*******************************************************************************
* File Name          : hw_init.c
* Author             : csyxj@728920175@qq.com
* Version            : V0.1
* Date               : 2017-03-19
* Description        : MCU初始化
*******************************************************************************/

#include "gtw_Head.h"

#define IRQ_RTC_INT             0
#define IRQ_TIM2_INT            1
#define IRQ_TIM3_INT            2
#define IRQ_TIM4_INT            3

#define IRQ_DMA_SPI1_SEND       4
#define IRQ_DMA_SPI1_RECV       5

#define IRQ_XFS_UART_INT        7

#define IRQ_GPS_UART_INT        8
#define IRQ_DMA_GPS_UART        9

#define IRQ_RJM_UART_INT        10

#define IRQ_M35_UART_INT        11

#define IRQ_I2411E_UART_INT     12

#define IRQ_USB_INT             13
#define IRQ_DEBUG_UART_INT      14

#define IRQ_PVD_INT             15


ArrQueue* m_FIFO_Debug   = NULL;
ArrQueue* m_FIFO_XFS     = NULL;
ArrQueue* m_FIFO_I2411E  = NULL;
ArrQueue* m_FIFO_M35     = NULL;


#ifdef RJMU101_UART
ArrQueue* m_FIFO_RJMU101 = NULL;
#endif


#define SPI1_RxBuf_Size     64
#define SPI1_TxBuf_Size     64
unsigned char SPI1_RxBuf[ SPI1_RxBuf_Size ] = { 0 };
unsigned char SPI1_TxBuf[ SPI1_TxBuf_Size ] = { 0 };

const char __ver_id[] = "1.0.2.122";

#ifdef __PCB_V1__
const char __pcb_ver_id[] = "1.0";
#endif

#ifdef __PCB_V2__
const char __pcb_ver_id[] = "2.0";
#endif

#ifdef __PCB_V3__
const char __pcb_ver_id[] = "3.0";
#endif

#ifdef __PCB_V31__
const char __pcb_ver_id[] = "3.1";
#endif

#ifdef __PCB_V32__
const char __pcb_ver_id[] = "3.2";
#endif


/* Private functions ---------------------------------------------------------*/
void RCC_Config(void);

void DEBUG_Config(void);
void XFS_Config(void);
void GPS_Config(void);
void GPS_DMA_Config(void);
void RJMU101_Config(void);
void I2411E_Config(void);
void SPI3_I2411E_Config(void);
void M35_Config(void);

void GPIO_Output_Config(void);
void GPIO_Input_Config(void);
void MX25L128_GPIO_Config(void);
void SI4432_GPIO_Config(void);

void TIM2_Config(void);
void TIM3_Config(void);
void TIM4_Config(void);

void IWDG_Config(void);
void PVD_Config(void);
unsigned char RTC_Config(void);

/* Public functions ---------------------------------------------------------*/
void MCU_Initialize(void);
void NVIC_Configuration(void);

#ifdef __USE_USB__
void USB_Interrupts_Config(void);
void USB_GPIO_Config(void);
void USB_Cable_Config (FunctionalState NewState);
#endif


/*******************************************************************************
* Function Name  : crc16
* Description    : 标准CRC16校验码
******************************************************************************/
unsigned short get_crc16(unsigned char* src, unsigned long sizes)
{
    unsigned long n;
    const char *ptr;

    unsigned short crc;
    unsigned short tmp;
    unsigned short sc;

    unsigned short i;
    unsigned short j;
    unsigned short ccitt;
    unsigned short c;

    unsigned short  tab_ccitt[256];

    for (i = 0; i < 256; i++) {

        ccitt = 0;
        c  = i << 8;

        for (j = 0; j < 8; j++) {

            if ( (ccitt ^ c) & 0x8000 ) {
                ccitt = ( ccitt << 1 ) ^ 0x1021;
            }
            else {
                ccitt =   ccitt << 1;
            }

            c = c << 1;
        }

        tab_ccitt[i] = ccitt;
    }

    ptr = (const char *)src;
    crc = 0xffff;

    if( ptr != NULL ) {
        for (n = 0; n < sizes; n++) {
            sc      = 0x00ff & (unsigned short) *ptr;
            tmp     = (crc >> 8) ^ sc;
            crc     = (crc << 8) ^ tab_ccitt[tmp];

            ptr++;
        }
    }

    return crc;
}


/*******************************************************************************
* Function Name  : MCU_Initialize
* Description    : stm32各接口配置，初始化，
******************************************************************************/
void MCU_Initialize(void)
{
    RCC_Config();               // 时钟配置

    // UART端口初始化
    DEBUG_Config();
#ifdef DEBUG_UART
    m_FIFO_Debug = QueueCreate( 200 );  // Debug缓冲队列
#endif

    XFS_Config();
#ifdef XFS_UART
    m_FIFO_XFS = QueueCreate( 200 );
#endif

    GPS_Config();
#ifdef __GPS_DMA__
    GPS_DMA_Config();
#endif

    RJMU101_Config();
#ifdef RJMU101_UART
    m_FIFO_RJMU101 = QueueCreate( 200 );
#endif

    I2411E_Config();
    m_FIFO_I2411E = QueueCreate( 200 );

    M35_Config();
    m_FIFO_M35 = QueueCreate( 200 );

    // 定时器初始化
    TIM2_Config();
    TIM3_Config();
#ifndef __GPS_DMA__
    TIM4_Config();
#endif

    MX25L128_GPIO_Config();
    MX25L128_Initialize();

    STMFLASH_Read(UPDATE_INFO_ADDR, &updateInfo.flah_new, sizeof(UpdateInfo_Struct)/2);

    SI4432_GPIO_Config();
    Si4432_Initialize();
    Si4432_SendTimer(m_carinfo.Car_5SN);

    GPIO_Output_Config();
    GPIO_Input_Config();

    PVD_Config();

    RTC_Config();

#ifdef XFS_UART
    XFS_Initialize();
#endif

#ifdef __USE_MPU6050__
    MPU_Init();
#endif
}

/*******************************************************************************
* Function Name  : GPIO_Config
* Description    : Configures GPIO
*******************************************************************************/
void USB_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DISCONNECT, ENABLE);

    GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(USB_DISCONNECT_PORT, &GPIO_InitStructure);
}

//****************************************************************************
// Function Name  : USB_Cable_Config.
// Description    : Software Connection/Disconnection of USB Cable.
//****************************************************************************
void USB_Cable_Config(FunctionalState NewState)
{
    if (NewState != DISABLE) {
        GPIO_ResetBits(USB_DISCONNECT_PORT, USB_DISCONNECT_PIN);
    }
    else {
        GPIO_SetBits(USB_DISCONNECT_PORT, USB_DISCONNECT_PIN);
    }
}

//*****************************************************************************
// Function Name  : USB_Interrupts_Config.
// Description    : Configures the USB interrupts.
//*****************************************************************************
void USB_Interrupts_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_USB_INT;//15;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : RCC_Config
* Description    : 时钟初始化:系统时钟64M,SysTick定时器配置为1ms中断
*                :APB2=64M,APB1=32M
******************************************************************************/
void RCC_Config(void)
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
* Function Name  : NVIC_Configuration
* Description    : 中断优先级配置
******************************************************************************/
void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    // Configure the NVIC Preemption Priority Bits
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

    // Enable the RTC Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_RTC_INT;//6
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable the TIM3 Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_TIM3_INT;//7
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable the TIM2 Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_TIM2_INT;//8
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

#ifdef XFS_UART
    // Enable the USARTy Interrupt: XFS
    NVIC_InitStructure.NVIC_IRQChannel = XFS_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_XFS_UART_INT;//10
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

#ifndef __GPS_DMA__
    // Enable the TIM4 Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_TIM4_INT;//9
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable the USARTy Interrupt: GPS
    NVIC_InitStructure.NVIC_IRQChannel = GPS_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_GPS_UART_INT;//11
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#else
    // DMA1 Channel Interrupt ENABLE
    NVIC_InitStructure.NVIC_IRQChannel = GPS_DMA_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_DMA_GPS_UART;//11
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

    // Enable the USARTy Interrupt: GSM/GPRS
    NVIC_InitStructure.NVIC_IRQChannel = M35_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_M35_UART_INT;//13;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // Enable the USARTy Interrupt:Wifi/Bt
    NVIC_InitStructure.NVIC_IRQChannel = I2411E_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_I2411E_UART_INT;//14
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

#ifdef DEBUG_UART
    // Enable the USARTy Interrupt: DEBUG
    NVIC_InitStructure.NVIC_IRQChannel = DEBUG_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_DEBUG_UART_INT;//14
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif

    // Enable the PVD Interrupt
    NVIC_InitStructure.NVIC_IRQChannel = PVD_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_PVD_INT;//15;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/*******************************************************************************
* Function Name  : DEBUG_Config
* Description    : 调试信息接口配置
******************************************************************************/
void DEBUG_Config(void)
{
#ifdef DEBUG_UART
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

#ifdef __DEBUG_RCC_APB1Periph
    RCC_APB1PeriphClockCmd(DEBUG_RCC_APB1Periph, ENABLE);
#endif

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | DEBUG_RCC_APB2Periph, ENABLE);

    // 端口映射
#ifdef __DEBUG_UART_REMAP
    GPIO_PinRemapConfig(DEBUG_UART_REMAP, ENABLE);
#endif

    // Configure DEBUG_UART Tx as alternate function push-pull
    GPIO_InitStructure.GPIO_Pin =  DEBUG_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(DEBUG_TX_PORT, &GPIO_InitStructure);

    // Configure DEBUG_UART Rx as input floating
    GPIO_InitStructure.GPIO_Pin =  DEBUG_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(DEBUG_RX_PORT, &GPIO_InitStructure);

    // DEBUG_UART mode config
    USART_InitStructure.USART_BaudRate = DEBUG_BAUD;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(DEBUG_UART, &USART_InitStructure);

    USART_ITConfig(DEBUG_UART, USART_IT_RXNE, ENABLE);
    USART_ITConfig(DEBUG_UART, USART_IT_IDLE, ENABLE);
    USART_Cmd(DEBUG_UART, ENABLE);

    USART_ClearFlag(DEBUG_UART, USART_FLAG_TC);

#endif
}

/*******************************************************************************
* Function Name  : XFS_Config
* Description    : 语音模块接口配置
******************************************************************************/
void XFS_Config(void)
{
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | XFS_RCC_APB2Periph, ENABLE);

    // XFS5152 RST GPIO config
    GPIO_InitStructure.GPIO_Pin = XFS5152_RST;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(XFS5152_RST_PORT, &GPIO_InitStructure);

    // XFS5152 EN GPIO config
#ifdef XFS5152_EN
    GPIO_InitStructure.GPIO_Pin = XFS5152_EN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(XFS5152_EN_PORT, &GPIO_InitStructure);
    XFS5152_PWUP();
#endif

    // TDA7266 AMP GPIO config
    GPIO_InitStructure.GPIO_Pin = AMP_OPEN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(AMP_OPEN_PORT, &GPIO_InitStructure);
    OFF_AMP();

#ifdef XFS_UART

#ifdef XFS_RCC_APB1Periph
    RCC_APB1PeriphClockCmd(XFS_RCC_APB1Periph, ENABLE);
#endif

// 端口映射
#ifdef XFS_UART_REMAP
    GPIO_PinRemapConfig(XFS_UART_REMAP, ENABLE);
#endif

    // Configure XFS_UART Tx as alternate function push-pull
    GPIO_InitStructure.GPIO_Pin =  XFS_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(XFS_TX_PORT, &GPIO_InitStructure);

    // Configure XFS_UART Rx as input floating
    GPIO_InitStructure.GPIO_Pin =  XFS_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(XFS_RX_PORT, &GPIO_InitStructure);

    // XFS_UART mode config
    USART_InitStructure.USART_BaudRate = XFS_BAUD;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(XFS_UART, &USART_InitStructure);
    USART_ITConfig(XFS_UART, USART_IT_RXNE, ENABLE);
    USART_Cmd(XFS_UART, ENABLE);

#endif
}

/*******************************************************************************
* Function Name  : GPS_Config
* Description    : GPS模块接口配置
******************************************************************************/
void GPS_Config(void)
{
#ifdef GPS_UART
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

#ifdef GPS_RCC_APB1Periph
    RCC_APB1PeriphClockCmd(GPS_RCC_APB1Periph, ENABLE);
#endif

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | GPS_RCC_APB2Periph, ENABLE);

    // 端口映射
#ifdef GPS_UART_REMAP
    GPIO_PinRemapConfig(GPS_UART_REMAP, ENABLE);
#endif

    // GPS 电源使能引脚
    GPIO_InitStructure.GPIO_Pin = GPS_ON;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPS_ON_PORT, &GPIO_InitStructure);
    GPIO_SetBits(GPS_ON_PORT,GPS_ON);

    // Configure GPS_UART Tx as alternate function push-pull
    GPIO_InitStructure.GPIO_Pin =  GPS_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPS_TX_PORT, &GPIO_InitStructure);

    // Configure GPS_UART Rx as input floating
    GPIO_InitStructure.GPIO_Pin =  GPS_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPS_RX_PORT, &GPIO_InitStructure);

    // GPS_UART mode config
    USART_InitStructure.USART_BaudRate = GPS_BAUD;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(GPS_UART, &USART_InitStructure);
#ifndef __GPS_DMA__
    USART_ITConfig(GPS_UART, USART_IT_RXNE, ENABLE);
#endif
    USART_Cmd(GPS_UART, ENABLE);

    USART_ClearFlag(GPS_UART, USART_FLAG_TC);
#endif
}

/*******************************************************************************
* Function Name  : GPS_Config
* Description    : GPS模块接口配置
******************************************************************************/
#ifdef __GPS_DMA__
void GPS_DMA_Config(void)
{
    DMA_InitTypeDef DMA_InitStructure;

    RCC_AHBPeriphClockCmd(GPS_DMA_CLK, ENABLE);
    DMA_InitStructure.DMA_PeripheralBaseAddr = GPS_DATA_ADDR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)GPS_RxBuff;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = GPS_RBUFF_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(GPS_DMA_CHANNEL, &DMA_InitStructure);

    DMA_ITConfig(GPS_DMA_CHANNEL,DMA_IT_HT|DMA_IT_TC,ENABLE);  //配置DMA发送完成后产生中断
    DMA_Cmd (GPS_DMA_CHANNEL,ENABLE);

    USART_DMACmd(GPS_UART, USART_DMAReq_Rx, ENABLE);
}
#endif

/*******************************************************************************
* Function Name  : RJMU101_Config
* Description    : 加解密模块接口配置
******************************************************************************/
void RJMU101_Config(void)
{
#ifdef RJMU101_UART
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

#ifdef RJMU101_RCC_APB1Periph
    RCC_APB1PeriphClockCmd(RJMU101_RCC_APB1Periph, ENABLE);
#endif

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RJMU101_RCC_APB2Periph, ENABLE);

#ifdef RJMU101_UART_REMAP
    GPIO_PinRemapConfig(RJMU101_UART_REMAP, ENABLE);
#endif

    // Configure RJMU101_UART Tx as alternate function push-pull
    GPIO_InitStructure.GPIO_Pin =  RJMU101_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(RJMU101_TX_PORT, &GPIO_InitStructure);

    // Configure RJMU101_UART Rx as input floating
    GPIO_InitStructure.GPIO_Pin =  RJMU101_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(RJMU101_RX_PORT, &GPIO_InitStructure);

    // RJMU101_UART mode config
    USART_InitStructure.USART_BaudRate = RJMU101_BAUD;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(RJMU101_UART, &USART_InitStructure);

    USART_ITConfig(RJMU101_UART, USART_IT_RXNE, ENABLE);
    USART_Cmd(RJMU101_UART, ENABLE);

    USART_ClearFlag(RJMU101_UART, USART_FLAG_TC);
#endif
}

/*******************************************************************************
* Function Name  : I2411E_Config
* Description    : Wifi/Bluetooth模块接口配置
******************************************************************************/
void I2411E_Config(void)
{
#ifdef I2411E_UART
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

#ifdef I2411E_RCC_APB1Periph
    RCC_APB1PeriphClockCmd(I2411E_RCC_APB1Periph, ENABLE);
#endif

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | I2411E_RCC_APB2Periph, ENABLE);

#ifdef I2411E_UART_REMAP
    GPIO_PinRemapConfig(I2411E_UART_REMAP, ENABLE);
#endif

    // Configure I2411E_UART Tx as alternate function push-pull
    GPIO_InitStructure.GPIO_Pin =  I2411E_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(I2411E_TX_PORT, &GPIO_InitStructure);

    // Configure I2411E_UART Rx as input floating
    GPIO_InitStructure.GPIO_Pin =  I2411E_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(I2411E_RX_PORT, &GPIO_InitStructure);

    // I2411E_UART mode config
    USART_InitStructure.USART_BaudRate = I2411E_BAUD;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(I2411E_UART, &USART_InitStructure);

    USART_ITConfig(I2411E_UART, USART_IT_RXNE, ENABLE);
    USART_Cmd(I2411E_UART, ENABLE);

    USART_ClearFlag(I2411E_UART, USART_FLAG_TC);
#endif
}

/*******************************************************************************
* Function Name  : SPI3_I2411E_Config
* Description    : SPI3配置
******************************************************************************/
void SPI3_I2411E_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    SPI_InitTypeDef   SPI_InitStructure;

    // 片选引脚
    GPIO_InitStructure.GPIO_Pin = I2411E_CS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(I2411E_CS_PORT, &GPIO_InitStructure);

    // 使能引脚
#ifdef I2411E_EN
    GPIO_InitStructure.GPIO_Pin = I2411E_EN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(I2411E_EN_PORT, &GPIO_InitStructure);
#endif

    // 初始化 SCK,MISO,MOSI 引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5);

    // 初始化配置STM32 SPI3
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode=SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;
    SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial=7;
    SPI_Init(SPI3,&SPI_InitStructure);
    SPI_Cmd(SPI3, ENABLE);	// 使能SPI3
}


/*******************************************************************************
* Function Name  : M35_Config
* Description    : GPRS/GSM模块接口配置
******************************************************************************/
void M35_Config(void)
{
#ifdef M35_UART
    USART_InitTypeDef USART_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

#ifdef M35_RCC_APB1Periph
    RCC_APB1PeriphClockCmd(M35_RCC_APB1Periph, ENABLE);
#endif

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | M35_RCC_APB2Periph, ENABLE);

    // 端口映射
#ifdef M35_UART_REMAP
    GPIO_PinRemapConfig(M35_UART_REMAP, ENABLE);
#endif

    // M35 电源使能引脚
    GPIO_InitStructure.GPIO_Pin = M35_EN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(M35_EN_PORT, &GPIO_InitStructure);

    // GSM芯片硬件复位引脚
    GPIO_InitStructure.GPIO_Pin = M35_PWR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(M35_PWR_PORT, &GPIO_InitStructure);

    // 备用引脚
    GPIO_InitStructure.GPIO_Pin = M35_RTS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(M35_RTS_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(M35_RTS_PORT,M35_RTS);//默认接到GND

    // GSM状态读取引脚
    GPIO_InitStructure.GPIO_Pin = M35_STATUS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(M35_STATUS_PORT, &GPIO_InitStructure);

    // 备用引脚
    GPIO_InitStructure.GPIO_Pin = M35_DTR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(M35_DTR_PORT,&GPIO_InitStructure);

    // SIM卡拔插检测口
#ifdef M35_SIMIN
    GPIO_InitStructure.GPIO_Pin =  M35_SIMIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(M35_SIMIN_PORT, &GPIO_InitStructure);
#endif

    // Configure M35_UART Tx as alternate function push-pull
    GPIO_InitStructure.GPIO_Pin =  M35_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(M35_TX_PORT, &GPIO_InitStructure);

    // Configure M35_UART Rx as input floating
    GPIO_InitStructure.GPIO_Pin =  M35_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(M35_RX_PORT, &GPIO_InitStructure);

    // M35_UART mode config
    USART_InitStructure.USART_BaudRate = M35_BAUD;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(M35_UART, &USART_InitStructure);

    USART_ITConfig(M35_UART, USART_IT_RXNE, ENABLE);
    USART_ITConfig(M35_UART, USART_IT_IDLE, ENABLE);
    USART_Cmd(M35_UART, ENABLE);

    USART_ClearFlag(M35_UART, USART_FLAG_TC);
#endif
}


/*******************************************************************************
* Function Name  : GPIO_Output_Config
* Description    :
*******************************************************************************/
void GPIO_Output_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 右转灯
    GPIO_InitStructure.GPIO_Pin = LED_R;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_R_PORT, &GPIO_InitStructure);

    // 左转灯
    GPIO_InitStructure.GPIO_Pin = LED_L;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(LED_L_PORT, &GPIO_InitStructure);

    // 备用输出
    GPIO_InitStructure.GPIO_Pin = O3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(O3_PORT,&GPIO_InitStructure);

    // 指示灯
    GPIO_InitStructure.GPIO_Pin = LIGHT;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(LIGHT_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(LIGHT_PORT,LIGHT);
}


/*******************************************************************************
* Function Name  : GPIO_Input_Config
* Description    :
*******************************************************************************/
void GPIO_Input_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 用于检测驾驶员一侧的门------------------
    GPIO_InitStructure.GPIO_Pin = DRIVER_DOOR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DRIVER_DOOR_PORT, &GPIO_InitStructure);

    // 用于检测副驾驶员一侧的门------------------
    GPIO_InitStructure.GPIO_Pin = SECOND_DOOR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(SECOND_DOOR_PORT, &GPIO_InitStructure);

    // 点火检测引脚----------------------------
    GPIO_InitStructure.GPIO_Pin = IGNITE;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(IGNITE_PORT, &GPIO_InitStructure);

    // 远光灯检测引脚----------------------------
    GPIO_InitStructure.GPIO_Pin = HIGH_BEAM;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(HIGH_BEAM_PORT, &GPIO_InitStructure);

    // 近光灯检测引脚----------------------------
    GPIO_InitStructure.GPIO_Pin = LOW_BEAM;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(LOW_BEAM_PORT, &GPIO_InitStructure);

    // 安全带检测引脚---------------------------
    GPIO_InitStructure.GPIO_Pin = DRIVER_BELT;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DRIVER_BELT_PORT, &GPIO_InitStructure);

    // 副安全带检测引脚1--------------------------
    GPIO_InitStructure.GPIO_Pin = DEPUTY_BELT;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DEPUTY_BELT_PORT, &GPIO_InitStructure);

    // 副安全带检测引脚2--------------------------
    GPIO_InitStructure.GPIO_Pin = DEPUTY_BELT2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DEPUTY_BELT2_PORT, &GPIO_InitStructure);

    // 喇叭检测引脚------------------------------
    GPIO_InitStructure.GPIO_Pin = HONK;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(HONK_PORT, &GPIO_InitStructure);

    // IC10检测引脚------------------------------
    GPIO_InitStructure.GPIO_Pin = IC1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(IC1_PORT, &GPIO_InitStructure);

    // KEY检测引脚------------------------------
    GPIO_InitStructure.GPIO_Pin = KEY;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_PORT, &GPIO_InitStructure);

    // 自擦除引脚检测
    GPIO_InitStructure.GPIO_Pin = C_KEY;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(C_KEY_PORT, &GPIO_InitStructure);
}


/*******************************************************************************
* Function Name  : MX25L128_GPIO_Config
* Description    : 用于初始化MX25L128存储芯片的引脚，实用模拟SPI方式操作该芯片
                   该芯片的引脚与SW下载口服用，调试该芯片是需要拔掉SW下载线。
*******************************************************************************/
void MX25L128_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // MX25L128  数据存储芯片的数据输入引脚
    GPIO_InitStructure.GPIO_Pin = MX25_DO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MX25_DO_PORT, &GPIO_InitStructure);

    // MX25L128  数据存储芯片的写保护引脚
    GPIO_InitStructure.GPIO_Pin = MX25_WP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(MX25_WP_PORT, &GPIO_InitStructure);

    // MX25L128  数据存储芯片的数据输入引脚
    GPIO_InitStructure.GPIO_Pin = MX25_DI;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(MX25_DI_PORT, &GPIO_InitStructure);

    // MX25L128  数据存储芯片的时钟输入引脚
    GPIO_InitStructure.GPIO_Pin = MX25_CLK;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(MX25_CLK_PORT, &GPIO_InitStructure);

    // MX25L128  数据存储芯片的片选引脚
    GPIO_InitStructure.GPIO_Pin = MX25_CS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(MX25_CS_PORT, &GPIO_InitStructure);
    GPIO_WriteBit(MX25_CS_PORT, MX25_CS, Bit_SET);//该引脚为片选脚，默认拉高使能，等使用的时候再拉低
}


/*******************************************************************************
* Function Name  : SI4432_GPIO_Config
* Description    : 用于初始化RF模块的引脚
*******************************************************************************/
void SI4432_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // si4432  RF无线模块的SDN引脚
    GPIO_InitStructure.GPIO_Pin = Si4432_SDN;		//(主备共用)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Si4432_SDN_PORT, &GPIO_InitStructure);

    // si4432  RF无线模块的IRQ引脚
    GPIO_InitStructure.GPIO_Pin = Si4432_IRQ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(Si4432_IRQ_PORT, &GPIO_InitStructure);

    // si4432  RF无线模块的SEL引脚
    GPIO_InitStructure.GPIO_Pin = Si4432_SEL;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//GPIO_Mode_IPU;
    GPIO_Init(Si4432_SEL_PORT, &GPIO_InitStructure);

    // si4432  RF无线模块的SCK引脚
    GPIO_InitStructure.GPIO_Pin = Si4432_SCK;		//(主备共用)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//GPIO_Mode_IPU;
    GPIO_Init(Si4432_SCK_PORT, &GPIO_InitStructure);

    // si4432  RF无线模块的SDI引脚
    GPIO_InitStructure.GPIO_Pin = Si4432_SDI;		//(主备共用)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//GPIO_Mode_IPU;
    GPIO_Init(Si4432_SDI_PORT, &GPIO_InitStructure);

    // si4432  RF无线模块的SDO引脚
    GPIO_InitStructure.GPIO_Pin = Si4432_SDO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(Si4432_SDO_PORT, &GPIO_InitStructure);
}

/*******************************************************************************
* Function Name  : SPI1_DMA_Configuration
* Description    : 配置SPI1_RX的DMA通道2，SPI1_TX的DMA通道3
*******************************************************************************/
void SPI1_DMA_Configuration( void )
{
    SPI_InitTypeDef  SPI_InitStructure;
    DMA_InitTypeDef  DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    // 开相关设备时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);//GPIOA、AFIO时钟使能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);//使能SPI2时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);//使能DMA1时钟

    // SPI接口GPIO初始化
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//将SCK和MOSI管脚设置为复用推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//NSS作为GPIO口使用，设置为推挽输出
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // SPI初始化
    SPI_Cmd(SPI1, DISABLE);
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex ;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    // NVIC初始化
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_DMA_SPI1_SEND;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Channel2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_DMA_SPI1_RECV;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // DMA初始化
    // DMA1 Channel2 (triggered by SPI1 Rx event) Config
    DMA_DeInit(DMA1_Channel2);
    DMA_InitStructure.DMA_PeripheralBaseAddr = SPI1->DR;                    // 设置 SPI1 发送外设(0x4001300C) 地址(目的地址)
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)SPI1_RxBuf;            // 设置 SRAM 存储地址(目的地址)
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;                      // 传输方向 外设==>内存
    DMA_InitStructure.DMA_BufferSize = SPI1_RxBuf_Size;                     // 设置 SPI1 发送长度
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel2, &DMA_InitStructure);

    DMA_ITConfig(DMA1_Channel2, DMA_IT_TC, ENABLE);

    // Enable SPI1 DMA RX request
    SPI1->CR2 |= 1<<0;
    DMA_Cmd(DMA1_Channel2, ENABLE);


    // DMA1 Channel3 (triggered by SPI1 Tx event) Config
    DMA_DeInit(DMA1_Channel3);
    DMA_InitStructure.DMA_PeripheralBaseAddr = SPI1->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)SPI1_TxBuf;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = SPI1_TxBuf_Size;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);

    DMA_ITConfig(DMA1_Channel3, DMA_IT_TC, ENABLE);
    DMA_ITConfig(DMA1_Channel3, DMA_IT_TE, ENABLE);

    // Enable SPI1 DMA TX request
    SPI1->CR2 |= 1<<1;
    DMA_Cmd(DMA1_Channel3, DISABLE);

    // 使能SPI1 DMA发送
    SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx, ENABLE);
}

void DMA1_Channel3_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC3) == SET)
    {
        DMA_Cmd(DMA1_Channel3, DISABLE);
        DMA1_Channel3->CNDTR = 64;//ADC_DATASIZE;
        DMA_Cmd(DMA1_Channel3, ENABLE);
        DMA_ClearITPendingBit(DMA1_IT_TC3);
    }
}


/*******************************************************************************
* Function Name  : SPI1_Send
* Description    : SPI1的DMA方式发送
*******************************************************************************/
void SPI1_Send( u8 *buf, u32 len )
{
    DMA1_Channel3->CPAR = SPI1->DR;
    DMA1_Channel3->CMAR = (u32)buf;
    DMA1_Channel3->CNDTR = len ;
    DMA1_Channel3->CCR = (0 << 14) |
                         (2 << 12) |
                         (0 << 11) |
                         (0 << 10) |
                         (0 <<  9) |
                         (0 <<  8) |
                         (1 <<  7) |
                         (0 <<  6) |
                         (0 <<  5) |
                         (1 <<  4) |
                         (1 <<  3) |
                         (0 <<  2) |
                         (1 <<  1) |
                         (1);
}

/*******************************************************************************
* Function Name  : SPI1_Recive
* Description    : SPI1的DMA方式接收
*******************************************************************************/
void SPI1_Recive( u8 *buf, u32 len )
{
    DMA1_Channel2->CCR &= ~( 1 << 0 );

    DMA1_Channel2->CPAR = SPI1->DR;
    DMA1_Channel2->CMAR = (uint32_t)buf;
    DMA1_Channel2->CNDTR = len ;
    DMA1_Channel2->CCR = (0 << 14) |
                         (2 << 12) |
                         (0 << 11) |
                         (0 << 10) |
                         (0 <<  9) |
                         (0 <<  8) |
                         (1 <<  7) |
                         (0 <<  6) |
                         (0 <<  5) |
                         (0 <<  4) |
                         (0 <<  3) |
                         (0 <<  2) |
                         (1 <<  1) |
                         (1);
}


/*******************************************************************************
* Function Name  : TIM2_Config
* Description    : 定时器2配置为100us产生中断
*******************************************************************************/
void TIM2_Config(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);    // PCLK1经过2倍频后作为TIM2的时钟源等于64MHz

    TIM_DeInit(TIM2);
    TIM_TimeBaseStructure.TIM_Period = 1;                   // 自动重装载寄存器周期的值(计数值)：1sec = 1000 * 10 * 100us

    // 累计 TIM_Period个频率后产生一个更新或者中断
    TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 20000) - 1; // 时钟预分频数: 50us计时
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
*******************************************************************************/
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
*******************************************************************************/
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
*******************************************************************************/
void IWDG_Config(void)
{
    // 独立看门狗由内部专用40khz时钟驱动。
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);// 使能对寄存器IWDG_PR和IWDG_RLR寄存器的写操作
    IWDG_SetPrescaler(IWDG_Prescaler_64);//设置IWDG预分频值：取值范围0~7，低3位有效
    IWDG_SetReload(1250);//设置IWDG重装载值：取值范围0~2047，低11位有效
    IWDG_ReloadCounter();//按照IWDG重载寄存器的值重装载IWDG计数器
    IWDG_Enable();//使能IWDG  Time = (64/(40*10^3))*1250 =2s
}

/*******************************************************************************
* Function Name  : PVD_Config
* Description    : 掉电检测初始化,需要与EXTI_Line16相关联
*******************************************************************************/
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


/*******************************************************************************
* 函数名  : RTC_Config
* 描述    : 实时时钟配置,初始化RTC时钟,同时检测时钟是否工作正常,BKP->DR1用于保存是否第一次配置的设置
*******************************************************************************/
unsigned char RTC_Config(void)
{
    unsigned short temp = 0;

#ifdef __USE_LSE__
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE );
    PWR_BackupAccessCmd( ENABLE );
#else
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_PWR, ENABLE );
#endif

#ifdef __USE_LSE__
    if (BKP_ReadBackupRegister( BKP_DR1 ) != 0xA5A5) {
        BKP_DeInit();
#endif

#ifdef __USE_LSE__
        RCC_LSEConfig(RCC_LSE_ON);
        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && temp < 250)	{
            temp++;
            Delay_ms(10);
        }
#else
        RCC_LSICmd(ENABLE);
        while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET && temp < 250) {
            temp++;
            Delay_ms(10);
        }
#endif

        if(temp >= 250) {
            return 1;       // 初始化时钟失败,晶振有问题
        }

#ifdef __USE_LSE__
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
#else
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
#endif

        RCC_RTCCLKCmd(ENABLE);

#ifdef __USE_LSE__
        BKP_TamperPinCmd(DISABLE);
        BKP_RTCOutputConfig(DISABLE);
#endif

        RTC_WaitForSynchro();
        RTC_WaitForLastTask();
        RTC_EnterConfigMode();              // 允许配置
        RTC_WaitForLastTask();

#ifdef __USE_LSE__
        RTC_SetPrescaler(32767);
        RTC_WaitForLastTask();
#else
        RTC_SetPrescaler(40000);
#endif

        RTC_SetTime( __VER_YEAR, __VER_MONTH, __VER_DAY, 12, 0, 0 );
        RTC_ExitConfigMode();               //退出配置模式

#ifdef __USE_LSE__
        BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
    }
#endif

    RTC_ITConfig(RTC_IT_SEC, ENABLE);

    if( RTC_GetCounter() == 0 ) {
        RTC_SetTime( __VER_YEAR, __VER_MONTH, __VER_DAY, 12, 0, 0 );
    }
    RTC_GetTime();

    return 0;
}
