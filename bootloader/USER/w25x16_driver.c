/******************** (C) COPYRIGHT soarsky ********************
* File Name          :
* Author             :
* Version            :
* Date               :
* Description        : Flash�������ǣ�д����ֻ�ܽ�1дΪ0���������ݾ���д1
*                      �����������������ݵ�falsh��д���µ����ݣ�������Ȳ�����
*                      ����Flash�ڲ�����ʱ������������
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/

#ifdef __COLLECTOR__

#include "w25x16_driver.h"

/*---------------------------------------------------------------------------*/


#define FLASH_CS_LOW    GPIO_WriteBit(FLASH_CS_PORT ,FLASH_CS ,Bit_RESET);
#define FLASH_CS_HIGH   GPIO_WriteBit(FLASH_CS_PORT ,FLASH_CS ,Bit_SET);
#define FLASH_CLK_Low   GPIO_WriteBit(FLASH_CLK_PORT,FLASH_CLK,Bit_RESET);
#define FLASH_CLK_High  GPIO_WriteBit(FLASH_CLK_PORT,FLASH_CLK,Bit_SET);
#define FLASH_DI_Low    GPIO_WriteBit(FLASH_DI_PORT ,FLASH_DI ,Bit_RESET);
#define FLASH_DI_High   GPIO_WriteBit(FLASH_DI_PORT ,FLASH_DI ,Bit_SET);
#define FLASH_WP_Low		GPIO_WriteBit(FLASH_WP_PORT ,FLASH_WP ,Bit_RESET)
#define FLASH_WP_High	  GPIO_WriteBit(FLASH_WP_PORT ,FLASH_WP ,Bit_SET)
#define FLASH_Read_DO   GPIO_ReadInputDataBit(FLASH_DO_PORT, FLASH_DO)
#define SPI_FLASH_PageSize  256

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
void FLASH_BufferWrite(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
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
            FLASH_Page_Write(pBuffer, WriteAddr, NumByteToWrite);
        }
        else /* NumByteToWrite > SPI_FLASH_PageSize */
        {
            while (NumOfPage--)
            {
                FLASH_Page_Write(pBuffer, WriteAddr,SPI_FLASH_PageSize);
                WriteAddr +=  SPI_FLASH_PageSize;
                pBuffer += SPI_FLASH_PageSize;
            }

            FLASH_Page_Write(pBuffer, WriteAddr, NumOfSingle);
        }
    }
    else /* WriteAddr is not SPI_FLASH_PageSize aligned  */
    {
        if (NumOfPage == 0) /* NumByteToWrite < SPI_FLASH_PageSize */
        {
            if (NumOfSingle > count) /* (NumByteToWrite + WriteAddr) > SPI_FLASH_PageSize */
            {
                temp = NumOfSingle - count;
                FLASH_Page_Write(pBuffer, WriteAddr, count);
                WriteAddr +=  count;
                pBuffer += count;

                FLASH_Page_Write(pBuffer, WriteAddr, temp);
            }
            else
            {
                FLASH_Page_Write(pBuffer, WriteAddr, NumByteToWrite);
            }
        }
        else /* NumByteToWrite > SPI_FLASH_PageSize */
        {
            NumByteToWrite -= count;
            NumOfPage =  NumByteToWrite / SPI_FLASH_PageSize;
            NumOfSingle = NumByteToWrite % SPI_FLASH_PageSize;

            FLASH_Page_Write(pBuffer, WriteAddr, count);
            WriteAddr +=  count;
            pBuffer += count;

            while (NumOfPage--)
            {
                FLASH_Page_Write(pBuffer, WriteAddr, SPI_FLASH_PageSize);
                WriteAddr +=  SPI_FLASH_PageSize;
                pBuffer += SPI_FLASH_PageSize;
            }

            if (NumOfSingle != 0)
            {
                FLASH_Page_Write(pBuffer, WriteAddr, NumOfSingle);
            }
        }
    }
}

/*******************************************************************************
* Function Name  :
* Description    : flash һ��pageΪ256byte ,���øú���ʱҪȷ��д��ĵ�ַaddr���Ѿ���������
*******************************************************************************/
void FLASH_Page_Write(u8* pBuffer,u32 addr,u16 NumByte)
{
    FLASH_WREN();
    FLASH_CS_LOW;
    FLASH_WriteByte(0x02);
    FLASH_WriteByte( (addr >> 16)&0xFF );
    FLASH_WriteByte( (addr >> 8)&0xFF );
    FLASH_WriteByte( addr&0xFF );
    while (NumByte--)
    {
        /* Send the current byte */
        FLASH_WriteByte(* pBuffer);
        /* Point on the next byte to be written */
        pBuffer++;
    }
    FLASH_CS_HIGH;
    FLASH_WaitForWriteEnd();
}


/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void FLASH_BufferRead(u8* pBuffer,u32 addr,u16 NumByte)
{
    FLASH_CS_LOW;
    FLASH_WriteByte(0x03);
    FLASH_WriteByte( (addr >> 16)&0xFF );
    FLASH_WriteByte( (addr >> 8)&0xFF );
    FLASH_WriteByte( addr&0xFF );
    while (NumByte--) /* while there is data to be read */
    {
        * pBuffer = FLASH_ReadByte();
        pBuffer++;
    }
    FLASH_CS_HIGH;
}





/*******************************************************************************
* Function Name  : FLASH_Word_Write
* Description    : д��һ��word
* Input          : - addr : FLASH's internal address to write to.
                    -word :the data to be read to the FLASH.
* Output         : None
* Return         : None
*******************************************************************************/
void FLASH_Word_Write(u32 addr,u32 word)
{

    FLASH_WaitForWriteEnd();
    FLASH_WREN();
    FLASH_CS_LOW;
    FLASH_WriteByte(0x02);
    FLASH_WriteByte( (addr >> 16)&0xFF );
    FLASH_WriteByte( (addr >> 8)&0xFF );
    FLASH_WriteByte( addr&0xFF );

    FLASH_WriteByte((u8)(word>>24));
    FLASH_WriteByte((u8)(word>>16));
    FLASH_WriteByte((u8)(word>>8));
    FLASH_WriteByte((u8)word);

    FLASH_CS_HIGH;
    FLASH_WaitForWriteEnd();
}


/*******************************************************************************
* Function Name  : FLASH_Word_read
* Description    : ��һ��word
* Input          : - addr : FLASH's internal address to read to.
* Output         : None
* Return         : TempData ��Flash�ж�ȡ��������
*******************************************************************************/
u32 FLASH_Word_read(u32 addr)
{
    u8  TempDataArray[4];
    u32 TempData;
    FLASH_WaitForWriteEnd();
    FLASH_CS_LOW;
    FLASH_WriteByte(0x03);
    FLASH_WriteByte( (addr >> 16)&0xFF );
    FLASH_WriteByte( (addr >> 8)&0xFF );
    FLASH_WriteByte( addr&0xFF );

    TempDataArray[0] = FLASH_ReadByte();
    TempDataArray[1] = FLASH_ReadByte();
    TempDataArray[2] = FLASH_ReadByte();
    TempDataArray[3] = FLASH_ReadByte();
    FLASH_CS_HIGH;
    FLASH_WaitForWriteEnd();
    TempData = (TempDataArray[0]<<24)|(TempDataArray[1]<<16)|(TempDataArray[2]<<8)|(TempDataArray[3]);
    return TempData;
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
u32 FLASH_ReadID(void)
{
    u32 TempID;

    FLASH_CS_LOW;
    FLASH_WriteByte(0x9F);
    TempID = FLASH_ReadByte();		            //Manufacturer Identification
    TempID = (TempID<<8)| FLASH_ReadByte();	//Device Identification
    TempID = (TempID<<8)| FLASH_ReadByte();
    FLASH_CS_HIGH;
    return TempID;
}


/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
void FLASH_WriteByte(unsigned char _dat)
{
    unsigned char i;//����ѭ��
    unsigned char Flag_Bit = 0x80;//����д������ݵĸ�λ��MSB����ʼ

    for(i = 0; i < 8; i++)
    {
        FLASH_CLK_Low;
        if( (_dat & Flag_Bit) == 0 )//�ж�_dat�ĵ�Flag_Bitλ�ϵ�����
        {
            FLASH_DI_Low;//����͵�ƽ
        } else
        {
            FLASH_DI_High;//����ߵ�ƽ
        }
        Flag_Bit >>= 1;
        FLASH_CLK_High;
    }
    FLASH_CLK_Low;
}

/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
u8 FLASH_ReadByte(void)
{
    u8 i;
    u8 tmpData = 0;
    for(i = 0; i < 8; i++)
    {
        FLASH_CLK_Low;
        if( FLASH_Read_DO == 1 )
        {
            tmpData = (tmpData | (0x80>>i));
        }
        FLASH_CLK_High;
    }
    FLASH_CLK_Low;
    return tmpData;
}



/*******************************************************************************
* Function Name  :
* Description    : ��������оƬ��ȫ��д1.
*******************************************************************************/
void FLASH_Chip_Erase(void)
{
    FLASH_WREN();
    FLASH_CS_LOW;
    FLASH_WriteByte(0xC7);
    FLASH_CS_HIGH;
    FLASH_WaitForWriteEnd();
}


/*******************************************************************************
* Function Name  : FLASH_Block_Erase
* Description    : ����һ��Block 64k
*******************************************************************************/
void FLASH_Block_Erase(unsigned int addr)
{
    FLASH_WREN();
    FLASH_CS_LOW;
    FLASH_WriteByte(0xD8);
    FLASH_WriteByte( (addr >> 16)&0xFF );
    FLASH_WriteByte( (addr >> 8)&0xFF );
    FLASH_WriteByte( addr&0xFF );
    FLASH_CS_HIGH;
    FLASH_WaitForWriteEnd();
}

/*******************************************************************************
* Function Name  : FLASH_Sector_Erase
* Description    : flashһ��sector��СΪ4096��0xFFF������16��page���
*                  ���øú���ʱ��Ҫȷ����16��page����������û����Ѿ�����
*******************************************************************************/
void FLASH_Sector_Erase(unsigned int addr)
{
    FLASH_WREN();
    FLASH_CS_LOW;
    FLASH_WriteByte(0x20);
    FLASH_WriteByte( (addr >> 16)&0xFF );
    FLASH_WriteByte( (addr >> 8)&0xFF );
    FLASH_WriteByte( addr&0xFF );
    FLASH_CS_HIGH;
    FLASH_WaitForWriteEnd();
}


/*******************************************************************************
* Function Name  : FLASH_WREN
* Description    :
*******************************************************************************/
void FLASH_WREN(void)
{
    FLASH_CS_LOW;
    FLASH_WriteByte(0x06);
    FLASH_CS_HIGH;
}


/*******************************************************************************
* Function Name  :
* Description    :
- ���¶�����ʹ�ñ�־λ�ĵĵ�һλWEL BIT����
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
void FLASH_WaitForWriteEnd(void)
{
    u8 FLASH_Status = 0;
    /* Select the FLASH: Chip Select low */
    FLASH_CS_LOW;
    /* Send "Read Status Register" instruction */
    FLASH_WriteByte(0x05);
    /* Loop as long as the memory is busy with a write cycle */
    do
    {
        IWDG_ReloadCounter();
        FLASH_Status = FLASH_ReadByte();
    }
    while ((FLASH_Status & 0x01) == SET); /* Write in progress */
    /* Deselect the FLASH: Chip Select high */
    FLASH_CS_HIGH;
}
/****************************************END OF FILE*******************************************************/

#endif



