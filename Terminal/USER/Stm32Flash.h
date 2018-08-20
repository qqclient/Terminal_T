#ifndef __STM32FLASH_H
#define __STM32FLASH_H

#include "stm32f10x.h"

void STMFLASH_WriteAppBin(u32 appxaddr,u8 *appbuf,u16 appsize);
u16  STMFLASH_ReadHalfWord(u32 faddr);
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);


#endif

