#ifndef __DEVICE_INIT_H
#define	__DEVICE_INIT_H

#include "stm32f10x.h"
#include "stm32f10x_conf.h"

extern const char __ver_id[];

#define HARDWARE_VER2    //ʹ�õڶ���

/*si4432 ���ź궨��-----------------------------------------------------------------*/
#define Si4432_SCK		 GPIO_Pin_5	//����Si4432��SCK����	 
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

/*MX25L128 ���ź궨��-----------------------------------------------------------------*/
#define MX25_DO     	GPIO_Pin_3   //���������
#define MX25_DO_PORT	GPIOC
#define MX25_WP	        GPIO_Pin_1   //д������
#define MX25_WP_PORT	GPIOC
#define MX25_DI	        GPIO_Pin_0   //���������
#define MX25_DI_PORT    GPIOC
#define MX25_CLK	    GPIO_Pin_4   //ʱ������
#define MX25_CLK_PORT	GPIOC
#define MX25_CS	        GPIO_Pin_2   //Ƭѡ����
#define MX25_CS_PORT	GPIOC

/*XFS5152 ���ź궨��-----------------------------------------------------------------*/
#define XFS5152_RST		 GPIO_Pin_5
#define XFS5152_RST_PORT GPIOC
#define AMP_OPEN         GPIO_Pin_6
#define AMP_OPEN_PORT    GPIOC

/*M35���Ŷ���-----------------------------------------------------------------------*/
#define M35_PWR	         GPIO_Pin_8 //����Ӳ������,�ػ�
#define M35_PWR_PORT	   GPIOC
#define M35_STATUS       GPIO_Pin_9 //���ڲ鿴ģ��״̬
#define M35_STATUS_PORT  GPIOC
#define M35_DTR          GPIO_Pin_7 //DTE׼������(����)
#define M35_DTR_PORT     GPIOC
#define M35_RTS          GPIO_Pin_1 //DTE����������,���ӵ�GND(����)
#define M35_RTS_PORT     GPIOE
#define M35_CTS          GPIO_Pin_0 //ģ���������(����)
#define M35_CTS_PORT     GPIOD

/*GPS���Ŷ���-----------------------------------------------------------------------*/
#define GPS_ON           GPIO_Pin_1 //���ڿ���GPSģ�鿪��
#define GPS_ON_PORT      GPIOD

/*M35���Ŷ���-----------------------------------------------------------------------*/
#define M35_EN           GPIO_Pin_15//���ڿ���GPRS��Դ����
#define M35_EN_PORT      GPIOD

/*����------------------------------------------------------------------------------*/
#define C_KEY            GPIO_Pin_4
#define C_KEY_PORT       GPIOD

/*RJMU101��ȫоƬ���Ŷ���-----------------------------------------------------------*/
#define RJMU_RST         GPIO_Pin_11 //���ڸ�λRJMU101
#define RJMU_RST_PORT    GPIOB

/*IO�����������---------------------------------------------------------------*/
#define OUT1             GPIO_Pin_10
#define OUT1_PORT        GPIOD
#define OUT2             GPIO_Pin_11
#define OUT2_PORT        GPIOD
#define OUT3             GPIO_Pin_12
#define OUT3_PORT        GPIOD

/*I2411E����wifiģ�����Ŷ���*/
#define I2411E_CS        GPIO_Pin_15
#define I2411E_CS_PORT   GPIOA
//#define I2411E_EN        GPIO_Pin_6
//#define I2411E_EN_PORT   GPIOB

/*IO���������Ŷ���----------------------------------------------------------------*/
#define IC1              GPIO_Pin_7//����
#define IC1_PORT         GPIOE
#define LOW_BEAM		 GPIO_Pin_8//�����
#define LOW_BEAM_PORT    GPIOE
#define HIGH_BEAM		 GPIO_Pin_9//Զ���
#define HIGH_BEAM_PORT   GPIOE
#define HONK             GPIO_Pin_10//����
#define HONK_PORT        GPIOE
#define DRIVER_BELT      GPIO_Pin_11//����ȫ��
#define DRIVER_BELT_PORT GPIOE
#define DEPUTY_BELT 		 GPIO_Pin_12//����ȫ��1
#define DEPUTY_BELT_PORT GPIOE
#define DEPUTY_BELT2 			GPIO_Pin_13//����ȫ��2
#define DEPUTY_BELT2_PORT GPIOE
#define DRIVER_DOOR      GPIO_Pin_14//�ſ���1
#define DRIVER_DOOR_PORT GPIOE
#define IGNITE           GPIO_Pin_15//���
#define IGNITE_PORT      GPIOE
#define SECOND_DOOR      GPIO_Pin_9//�ſ���2
#define SECOND_DOOR_PORT GPIOD
#define KEY              GPIO_Pin_8//������
#define KEY_PORT         GPIOD

/*IO����������Ŷ���--------------------------------------------------------------*/
#define LED_L		       GPIO_Pin_10//����
#define LED_L_PORT		   GPIOD
#define LED_R		       GPIO_Pin_11//����
#define LED_R_PORT		   GPIOD
#define O3		           GPIO_Pin_12//����
#define O3_PORT		       GPIOD
#define LIGHT		       GPIO_Pin_13//����
#define LIGHT_PORT		   GPIOD


// �����ʶ���
#define DEBUG_BAUD        115200
#define GPS_BAUD          9600
#define XFS_BAUD          9600
#define I2411E_BAUD       115200
#define M35_BAUD          115200


// ģ��ӿڶ���
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



// ���ҵƲ�����
#define ON_LED()  { GPIO_SetBits(LED_L_PORT,LED_L); GPIO_SetBits(LED_R_PORT,LED_R);}
#define OFF_LED() { GPIO_ResetBits(LED_R_PORT,LED_R); GPIO_ResetBits(LED_L_PORT,LED_L);}

// �����Ŵ�������ƺ�
#define ON_AMP()    GPIO_SetBits(AMP_OPEN_PORT, AMP_OPEN)
#define OFF_AMP()   GPIO_ResetBits(AMP_OPEN_PORT, AMP_OPEN);

// XFS5152 Reset����
#define rstXFS5152()    GPIO_WriteBit(XFS5152_RST_PORT, XFS5152_RST, (BitAction)0)
#define setXFS5152()    GPIO_WriteBit(XFS5152_RST_PORT, XFS5152_RST, (BitAction)1)


// USB_Ctrl interface(STM32F103RC)
#define USB_DISCONNECT_PORT                 GPIOD
#define USB_DISCONNECT_PIN                  GPIO_Pin_3
#define RCC_APB2Periph_GPIO_DISCONNECT      RCC_APB2Periph_GPIOD

#define USB_ConnEn()  GPIO_ResetBits(USB_DISCONNECT_PORT, USB_DISCONNECT_PIN);
#define USB_ConnDis() GPIO_SetBits(USB_DISCONNECT_PORT, USB_DISCONNECT_PIN);


void XFS_Config(void);
void USART1_Config(void);//����1����
void MCU_Initialize(void);
void NVIC_Configuration(void);
void IWDG_Config(void);     //�������Ź�����

#endif /* __USART3_H */
















