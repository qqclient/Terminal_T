/********************************************************************************
* File Name          : usb_endp.c
* Author             : CSYXJ
* Version            : V2.2.1
* Date               : 09/22/2008
* Description        : Endpoint routines
*******************************************************************************/

#include "usb_lib.h"
#include "usb_istr.h"
#include "usb_desc.h"
#include <string.h>

unsigned char buffer_out[USB_DATA_SIZE];
unsigned char buffer_in[USB_DATA_SIZE];
unsigned int count_out = 0;
unsigned int count_in = 0;

extern void Debug_Analysis(unsigned char* src, unsigned short len);
    
/*******************************************************************************
* Function Name  : EP1_OUT_Callback.
* Description    : EP1 OUT Callback Routine.
*******************************************************************************/
void EP1_OUT_Callback(void)
{
    count_in = GetEPRxCount(ENDP1);
    PMAToUserBufferCopy(buffer_in, ENDP1_RXADDR, count_in);
    
    Debug_Analysis( buffer_in, count_in );
    
    SetEPRxStatus(ENDP1, EP_RX_VALID);
}

/*******************************************************************************
* Function Name  : EP1_IN_Callback
* Description    : EP1 IN Callback Routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_IN_Callback(void)
{
}

