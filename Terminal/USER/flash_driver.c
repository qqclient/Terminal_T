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
* Description    : flash һ��pageΪ256byte ,���øú���ʱҪȷ��д��ĵ�ַaddr���Ѿ���������
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
* Description    : ��ȡһҳ����
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
* Description    : д��һ��word
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
* Description    : ��һ��word
* Input          : - addr : FLASH's internal address to read to.
* Output         : None
* Return         : TempData ��Flash�ж�ȡ��������
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
* Description    : ��ȡFlash��ID��
* Input          : None
* Output         : None
* Return         : TempID ��Flash�ж�ȡ��ID��
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
* Description    : дһ��Byte
* Input          : _dat ��Ҫд�������
* Output         : None
* Return         : None
*******************************************************************************/
void MX25L128_WriteByte(unsigned char _dat)
{
    unsigned char i;//����ѭ��
    unsigned char Flag_Bit = 0x80;//����д������ݵĸ�λ��MSB����ʼ

    for(i = 0; i < 8; i++)
    {
        MX25_CLK_Low;
        if( (_dat & Flag_Bit) == 0 )//�ж�_dat�ĵ�Flag_Bitλ�ϵ�����
        {
            MX25_DI_Low;//����͵�ƽ
        } else
        {
            MX25_DI_High;//����ߵ�ƽ
        }
        Flag_Bit >>= 1;
        MX25_CLK_High;
    }
    MX25_CLK_Low;
}

/*******************************************************************************
* Function Name  : MX25L128_ReadByte
* Description    : ��ȡһ���ֽ�
* Input          : None
* Output         : None
* Return         : tmpData ��ȡ������
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
* Description    : ��������оƬ��ȫ��д1.
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
* Description    : flashһ��sector��СΪ4096��0xFFF������16��page���
*                  ���øú���ʱ��Ҫȷ����16��page����������û����Ѿ�����
* Input          : addr ��Ҫ�����������ĵ�ַ
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
* Description    : дʹ��
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
* Description    : �ȴ�д����
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





