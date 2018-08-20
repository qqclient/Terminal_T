#ifndef __FLASH_DRIVER_H
#define __FLASH_DRIVER_H

/*  functions ---------------------------------------------------------*/
void MX25L128_BufferWrite(unsigned char* pBuffer, unsigned int WriteAddr, unsigned short int NumByteToWrite);
void MX25L128_BufferRead(unsigned char* pBuffer, unsigned int ReadAddr, unsigned short int NumByteToRead);
extern void MX25L128_WREN(void);
extern void MX25L128_Sector_Erase(unsigned long addr);
extern void MX25L128_Page_Write(unsigned char* pBuffer,unsigned int addr,unsigned short int NumByte);
extern void MX25L128_Chip_Erase(void);
extern void MX25L128_WriteByte(unsigned char _dat);
extern unsigned char MX25L128_ReadByte(void);
extern unsigned int  MX25L128_ReadID(void);
extern void MX25L128_WaitForWriteEnd(void);
extern void MX25L128_Word_Write(unsigned int addr,unsigned int word);
extern unsigned int MX25L128_Word_read(unsigned int addr);

#endif




