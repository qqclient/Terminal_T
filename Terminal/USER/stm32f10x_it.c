/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
//static volatile unsigned char USART1_RxCount = 0;
extern volatile unsigned int CheckerAnswer_timer;
extern unsigned int  WaitTime;
extern unsigned char wftlF,wftlH; 
extern unsigned int  wftH[2];
extern unsigned int  wftL[2];
volatile unsigned short Park_TimeOver = 0;        // 泊车后超时切换状态


/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}




/*******************************************************************************
* Function Name  : PVD_IRQHandler
* Description    : 掉电检测中断
*******************************************************************************/
void PVD_IRQHandler(void)
{
    EXTI_ClearITPendingBit(EXTI_Line16);//必须在if外面
    if(PWR_GetFlagStatus(PWR_FLAG_PVDO))
    {
        PWR_ClearFlag(PWR_FLAG_PVDO);
        XFS_WriteBuffer(xfs_auto, "断电");
    }
}


/*******************************************************************************
* Function Name  : SysTick_Handler
* Description    : 系统定时器服务中断函数
*******************************************************************************/
void SysTick_Handler(void)
{

}

/*******************************************************************************
* Function Name  : TIM2_IRQHandler
* Description    : 定时器2中断服务函数,100us定时中断
*******************************************************************************/
void TIM2_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM2,TIM_IT_Update) != RESET )
    {
        TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);
        if(CheckerAnswer_timer > 0) {//采集仪稽查计数器
            CheckerAnswer_timer--;
        }
        if(WaitTime >0) {
            WaitTime--;
        }
				
			//---------------------------------------
				
				if( wftlF == 1 ){   //wftlF = 1;  //开始计时
					wftL[0]++;
					if( wftL[0] > 0xfffe ){
						wftL[0]=0;             //记录波形的低电平时间
					}
				}
				if( wftlH == 1 ){   //wftlH = 1;  //开始计时
					wftH[0]++;
					if( wftH[0] > 0xfffe ){
						wftH[0]=0;             //记录波形的低电平时间
					}
				}
				
			//---------------------------------------
				
    }
}


/*******************************************************************************
* Function Name  : TIM3_IRQHandler
* Description    : 定时器1中断服务函数,1ms中断
*******************************************************************************/
void TIM3_IRQHandler(void)
{
    static unsigned int Count_1S = 0;
    static unsigned int Count_100ms = 0;

    if(TIM_GetITStatus(TIM3,TIM_IT_Update) != RESET ) {
        TIM_ClearITPendingBit(TIM3,TIM_FLAG_Update);

        if(Count_1S >= 1000) {
            Count_1S = 0;
   
					
				//	Debug_Printf(SET,"当前状态 0X%x  终端状态  0X%x", FlagTerminalStat,TerminalStatus);
					
            if(GPRS_LocationTime > 0) {
                GPRS_LocationTime--;
            }
            Si4432_SendTimeOut ++;

            if(GTW_IllegalPark.Sec_OverTime > 0) {
                GTW_IllegalPark.Sec_OverTime--;

                if(GTW_IllegalPark.Sec_OverTime == 0) {
                    GTW_IllegalPark.Flag_Main = RESET;
                }
            }

            if(m_var.FollowCar_TimeOver > 0 ) {
                m_var.FollowCar_TimeOver--;

                if( m_var.FollowCar_TimeOver == 0) {
                    Flag_ApplyResult = RESET;
                    m_var.Flag_ApplyDriver = RESET;
                    ApplyLicInit();
                    if( m_var.Apply1_TimeOver == 0 ) {
                        if( TerminalStatus == Status_Park ) {
                            I2411E_ChangeCarStatus('1');
                        }
                        else {
                            I2411E_ChangeCarStatus('2');
                        }
                    }
                }
            }

            if(m_var.Apply1_TimeOver > 0 ) {
                m_var.Apply1_TimeOver--;

                if( m_var.Apply1_TimeOver == 0 ) {
                    if( TerminalStatus == Status_Park ) {
                        I2411E_ChangeCarStatus('1');
                    }
                    else {
                        I2411E_ChangeCarStatus('2');
                    }
                }
            }

            if( Park_TimeOver > 0 ) {
                Park_TimeOver--;

                if(Park_TimeOver == 0) {
									  
                    Flag_ApplyResult = RESET;
                    m_var.Flag_ApplyDriver = RESET;
                    ApplyLicInit();
                    I2411E_ChangeCarStatus('1');
                }
								Debug_Printf(SET,"返回到熄火停车状态,  当前状态 0X%x",FlagTerminalStat);
            }

            if( m_var.UpdateClock_TimeOver > 0 ) {
                m_var.UpdateClock_TimeOver--;

                if( m_var.UpdateClock_TimeOver == 0 ) {
                    m_var.FLag_UpdateClock = 0;
                }
            }
        }
        else {
            Count_1S++;
        }

        Count_100ms++;
        if( Count_100ms > 200 ) {
            Count_100ms = 0;

            if( Count_FindCop > 0 && ( FlagTerminalStat == '1' || Flag_Setup_LAMP == SET )) {
                Count_FindCop--;

                if( Count_FindCop % 5 == 0 || Count_FindCop % 5 == 1 ) {
                    ON_LED();
					          Debug_Printf(SET, "开灯");
                }
                else {
                    OFF_LED();
					          Debug_Printf(SET, "关灯");
                }
								
								//Debug_Printf(SET, "闪灯");
            }
            else {
                OFF_LED();
                Flag_Setup_LAMP = RESET;
                Count_FindCop = 0;
            }
        }

        if( m_var.gtw_recv_timeout > 0) {
            m_var.gtw_recv_timeout--;
        }
        if(Si4432_TimeSendChangeLight > 0)
        {
            Si4432_TimeSendChangeLight--;
        }
        if( Si4432_TimeSendSn > 0 )
            Si4432_TimeSendSn--;

        if(XFS_TimeCount > 0x00) {
            XFS_TimeCount--;
        }

        if(m_delaySave > 0) {
            m_delaySave--;
        }

        if(M35_TimeCount > 0x00) {
            M35_TimeCount--;
        }

        if (TimingDelay > 0x00) {
            TimingDelay--;
        }

        if(I2411E_TimeCount > 0x00) {
            I2411E_TimeCount--;
        }

        if(Count_I2411E_SCAN > 0x00) {
            Count_I2411E_SCAN--;
        }
				
				if( m_var.ReadBeamTimer <=  2000 ){
					m_var.ReadBeamTimer++;
				}
    }
}



/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
//void SysTick_Handler(void)
//{
//
//
//}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
