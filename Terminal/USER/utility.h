#ifndef __UTILITY_H
#define __UTILITY_H

#include <stdio.h>
#include "stm32f10x.h"

extern volatile unsigned int TimingDelay;

u8 ReadDataBit(u8 data,u8 bit);
u8 ReadData32Bit(u32 data,u32 bit);

void Usart_SendByte( USART_TypeDef * pUSARTx, u8 ch );
void Usart_SendNByte( USART_TypeDef * pUSARTx, u8 *array,u16 len );
void Usart_SendString( USART_TypeDef * pUSARTx, char *str);
int  fputc(int ch, FILE *f);
void Delay_ms(volatile u32 nTime);
void Delay_us(volatile u32 nTime);
void WordToCharArray(u32 Word,u8 *CharArray);

//unsigned short CRC16(unsigned char *ptr, unsigned short len);  // crc16
unsigned char CheckSum(unsigned char* src, unsigned short len);
unsigned char Asc2Byte(char* src, unsigned char len);
unsigned short Asc2Word(char* src, unsigned char len);

unsigned char Byte2Asc(unsigned char* src, unsigned char len);
unsigned char Bcd2Deci(unsigned char* asc,unsigned char* bcd);
unsigned char Str2BcdDriverNo(unsigned char* str,unsigned char* bcd);
unsigned char Year2Bcd(u16 dsr,unsigned char * src);
unsigned char Byte2Bcd(u8 dsr);
unsigned char Str2BcdTime(unsigned char* str,unsigned char* bcd,unsigned char len);
float Asc2Float(char* src, unsigned char len);

unsigned char IsDayOrNight(void);
void  PhoneAlign( unsigned char* src, unsigned char width );
void SystemReset(void);
#endif



