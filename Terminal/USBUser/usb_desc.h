/********************************************************************************
* File Name          : usb_desc.h
* Author             : CSYXJ
* Version            : V2.2.1
* Date               : 09/22/2008
* Description        : Descriptor Header for Custom HID Demo
*******************************************************************************/

#ifndef __USB_DESC_H
#define __USB_DESC_H

#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05

#define HID_DESCRIPTOR_TYPE                     0x21
#define USB_SIZ_HID_DESC                  		0x09
#define USB_OFF_HID_DESC                  		0x12

#define USB_SIZ_DEVICE_DESC               		18
#define USB_SIZ_CONFIG_DESC               		67

#define USB_SIZ_STRING_LANGID             		4
#define USB_SIZ_STRING_VENDOR             		38
#define USB_SIZ_STRING_PRODUCT            		32
#define USB_SIZ_STRING_SERIAL             		26
#define USB_SIZ_REPORT_DESC               		38//33//162

#define STANDARD_ENDPOINT_DESC_SIZE             0x09
#define USB_DATA_SIZE							64  // Report Packet Size
#define USB_INT_SIZE               				8

/* Exported functions ------------------------------------------------------- */
extern const unsigned char USB_DeviceDescriptor[USB_SIZ_DEVICE_DESC];
extern const unsigned char USB_ConfigDescriptor[USB_SIZ_CONFIG_DESC];
extern const unsigned char USB_StringLangID[USB_SIZ_STRING_LANGID];
extern const unsigned char USB_StringVendor[USB_SIZ_STRING_VENDOR];
extern const unsigned char USB_StringProduct[USB_SIZ_STRING_PRODUCT];
extern unsigned char USB_StringSerial[USB_SIZ_STRING_SERIAL];

#endif /* __USB_DESC_H */
