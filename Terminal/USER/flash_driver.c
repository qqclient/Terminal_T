/******************** (C) COPYRIGHT soarsky ********************
* File Name          :
* Author             :
* Version            :
* Date               :
* Description        : Flash的特性是：写数据只能将1写为0，擦除数据就是写1
*                      因此如果想在以有数据的falsh上写入新的数据，则必须先擦除。
*                      还有Flash在擦除的时候必须整块擦除
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"

/* Private define ------------------------------------------------------------*/
#define MX25_CS_LOW    GPIO_WriteBit(MX25_CS_PORT ,MX25_CS ,Bit_RESET);
#define MX25_CS_HIGH   GPIO_WriteBit(MX25_CS_PORT ,MX25_CS ,Bit_SET);
#define MX25_CLK_Low   GPIO_WriteBit(MX25_CLK_PORT,MX25_CLK,Bit_RESET);
#define MX25_CLK_High  GPIO_WriteBit(MX25_CLK_PORT,MX25_CLK,Bit_SET);
#define MX25_DI_Low    GPIO_WriteBit(MX25_DI_PORT ,MX25_DI ,Bit_RESET);
#define MX25_DI_High   GPIO_WriteBit(MX25_DI_PORT ,MX25_DI ,Bit_SET);
#define MX25_WP_Low		 GPIO_WriteBit(MX25_WP_PORT ,MX25_WP ,Bit_RESET)
#define MX25_WP_High	 GPIO_WriteBit(MX25_WP_PORT ,MX25_WP ,Bit_SET)
#define MX25_Read_DO   GPIO_ReadInputDataBit(MX25_DO_PORT, MX25_DO)
#define SPI_FLASH_PageSize         256


/*******************************************************************************
* Function Name  : MX25L128_BufferWrite
* Description    : Writes block of data to the FLASH. In this function, the
*                  number of WRITE cycles are reduced, using Page WRITE sequence.
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByteToWrite : number of bytes to write to the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void MX25L128_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
    u8 NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;

    Addr = WriteAddr % SPI_FLASH_PageSize;
    count = SPI_FLASH_PageSize - Addr;
    NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
    NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

    if (Addr == 0) /* WriteAddr is SPI_FLASH_PageSize aligned  */
    {
        if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
        {
            MX25L128_Page_Write(pBuffer, WriteAddr, NumByteToWrite);
        }
        else /* NumByteToWrite > SPI_FLASH_PageSize */
        {
            while (NumOfPage--)
            {
                MX25L128_Page_Write(pBuffer, WriteAddr,SPI_FLASH_PageSize);
                WriteAddr +=  SPI_FLASH_PageSize;
                pBuffer += SPI_FLASH_PageSize;
            }

            MX25L128_Page_Write(pBuffer, WriteAddr, NumOfSingle);
        }
    }
    else /* WriteAddr is not SPI_FLASH_PageSize aligned  */
    {
        if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
        {
            if (NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PageSize */
            {
                temp = NumOfSingle - count;

                MX25L128_Page_Write(pBuffer, WriteAddr, count);
                WriteAddr +=  count;
                pBuffer += count;

                MX25L128_Page_Write(pBuffer, WriteAddr, temp);
            }
            else
            {
                MX25L128_Page_Write(pBuffer, WriteAddr, NumByteToWrite);
            }
        }
        else /* NumByteToWrite > SPI_FLASH_PageSize */
        {
            NumByteToWrite -= count;
            NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
            NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

            MX25L128_Page_Write(pBuffer, WriteAddr, count);
            WriteAddr +=  count;
            pBuffer += count;

            while (NumOfPage--)
            {
                MX25L128_Page_Write(pBuffer, WriteAddr, SPI_FLASH_PageSize);
                WriteAddr +=  SPI_FLASH_PageSize;
                pBuffer += SPI_FLASH_PageSize;
            }

            if (NumOfSingle != 0)
            {
                MX25L128_Page_Write(pBuffer, WriteAddr, NumOfSingle);
            }
        }
    }
}


/*******************************************************************************
* Function Name  : MX25L128_Page_Write
* Description    : flash 一个page为256byte ,调用该函数时要确保写入的地址addr是已经擦除过的
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    written to the FLASH.
*                  - WriteAddr : FLASH's internal address to write to.
*                  - NumByte : number of bytes to write to the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void MX25L128_Page_Write(u8* pBuffer,u32 addr,u16 NumByte)
{

    MX25L128_WaitForWriteEnd();
    MX25L128_WREN();
    MX25_CS_LOW;
    MX25L128_WriteByte(0x02);
    MX25L128_WriteByte( (addr >> 16)&0xFF );
    MX25L128_WriteByte( (addr >> 8)&0xFF );
    MX25L128_WriteByte( addr&0xFF );

    if(NumByte > SPI_FLASH_PageSize)
    {
        NumByte = SPI_FLASH_PageSize;
    }

    while (NumByte--)
    {
        /* Send the current byte */
        MX25L128_WriteByte(* pBuffer);
        /* Point on the next byte to be written */
        pBuffer++;
    }
    MX25_CS_HIGH;
    MX25L128_WaitForWriteEnd();
}


/*******************************************************************************
* Function Name  : MX25L128_BufferRead
* Description    : 读取一页数据
* Input          : - pBuffer : pointer to the buffer  containing the data to be
*                    read to the FLASH.
*                  - addr : FLASH's internal address to read to.
*                  - NumByte : number of bytes to read to the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void MX25L128_BufferRead(u8* pBuffer,u32 addr,u16 NumByte)
{
//    MX25L128_WaitForWriteEnd();
    MX25_CS_LOW;
    MX25L128_WriteByte(0x03);
    MX25L128_WriteByte( (addr >> 16)&0xFF );
    MX25L128_WriteByte( (addr >> 8)&0xFF );
    MX25L128_WriteByte( addr&0xFF );
    while (NumByte--) /* while there is data to be read */
    {
        * pBuffer = MX25L128_ReadByte();
        pBuffer++;
    }
    MX25_CS_HIGH;
    MX25L128_WaitForWriteEnd();
}


/*******************************************************************************
* Function Name  : MX25L128_Word_Write
* Description    : 写入一个word
* Input          : - addr : FLASH's internal address to write to.
                    -word :the data to be read to the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void MX25L128_Word_Write(u32 addr,u32 word)
{

    MX25L128_WaitForWriteEnd();
    MX25L128_WREN();
    MX25_CS_LOW;
    MX25L128_WriteByte(0x02);
    MX25L128_WriteByte( (addr >> 16)&0xFF );
    MX25L128_WriteByte( (addr >> 8)&0xFF );
    MX25L128_WriteByte( addr&0xFF );

    MX25L128_WriteByte((u8)(word>>24));
    MX25L128_WriteByte((u8)(word>>16));
    MX25L128_WriteByte((u8)(word>>8));
    MX25L128_WriteByte((u8)word);

    MX25_CS_HIGH;
    MX25L128_WaitForWriteEnd();
}


/*******************************************************************************
* Function Name  : MX25L128_Word_read
* Description    : 读一个word
* Input          : - addr : FLASH's internal address to read to.
* Output         : None
* Return         : TempData 从Flash中读取到的数据
*******************************************************************************/
u32 MX25L128_Word_read(u32 addr)
{
    u8 TempDataArray[4];
    u32 TempData;
    MX25L128_WaitForWriteEnd();
    MX25_CS_LOW;
    MX25L128_WriteByte(0x03);
    MX25L128_WriteByte( (addr >> 16)&0xFF );
    MX25L128_WriteByte( (addr >> 8)&0xFF );
    MX25L128_WriteByte( addr&0xFF );

    TempDataArray[0] = MX25L128_ReadByte();
    TempDataArray[1] = MX25L128_ReadByte();
    TempDataArray[2] = MX25L128_ReadByte();
    TempDataArray[3] = MX25L128_ReadByte();
    MX25_CS_HIGH;
    MX25L128_WaitForWriteEnd();
    TempData = (TempDataArray[0]<<24)|(TempDataArray[1]<<16)|(TempDataArray[2]<<8)|(TempDataArray[3]);
    return TempData;
}


/*******************************************************************************
* Function Name  : MX25L128_ReadID
* Description    : 读取Flash的ID号
* Input          : None
* Output         : None
* Return         : TempID 从Flash中读取的ID号
*******************************************************************************/
u32 MX25L128_ReadID(void)
{
    u32 TempID;

    MX25_CS_LOW;
    MX25L128_WriteByte(0x9F);
    TempID = MX25L128_ReadByte();		            //Manufacturer Identification
    TempID = (TempID<<8)| MX25L128_ReadByte();	//Device Identification
    TempID = (TempID<<8)| MX25L128_ReadByte();
    MX25_CS_HIGH;
    return TempID;
}


/*******************************************************************************
* Function Name  : MX25L128_WriteByte
* Description    : 写一个Byte
* Input          : _dat 需要写入的数据
* Output         : None
* Return         : None
*******************************************************************************/
void MX25L128_WriteByte(unsigned char _dat)
{
    unsigned char i;//用于循环
    unsigned char Flag_Bit = 0x80;//数据写入从数据的高位（MSB）开始

    for(i = 0; i < 8; i++)
    {
        MX25_CLK_Low;
        if( (_dat & Flag_Bit) == 0 )//判断_dat的第Flag_Bit位上的数据
        {
            MX25_DI_Low;//输出低电平
        } else
        {
            MX25_DI_High;//输出高电平
        }
        Flag_Bit >>= 1;
        MX25_CLK_High;
    }
    MX25_CLK_Low;
}

/*******************************************************************************
* Function Name  : MX25L128_ReadByte
* Description    : 读取一个字节
* Input          : None
* Output         : None
* Return         : tmpData 读取的数据
*******************************************************************************/
u8 MX25L128_ReadByte(void)
{
    u8 i;
    u8 tmpData = 0;
    for(i = 0; i < 8; i++)
    {
        MX25_CLK_Low;
        if( MX25_Read_DO == 1 )
        {
            tmpData = (tmpData | (0x80>>i));
        }
        MX25_CLK_High;
    }
    MX25_CLK_Low;
    return tmpData;
}



/*******************************************************************************
* Function Name  : MX25L128_Chip_Erase
* Description    : 擦除整个芯片，全部写1.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MX25L128_Chip_Erase(void)
{
    MX25_CS_LOW;
    MX25L128_WriteByte(0x60);
    MX25_CS_HIGH;
    MX25L128_WaitForWriteEnd();
}


/*******************************************************************************
* Function Name  : MX25L128_Sector_Erase
* Description    : flash一个sector大小为4096（0xFFF），由16个page组成
*                  调用该函数时，要确保这16个page里的数据无用或者已经备份
* Input          : addr 需要擦除的扇区的地址
* Output         : None
* Return         : None
*******************************************************************************/
void MX25L128_Sector_Erase(unsigned long addr)
{
    MX25L128_WREN();
    MX25L128_WaitForWriteEnd();
    MX25_CS_LOW;
    MX25L128_WriteByte( 0x20 );
    MX25L128_WriteByte( (u8)(addr >> 16));
    MX25L128_WriteByte( (u8)(addr >> 8));
    MX25L128_WriteByte( (u8)addr);
    MX25_CS_HIGH;
    MX25L128_WaitForWriteEnd();
}


/*******************************************************************************
* Function Name  : MX25L128_WREN
* Description    : 写使能
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MX25L128_WREN(void)
{
    MX25_CS_LOW;
    MX25L128_WriteByte(0x06);
    MX25_CS_HIGH;
}


/*******************************************************************************
* Function Name  : MX25L128_WaitForWriteEnd
* Description    : 等待写结束
- 以下动作会使得标志位的的第一位WEL BIT清零
- Write Disable (WRDI) command completion
- Write Status Register (WRSR) command completion
- Page Program (PP, 4PP) command completion
- Continuously Program mode (CP) instruction completion
- Sector Erase (SE) command completion
- Block Erase (BE, BE32K) command completion
- Chip Erase (CE) command completion
- Single Block Lock/Unlock (SBLK/SBULK) instruction completion
- Gang Block Lock/Unlock (GBLK/GBULK) instruction completion
*******************************************************************************/
void MX25L128_WaitForWriteEnd(void)
{
    u8 FLASH_Status = 0;
    /* Select the FLASH: Chip Select low */
    MX25_CS_LOW;
    /* Send "Read Status Register" instruction */
    MX25L128_WriteByte(0x05);
    /* Loop as long as the memory is busy with a write cycle */
    do
    {
        FLASH_Status = MX25L128_ReadByte();
    }
    while ((FLASH_Status & 0x01) == SET); /* Write in progress */
    /* Deselect the FLASH: Chip Select high */
    MX25_CS_HIGH;
}
/****************************************END OF FILE*******************************************************/





