
#define USB_DATA_SIZE   64 // Report Packet Size

// 外部定义的函数
// void USB_GPIO_Config(void);
// void USB_Cable_Config (FunctionalState NewState);
// void Debug_Analysis(unsigned char* src, unsigned short len);

void MyUSB_Init(void);
void usb_SendBuf(unsigned char* _pTxBuf, unsigned char _ucLen);
