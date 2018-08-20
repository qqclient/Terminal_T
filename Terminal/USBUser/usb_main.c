/*******************************************************************************
* File Name          : main.c
* Author             : CSYXJ
* Version            : V2.2.1
* Date               : 09/22/2008
* Description        : Custom HID demo main file
*******************************************************************************/
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "usb_endp.h"
#include "usb_istr.h"
#include "usb_main.h"

extern void USB_GPIO_Config(void);
extern void USB_Interrupts_Config(void);


/*******************************************************************************
* Function Name  : USB_Init.
* Description    : USB_Init routine.
*******************************************************************************/
void MyUSB_Init(void)
{
    USB_GPIO_Config();
    USB_Interrupts_Config();
	Set_USBClock();
	USB_Init();
}


/*******************************************************************************
* Function Name  : USB_LP_CAN1_RX0_IRQHandler
* Description    : 
*******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
    USB_Istr();
}

/*******************************************************************************
* Function Name  : usb_SendBuf
* Description    : 
*******************************************************************************/
void usb_SendBuf(unsigned char* src, unsigned char len)
{
    UserToPMABufferCopy( src, ENDP1_TXADDR, len);
    SetEPTxCount( ENDP1, len );
    SetEPTxValid( ENDP1 ); 
}
