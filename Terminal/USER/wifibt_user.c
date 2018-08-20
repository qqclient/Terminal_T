/******************** (C) COPYRIGHT soarsky ********************
* File Name          : wifibt_user.c
* Description        :
* Author             :
* Version            :
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"


/***************���ػ���������*************/

/***************ȫ�ֱ�������*************/
volatile unsigned short I2411E_TimeCount;
volatile unsigned int iwUART3_Rx;
unsigned char  FlagTerminalStat = '1';			

volatile unsigned int Time_OperDoor = 0;       
volatile unsigned int Wifi_TimeOver = 0;      


/***************��־λ����*************/
u8  Flag_I2411E_Ready = RESET;      
u8  Flag_I2411E_Init = RESET;		
u8  Flag_I2411E_Rename = RESET;    

u16 Count_I2411E_SCAN = 0;
u8  Flag_FindCopName = RESET;		
u8  Flag_RevCopData = RESET;			
u16 Count_FindCop = 0;

u8  Flag_FinishScan = SET;			


/***************���غ�������*************/
void I24_CheckScanEnd(u8 data);
void I24_CheckCopName(u8 data);
void I24_CheckIMReady(u8 data);
void I24_CheckHandle(u8 data);
void I24_ReceiveBuffer(u8 data);
u8   VerifyTelephone(u8 *TmpTelephone);

void I2411E_SetName(char* carid, char carstate);
void I24_SendBuffer(unsigned short object, unsigned int handle, unsigned char* src, unsigned short len);
void I24_SendBuffer_pack(unsigned short object, unsigned int handle, unsigned char* src, unsigned short len);  

/*******************************************************************************
* Function Name  : I2411E_ChangeCarID
* Description    : �ı�AP����,����Ϊ���ƺ�
*******************************************************************************/
void I2411E_ChangeCarID(u8 * TmpCarInfor)
{
    if ( TmpCarInfor == NULL ) return;

    memcpy( m_carinfo.Car_9SN, TmpCarInfor, __CAR9SN_LEN__ );    // ���ó���
    Flag_I2411E_Rename = SET;
}

/*******************************************************************************
* Function Name  : I2411E_ChangeCarStatus
* Description    : �ı�AP���ƣ��ı䳵����״̬λ
*******************************************************************************/
void I2411E_ChangeCarStatus(u8 Car_InputCheck)
{
    FlagTerminalStat = Car_InputCheck;
    Flag_I2411E_Rename = SET;
}

/*******************************************************************************
* Function Name  : I2411E_Process
* Description    : I2411Eģ���ʼ��,�ú���ֻ����һ��,����⵽ģ�鷢��IM_READY����øú���
*******************************************************************************/
void I2411E_Process(void)
{
    enum {
        i24_init_st1 = 1,
        i24_init_st2,
        i24_init_st3,
        i24_init_st4,
    };
    static unsigned char  inspect_status = i24_init_st1;

    switch(inspect_status)
    {
    case i24_init_st1:
        if( Flag_I2411E_Ready == SET || Sys_TimeCount - Sys_RunTime > 5 )// ����5sec
        {
            inspect_status = i24_init_st2;
        }
        break;

    case i24_init_st2:
        // ���� WIFI(AP)
        Usart_SendString(I2411E_UART,"AT+WAPIPCFG=192.168.43.20,192.168.43.200,192.168.43.254,255.255.255.0\r");//������ʼ��ַ����������
        Usart_SendString(I2411E_UART,"AT+WAPLNCFG=3\r");    //�������������
        Usart_SendString(I2411E_UART,"AT+WAPMODE=2\r");     //����ΪAPģʽ:2=AP;3=AP+STA
        Usart_SendString(I2411E_UART,"AT+WAPCHCFG=1\r");    //����channel
        Usart_SendString(I2411E_UART,"AT+WAP\r");           //����wifi
        Usart_SendString(I2411E_UART,"AT+NTSSTART=1,8899,3\r");//����TCP Server

        // ���� BlueTooth
        Usart_SendString(I2411E_UART,"AT+SSP=1\r");         // ����BlueTooth SSP��ʽ

        Flag_I2411E_Rename = SET;                           // ����i2411eģ������

        inspect_status = i24_init_st3;
        Flag_I2411E_Init = SET; //��־��ʼ�����

#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "I2411E-s��ʼ�����");
#endif
        break;

    case i24_init_st3:
        //ͨ�����ı�״̬
        if((Car_InputCheck.Value_Ignite == SET || Car_InputCheck.Value_DriverDoor == SET ) && // ��⵽�п��Ż��ߵ����
                FlagTerminalStat == '1')    // ��ǰ����״̬��
        {
            Time_OperDoor = Sys_TimeCount;
            I2411E_ChangeCarStatus('2');    // ����״̬��
        }

        //����Ƿ����޸�wifi���Ƶ�����
        if(Flag_I2411E_Rename == SET ) {
            Flag_I2411E_Rename = RESET;

#ifdef __DEBUG
            //XFS_WriteBuffer(xfs_auto, "״̬%c", FlagTerminalStat);
#endif
            I2411E_SetName( (char*)m_carinfo.Car_9SN, FlagTerminalStat );// ����i2411eģ������
            Usart_SendString(I2411E_UART,"AT+WAP\r");//����wifi
        }
        break;

    default:
        break;
    }
}


/*******************************************************************************
* Function Name  : I2411E_Inspection
* Description    : ���鹦��,����������ͣ��״̬���øú�����������
******************************************************************************/
void I2411E_Inspection(void)
{
#ifdef __HARDWARE_TEST__
    if(Count_I2411E_SCAN == 0 && Flag_FinishScan == RESET) {
        Count_I2411E_SCAN = 4000;   //4000ms
        Debug_Printf(SET, "\r\nI2411E-s Error");
    }
#else
    // ����ʼ����ɺ󣬹̶���һ��ʱ��ִ��ɨ��ָ��
    if((Flag_FinishScan == SET || Count_I2411E_SCAN == 0) &&
            Flag_I2411E_Init == SET && FlagTerminalStat == '1' )
		
		//if((Flag_FinishScan == SET || Count_I2411E_SCAN == 0) &&
         //   Flag_I2411E_Init == SET   )
    {
        if( m_carinfo.Flag_Inspection_Channel == 0xff ) {
            Usart_SendString(I2411E_UART, "AT+DISCOVERDEVICE=1,20,1\r");//��������
					//Debug_Printf(SET, "\r\n��������");
					//Debug_Printf(SET,"��ǰ״̬ 0X%02x ", FlagTerminalStat);
        }
        else {
            Usart_SendString(I2411E_UART, "AT+WSCAN\r");//����wifi
					Debug_Printf(SET, "\r\n����WIFI");
        }
        Flag_FinishScan = RESET;
        Count_I2411E_SCAN = 3000;
    }
#endif

    // ����һ���������޸�wifi blue������
    // ���ܵ�����ͨ���͹��������ݻ����ҵ�����ͨ������
    if(Flag_RevCopData == SET || Flag_FindCopName == SET) {
        Flag_RevCopData = RESET;
        Flag_FindCopName = RESET;
        Count_FindCop = m_carinfo.SD01_TimeOver * 10;    // ����ʱ��, ����100ms
    }
}

/*******************************************************************************
* Function Name  : I24_CheckScanEnd
* Description    : ���ڼ��ɨ���Ƿ����,����Ĺ��ܺ���,����Ƿ����һ��ɨ������
******************************************************************************/
static void I24_CheckScanEnd(u8 data)
{
    static u8 Index = 0;
    uc8 SCAN_END[9] = "_COMPLETE";
    if(data == SCAN_END[Index])
    {
        if(Index == 8)
        {
            Flag_FinishScan = SET;
#ifdef __HARDWARE_TEST__
            Debug_Printf(SET, "\r\nI2411E-s OK");
#endif
        }
        else
        {
            Index++;
        }
    }
    else
    {
        Index = 0;
    }
}

/*******************************************************************************
* Function Name  : I24_CheckCopName
* Description    : ���ڼ��ɨ�赽���ȵ�����
******************************************************************************/
static void I24_CheckCopName(u8 data)
{
    static u8 Index_P = 0;
    static u8 Index_V = 0;
    static u8 Num = 0;
    static u8 policeApValue1[2] = {0};//���ڱ��ֵ�һ�ε�APֵ
    static u8 policeApValue2[2] = {0};//���ڱ��ֵڶ��ε�APֵ
    static u8 Flag_GetPoliceAp = RESET;
    uc8 CopName[8] = "policeAp";//

    if(data == CopName[Index_P] && Index_P <= 7)
    {
        if(Index_P == 7)
        {
            Flag_GetPoliceAp = SET;
            Num++;
            return;
        } else
        {
            Index_P++;
        }
    } else
    {
        Index_P = 0;
    }

    if(Flag_GetPoliceAp == SET )//ƥ��policeAp�ɹ�
    {
			  //Debug_Printf(SET, "\r\nƥ��policeAp�ɹ�");
        if( Index_V <= 1)
        { //Debug_Printf(SET, "\r\n������������������");
            if(Num%2 == 0) {
                policeApValue1[Index_V] = data;
            }
            else {
                policeApValue2[Index_V] = data;
            }
            Index_V++;
            if(Index_V == 2)
            {
                if(policeApValue1[0] != policeApValue2[0] || policeApValue1[1] != policeApValue2[1])
                {Debug_Printf(SET, "\r\npoliceA");
                    Flag_FindCopName = SET;
                }
                Flag_GetPoliceAp = RESET;
                Index_V = 0;
                Index_P = 0;
            }
        }
    }
}

/*******************************************************************************
* Function Name  : I24_CheckIMReady
* Description    : ���ڼ��wifi����ģ���Ƿ�׼������,���IM_READY
******************************************************************************/
static void I24_CheckIMReady(u8 data)
{
    const unsigned char IsReady[] = "IM_READY";
    static u8 Index = 0;

    if(data == IsReady[Index]) {
        if(Index == 7) {
            Flag_I2411E_Ready = SET;
        }
        else {
            Index++;
        }
    }
    else {
        Index = 0;
    }
}


/*******************************************************************************
* Function Name  : VerifyTelephone
* Description    : ������֤�������
******************************************************************************/
u8 VerifyTelephone(u8 *TmpTelephone)
{
    u8 i;
    for(i=0; i<5; i++ ) {
        if(memcmp(m_carinfo.Family_Phone[i], TmpTelephone, __PHONE_LENGTH__ ) == 0) {
            return 1;
        }
    }
    return 0;
}

/*******************************************************************************
* Function Name  : I24_ReceiveBuffer
* Description    : i2411e���ݽ��
******************************************************************************/
void I24_ReceiveBuffer(unsigned char data)
{
    enum {
        i24_recv_st1 = 1,
        i24_recv_st2,
        i24_recv_st3,
        i24_recv_st4,
        i24_recv_st5,
        i24_recv_st6,
        i24_recv_st7,
        i24_recv_st8
    };

    static unsigned char  recv_status = i24_recv_st1;
    static unsigned short recv_point = 0;
    static unsigned short recv_count = 0;
    static unsigned short recv_object = 0;
    static unsigned int   recv_handle = 0;
    static unsigned char  recv_head[85] = { 0 };
    static i2b_Def i2b;

		if( iwUART3_Rx >= 85 )  //iwUART3_Rx ��ֹ�±����
			 iwUART3_Rx = 0;
    recv_head[iwUART3_Rx++] = data;
//Debug_Printf( SET, "�յ��û��ֻ�APP��Ϣ=%04d", iwUART3_Rx);//m_var.Flag_Debug_Enable );
    switch(recv_status) {
    case i24_recv_st1: // ȡ��ʼλ��
        if(iwUART3_Rx > 2) {
            if(recv_head[0] == '\r' && recv_head[1] == '\n' && recv_head[2] == '<') {
                // �յ���ʼ��־
                recv_status = i24_recv_st2;
                recv_handle = 0;
            }
            else {
                recv_head[0] = recv_head[1];
                recv_head[1] = recv_head[2];
                recv_head[2] = 0;
                iwUART3_Rx = 2;
            }
        }
        break;

    case i24_recv_st2:  // ��wifi ���� bluetooth ��־��8λ
        recv_object = data;
        recv_status = i24_recv_st3;
        break;

    case i24_recv_st3:  // ��wifi ���� bluetooth ��־��8λ
        recv_object += (data << 8);
        recv_status = i24_recv_st4;
        break;

    case i24_recv_st4:  // �����ݳ��ȵ�8λ
        recv_count = data;
        recv_status = i24_recv_st5;
        break;

    case i24_recv_st5:  // �����ݳ��ȸ�8λ
        recv_count += (data << 8);
        recv_status = i24_recv_st6;
        recv_point = 0;
        i2b.x = 0;
        break;

    case i24_recv_st6:  // ����Handle
        i2b.c[recv_point++] = data;
        if(recv_object == 0x0201) {//wifi
            if(recv_point >= 4) {
                recv_handle = i2b.x; 
                recv_status = i24_recv_st7;
                recv_count -= 4;
                recv_point = 0;
            }

            Wifi_TimeOver = Sys_TimeCount; // ���ý��ճ�ʱ
        }
        else if(recv_object == 0x0101) { //����
            if(recv_point >= 2) {
                recv_handle = i2b.x;
                recv_status = i24_recv_st7;
                recv_count -= 2;
                recv_point = 0;
            }
        }
        else {
            recv_status = i24_recv_st1;
            iwUART3_Rx = 0;
        }
        break;

    case i24_recv_st7:  // ��������
        GTW_ReceiveData( data, recv_object, recv_handle);
        recv_point++;
        if(recv_point >= recv_count ) {
            // ������һ֡
            recv_status = i24_recv_st1;
            recv_point = 0;
        }
        break;

    default:
        recv_status = i24_recv_st1;
        recv_point = 0;
        iwUART3_Rx = 0;
        break;
    }
}


/*******************************************************************************
* Function Name  : I24_SendBuffer_pack
* Description    : i2411e���ݷְ�����
******************************************************************************/
void I24_SendBuffer_pack(unsigned short object, unsigned int handle, unsigned char* src, unsigned short len)
{
    i2b_Def i2b;
    unsigned char buf[200] = { 0 };
    unsigned char* ptr = buf;

    // Handleֵ
    i2b.x = handle;
    // WIFI��Ϣ��ʽͷ,����һ�η����������
    *ptr++ = 'A';
    *ptr++ = 'T';
    *ptr++ = '>';
    *ptr++ = LOBYTE(object);
    *ptr++ = HIBYTE(object);

    if(object == 0x0201) {
        *ptr++ = LOBYTE(len + 4); //���ݳ���+Handleֵ
        *ptr++ = HIBYTE(len + 4); //���ݳ���+Handleֵ

        *ptr++  = i2b.c[0]; // (u8)Handle;//
        *ptr++  = i2b.c[1]; // (u8)(Handle>>8);
        *ptr++  = i2b.c[2]; // (u8)(Handle>>16);
        *ptr++  = i2b.c[3]; // (u8)(Handle>>24);
    }
    else if(object == 0x0101) {
        *ptr++ = LOBYTE(len + 2); //���ݳ���+Handleֵ
        *ptr++ = HIBYTE(len + 2); //���ݳ���+Handleֵ

        *ptr++  = i2b.c[0]; // (u8)Handle;//
        *ptr++  = i2b.c[1]; // (u8)(Handle>>8);
    }

    // �������
    memcpy(ptr, src, len);
    ptr += len;

    *ptr++ = 0xd;

    // ͨ��������������
    Usart_SendNByte( I2411E_UART, buf, ptr - buf );
}


/*******************************************************************************
* Function Name  : I24_SendBuffer
* Description    : i2411e���ݷ���
******************************************************************************/
void I24_SendBuffer(unsigned short object, unsigned int handle, unsigned char* src, unsigned short len)
{
    i2b_Def i2b;
    unsigned char* ptr = src;

    // WIFI��Ϣ��ʽͷ
    *ptr++ = 'A';
    *ptr++ = 'T';
    *ptr++ = '>';
    *ptr++ = LOBYTE(object);
    *ptr++ = HIBYTE(object);

    // Handleֵ
    i2b.x = handle;
    if(object == 0x0201) {
        *ptr++ = LOBYTE(len + 4); 
        *ptr++ = HIBYTE(len + 4); 

        *ptr++  = i2b.c[0]; 
        *ptr++  = i2b.c[1];
        *ptr++  = i2b.c[2]; 
        *ptr++  = i2b.c[3]; 

        *(src+len+11) = 0x0d;
    }
    else if(object == 0x0101) {
        *ptr++ = LOBYTE(len + 2);
        *ptr++ = HIBYTE(len + 2);

        *ptr++  = i2b.c[0]; 
        *ptr++  = i2b.c[1];

        *(src+len+9) = 0x0d;
    }

    // ͨ��WIFI��������
    Usart_SendNByte(I2411E_UART, src, len + ptr - src+1);
}


/*******************************************************************************
* Function Name  : USART3_IRQHandler
* Description    : ����3�жϴ�����,
******************************************************************************/
void I2411E_IRQHandler(void)
{
    u8 data = 0;
    //u8 i;
    u8 Clear = Clear;

    if(USART_GetITStatus(I2411E_UART, USART_IT_RXNE) != RESET) {
        USART_ClearITPendingBit(I2411E_UART,USART_IT_RXNE);        //��������־λ

        data = USART_ReceiveData(I2411E_UART);

        if(Flag_I2411E_Ready != SET) {
            I24_CheckIMReady(data);     // ���ģ���Ƿ�׼������
        }
        else {
            if( TerminalStatus == Status_Park )
            {
                I24_CheckCopName(data);     // ��⾯��ͨ������
                I24_CheckScanEnd(data);     // ���ɨ���Ƿ����
            }

            I24_ReceiveBuffer(data);        // �������=>gtw_RxBuf
        }

        if( m_var.Flag_Init_I2411E == 0 )
            Debug_Data(data);
    }
    else if(USART_GetITStatus(I2411E_UART, USART_IT_IDLE) != RESET) {  
        Clear = I2411E_UART->SR;
        Clear = I2411E_UART->DR;
    }
}


/*******************************************************************************
* Function Name  : I2411E_SetName
* Description    : ����Wifi/BlueToothģ������
******************************************************************************/
void I2411E_SetName(char* carid, char carstate)
{
    char buf1[24] = {0};
    char buf[36] = {0};

    char carst = carstate < '1' && carstate > '4' ? '1' : carstate; 

    memset( buf, 0, sizeof(buf) );
    memcpy( buf, carid, __CAR9SN_LEN__ );

    sprintf(buf1, "%c%02d%s-", carst, m_carinfo.Car_Type, buf);

#ifdef __DEBUG
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "����wifi/bluetooth����:%c%02d%s-", carst, m_carinfo.Car_Type, carid );
#endif

    sprintf(buf, "AT+WAPNCFG=%s\r", buf1);
    Usart_SendNByte( I2411E_UART, (u8 *)buf, 29); 

    sprintf(buf, "AT+NAME=%s\r", buf1);
    Usart_SendNByte( I2411E_UART, (u8 *)buf, 26);
}

/****************************************END OF FILE**********************************/
