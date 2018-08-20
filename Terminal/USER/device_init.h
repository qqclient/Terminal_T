#ifndef __DEVICE_INIT_H
#define	__DEVICE_INIT_H

#include "stm32f10x.h"
#include "stm32f10x_conf.h"

extern const char __ver_id[];

#define HARDWARE_VER2    //使用第二版

/*si4432 引脚宏定义-----------------------------------------------------------------*/
#define Si4432_SCK		 GPIO_Pin_5	//定义Si4432的SCK引脚	 
#define Si4432_SCK_PORT	 GPIOA
#define Si4432_SDI       GPIO_Pin_7
#define Si4432_SDI_PORT  GPIOA
#define Si4432_SEL       GPIO_Pin_4
#define Si4432_SEL_PORT  GPIOA
#define Si4432_SDN       GPIO_Pin_1
#define Si4432_SDN_PORT  GPIOA
#define Si4432_SDO       GPIO_Pin_6
#define Si4432_SDO_PORT  GPIOA
#define Si4432_IRQ       GPIO_Pin_8
#define Si4432_IRQ_PORT  GPIOA

/*MX25L128 引脚宏定义-----------------------------------------------------------------*/
#define MX25_DO     	GPIO_Pin_3   //数据输出脚
#define MX25_DO_PORT	GPIOC
#define MX25_WP	        GPIO_Pin_1   //写保护脚
#define MX25_WP_PORT	GPIOC
#define MX25_DI	        GPIO_Pin_0   //数据输入脚
#define MX25_DI_PORT    GPIOC
#define MX25_CLK	    GPIO_Pin_4   //时钟引脚
#define MX25_CLK_PORT	GPIOC
#define MX25_CS	        GPIO_Pin_2   //片选引脚
#define MX25_CS_PORT	GPIOC

/*XFS5152 引脚宏定义-----------------------------------------------------------------*/
#define XFS5152_RST		 GPIO_Pin_5
#define XFS5152_RST_PORT GPIOC
#define AMP_OPEN         GPIO_Pin_6
#define AMP_OPEN_PORT    GPIOC

/*M35引脚定义-----------------------------------------------------------------------*/
#define M35_PWR	         GPIO_Pin_8 //用于硬件开机,关机
#define M35_PWR_PORT	   GPIOC
#define M35_STATUS       GPIO_Pin_9 //用于查看模块状态
#define M35_STATUS_PORT  GPIOC
#define M35_DTR          GPIO_Pin_7 //DTE准备就绪(备用)
#define M35_DTR_PORT     GPIOC
#define M35_RTS          GPIO_Pin_1 //DTE请求发送数据,连接到GND(备用)
#define M35_RTS_PORT     GPIOE
#define M35_CTS          GPIO_Pin_0 //模块清除发送(备用)
#define M35_CTS_PORT     GPIOD

/*GPS引脚定义-----------------------------------------------------------------------*/
#define GPS_ON           GPIO_Pin_1 //用于控制GPS模块开关
#define GPS_ON_PORT      GPIOD

/*M35引脚定义-----------------------------------------------------------------------*/
#define M35_EN           GPIO_Pin_15//用于控制GPRS电源开关
#define M35_EN_PORT      GPIOD

/*其它------------------------------------------------------------------------------*/
#define C_KEY            GPIO_Pin_4
#define C_KEY_PORT       GPIOD

/*RJMU101安全芯片引脚定义-----------------------------------------------------------*/
#define RJMU_RST         GPIO_Pin_11 //用于复位RJMU101
#define RJMU_RST_PORT    GPIOB

/*IO输出控制引脚---------------------------------------------------------------*/
#define OUT1             GPIO_Pin_10
#define OUT1_PORT        GPIOD
#define OUT2             GPIO_Pin_11
#define OUT2_PORT        GPIOD
#define OUT3             GPIO_Pin_12
#define OUT3_PORT        GPIOD

/*I2411E蓝牙wifi模块引脚定义*/
#define I2411E_CS        GPIO_Pin_15
#define I2411E_CS_PORT   GPIOA
//#define I2411E_EN        GPIO_Pin_6
//#define I2411E_EN_PORT   GPIOB

/*IO输入检测引脚定义----------------------------------------------------------------*/
#define IC1              GPIO_Pin_7//备用
#define IC1_PORT         GPIOE
#define LOW_BEAM		 GPIO_Pin_8//近光灯
#define LOW_BEAM_PORT    GPIOE
#define HIGH_BEAM		 GPIO_Pin_9//远光灯
#define HIGH_BEAM_PORT   GPIOE
#define HONK             GPIO_Pin_10//喇叭
#define HONK_PORT        GPIOE
#define DRIVER_BELT      GPIO_Pin_11//主安全带
#define DRIVER_BELT_PORT GPIOE
#define DEPUTY_BELT 		 GPIO_Pin_12//副安全带1
#define DEPUTY_BELT_PORT GPIOE
#define DEPUTY_BELT2 			GPIO_Pin_13//副安全带2
#define DEPUTY_BELT2_PORT GPIOE
#define DRIVER_DOOR      GPIO_Pin_14//门开关1
#define DRIVER_DOOR_PORT GPIOE
#define IGNITE           GPIO_Pin_15//点火
#define IGNITE_PORT      GPIOE
#define SECOND_DOOR      GPIO_Pin_9//门开关2
#define SECOND_DOOR_PORT GPIOD
#define KEY              GPIO_Pin_8//按键板
#define KEY_PORT         GPIOD

/*IO输出控制引脚定义--------------------------------------------------------------*/
#define LED_L		       GPIO_Pin_10//右闪
#define LED_L_PORT		   GPIOD
#define LED_R		       GPIO_Pin_11//左闪
#define LED_R_PORT		   GPIOD
#define O3		           GPIO_Pin_12//左闪
#define O3_PORT		       GPIOD
#define LIGHT		       GPIO_Pin_13//左闪
#define LIGHT_PORT		   GPIOD


// 波特率定义
#define DEBUG_BAUD        115200
#define GPS_BAUD          9600
#define XFS_BAUD          9600
#define I2411E_BAUD       115200
#define M35_BAUD          115200


// 模块接口定义
#ifdef HARDWARE_VER2

#define DEBUG_UART       USART3
#define XFS_UART          USART1
#define XFS_IRQHandler    USART1_IRQHandler
#define GPS_UART          USART2
#define GPS_IRQHandler    USART2_IRQHandler
#define I2411E_UART       UART4
#define I2411E_IRQHandler UART4_IRQHandler
#define M35_UART         UART5
#define M35_IRQHandler    UART5_IRQHandler

#else

#define DEBUG_UART        USART1
#define XFS_UART          USART2
#define XFS_IRQHandler    USART2_IRQHandler
#define I2411E_UART       USART3
#define I2411E_IRQHandler USART3_IRQHandler
#define M35_UART          UART4
#define M35_IRQHandler    UART4_IRQHandler
#define GPS_UART          UART5
#define GPS_IRQHandler    UART5_IRQHandler

#endif



// 左右灯操作宏
#define ON_LED()  { GPIO_SetBits(LED_L_PORT,LED_L); GPIO_SetBits(LED_R_PORT,LED_R);}
#define OFF_LED() { GPIO_ResetBits(LED_R_PORT,LED_R); GPIO_ResetBits(LED_L_PORT,LED_L);}

// 语音放大输出控制宏
#define ON_AMP()    GPIO_SetBits(AMP_OPEN_PORT, AMP_OPEN)
#define OFF_AMP()   GPIO_ResetBits(AMP_OPEN_PORT, AMP_OPEN);

// XFS5152 Reset操作
#define rstXFS5152()    GPIO_WriteBit(XFS5152_RST_PORT, XFS5152_RST, (BitAction)0)
#define setXFS5152()    GPIO_WriteBit(XFS5152_RST_PORT, XFS5152_RST, (BitAction)1)


// USB_Ctrl interface(STM32F103RC)
#define USB_DISCONNECT_PORT                 GPIOD
#define USB_DISCONNECT_PIN                  GPIO_Pin_3
#define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOD

#define USB_ConnEn()  GPIO_ResetBits(USB_DISCONNECT_PORT, USB_DISCONNECT_PIN);
#define USB_ConnDis() GPIO_SetBits(USB_DISCONNECT_PORT, USB_DISCONNECT_PIN);


void XFS_Config(void);
void USART1_Config(void);//串口1配置
void MCU_Initialize(void);
void NVIC_Configuration(void);
void IWDG_Config(void);     //独立看门狗配置

#endif /* __USART3_H */
















