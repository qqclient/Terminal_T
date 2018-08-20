/******************** (C) COPYRIGHT soarsky ************************************
* File Name          : usb_desc.c
* Author             : CSYXJ
* Version            : V2.2.1
* Date               : 09/22/2008
* Description        : DescriptorsforCustomHIDDemo
*******************************************************************************/

#include "usb_lib.h"
#include "usb_desc.h"

#define LOBYTE(x)((unsigned char)(x & 0x00FF))
#define HIBYTE(x)((unsigned char)((x & 0xFF00)>>8))

//USBStandardDeviceDescriptor
const unsigned char USB_DeviceDescriptor[]=
{
    0x12,
    USB_DEVICE_DESCRIPTOR_TYPE,
    0x00,0x02,
    0x02,
    0x02,
    0x00,
    0x40,
    0x83, 0x04,
    0x40, 0x57,
    0x00, 0x02,
    1,
    2,
    3,
    0x01
};


const unsigned char USB_ConfigDescriptor[]=
{
    0x09,
    USB_CONFIGURATION_DESCRIPTOR_TYPE,
    USB_SIZ_CONFIG_DESC,
    0x00,
    0x02,
    0x01,
    0x00,
    0xC0,
    250,

    0x09,
    USB_INTERFACE_DESCRIPTOR_TYPE,
    0x00,
    0x00,
    0x01,
    0x02,
    0x02,
    0x01,
    0x00,
    0x05,
    0x24,
    0x00,
    0x10,
    0x01,
    0x05,
    0x24,
    0x01,
    0x00,
    0x01,
    0x04,
    0x24,
    0x02,
    0x02,
    0x05,
    0x24,
    0x06,
    0x00,
    0x01,
    0x07,
    USB_ENDPOINT_DESCRIPTOR_TYPE,
    0x82,
    0x03,
    LOBYTE(USB_DATA_SIZE),
    HIBYTE(USB_DATA_SIZE),
    0x10,

    0x09,
    USB_INTERFACE_DESCRIPTOR_TYPE,
    0x01,
    0x00,
    0x02,
    0x0A,
    0x00,
    0x00,
    0x00,
    0x07,
    USB_ENDPOINT_DESCRIPTOR_TYPE,
    0x01,
    0x02,
    LOBYTE(USB_DATA_SIZE),
    HIBYTE(USB_DATA_SIZE),
    0x00,
    0x07,
    USB_ENDPOINT_DESCRIPTOR_TYPE,
    0x81,
    0x02,
    LOBYTE(USB_DATA_SIZE),
    HIBYTE(USB_DATA_SIZE),
    0x00
};

const unsigned char USB_StringLangID[]=
{
    USB_SIZ_STRING_LANGID,
    USB_STRING_DESCRIPTOR_TYPE,
    0x09, 0x04
};

const unsigned char USB_StringVendor[]=
{
    USB_SIZ_STRING_VENDOR,
    USB_STRING_DESCRIPTOR_TYPE,
    'S',0,'o',0,'a',0,'r',0,'S',0,'k',0,'y',0,'-',0,
    'e',0,' ',0,'D',0,'e',0,'v',0,'i',0,'c',0,'e',0
};

const unsigned char USB_StringProduct[]=
{
    USB_SIZ_STRING_PRODUCT,
    USB_STRING_DESCRIPTOR_TYPE,
    'S',0,'o',0,'a',0,'r',0,' ',0,'D',0,'e',0,'v',0,
    'i',0,'c',0,'e',0,'s',0
};

unsigned char USB_StringSerial[]=
{
    USB_SIZ_STRING_SERIAL,
    USB_STRING_DESCRIPTOR_TYPE,
    'S',0,'o',0,'a',0,'r',0,'-',0,'G',0,'T',0,'W',0
};
