/******************** (C) COPYRIGHT soarsky ********************
* File Name          : device_init.c
* Author             :
* Version            : V0.1
* Date               :
* Description        : ���ļ�ʵ��STM32�������ʼ��
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "device_init.h"


/* Private functions ---------------------------------------------------------*/
void RCC_Config (void);     //ʱ������

void USART2_Config(void);   //����2����
void USART3_Config(void);   //����3����
void UART4_Config(void);    //����4����
void UART5_Config(void);    //����5����
void MX25L128_GPIO_Config(void);//Flash�洢������
void SI4432_GPIO_Config(void);//RF��������
void GPIO_Input_Config(void);//GPIO����
void GPIO_Output_Config(void);//GPIO����
void NVIC_Config(void);     //�ж����ȼ�����
void TIM2_Config(void);     //��ʱ��2����
void TIM3_Config(void);     //��ʱ��1����
void TIM4_Config(void);

unsigned char RTC_Config(void);      //��������
void PVD_Config(void);

/*******************************************************************************
* Function Name  : MCU_Initialize
* Description    : stm32���ӿ����ã���ʼ����
******************************************************************************/
void MCU_Initialize(void)
{
    RCC_Config();               // ʱ������

#ifndef HARDWARE_VER2
    USART1_Config();            // ����1��Ҫ�������ص���
#else
    XFS_Config();
#endif

    USART2_Config();            // ����2��GSMģ������

    USART3_Config();            // ����3��WIFIģ������

    UART4_Config();             // ����4�������

    UART5_Config();             // ����5�������

    TIM2_Config();              // ��ʱ��2����Ϊ1s�жϷ�ʽ

    TIM3_Config();              // ��ʱ��3����Ϊ1ms�жϷ�ʽ

    TIM4_Config();              // ��ʱ��4����Ϊ10ms�жϷ�ʽ

    MX25L128_GPIO_Config();     // Flash�洢оƬ��������

    MX25L126_Initialize();      // Flash��ʼ��

    SI4432_GPIO_Config();       // RF����ģ����������

    Si4432_Initialize();        // RF����ģ��Ĵ�����ʼ��

    GPIO_Output_Config();       // ��������˿�
    GPIO_Input_Config();        // ��������˿�

    PVD_Config();               // ����������

    RTC_Config();               // ��������

    XFS_Initialize();           // XFS5152����ģ���ʼ��

//    IWDG_Config();              // �������Ź�����
}

/*******************************************************************************
* Function Name  : RCC_Config
* Description    : ʱ�ӳ�ʼ��:ϵͳʱ��64M,SysTick��ʱ������Ϊ1ms�ж�
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
* ������  : RTC_Config
* ����    : ʵʱʱ������,��ʼ��RTCʱ��,ͬʱ���ʱ���Ƿ�������,BKP->DR1���ڱ����Ƿ��һ�����õ�����
* ����    : ��
* ���    : ��
* ����ֵ  : ����0:����  ����:�������
*******************************************************************************/
unsigned char RTC_Config(void)
{
    //����ǲ��ǵ�һ������ʱ��
    u8 temp = 0;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);	//ʹ��PWR��BKP����ʱ��
    PWR_BackupAccessCmd(ENABLE);	//ʹ�ܺ󱸼Ĵ�������
    if (BKP_ReadBackupRegister(BKP_DR1) != 0x5050)		//��ָ���ĺ󱸼Ĵ����ж�������:��������д���ָ�����ݲ����
    {
        BKP_DeInit();	//��λ��������
        RCC_LSICmd(ENABLE);//�����ڲ�����ʱ��LSI
        while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET && temp < 250)	//���ָ����RCC��־λ�������,�ȴ����پ������
        {
            temp++;
            Delay_ms(10);
        }

//        RCC_LSEConfig(RCC_LSE_ON);	//�����ⲿ���پ���(LSE),ʹ��������پ���
//        while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET && temp<250)	//���ָ����RCC��־λ�������,�ȴ����پ������
//        {
//            temp++;
//            Delay_ms(10);
//        }

        if(temp >= 250)
        {
            return 1; //��ʼ��ʱ��ʧ��,����������
        }
        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);		//����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ��
//        RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);		//����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ��
        RCC_RTCCLKCmd(ENABLE);	//ʹ��RTCʱ��
        RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
        RTC_WaitForSynchro();		//�ȴ�RTC�Ĵ���ͬ��
        RTC_ITConfig(RTC_IT_SEC, ENABLE);		//ʹ��RTC���ж�
        RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
        RTC_EnterConfigMode();/// ��������
        RTC_SetPrescaler(40000); //����RTCԤ��Ƶ��ֵ
//        RTC_SetPrescaler(32767); //����RTCԤ��Ƶ��ֵ
        RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
        RTC_SetTime(2017,03,16,11,37,00); //����ʱ��
        RTC_ExitConfigMode(); //�˳�����ģʽ
        BKP_WriteBackupRegister(BKP_DR1, 0x5050);	//��ָ���ĺ󱸼Ĵ�����д���û���������
    }
    else//ϵͳ������ʱ
    {
//    RTC_WaitForSynchro();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
        RTC_ITConfig(RTC_IT_SEC, ENABLE);	//ʹ��RTC���ж�
        RTC_WaitForLastTask();	//�ȴ����һ�ζ�RTC�Ĵ�����д�������
    }
    RTC_GetTime();//����ʱ��
    return 0; //ok
}


/*******************************************************************************
* Function Name  : PVD_Config
* Description    : �������ʼ��,��Ҫ��EXTI_Line16�����
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
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;//������
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    PWR_PVDCmd(ENABLE);
}


#ifdef SPI3_I2411E
/*******************************************************************************
* Function Name  : SPI3_I2411E_Config
* Description    : SPI3����
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
static void SPI3_I2411E_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    SPI_InitTypeDef   SPI_InitStructure;

    /*Ƭѡ����*/
    GPIO_InitStructure.GPIO_Pin = I2411E_CS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(I2411E_CS_PORT, &GPIO_InitStructure);

    /*ʹ������*/
    GPIO_InitStructure.GPIO_Pin = I2411E_EN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(I2411E_EN_PORT, &GPIO_InitStructure);

    /* ��ʼ��SCK��MISO��MOSI���� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5);

    /* ��ʼ������STM32 SPI3 */
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	//SPI����Ϊ˫��˫��ȫ˫��
    SPI_InitStructure.SPI_Mode=SPI_Mode_Master;							//����Ϊ��SPI
    SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;						//SPI���ͽ���8λ֡�ṹ
    SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;							//ʱ�����յ�
    SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;							//���ݲ����ڵ�1��ʱ����
    SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;								//NSS���ⲿ�ܽŹ���
    SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;	//������Ԥ��ƵֵΪ2
    SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;					//���ݴ����MSBλ��ʼ
    SPI_InitStructure.SPI_CRCPolynomial=7;								//CRC����ʽΪ7
    SPI_Init(SPI3,&SPI_InitStructure);									//����SPI_InitStruct��ָ���Ĳ�����ʼ������SPI2�Ĵ���
    SPI_Cmd(SPI3, ENABLE);	//STM32ʹ��SPI3
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

    /*��ת��*/
    GPIO_InitStructure.GPIO_Pin = LED_R;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_R_PORT, &GPIO_InitStructure);

    /*��ת��*/
    GPIO_InitStructure.GPIO_Pin = LED_L;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LED_L_PORT, &GPIO_InitStructure);

    /*�������*/
    GPIO_InitStructure.GPIO_Pin = O3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(O3_PORT,&GPIO_InitStructure);

    /*ָʾ��*/
    GPIO_InitStructure.GPIO_Pin = LIGHT;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(LIGHT_PORT, &GPIO_InitStructure);


//  GPIO_SetBits(LED_R_PORT,LED_R);
//  GPIO_SetBits(LED_L_PORT,LED_L);
//  GPIO_SetBits(O3_PORT,O3);
//  GPIO_SetBits(LIGHT_PORT,LIGHT);       // ��
//
//  GPIO_ResetBits(LED_R_PORT,LED_R);
//  GPIO_ResetBits(LED_L_PORT,LED_L);
//  GPIO_ResetBits(O3_PORT,O3);
    GPIO_ResetBits(LIGHT_PORT,LIGHT);     // ��
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

    /*���ڼ���ʻԱһ�����------------------*/
    GPIO_InitStructure.GPIO_Pin = DRIVER_DOOR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DRIVER_DOOR_PORT, &GPIO_InitStructure);

    /*���ڼ�⸱��ʻԱһ�����------------------*/
    GPIO_InitStructure.GPIO_Pin = SECOND_DOOR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(SECOND_DOOR_PORT, &GPIO_InitStructure);

    /*���������----------------------------*/
    GPIO_InitStructure.GPIO_Pin = IGNITE;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(IGNITE_PORT, &GPIO_InitStructure);

    /*Զ��Ƽ������----------------------------*/
    GPIO_InitStructure.GPIO_Pin = HIGH_BEAM;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(HIGH_BEAM_PORT, &GPIO_InitStructure);

    /*����Ƽ������----------------------------*/
    GPIO_InitStructure.GPIO_Pin = LOW_BEAM;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(LOW_BEAM_PORT, &GPIO_InitStructure);

    /*��ȫ���������---------------------------*/
    GPIO_InitStructure.GPIO_Pin = DRIVER_BELT;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DRIVER_BELT_PORT, &GPIO_InitStructure);

    /*����ȫ���������1--------------------------*/
    GPIO_InitStructure.GPIO_Pin = DEPUTY_BELT;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DEPUTY_BELT_PORT, &GPIO_InitStructure);

    /*����ȫ���������2--------------------------*/
    GPIO_InitStructure.GPIO_Pin = DEPUTY_BELT2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(DEPUTY_BELT2_PORT, &GPIO_InitStructure);

    /*���ȼ������------------------------------*/
    GPIO_InitStructure.GPIO_Pin = HONK;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(HONK_PORT, &GPIO_InitStructure);

    /*IC10�������------------------------------*/
    GPIO_InitStructure.GPIO_Pin = IC1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(IC1_PORT, &GPIO_InitStructure);

    /*KEY�������------------------------------*/
    GPIO_InitStructure.GPIO_Pin = KEY;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(KEY_PORT, &GPIO_InitStructure);

    /*�Բ������ż��*/
    GPIO_InitStructure.GPIO_Pin = C_KEY;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(C_KEY_PORT, &GPIO_InitStructure);
}


/*******************************************************************************
* Function Name  : MX25L128_GPIO_Config
* Description    : ���ڳ�ʼ��MX25L128�洢оƬ�����ţ�ʵ��ģ��SPI��ʽ������оƬ
                   ��оƬ��������SW���ؿڷ��ã����Ը�оƬ����Ҫ�ε�SW�����ߡ�
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void MX25L128_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /*MX25L128  ���ݴ洢оƬ��������������*/
    GPIO_InitStructure.GPIO_Pin = MX25_DO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MX25_DO_PORT, &GPIO_InitStructure);

    /*MX25L128  ���ݴ洢оƬ��д��������*/
    GPIO_InitStructure.GPIO_Pin = MX25_WP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MX25_WP_PORT, &GPIO_InitStructure);

    /*MX25L128  ���ݴ洢оƬ��������������*/
    GPIO_InitStructure.GPIO_Pin = MX25_DI;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MX25_DI_PORT, &GPIO_InitStructure);

    /*MX25L128  ���ݴ洢оƬ��ʱ����������*/
    GPIO_InitStructure.GPIO_Pin = MX25_CLK;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MX25_CLK_PORT, &GPIO_InitStructure);

    /*MX25L128  ���ݴ洢оƬ��Ƭѡ����*/
    GPIO_InitStructure.GPIO_Pin = MX25_CS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MX25_CS_PORT, &GPIO_InitStructure);
    GPIO_WriteBit(MX25_CS_PORT,MX25_CS,Bit_SET);//������ΪƬѡ�ţ�Ĭ������ʹ�ܣ���ʹ�õ�ʱ��������
}


/*******************************************************************************
* Function Name  : SI4432_GPIO_Config
* Description    : ���ڳ�ʼ��RFģ�������
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void SI4432_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /*si4432  RF����ģ���SDN����*/
    GPIO_InitStructure.GPIO_Pin = Si4432_SDN;		//(��������)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Si4432_SDN_PORT, &GPIO_InitStructure);
    /*si4432  RF����ģ���IRQ����*/
    GPIO_InitStructure.GPIO_Pin = Si4432_IRQ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Si4432_IRQ_PORT, &GPIO_InitStructure);
    /*si4432  RF����ģ���SEL����*/
    GPIO_InitStructure.GPIO_Pin = Si4432_SEL;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Si4432_SEL_PORT, &GPIO_InitStructure);
    /*si4432  RF����ģ���SCK����*/
    GPIO_InitStructure.GPIO_Pin = Si4432_SCK;		//(��������)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Si4432_SCK_PORT, &GPIO_InitStructure);
    /*si4432  RF����ģ���SDI����*/
    GPIO_InitStructure.GPIO_Pin = Si4432_SDI;		//(��������)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Si4432_SDI_PORT, &GPIO_InitStructure);
    /*si4432  RF����ģ���SDO����*/
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

    // ��GPIO��USART������ʱ��
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
* Description    : ����2����
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void USART2_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // ��GPIO��USART������ʱ��
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
* Description    : ����3����
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
* Description    : ����4����
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void UART4_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // ��GPIO��USART������ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);

    /*M35 ��Դʹ������*/
    GPIO_InitStructure.GPIO_Pin = M35_EN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(M35_EN_PORT, &GPIO_InitStructure);

    //GSMоƬӲ����λ����
    GPIO_InitStructure.GPIO_Pin = M35_PWR;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(M35_PWR_PORT, &GPIO_InitStructure);

    //��������
    GPIO_InitStructure.GPIO_Pin = M35_RTS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(M35_RTS_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(M35_RTS_PORT,M35_RTS);//Ĭ�Ͻӵ�GND

    //GSM״̬��ȡ����
    GPIO_InitStructure.GPIO_Pin = M35_STATUS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(M35_STATUS_PORT, &GPIO_InitStructure);

    //��������
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
* Description    : ����5����
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void UART5_Config(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    // ��GPIO��USART������ʱ��
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);

    /*GPS ��Դʹ������*/
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
* Description    : �ж����ȼ�����
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Configure the NVIC Preemption Priority Bits */
    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );//���ȼ�����  ȫΪ��ռʽ���ȼ�
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
* Description    : ��ʱ��2����Ϊ1s�����ж�
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void TIM2_Config(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);    // PCLK1����2��Ƶ����ΪTIM2��ʱ��Դ����64MHz

    TIM_DeInit(TIM2);
    TIM_TimeBaseStructure.TIM_Period = 10000;               // �Զ���װ�ؼĴ������ڵ�ֵ(����ֵ)��1sec = 1000 * 10 * 100us

    // �ۼ� TIM_Period��Ƶ�ʺ����һ�����»����ж�
    TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 10000) - 1; // ʱ��Ԥ��Ƶ��: 100us��ʱ
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // ������Ƶ
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // ���ϼ���ģʽ
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);                   // �������жϱ�־
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);                                  // ����ʱ��
}


/*******************************************************************************
* Function Name  : TIM3_Config
* Description    : ��ʱ��3����Ϊ1ms�����ж�
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void TIM3_Config(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_DeInit(TIM3);
    TIM_TimeBaseStructure.TIM_Period = 1000;                // �Զ���װ�ؼĴ������ڵ�ֵ(����ֵ)��1ms = 1000 * 1us

    // �ۼ� TIM_Period��Ƶ�ʺ����һ�����»����ж�
    TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 1000000) - 1; // ʱ��Ԥ��Ƶ��: 1us��ʱ
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // ������Ƶ
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // ���ϼ���ģʽ
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    TIM_ClearFlag(TIM3, TIM_FLAG_Update);                   // �������жϱ�־
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE);                                  // ����ʱ��
}


/*******************************************************************************
* Function Name  : TIM4_Config
* Description    : ͨ�ö�ʱ���жϳ�ʼ��  10ms�ж�
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void TIM4_Config(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);    // ʱ��ʹ��//TIM4ʱ��ʹ��

    //��ʱ��TIM3��ʼ��
    TIM_TimeBaseStructure.TIM_Period        = 1000;         // �Զ���װ�ؼĴ������ڵ�ֵ(����ֵ)��10ms = 1000 * 10us
    TIM_TimeBaseStructure.TIM_Prescaler     = (SystemCoreClock / 100000) - 1; // ʱ��Ԥ��Ƶ��: 10us��ʱ
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; // ����ʱ�ӷָ�:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;   // TIM���ϼ���ģʽ
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);         // ����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

    TIM_ClearFlag(TIM4, TIM_FLAG_Update);                   // �������жϱ�־
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE );             // ʹ��ָ����TIM4�ж�,��������ж�
    TIM_Cmd(TIM4, ENABLE);                                  // ����ʱ��
}

/*******************************************************************************
* Function Name  : IWDG_Config
* Description    : �������Ź���ʼ��
                   ι��ʱ�� = ((4 * 2^��Ƶֵ) * װ��ֵ) / ʱ�� = (4*2^4)*625/40khz = 1000ms
* Input          : None
* Output         : None
* Return         : None
******************************************************************************/
void IWDG_Config(void)
{
    /*�������Ź����ڲ�ר��40khzʱ��������*/
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);/*ʹ�ܶԼĴ���IWDG_PR��IWDG_RLR�Ĵ�����д����*/
    IWDG_SetPrescaler(IWDG_Prescaler_64);//����IWDGԤ��Ƶֵ��ȡֵ��Χ0~7����3λ��Ч
    IWDG_SetReload(1250);//����IWDG��װ��ֵ��ȡֵ��Χ0~2047����11λ��Ч
    IWDG_ReloadCounter();//����IWDG���ؼĴ�����ֵ��װ��IWDG������
    IWDG_Enable();//ʹ��IWDG  Time = (64/(40*10^3))*1250 =2s
}




/******************* (C) COPYRIGHT 2012 WildFire Team *****END OF FILE************/

