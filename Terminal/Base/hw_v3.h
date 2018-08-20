/*******************************************************************************
* File Name          : hw_v3.h
* Author             : CSYXJ
* Version            : V0.1
* Date               : 2017-03-19
* Description        : MCU接口定义文件
*******************************************************************************/

#ifndef __HW_V3_H
#define __HW_V3_H

#include "stm32f10x.h"

#define __USE_USB__         // 使用USB,否则使用串口3
#define __USE_MPU6050__     // 使用MPU60506轴加速度传感器

// 外围模块:
// Si4332       RF433MHz模块        软件模拟SPI接口
// MX25L128     FLASH存储器         软件模拟SPI接口
// DEBUG        调试信息输出口      USART1:PA9/PA10(115200,n,8,1)
// XFS5152      TTS语音合成模块     USART1:映射口PB6/PB7(9600,n,8,1)
// RJMU101      加解密安全芯片      USART3(115200,n,8,1)
// I2411E       蓝牙wifi模块        UART4(115200,n,8,1)
// M35          GSM/GPRS模块        UART5(115200,n,8,1)
// ATG336H      中芯微GPS模块       USART2(9600,n,8,1)

// 波特率定义
#define DEBUG_BAUD              115200
#define GPS_BAUD                9600
#define XFS_BAUD                115200
#define I2411E_BAUD             115200
#define M35_BAUD                115200
#define RJMU101_BAUD            115200

// 模块接口定义
// == 调试输出接口 ==
//#define DEBUG_UART              USART1
#define DEBUG_IRQn              USART1_IRQn
#define DEBUG_RCC_APB2Periph    RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA
#define DEBUG_IRQHandler        USART1_IRQHandler
#define DEBUG_TX_PORT           GPIOA
#define DEBUG_TX_PIN            GPIO_Pin_9
#define DEBUG_RX_PORT           GPIOA
#define DEBUG_RX_PIN            GPIO_Pin_10

// == XFS5152 <==> USART1映射口 ==
#define XFS_UART                USART1
#define XFS_IRQn                USART1_IRQn
#define XFS_UART_REMAP          GPIO_Remap_USART1
#define XFS_RCC_APB2Periph      RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC
#define XFS_IRQHandler          USART1_IRQHandler
#define XFS_TX_PORT             GPIOB
#define XFS_TX_PIN              GPIO_Pin_6
#define XFS_RX_PORT             GPIOB
#define XFS_RX_PIN              GPIO_Pin_7

// XFS5152 引脚宏定义
#define XFS5152_RST		        GPIO_Pin_5
#define XFS5152_RST_PORT        GPIOC
#define AMP_OPEN                GPIO_Pin_6
#define AMP_OPEN_PORT           GPIOC

// XFS5152 Reset操作
#define rstXFS5152()            GPIO_ResetBits(XFS5152_RST_PORT, XFS5152_RST)
#define setXFS5152()            GPIO_SetBits(XFS5152_RST_PORT, XFS5152_RST)

// 语音放大输出控制宏
#define ON_AMP()                GPIO_SetBits(AMP_OPEN_PORT, AMP_OPEN)
#define OFF_AMP()               GPIO_ResetBits(AMP_OPEN_PORT, AMP_OPEN)


// == GPS模块 ==
#define GPS_UART                USART2
#define GPS_IRQn                USART2_IRQn
#define GPS_RCC_APB1Periph      RCC_APB1Periph_USART2
#define GPS_RCC_APB2Periph      RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOD
#define GPS_IRQHandler          USART2_IRQHandler
#define GPS_TX_PORT             GPIOA
#define GPS_TX_PIN              GPIO_Pin_2
#define GPS_RX_PORT             GPIOA
#define GPS_RX_PIN              GPIO_Pin_3

// GPS引脚定义
#define GPS_ON                  GPIO_Pin_1                  // 控制GPS模块开关
#define GPS_ON_PORT             GPIOD

#define GPS_ENABLE()            GPIO_SetBits(GPS_ON_PORT, GPS_ON)
#define GPS_DISABLE()           GPIO_ResBits(GPS_ON_PORT, GPS_ON)

// GPS DMA定义
#define GPS_DMA                 DMA1
#define GPS_DMA_IRQn            DMA1_Channel6_IRQn          // GPS中断源
#define GPS_DMA_CLK             RCC_AHBPeriph_DMA1
#define GPS_DMA_CHANNEL         DMA1_Channel6
#define GPS_DMA_IRQHandler      DMA1_Channel6_IRQHandler
#define GPS_DMA_FLAG_TC         DMA1_FLAG_TC6
#define GPS_DMA_FLAG_TE         DMA1_FLAG_TE6
#define GPS_DMA_FLAG_HT         DMA1_FLAG_HT6
#define GPS_DMA_FLAG_GL         DMA1_FLAG_GL6
#define GPS_DMA_IT_HT           DMA1_IT_HT6
#define GPS_DMA_IT_TC           DMA1_IT_TC6
#define GPS_DATA_ADDR           ((u32)&GPS_UART->DR)


// == 加密模块 ==
#define RJMU101_UART            USART3
#define RJMU101_IRQn            USART3_IRQn
#define RJMU101_RCC_APB1Periph  RCC_APB1Periph_USART3
#define RJMU101_RCC_APB2Periph  RCC_APB2Periph_GPIOB
#define RJMU101_IRQHandler      USART3_IRQHandler
#define RJMU101_TX_PORT         GPIOB
#define RJMU101_TX_PIN          GPIO_Pin_10
#define RJMU101_RX_PORT         GPIOB
#define RJMU101_RX_PIN          GPIO_Pin_11

// RJMU101安全芯片引脚定义
#define RJMU_RST                GPIO_Pin_14 //用于复位RJMU101
#define RJMU_RST_PORT           GPIOD


// == I2411E(wifi/bluetooth)模块 ==
#define I2411E_UART             UART4
#define I2411E_IRQn             UART4_IRQn
#define I2411E_RCC_APB1Periph   RCC_APB1Periph_UART4
#define I2411E_RCC_APB2Periph   RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC
#define I2411E_IRQHandler       UART4_IRQHandler
#define I2411E_TX_PORT          GPIOC
#define I2411E_TX_PIN           GPIO_Pin_10
#define I2411E_RX_PORT          GPIOC
#define I2411E_RX_PIN           GPIO_Pin_11

// I2411E蓝牙wifi模块引脚定义
#define I2411E_CS               GPIO_Pin_15
#define I2411E_CS_PORT          GPIOA
//#define I2411E_EN               GPIO_Pin_6
//#define I2411E_EN_PORT          GPIOB


// == M35(GSM/GPRS)模块 ==
#define M35_UART                UART5
#define M35_IRQn                UART5_IRQn
#define M35_RCC_APB1Periph      RCC_APB1Periph_UART5
#define M35_RCC_APB2Periph      RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD
#define M35_IRQHandler          UART5_IRQHandler
#define M35_TX_PORT             GPIOC
#define M35_TX_PIN              GPIO_Pin_12
#define M35_RX_PORT             GPIOD
#define M35_RX_PIN              GPIO_Pin_2

// M35引脚定义
#define M35_PWR	                GPIO_Pin_8  // 用于硬件开机,关机
#define M35_PWR_PORT	        GPIOC
#define M35_STATUS              GPIO_Pin_9  // 用于查看模块状态
#define M35_STATUS_PORT         GPIOC
#define M35_DTR                 GPIO_Pin_7  // DTE准备就绪(备用)
#define M35_DTR_PORT            GPIOC
#define M35_RTS                 GPIO_Pin_1  // DTE请求发送数据,连接到GND(备用)
#define M35_RTS_PORT            GPIOE
#define M35_CTS                 GPIO_Pin_0  // 模块清除发送(备用)
#define M35_CTS_PORT            GPIOD
#define M35_EN                  GPIO_Pin_15 // 用于控制GPRS电源开关
#define M35_EN_PORT             GPIOD
#define M35_SIMIN               GPIO_Pin_2  // SIM卡拔插检测口
#define M35_SIMIN_PORT          GPIOE


// == si4432 引脚宏定义 ==
#define Si4432_SCK		        GPIO_Pin_5	//定义Si4432的SCK引脚	 
#define Si4432_SCK_PORT	        GPIOA
#define Si4432_SDI              GPIO_Pin_7
#define Si4432_SDI_PORT         GPIOA
#define Si4432_SEL              GPIO_Pin_4
#define Si4432_SEL_PORT         GPIOA
#define Si4432_SDN              GPIO_Pin_1
#define Si4432_SDN_PORT         GPIOA
#define Si4432_SDO              GPIO_Pin_6
#define Si4432_SDO_PORT         GPIOA
#define Si4432_IRQ              GPIO_Pin_8
#define Si4432_IRQ_PORT         GPIOA


// == MX25L128 引脚宏定义 ==
#define MX25_DO     	        GPIO_Pin_3   //数据输出脚
#define MX25_DO_PORT	        GPIOC
#define MX25_WP	                GPIO_Pin_1   //写保护脚
#define MX25_WP_PORT	        GPIOC
#define MX25_DI	                GPIO_Pin_0   //数据输入脚
#define MX25_DI_PORT            GPIOC
#define MX25_CLK	            GPIO_Pin_4   //时钟引脚
#define MX25_CLK_PORT	        GPIOC
#define MX25_CS	                GPIO_Pin_2   //片选引脚
#define MX25_CS_PORT	        GPIOC


// == IO输入检测引脚定义 ==
#define IC1                     GPIO_Pin_7//备用
#define IC1_PORT                GPIOE
#define LOW_BEAM		        GPIO_Pin_8//近光灯
#define LOW_BEAM_PORT           GPIOE
#define HIGH_BEAM		        GPIO_Pin_9//远光灯
#define HIGH_BEAM_PORT          GPIOE
#define HONK                    GPIO_Pin_10//喇叭
#define HONK_PORT               GPIOE
#define DRIVER_BELT             GPIO_Pin_11//主安全带
#define DRIVER_BELT_PORT        GPIOE
#define DEPUTY_BELT 		    GPIO_Pin_12//副安全带1
#define DEPUTY_BELT_PORT        GPIOE
#define DEPUTY_BELT2 			GPIO_Pin_13//副安全带2
#define DEPUTY_BELT2_PORT       GPIOE
#define DRIVER_DOOR             GPIO_Pin_14//门开关1
#define DRIVER_DOOR_PORT        GPIOE
#define IGNITE                  GPIO_Pin_15//点火 
#define IGNITE_PORT             GPIOE
#define SECOND_DOOR             GPIO_Pin_9//门开关2
#define SECOND_DOOR_PORT        GPIOD
#define KEY                     GPIO_Pin_8//学习按键
#define KEY_PORT                GPIOD
#define C_KEY                   GPIO_Pin_4
#define C_KEY_PORT              GPIOD


// == IO输出控制引脚定义 ==
#define LED_L		            GPIO_Pin_12//左闪  
#define LED_L_PORT		        GPIOD
#define LED_R		            GPIO_Pin_11//右闪  
#define LED_R_PORT		        GPIOD
#define O3		                GPIO_Pin_10//备用
#define O3_PORT		            GPIOD
#define LIGHT		            GPIO_Pin_13//学习指示灯
#define LIGHT_PORT		        GPIOD

// 左右灯操作宏
#define ON_LED()                { GPIO_SetBits(LED_L_PORT,LED_L);   GPIO_SetBits(LED_R_PORT,LED_R); }
#define OFF_LED()               { GPIO_ResetBits(LED_R_PORT,LED_R); GPIO_ResetBits(LED_L_PORT,LED_L); }


// == USB_Ctrl interface(STM32F103RC) ==
#define USB_DISCONNECT_PORT                 GPIOD
#define USB_DISCONNECT_PIN                  GPIO_Pin_3
#define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOD

// USB_Ctrl interface(STM32F103VC)
#define USB_ConnEn()            GPIO_ResetBits(USB_DISCONNECT_PORT, USB_DISCONNECT_PIN);
#define USB_ConnDis()           GPIO_SetBits(USB_DISCONNECT_PORT, USB_DISCONNECT_PIN);

#endif
