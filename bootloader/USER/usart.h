#ifndef __USAR_H
#define __USAR_H

#include "stm32f10x.h"

//#define __DEBUG


#ifdef __DEBUG
void USART1_Config(void);
void Usart_SendByte( USART_TypeDef * pUSARTx, u8 ch );
void Usart_SendNByte( USART_TypeDef * pUSARTx, u8 *array, u8 len );
void Usart_SendString( USART_TypeDef * pUSARTx, char *str);
#endif


#endif

