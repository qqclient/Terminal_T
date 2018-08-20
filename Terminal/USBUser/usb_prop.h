/*******************************************************************************
* File Name          : usb_prop.h
* Author             : CSYXJ
* Version            : V2.2.1
* Date               : 09/22/2008
* Description        : All processings related to Custom HID demo
*******************************************************************************/
#ifndef __USB_PROP_H
#define __USB_PROP_H

typedef enum _HID_REQUESTS
{
  GET_REPORT = 1,
  GET_IDLE,
  GET_PROTOCOL,

  SET_REPORT = 9,
  SET_IDLE,
  SET_PROTOCOL
} HID_REQUESTS;

typedef struct
{
  unsigned int  bitrate;
  unsigned char format;
  unsigned char paritytype;
  unsigned char datatype;
}LINE_CODING;

void Get_SerialNum(void);
void USB_init(void);
void USB_Reset(void);
void USB_SetConfiguration(void);
void USB_SetDeviceAddress (void);
void USB_Status_In (void);
void USB_Status_Out (void);
RESULT USB_Data_Setup(unsigned char);
RESULT USB_NoData_Setup(unsigned char);
RESULT USB_Get_Interface_Setting(unsigned char Interface, unsigned char AlternateSetting);
unsigned char *USB_GetDeviceDescriptor(u16 );
unsigned char *USB_GetConfigDescriptor(u16);
unsigned char *USB_GetStringDescriptor(u16);
unsigned char *USB_GetLineCoding(u16 Length);
unsigned char *USB_SetLineCoding(u16 Length);

/* Exported define -----------------------------------------------------------*/
#define USB_GetConfiguration          NOP_Process
//#define USB_SetConfiguration          NOP_Process
#define USB_GetInterface              NOP_Process
#define USB_SetInterface              NOP_Process
#define USB_GetStatus                 NOP_Process
#define USB_ClearFeature              NOP_Process
#define USB_SetEndPointFeature        NOP_Process
#define USB_SetDeviceFeature          NOP_Process
//#define USB_SetDeviceAddress          NOP_Process

#define SEND_ENCAPSULATED_COMMAND   0x00
#define GET_ENCAPSULATED_RESPONSE   0x01
#define SET_COMM_FEATURE            0x02
#define GET_COMM_FEATURE            0x03
#define CLEAR_COMM_FEATURE          0x04
#define SET_LINE_CODING             0x20
#define GET_LINE_CODING             0x21
#define SET_CONTROL_LINE_STATE      0x22
#define SEND_BREAK                  0x23


#define REPORT_DESCRIPTOR                  0x22

#endif /* __USB_PROP_H */
