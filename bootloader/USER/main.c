/******************** (C) COPYRIGHT soarsky ********************
* File Name          :
* Author             :
* Version            :
* Date               :
* Description        :
bootloader��ַ; 0x08000000 - 0x08001FFF ��8k
App1��ַ:       0x08002000 - 0x08008FFF ��28k
App2��ַ:       0x08009000 - 0x0800FFFF ��28k
*******************************************************************************/

#include "stm32f10x.h"
#include "usart.h"
#include "Stm32Flash.h"

typedef struct
{
    u16 flah_new;//��־�Ƿ����µ��ն˳���
    u16 pack_count;//��1024Ϊһ��,����pack_num����
    u16 check_sum;//У���
//    u32 vec_tab;//���ڱ����ж�������
//    u16 flag_first;//��һ��д����
} UpdateInfo_Struct;
UpdateInfo_Struct updateInfo;

typedef  void (*iapfun)(void);				//����һ���������͵Ĳ���.
iapfun   jump2app;

/****************************************************************************
//����ջ����ַ
//addr:ջ����ַ
****************************************************************************/
__asm void MSR_MSP(u32 addr)
{
    MSR MSP, r0 			//set Main Stack value
    BX r14
}


/****************************************************************************
 //��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ.
****************************************************************************/
void iap_load_app(u32 appxaddr)
{
    u32 temp1 = ( *(vu32*)appxaddr ) & 0x2FFE0000 ;
    if( ( ( *(vu32*)appxaddr ) & 0x2FFE0000 ) == 0x20000000 )	//���ջ����ַ�Ƿ�Ϸ�.
    {
        jump2app = (iapfun)*(vu32*)(appxaddr+4);      //�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)
        MSR_MSP(*(vu32*)appxaddr);					//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
        jump2app();									//��ת��APP.
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
    Usart_SendString(USART1,"\r\nIAP������������\r\n");
#endif

    STMFLASH_Read(addr_update_info,&updateInfo.flah_new,sizeof(UpdateInfo_Struct)/2);//��ȡ����

//    if(updateInfo.flag_first == 0xFFFF) { //�״����г���
//        updateInfo.flag_first = 0x00AA;
//        updateInfo.vec_tab = 0x2000;
//        updateInfo.flah_new = 0;
//        updateInfo.pack_count = 0;
//        updateInfo.check_sum = 0;
//        FLASH_Unlock();//����
//        FLASH_ErasePage(addr_update_info);//����Flash
//        STMFLASH_Write_NoCheck(addr_update_info,&updateInfo.flah_new,sizeof(UpdateInfo_Struct)/2);//��������
//        FLASH_Lock();//����
//    }

    if(updateInfo.flah_new == 1) { //���µ�APP
        FLASH_Unlock();//����
        for(i = 0; i< updateInfo.pack_count; i++) {
            STMFLASH_Read(addr_read_app,iapbuf,SECTOR_SIZE/2);
//            Delay_ms(5);
            STMFLASH_Write(addr_write_app,iapbuf,SECTOR_SIZE/2);
//            Delay_ms(5);
            addr_read_app += SECTOR_SIZE;
            addr_write_app += SECTOR_SIZE;
        }

        updateInfo.flah_new = 0;//�����־
        updateInfo.pack_count = 0;//�������
//        updateInfo.check_sum = 0;//���У����
        
        FLASH_ErasePage(addr_update_info);//����Flash
        STMFLASH_Write_NoCheck(addr_update_info,&updateInfo.flah_new,sizeof(UpdateInfo_Struct)/2);//��������
        FLASH_Lock();//����
    }

    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x2000);
    jump2app = (iapfun)*(vu32*)(addr_app_run+4);      //�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)
    MSR_MSP(*(vu32*)addr_app_run);					  //��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
    jump2app();									      //��ת��APP.
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


