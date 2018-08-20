/******************** (C) COPYRIGHT soarsky ********************
* File Name          :
* Author             :
* Version            :
* Date               :
* Description        :
最大地址:0x8040000
1k = 0x400
128 = 0x80
128k = 0x20000
*******************************************************************************/
#include "Stm32Flash.h"


u16 STMFLASH_BUF[SECTOR_SIZE/2];//用于保存一个扇区的内容
u16 iapbuf[SECTOR_SIZE/2];//用于保存u16的升级数据


/*******************************************************************************
* 函数名  : STMFLASH_WriteAppBin
* 描述    : 升级程序分包下载,一个包的大小刚好1k,可以一次性写入Flash
* 输入    : appxaddr:应用程序的起始地址  appbuf:应用程序CODE.  appsize:应用程序大小(字节).
* 返回值  :
*******************************************************************************/
void STMFLASH_WriteAppBin(u32 AppAddr,u8 *AppBuf,u16 AppSize)
{
    u16 t;
    u16 i=0;
    u16 temp;
    u32 fwaddr=AppAddr;//当前写入的地址
    u8  *dfu = AppBuf;
    for(t=0; t<AppSize; t+=2) //将8bit的数组转换为16bit的数组
    {
        temp=(u16)dfu[1]<<8;
        temp+=(u16)dfu[0];
        dfu+=2;//偏移2个字节
        iapbuf[i++]=temp;
    }

    STMFLASH_Write(fwaddr,iapbuf,SECTOR_SIZE/2+SECTOR_SIZE%2);

    fwaddr+=SECTOR_SIZE;//偏移1024
}


/*******************************************************************************
* 函数名  : STMFLASH_ReadHalfWord
* 描述    : 读取指定地址的半字(16位数据)
* 输入    : faddr 读地址(此地址必须为2的倍数!!)
* 返回值  : 对应数据.
*******************************************************************************/
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
    return *(vu16*)faddr;
}

/*******************************************************************************
* 函数名  : STMFLASH_Read
* 描述    : 从指定地址开始读出指定长度的数据
* 输入    : ReadAddr:起始地址  pBuffer:数据指针  NumToWrite:半字(16位)数
* 返回值  :
*******************************************************************************/
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)
{
    u16 i;
    for(i=0; i<NumToRead; i++)
    {
        pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节.
        ReadAddr+=2;//偏移2个字节.
    }
}


/*******************************************************************************
* 函数名  : STMFLASH_Write_NoCheck
* 描述    : 不检查的写入
* 输入    : WriteAddr:起始地址   pBuffer:数据指针  NumToWrite:半字(16位)数
* 返回值  :
*******************************************************************************/
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{
    u16 i;
    if(WriteAddr%2 != 0)
    {
        return;
    }
    for(i=0; i<NumToWrite; i++)
    {
        FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
        WriteAddr+=2;//地址增加2.
    }
}




/*******************************************************************************
* 函数名  : STMFLASH_Write
* 描述    : 从指定地址开始写入指定长度的数据
* 输入    : WriteAddr:起始地址(此地址必须为2的倍数!!)  pBuffer:数据指针  NumToWrite:半字(16位)数
* 返回值  :
*******************************************************************************/
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{
    u32 SecPos;	   //扇区地址
    u16 SecOff;	   //扇区内偏移地址(16位字计算)
    u16 SecRemain; //扇区内剩余地址(16位字计算)
    u16 i;
    u32 OffAddr;   //去掉0X08000000后的地址
    if(WriteAddr<FLASH_BASE || (WriteAddr>=(FLASH_BASE+1024*STM32_FLASH_SIZE)) || WriteAddr%2 != 0)
    {
        return;//非法地址
    }

    OffAddr=WriteAddr-FLASH_BASE;		//实际偏移地址.
    SecPos=OffAddr/SECTOR_SIZE;			  //扇区地址  0~64 for STM32F101CBT6
    SecOff=(OffAddr%SECTOR_SIZE)/2;		//在扇区内的偏移(2个字节为基本单位.)
    SecRemain=SECTOR_SIZE/2-SecOff;		//扇区剩余空间大小
    if(NumToWrite<=SecRemain)
    {
        SecRemain = NumToWrite;//不大于该扇区范围
    }
    while(1)
    {
        STMFLASH_Read( WriteAddr, STMFLASH_BUF, SECTOR_SIZE/2);//读出整个扇区的内容
        for(i=0; i<SecRemain; i++) //校验数据
        {
            if(STMFLASH_BUF[SecOff+i]!=0XFFFF)
            {
                break;//需要擦除
            }
        }
        if(i<SecRemain)//需要擦除
        {
            FLASH_ErasePage(WriteAddr);//擦除这个扇区
            for(i=0; i<SecRemain; i++) //复制
            {
                STMFLASH_BUF[i+SecOff] = pBuffer[i];
            }
            STMFLASH_Write_NoCheck(SecPos*SECTOR_SIZE+FLASH_BASE,STMFLASH_BUF,SECTOR_SIZE/2);//写入整个扇区
        }
        else
        {
            STMFLASH_Write_NoCheck(WriteAddr,pBuffer,SecRemain);//写已经擦除了的,直接写入扇区剩余区间.
        }
        if(NumToWrite==SecRemain)
        {
            break;//写入结束了
        } else//写入未结束
        {
            SecPos++;				//扇区地址增1
            SecOff=0;				//偏移位置为0
            pBuffer+=SecRemain;  	//指针偏移
            WriteAddr+=SecRemain;	//写地址偏移
            NumToWrite-=SecRemain;	//字节(16位)数递减
            if(NumToWrite>(SECTOR_SIZE/2))
            {
                SecRemain=SECTOR_SIZE/2;//下一个扇区还是写不完
            }
            else
            {
                SecRemain=NumToWrite;//下一个扇区可以写完了
            }
        }
    }

}



/*******************END OF FILE******************************************************************************/
