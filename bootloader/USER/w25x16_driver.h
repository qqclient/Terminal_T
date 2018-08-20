#ifndef __W25X16_DRIVER_H
#define __W25X16_DRIVER_H

#include "stm32f10x.h"

#define __COLLECTOR__


#ifdef __COLLECTOR__
/*MX25 Òý½Åºê¶¨Òå-----------------------------------------------------------------*/
#define FLASH_DO     	GPIO_Pin_1
#define FLASH_DO_PORT	GPIOB
#define FLASH_DI	    GPIO_Pin_0
#define FLASH_DI_PORT   GPIOA
#define FLASH_CLK	    GPIO_Pin_1
#define FLASH_CLK_PORT	GPIOA
#define FLASH_CS	    GPIO_Pin_2
#define FLASH_CS_PORT	GPIOA


void FLASH_WREN(void);
void FLASH_Chip_Erase(void);
void FLASH_Block_Erase(unsigned int addr);
void FLASH_Sector_Erase(unsigned int addr);
void FLASH_BufferWrite(unsigned char* pBuffer,unsigned int WriteAddr,unsigned short int NumByteToWrite);
void FLASH_Page_Write(unsigned char* pBuffer, unsigned int addr,unsigned short int NumByte);
void FLASH_BufferRead(unsigned char* pBuffer ,unsigned int addr,unsigned short int NumByte);
void FLASH_Word_Write(unsigned int addr,unsigned int word);
u32  FLASH_Word_read(unsigned int addr);
void FLASH_WriteByte(unsigned char _dat);
unsigned char FLASH_ReadByte(void);
unsigned int  FLASH_ReadID(void);
void FLASH_Test(void);
void FLASH_WaitForWriteEnd(void);
#endif
#endif




