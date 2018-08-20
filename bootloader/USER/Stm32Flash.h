#ifndef __STM32FLASH_H
#define __STM32FLASH_H


#include "stm32f10x.h"


#define SECTOR_SIZE  1024 //单位byte
#define STM32_FLASH_SIZE 256  //单位k


extern u16 STMFLASH_BUF[SECTOR_SIZE/2];//用于保存一个扇区的内容
extern u16 iapbuf[SECTOR_SIZE/2];//用于保存u16的升级数据


void STMFLASH_Test(void);
void STMFLASH_WriteAppBin(u32 appxaddr,u8 *appbuf,u16 appsize);
u16  STMFLASH_ReadHalfWord(u32 faddr);
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);







#endif

/*******************END OF FILE******************************************************************************/
