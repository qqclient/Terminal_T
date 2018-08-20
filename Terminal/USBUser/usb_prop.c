/********************************************************************************
* File Name          : usb_prop.c
* Author             : CSYXJ
* Version            : V2.2.1
* Date               : 09/22/2008
* Description        : All processings related to Custom HID Demo
******************************************************************************/

#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "usb_pwr.h"

u32 ProtocolValue;

unsigned char Request = 0;

LINE_CODING linecoding =
  {
    115200,
    0x00,
    0x00,
    0x08
  };
/* -------------------------------------------------------------------------- */
/*  Structures initializations */
/* -------------------------------------------------------------------------- */

DEVICE Device_Table =
  {
    EP_NUM,
    1
  };

DEVICE_PROP Device_Property =
  {
    USB_init,
    USB_Reset,
    USB_Status_In,
    USB_Status_Out,
    USB_Data_Setup,
    USB_NoData_Setup,
    USB_Get_Interface_Setting,
    USB_GetDeviceDescriptor,
    USB_GetConfigDescriptor,
    USB_GetStringDescriptor,
    0,
    0x40
  };
USER_STANDARD_REQUESTS User_Standard_Requests =
  {
    USB_GetConfiguration,
    USB_SetConfiguration,
    USB_GetInterface,
    USB_SetInterface,
    USB_GetStatus,
    USB_ClearFeature,
    USB_SetEndPointFeature,
    USB_SetDeviceFeature,
    USB_SetDeviceAddress
  };

ONE_DESCRIPTOR Device_Descriptor =
  {
    (unsigned char*)USB_DeviceDescriptor,
    USB_SIZ_DEVICE_DESC
  };

ONE_DESCRIPTOR Config_Descriptor =
  {
    (unsigned char*)USB_ConfigDescriptor,
    USB_SIZ_CONFIG_DESC
  };

ONE_DESCRIPTOR String_Descriptor[4] =
  {
    {(unsigned char*)USB_StringLangID, USB_SIZ_STRING_LANGID},
    {(unsigned char*)USB_StringVendor, USB_SIZ_STRING_VENDOR},
    {(unsigned char*)USB_StringProduct, USB_SIZ_STRING_PRODUCT},
    {(unsigned char*)USB_StringSerial, USB_SIZ_STRING_SERIAL}
  };

/*******************************************************************************
* Function Name  : Get_SerialNum.
* Description    : Create the serial number string descriptor.
*******************************************************************************/
void Get_SerialNum(void)
{
//  u32 Device_Serial0, Device_Serial1, Device_Serial2;
//  
//  Device_Serial0 = *(u32*)(0x1FFFF7E8);
//  Device_Serial1 = *(u32*)(0x1FFFF7EC);
//  Device_Serial2 = *(u32*)(0x1FFFF7F0);
}

/*******************************************************************************
* Function Name  : USB_init.
* Description    : Custom HID init routine.
*******************************************************************************/
void USB_init(void)
{
  /* Update the serial number string descriptor with the data from the unique ID*/
  Get_SerialNum();
    
  pInformation->Current_Configuration = 0;
  /* Connect the device */
  PowerOn();
  /* USB interrupts initialization */
  _SetISTR(0);               /* clear pending interrupts */
  wInterrupt_Mask = IMR_MSK;
  _SetCNTR(wInterrupt_Mask); /* set interrupts mask */

  bDeviceState = UNCONNECTED;
}

/*******************************************************************************
* Function Name  : USB_Reset.
* Description    : Custom HID reset routine.
*******************************************************************************/
void USB_Reset(void)
{
  pInformation->Current_Configuration = 0;

  pInformation->Current_Feature = USB_ConfigDescriptor[7];

  pInformation->Current_Interface = 0;
  SetBTABLE(BTABLE_ADDRESS);

  SetEPType(ENDP0, EP_CONTROL);
  SetEPTxStatus(ENDP0, EP_TX_STALL);
  SetEPRxAddr(ENDP0, ENDP0_RXADDR);
  SetEPTxAddr(ENDP0, ENDP0_TXADDR);
  Clear_Status_Out(ENDP0);
  SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
  SetEPRxValid(ENDP0);

  SetEPType(ENDP1, EP_BULK);
  SetEPTxAddr(ENDP1, ENDP1_TXADDR);
  SetEPRxAddr(ENDP1, ENDP1_RXADDR);
  SetEPTxCount(ENDP1, USB_DATA_SIZE);
  SetEPRxCount(ENDP1, USB_DATA_SIZE);
  SetEPTxStatus(ENDP1, EP_TX_NAK);
  SetEPRxStatus(ENDP1, EP_RX_VALID);

  SetEPType(ENDP2, EP_INTERRUPT);
  SetEPTxAddr(ENDP2, ENDP2_TXADDR);
  SetEPRxStatus(ENDP2, EP_RX_DIS);
  SetEPTxStatus(ENDP2, EP_TX_NAK);

  SetDeviceAddress(0);

  bDeviceState = ATTACHED;
}

/*******************************************************************************
* Function Name  : USB_SetConfiguration.
* Description    : Udpade the device state to configured and command the ADC 
*                  conversion.
*******************************************************************************/
void USB_SetConfiguration(void)
{
  if (pInformation->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;
  }
}

/*******************************************************************************
* Function Name  : USB_SetDeviceAddress.
* Description    : Udpade the device state to addressed.
*******************************************************************************/
void USB_SetDeviceAddress (void)
{
  bDeviceState = ADDRESSED;
}

/*******************************************************************************
* Function Name  : USB_Status_In.
* Description    : Joystick status IN routine.
*******************************************************************************/
void USB_Status_In(void)
{
  if (Request == SET_LINE_CODING)
  {
    Request = 0;
  }
}

/*******************************************************************************
* Function Name  : USB_Status_Out
* Description    : Joystick status OUT routine.
*******************************************************************************/
void USB_Status_Out (void)
{
}

/*******************************************************************************
* Function Name  : USB_Data_Setup
* Description    : Handle the data class specific requests.
*******************************************************************************/
RESULT USB_Data_Setup(unsigned char RequestNo)
{
  unsigned char *(*CopyRoutine)(u16);

  CopyRoutine = NULL;

  if (RequestNo == GET_LINE_CODING)
  {
    if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
      CopyRoutine = USB_GetLineCoding;
  }
  else if (RequestNo == SET_LINE_CODING)
  {
    if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
      CopyRoutine = USB_SetLineCoding;

    Request = SET_LINE_CODING;
  }

  if (CopyRoutine == NULL)
  {
    return USB_UNSUPPORT;
  }

  pInformation->Ctrl_Info.CopyData = CopyRoutine;
  pInformation->Ctrl_Info.Usb_wOffset = 0;
  (*CopyRoutine)(0);
  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : USB_NoData_Setup
* Description    : handle the no data class specific requests
*******************************************************************************/
RESULT USB_NoData_Setup(unsigned char RequestNo)
{
  if (Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT))
  {
    if (RequestNo == SET_COMM_FEATURE)
    {
      return USB_SUCCESS;
    }
    else if (RequestNo == SET_CONTROL_LINE_STATE)
    {
      return USB_SUCCESS;
    }
  }

  return USB_UNSUPPORT;
}

/*******************************************************************************
* Function Name  : USB_GetDeviceDescriptor.
* Description    : Gets the device descriptor.
*******************************************************************************/
unsigned char *USB_GetDeviceDescriptor(u16 Length)
{
  return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

/*******************************************************************************
* Function Name  : USB_GetConfigDescriptor.
* Description    : Gets the configuration descriptor.
*******************************************************************************/
unsigned char *USB_GetConfigDescriptor(u16 Length)
{
  return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

/*******************************************************************************
* Function Name  : USB_GetStringDescriptor
* Description    : Gets the string descriptors according to the needed index
*******************************************************************************/
unsigned char *USB_GetStringDescriptor(u16 Length)
{
  unsigned char wValue0 = pInformation->USBwValue0;
  if (wValue0 > 4)
  {
    return NULL;
  }
  else 
  {
    return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
  }
}

/*******************************************************************************
* Function Name  : USB_Get_Interface_Setting.
* Description    : tests the interface and the alternate setting according to the
*                  supported one.
*******************************************************************************/
RESULT USB_Get_Interface_Setting(unsigned char Interface, unsigned char AlternateSetting)
{
  if (AlternateSetting > 0)
  {
    return USB_UNSUPPORT;
  }
  else if (Interface > 1)
  {
    return USB_UNSUPPORT;
  }
  return USB_SUCCESS;
}

/*******************************************************************************
* Function Name  : USB_GetLineCoding.
* Description    : send the linecoding structure to the PC host.
*******************************************************************************/
unsigned char *USB_GetLineCoding(u16 Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = sizeof(linecoding);
    return NULL;
  }
  return(unsigned char *)&linecoding;
}

/*******************************************************************************
* Function Name  : USB_SetLineCoding.
* Description    : Set the linecoding structure fields.
*******************************************************************************/
unsigned char *USB_SetLineCoding(u16 Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = sizeof(linecoding);
    return NULL;
  }
  return(unsigned char *)&linecoding;
}
