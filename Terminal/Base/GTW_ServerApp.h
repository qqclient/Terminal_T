#ifndef __GTW_SERVERAPP_H
#define __GTW_SERVERAPP_H


// M35 gprs发送消息结构
typedef struct {
    unsigned short count;
    unsigned char* lpmsg;
} M35_SendDef;

extern M35_SendDef m_gprs;

// 违章处理结构定义
typedef struct {
    unsigned char  Flag_Main;
    unsigned char  Flag_SMS;
    unsigned char  Flag_Call;
    unsigned int   Sec_OverTime;
    unsigned char* lpszMsg;
} Illegal_Process_Def;
extern Illegal_Process_Def GTW_BurglarAlarm;
extern Illegal_Process_Def GTW_IllegalPark;
extern Illegal_Process_Def GTW_MoveCar;

extern unsigned short GPRS_LocationTime;

extern unsigned char  Flag_IllegalParkCancel;

unsigned short GTW_ServerAPP(unsigned char* src, unsigned char* dsr);

void M35_Process(void);

unsigned short M35_SendCarStatus(unsigned char* dsr, unsigned char Car_InputCheck,unsigned char Additon);
unsigned short M35_SendCarLocation(unsigned char* dsr, unsigned char Type);
unsigned short M35_SendIllegalPark( unsigned char* dsr );
unsigned short M35_SendSMS(unsigned char* dsr, unsigned char sms_id, unsigned char* phone, unsigned char* recv_sms);



#endif

