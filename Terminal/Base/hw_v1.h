/*******************************************************************************
* File Name          : hw_v1.h
* Author             : CSYXJ
* Version            : V0.1
* Date               : 2017-03-19
* Description        : MCU�ӿڶ����ļ�
*******************************************************************************/

#ifndef __HW_V1_H
#define __HW_V1_H

#include "stm32f10x.h"

// #define __USE_USB__         // ʹ��USB,����ʹ�ô���3


// ��Χģ��:
// Si4332       RF433MHzģ��        ���ģ��SPI�ӿ�
// MX25L128     FLASH�洢��         ���ģ��SPI�ӿ�
// DEBUG        ������Ϣ�����      USART1(115200,n,8,1)
// XFS5152      TTS�����ϳ�ģ��     USART2(9600,n,8,1)
// RJMU101      �ӽ��ܰ�ȫоƬ      USART3[��ʱδ��](115200,n,8,1)
// I2411E       ����wifiģ��        USART3(115200,n,8,1)
// M35          GSM/GPRSģ��        UART4(115200,n,8,1)
// ATG336H      ��о΢GPSģ��       UART5(9600,n,8,1)


// �����ʶ���
#define DEBUG_BAUD              115200
#define GPS_BAUD                9600
#define XFS_BAUD                9600
#define I2411E_BAUD             115200
#define M35_BAUD                115200
#define RJMU101_BAUD            115200


// ģ��ӿڶ���
#define DEBUG_UART              USART1
#define DEBUG_IRQn              USART1_IRQn
#define DEBUG_RCC_APB2Periph    RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA
#define DEBUG_IRQHandler        USART1_IRQHandler
#define DEBUG_TX_PORT           GPIOA
#define DEBUG_TX_PIN            GPIO_Pin_9
#define DEBUG_RX_PORT           GPIOA
#define DEBUG_RX_PIN            GPIO_Pin_10


// == XFS5152 ==
#define XFS_UART                USART2
#define XFS_IRQn                USART2_IRQn
#define XFS_RCC_APB1Periph      RCC_APB1Periph_USART2
#define XFS_RCC_APB2Periph      RCC_APB2Periph_GPIOA
#define XFS_IRQHandler          USART2_IRQHandler
#define XFS_TX_PORT             GPIOA
#define XFS_TX_PIN              GPIO_Pin_2
#define XFS_RX_PORT             GPIOA
#define XFS_RX_PIN              GPIO_Pin_3

// XFS5152 ���ź궨��
#define XFS5152_RST		        GPIO_Pin_5
#define XFS5152_RST_PORT        GPIOC
#define AMP_OPEN                GPIO_Pin_6
#define AMP_OPEN_PORT           GPIOC

// XFS5152 Reset����
#define rstXFS5152()            GPIO_ResetBits(XFS5152_RST_PORT, XFS5152_RST)
#define setXFS5152()            GPIO_SetBits(XFS5152_RST_PORT, XFS5152_RST)

// �����Ŵ�������ƺ�
#define ON_AMP()                GPIO_SetBits(AMP_OPEN_PORT, AMP_OPEN)
#define OFF_AMP()               GPIO_ResetBits(AMP_OPEN_PORT, AMP_OPEN)


// == I2411E(wifi/bluetooth)ģ�� ==
#define I2411E_UART             USART3
#define I2411E_IRQn             USART3_IRQn
#define I2411E_RCC_APB1Periph   RCC_APB1Periph_USART3
#define I2411E_RCC_APB2Periph   RCC_APB2Periph_GPIOB
#define I2411E_IRQHandler       USART3_IRQHandler
#define I2411E_TX_PORT          GPIOB
#define I2411E_TX_PIN           GPIO_Pin_10
#define I2411E_RX_PORT          GPIOB
#define I2411E_RX_PIN           GPIO_Pin_11

// I2411E����wifiģ�����Ŷ���
#define I2411E_CS               GPIO_Pin_15
#define I2411E_CS_PORT          GPIOA
//#define I2411E_EN               GPIO_Pin_6
//#define I2411E_EN_PORT          GPIOB


// == M35(GSM/GPRS)ģ�� ==
#define M35_UART                UART4
#define M35_IRQn                UART4_IRQn
#define M35_RCC_APB1Periph      RCC_APB1Periph_UART4
#define M35_RCC_APB2Periph      RCC_APB2Periph_GPIOC
#define M35_IRQHandler          UART4_IRQHandler
#define M35_TX_PORT             GPIOC
#define M35_TX_PIN              GPIO_Pin_10
#define M35_RX_PORT             GPIOC
#define M35_RX_PIN              GPIO_Pin_11

// M35���Ŷ���
#define M35_PWR	                GPIO_Pin_8 //Ӳ����/�ػ�
#define M35_PWR_PORT	        GPIOC
#define M35_STATUS              GPIO_Pin_9 //ģ��״̬
#define M35_STATUS_PORT         GPIOC
#define M35_DTR                 GPIO_Pin_7 //DTE׼������(����)
#define M35_DTR_PORT            GPIOC
#define M35_RTS                 GPIO_Pin_1 //DTE����������,���ӵ�GND(����)
#define M35_RTS_PORT            GPIOE
#define M35_CTS                 GPIO_Pin_0 //ģ���������(����)
#define M35_CTS_PORT            GPIOD
#define M35_EN                  GPIO_Pin_15//���ڿ���GPRS��Դ����
#define M35_EN_PORT             GPIOD


// == GPSģ�� ==
#define GPS_UART                UART5
#define GPS_IRQn                UART5_IRQn
#define GPS_RCC_APB1Periph      RCC_APB1Periph_UART5
#define GPS_RCC_APB2Periph      RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD
#define GPS_IRQHandler          UART5_IRQHandler
#define GPS_TX_PORT             GPIOC
#define GPS_TX_PIN              GPIO_Pin_12
#define GPS_RX_PORT             GPIOD
#define GPS_RX_PIN              GPIO_Pin_2

// GPS���Ŷ���
#define GPS_ON                  GPIO_Pin_1 //���ڿ���GPSģ�鿪��
#define GPS_ON_PORT             GPIOD

#define GPS_ENABLE()            GPIO_SetBits(GPS_ON_PORT, GPS_ON)
#define GPS_DISABLE()           GPIO_ResBits(GPS_ON_PORT, GPS_ON)

#define GPS_DMA                 DMA1
#define GPS_DMA_IRQHandler      DMA1_Channel6_IRQHandler
#define GPS_DMA_CLK             RCC_AHBPeriph_DMA1
#define GPS_DMA_CHANNEL         DMA1_Channel6
#define GPS_DMA_IRQn            DMA1_Channel6_IRQn         //GPS�ж�Դ
#define GPS_DMA_FLAG_TC         DMA1_FLAG_TC6
#define GPS_DMA_FLAG_TE         DMA1_FLAG_TE6
#define GPS_DMA_FLAG_HT         DMA1_FLAG_HT6
#define GPS_DMA_FLAG_GL         DMA1_FLAG_GL6
#define GPS_DMA_IT_HT           DMA1_IT_HT6
#define GPS_DMA_IT_TC           DMA1_IT_TC6
#define GPS_DATA_ADDR           ((u32)&GPS_UART->DR)


// == ����ģ�� ==
// #define RJMU101_UART            USART3
#define RJMU101_IRQn            USART3_IRQn
#define RJMU101_RCC_APB1Periph  RCC_APB1Periph_USART3
#define RJMU101_RCC_APB2Periph  RCC_APB2Periph_GPIOB
#define RJMU101_IRQHandler      USART3_IRQHandler
#define RJMU101_TX_PORT         GPIOB
#define RJMU101_TX_PIN          GPIO_Pin_10
#define RJMU101_RX_PORT         GPIOB
#define RJMU101_RX_PIN          GPIO_Pin_11

// RJMU101��ȫоƬ���Ŷ���
#define RJMU_RST                GPIO_Pin_11 //���ڸ�λRJMU101
#define RJMU_RST_PORT           GPIOB


// == si4432 ���ź궨�� ==
#define Si4432_SCK		        GPIO_Pin_5	//����Si4432��SCK����	 
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


// == MX25L128 ���ź궨�� ==
#define MX25_DO     	        GPIO_Pin_3   //���������
#define MX25_DO_PORT	        GPIOC
#define MX25_WP	                GPIO_Pin_1   //д������
#define MX25_WP_PORT	        GPIOC
#define MX25_DI	                GPIO_Pin_0   //���������
#define MX25_DI_PORT            GPIOC
#define MX25_CLK	            GPIO_Pin_4   //ʱ������
#define MX25_CLK_PORT	        GPIOC
#define MX25_CS	                GPIO_Pin_2   //Ƭѡ����
#define MX25_CS_PORT	        GPIOC


// == IO���������Ŷ��� ==
#define IC1                     GPIO_Pin_7//����
#define IC1_PORT                GPIOE
#define LOW_BEAM		        GPIO_Pin_8//�����
#define LOW_BEAM_PORT           GPIOE
#define HIGH_BEAM		        GPIO_Pin_9//Զ���
#define HIGH_BEAM_PORT          GPIOE
#define HONK                    GPIO_Pin_10//����
#define HONK_PORT               GPIOE
#define DRIVER_BELT             GPIO_Pin_11//����ȫ��
#define DRIVER_BELT_PORT        GPIOE
#define DEPUTY_BELT 		    GPIO_Pin_12//����ȫ��1
#define DEPUTY_BELT_PORT        GPIOE
#define DEPUTY_BELT2 			GPIO_Pin_13//����ȫ��2
#define DEPUTY_BELT2_PORT       GPIOE
#define DRIVER_DOOR             GPIO_Pin_14//�ſ���1
#define DRIVER_DOOR_PORT        GPIOE
#define IGNITE                  GPIO_Pin_15//���
#define IGNITE_PORT             GPIOE
#define SECOND_DOOR             GPIO_Pin_9//�ſ���2
#define SECOND_DOOR_PORT        GPIOD
#define KEY                     GPIO_Pin_8//ѧϰ����
#define KEY_PORT                GPIOD
#define C_KEY                   GPIO_Pin_4
#define C_KEY_PORT              GPIOD


// == IO����������Ŷ��� ==
#define LED_L		            GPIO_Pin_10//����
#define LED_L_PORT		        GPIOD
#define LED_R		            GPIO_Pin_11//����
#define LED_R_PORT		        GPIOD
#define O3		                GPIO_Pin_12//����
#define O3_PORT		            GPIOD
#define LIGHT		            GPIO_Pin_13//����
#define LIGHT_PORT		        GPIOD

// ���ҵƲ�����
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
