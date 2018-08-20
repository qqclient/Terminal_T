#ifndef __DEBUG_H
#define __DEBUG_H

extern volatile unsigned char Flag_Leave;
extern unsigned char  Debug_RxCounter;

void Debug_Data(unsigned char dt);
void Debug_Printf(unsigned char vNewLine, char *fmt,...);
extern void Debug_Analysis(unsigned char* src, unsigned short len);

#endif

