/******************** (C) COPYRIGHT soarsky ********************
* File Name          :
* Author             :
* Version            :
* Date               :
* Description        :
bootloader地址; 0x08000000 - 0x08001FFF 共8k
App1地址:       0x08002000 - 0x08008FFF 共28k
App2地址:       0x08009000 - 0x0800FFFF 共28k
*******************************************************************************/

#include "stm32f10x.h"
#include "usart.h"
#include "Stm32Flash.h"

typedef struct
{
    u16 flah_new;//标志是否有新的终端程序
    u16 pack_count;//以1024为一包,共有pack_num个包
    u16 check_sum;//校验和
//    u32 vec_tab;//用于保存中断向量表
//    u16 flag_first;//第一次写程序
} UpdateInfo_Struct;
UpdateInfo_Struct updateInfo;

typedef  void (*iapfun)(void);				//定义一个函数类型的参数.
iapfun   jump2app;

/****************************************************************************
//设置栈顶地址
//addr:栈顶地址
****************************************************************************/
__asm void MSR_MSP(u32 addr)
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}


/****************************************************************************
 //跳转到应用程序段
//appxaddr:用户代码起始地址.
****************************************************************************/
void iap_load_app(u32 appxaddr)
{
    u32 temp1 = ( *(vu32*)appxaddr ) & 0x2FFE0000 ;
    if( ( ( *(vu32*)appxaddr ) & 0x2FFE0000 ) == 0x20000000 )	//检查栈顶地址是否合法.
    {
        jump2app = (iapfun)*(vu32*)(appxaddr+4);      //用户代码区第二个字为程序开始地址(复位地址)
        MSR_MSP(*(vu32*)appxaddr);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
        jump2app();									//跳转到APP.
    }
}


/*******************************************************************************
* Function Name  :
* Description    :
******************************************************************************/
int main(void)
{
    u32 addr_app_run   = 0x08002000;
    u32 addr_update_info=0x08001F00;
    u32 addr_read_app  = 0x08022000;
    u32 addr_write_app = 0x08002000;
    u32 i;

#ifdef __DEBUG
    USART1_Config();
    Usart_SendString(USART1,"\r\nIAP升级程序运行\r\n");
#endif

    STMFLASH_Read(addr_update_info,&updateInfo.flah_new,sizeof(UpdateInfo_Struct)/2);//读取参数

//    if(updateInfo.flag_first == 0xFFFF) { //首次运行程序
//        updateInfo.flag_first = 0x00AA;
//        updateInfo.vec_tab = 0x2000;
//        updateInfo.flah_new = 0;
//        updateInfo.pack_count = 0;
//        updateInfo.check_sum = 0;
//        FLASH_Unlock();//解锁
//        FLASH_ErasePage(addr_update_info);//擦除Flash
//        STMFLASH_Write_NoCheck(addr_update_info,&updateInfo.flah_new,sizeof(UpdateInfo_Struct)/2);//保存数据
//        FLASH_Lock();//上锁
//    }

    if(updateInfo.flah_new == 1) { //有新的APP
        FLASH_Unlock();//解锁
        for(i = 0; i< updateInfo.pack_count; i++) {
            STMFLASH_Read(addr_read_app,iapbuf,SECTOR_SIZE/2);
//            Delay_ms(5);
            STMFLASH_Write(addr_write_app,iapbuf,SECTOR_SIZE/2);
//            Delay_ms(5);
            addr_read_app += SECTOR_SIZE;
            addr_write_app += SECTOR_SIZE;
        }

        updateInfo.flah_new = 0;//清除标志
        updateInfo.pack_count = 0;//清除计数
//        updateInfo.check_sum = 0;//清除校验码
        
        FLASH_ErasePage(addr_update_info);//擦除Flash
        STMFLASH_Write_NoCheck(addr_update_info,&updateInfo.flah_new,sizeof(UpdateInfo_Struct)/2);//保存数据
        FLASH_Lock();//上锁
    }

    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x2000);
    jump2app = (iapfun)*(vu32*)(addr_app_run+4);      //用户代码区第二个字为程序开始地址(复位地址)
    MSR_MSP(*(vu32*)addr_app_run);					  //初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
    jump2app();									      //跳转到APP.
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{

}



/****************************************END OF FILE*******************************/


