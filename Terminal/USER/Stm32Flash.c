/******************** (C) COPYRIGHT soarsky ********************
* File Name          : debug.c
* Author             : 宁建文
* Version            : V0.1
* Date               : 2017-03-19
* Description        : Bootload支持文件
*******************************************************************************/
#include "Stm32Flash.h"

#define STM_SECTOR_SIZE  1024 
#define STM32_FLASH_SIZE 256  

u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];
u16 iapbuf[STM_SECTOR_SIZE/2];


/*******************************************************************************
* 函数名  : STMFLASH_WriteAppBin
* 描述    : 升级程序分包下载
*******************************************************************************/
void STMFLASH_WriteAppBin(u32 AppAddr,u8 *AppBuf,u16 AppSize)
{
    u16 t;
    u16 i = 0;
    u16 temp;
    u32 fwaddr = AppAddr;//当前写入的地址
    u8  *dfu = AppBuf;

    //将8bit的数组转换为16bit的数组
    for(t=0; t<AppSize; t+=2) {
        temp  = (u16)dfu[1] << 8;
        temp += (u16)dfu[0];
        dfu  += 2;//偏移2个字节
        iapbuf[i++] = temp;
    }

    STMFLASH_Write(fwaddr, iapbuf, STM_SECTOR_SIZE / 2 + STM_SECTOR_SIZE % 2);

    fwaddr += STM_SECTOR_SIZE;//偏移1024
}


/*******************************************************************************
* 函数名  : STMFLASH_ReadHalfWord
* 描述    : 读取指定地址的半字(16位数据)
*******************************************************************************/
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
    return *(vu16*)faddr;
}

/*******************************************************************************
* 函数名  : STMFLASH_Read
* 描述    : 从指定地址开始读出指定长度的数据
*******************************************************************************/
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)
{
    u16 i;

    for(i=0; i<NumToRead; i++)
    {
        pBuffer[i] = STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节.
        ReadAddr  += 2;//偏移2个字节.
    }
}


/*******************************************************************************
* 函数名  : STMFLASH_Write_NoCheck
* 描述    : 不检查的写入
*******************************************************************************/
void STMFLASH_Write_NoCheck(u32 WriteAddr, u16 *pBuffer, u16 NumToWrite)
{
    u16 i;

    if(WriteAddr%2 != 0) {
        return;
    }
    for(i=0; i<NumToWrite; i++) {
        FLASH_ProgramHalfWord(WriteAddr, pBuffer[i]);
        WriteAddr += 2;//地址增加2.
    }
}


/*******************************************************************************
* 函数名  : STMFLASH_Write
* 描述    : 从指定地址开始写入指定长度的数据
*******************************************************************************/
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{
    u32 SecPos;	   
    u16 SecOff;	   
    u16 SecRemain; 
    u16 i;
    u32 OffAddr;   

    if( WriteAddr < FLASH_BASE || (WriteAddr >= ( FLASH_BASE + 1024 * STM32_FLASH_SIZE )) || WriteAddr%2 != 0 )
    {
        return;
    }
    FLASH_Unlock();
    OffAddr=WriteAddr-FLASH_BASE;		
    SecPos=OffAddr/STM_SECTOR_SIZE;			  
    SecOff=(OffAddr%STM_SECTOR_SIZE)/2;		
    SecRemain=STM_SECTOR_SIZE/2-SecOff;		
    if(NumToWrite<=SecRemain)
    {
        SecRemain = NumToWrite;
    }

    while(1)
    {
        STMFLASH_Read(SecPos * STM_SECTOR_SIZE+FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE/2);
        for(i=0; i<SecRemain; i++) {
            if(STMFLASH_BUF[SecOff + i] != 0XFFFF) {
                break;
            }
        }
        if(i < SecRemain) {
            FLASH_ErasePage(SecPos * STM_SECTOR_SIZE + FLASH_BASE);
            for(i=0; i<SecRemain; i++) {
                STMFLASH_BUF[i+SecOff] = pBuffer[i];
            }
            STMFLASH_Write_NoCheck( SecPos*STM_SECTOR_SIZE + FLASH_BASE, STMFLASH_BUF, STM_SECTOR_SIZE / 2 );
        }
        else {
            STMFLASH_Write_NoCheck(WriteAddr, pBuffer, SecRemain);
        }
        if( NumToWrite == SecRemain ) {
            break;
        }
        else {
            SecPos++;				
            SecOff = 0;				
            pBuffer += SecRemain;  	
            WriteAddr += SecRemain;	
            NumToWrite -= SecRemain;	
            if(NumToWrite > (STM_SECTOR_SIZE / 2)) {
                SecRemain = STM_SECTOR_SIZE / 2;
            }
            else {
                SecRemain = NumToWrite;
            }
        }
    }

    FLASH_Lock();
}

