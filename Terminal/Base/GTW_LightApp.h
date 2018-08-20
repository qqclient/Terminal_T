#ifndef GTW_LIGHTAPP_H
#define GTW_LIGHTAPP_H

unsigned short Get_SendCarNo(unsigned char* dsr);
unsigned short Get_ReadLamp(unsigned char* src, unsigned char* dsr);

unsigned short Get_ReadSN(unsigned char* src, unsigned char* dsr);
unsigned short Get_LightChange(unsigned char* src, unsigned char* dsr);
unsigned short GTW_LightAPP(unsigned char* src, unsigned char* dsr);

#endif
