/******************** (C) COPYRIGHT soarsky ************************************
* File Name          : debug.c
* Author             : CSYXJ
* Version            : V0.1
* Date               : 2017-03-19
* Description        : AT调试指令
*******************************************************************************/
#include "gtw_Head.h"

extern ArrQueue* m_FIFO_Debug;

#ifdef __HARDWARE_TEST__
extern volatile unsigned char GPS_HardwareTestON;
extern u8  Flag_FinishScan;
#endif

#ifndef __USE_USB__
#define USB_DATA_SIZE 64
#endif

char debug_buf[ 512 ] = { 0 };

unsigned char  Debug_RxCounter = 0;
unsigned char  Debug_RxBuffer[100];
volatile unsigned char Flag_Leave;

volatile unsigned char debug_send_ok;    // 发送进行中


#ifdef DEBUG_UART
/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : 串口1数据接受服务函数,当串口1接受到数据时,进入该中断接受数据
******************************************************************************/
void DEBUG_IRQHandler(void)
{
    u8 Clear = Clear;
    u8 data;

    if(USART_GetITStatus(DEBUG_UART, USART_IT_RXNE) != RESET) {
        //接收中断,接收1个字节
        USART_ClearITPendingBit(DEBUG_UART, USART_IT_RXNE);        //清除挂起标志位
        data = USART_ReceiveData(DEBUG_UART);

        //I2411E_UART->DR = data;
        //M35_UART->DR = data;

        Debug_RxBuffer[Debug_RxCounter++] = data;
        if( data == '\n' || data == '\r') {
            Debug_Analysis(Debug_RxBuffer, Debug_RxCounter);
            Debug_RxCounter = 0;
        }
    }
    else if(USART_GetITStatus(DEBUG_UART, USART_IT_IDLE) != RESET) {
        
        USART_ClearITPendingBit(DEBUG_UART, USART_IT_IDLE);

        Clear = DEBUG_UART->SR;
        Clear = DEBUG_UART->DR;

        Debug_Analysis(Debug_RxBuffer, Debug_RxCounter);
        Debug_RxCounter = 0;
    }
    UART_SendByInt( DEBUG_UART, m_FIFO_Debug );
}
#endif


/*******************************************************************************
* Function Name  : Debug_Data
* Description    : 调试信息输出函数
******************************************************************************/
void Debug_Data(unsigned char dt)
{
#ifdef DEBUG_UART
    DEBUG_UART->DR = dt;
#endif
#ifdef __USE_USB__
    unsigned char buf[4] = { 0 };
    buf[0] = dt;
    usb_SendBuf( buf, 1 );
#endif
}

/*******************************************************************************
* Function Name  : Debug_Printf
* Description    : 调试信息输出函数
******************************************************************************/
void Debug_Printf(unsigned char vNewLine, char *fmt,...)
{
    int d;
    float f1;
    char s1[ 16 ] = { 0 };
    char s2[ 64 ] = { 0 };
    const char *s;
    char* lpsz1;
#ifdef DEBUG_Tx_DMA
    int SysTick_Start = 0;
#endif
#ifdef __USE_USB__
    unsigned char count = 0;
#endif

    va_list ap;
    va_start(ap, fmt);

    lpsz1 = debug_buf;

    while( *fmt != 0 ) {    // 判断是否到达字符串结束符
        if( *fmt == '\\' ) {
            switch(*++fmt) {
            case 'r':       // 回车符
                *lpsz1++ = 0xd;
                fmt++;
                break;

            case 'n':       // 换行符
                *lpsz1++ = 0xa;
                fmt++;
                break;

            default:
                fmt++;
                break;
            }
        }
        else if( *fmt == '%' ) {
            switch (*++fmt) {
            case 'c':
                d = va_arg(ap, int);
                sprintf(s2, "%c", d);
                strcpy(lpsz1, s2);
                lpsz1 += strlen(s2);
                fmt++;
                break;

            case 's':   //字符串数
                s = va_arg(ap, const char *);
                strcpy(lpsz1, s);
                lpsz1 += strlen(s);
                fmt++;
                break;

            case 'd':   //十进制数
                d = va_arg(ap, int);
                sprintf(s2, "%d", d);
                strcpy(lpsz1, s2);
                lpsz1 += strlen(s2);
                fmt++;
                break;

            case 'x':   //十六进制数
                d = va_arg(ap, int);
                sprintf(s2, "%x", d);
                strcpy(lpsz1, s2);
                lpsz1 += strlen(s2);
                fmt++;
                break;

            case '.':   // %.7f 浮点数
                f1 = va_arg(ap, double);
                s1[0] = '%';
                s1[1] = *fmt++;
                s1[2] = *fmt++;
                s1[3] = *fmt;
                s1[4] = 0;
                sprintf(s2, s1, f1);
                strcpy(lpsz1, s2);
                lpsz1 += strlen(s2);
                fmt++;
                break;

            case '0':   // %04d/%04x
                s1[0] = '%';
                s1[1] = *fmt++;
                s1[2] = *fmt++;
                s1[3] = *fmt;
                s1[4] = 0;
                d = va_arg(ap, int);
                sprintf(s2, s1, d);
                strcpy(lpsz1, s2);
                lpsz1 += strlen(s2);
                fmt++;
                break;

            default:
                fmt++;
                break;
            }
        }
        else {
            *lpsz1++ = *fmt++;
        }
    }

    // 尾部添加0d 0a
    if( vNewLine == SET ) {
        *lpsz1++ = 0xd;
        *lpsz1++ = 0xa;
    }

#ifdef __USE_USB__
    d = lpsz1 - debug_buf;
    s = debug_buf;

    while( d > 0 ) {
        count =  d > USB_DATA_SIZE ? USB_DATA_SIZE : d; // 取发送一帧的数据个数
        usb_SendBuf( (unsigned char*)s, count );
        s += count;
        d -= count;
        if( d > 0 )
            Delay_ms(1);
    }
#endif

#ifdef DEBUG_Tx_DMA
    d = lpsz1 - debug_buf;
    s = debug_buf;
    while( d > 0 ) {
        SysTick_Start = RTC_GetCounter();
        while( Flag_Debug_Send == 1 ) {
            if( RTC_GetCounter() - SysTick_Start > 2 )  // 计算超时
                break;
        }
        count =  d > DEBUG_TxSize ? DEBUG_TxSize : d;   // 取发送一帧的数据个数
        memcpy( DEBUG_TxBuf, s, count);
        s += count;
        d -= count;
        DEBUG_Tx_DMA_Start( count );
    }
#else
#ifdef DEBUG_UART
    Usart_SendNByte( DEBUG_UART, (unsigned char*)debug_buf, lpsz1 - debug_buf );
#endif
#endif

}


/*******************************************************************************
* Function Name  : Debug_Analysis
* Description    : 调试信息解析函数
******************************************************************************/
void Debug_Analysis(unsigned char* src, unsigned short len)
{
    unsigned int    i = 0;
    unsigned int    j = 0;
    char            buf[200] = { 0 };
    char*           s1 = NULL;
    char*           s2 = NULL;
    unsigned char*  ptr = src;
    Illegal_Def     illegal1;
    unsigned int    ill_Addr;
    unsigned int    ill_size;

    if( len <= 0 )
        return;

    ill_size = (int)&illegal1.ino - (int)&illegal1.id + sizeof(illegal1.ino);

    if((u8*)strstr((const char *)ptr, "AT+VER=?") != NULL) {
        Debug_Printf(SET,
                     "AT+VER=硬件V%s,固件%s,日期%04d-%02d-%02d",
                     __pcb_ver_id,
                     __ver_id,
                     __VER_YEAR,
                     __VER_MONTH,
                     __VER_DAY );
    }
    else if((u8*)strstr((const char *)ptr, "AT+REBOOT=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+REBOOT=");
        s1 += strlen("AT+REBOOT=");
        switch( *s1 ) {
        case '1':
            SystemReset();
            break;
        case 'S':
            if( *(s1+1) == 'I' && *(s1+2) == 'M' ) {
                Debug_Printf(SET, "AT+REBOOT=SIM OK");
                GPRS_CmdNow = M35_PwrOn_Start;
            }
            break;
        }
    }
    else if((u8*)strstr((const char *)ptr, "AT+DIST=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+DIST=");
        s1 += strlen("AT+DIST=");
        switch( *s1 ) {
        case '?':
            Debug_Printf(SET, "AT+DIST=%.5f", m_carinfo.distance );
            return;
        case 'C':
            if( *(s1+1) == 'L' && *(s1+2) == 'E' && *(s1+3) == 'A' && *(s1+4) == 'R' ) {
                Debug_Printf(SET, "AT+DIST=CLEAR OK");
                m_carinfo.distance = 0;
                MX25L128_CreatSaveTask();
            }
            break;
        default:
            if( *s1 >= '0' && *s1 <= '9' ) {
                memset( buf, 0, sizeof( buf) );
                s2 = buf;
                while(1) {
                    if( *s1 >= '0' && *s1 <= '9' ) {
                        *s2++ = *s1++;
                    }
                    else {
                        m_carinfo.distance = atof( buf );
                        MX25L128_CreatSaveTask();
                        break;
                    }
                }
            }
            Debug_Printf(SET, "AT+DIST=%.5f OK", m_carinfo.distance);
            break;
        }
    }
    else if((u8*)strstr((const char *)ptr, "AT+TTSVOL=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+TTSVOL=");
        s1 += strlen("AT+TTSVOL=");
        if( *s1 == '?' ) {
            Debug_Printf(SET, "AT+TTSVOL=%d", m_carinfo.Volume_Value );
        }
        else {
            Debug_Printf(SET, "AT+TTSVOL=OK" );
            memset(buf, 0, sizeof(buf));
            for(i=0; i<3; i++) {
                if(*s1 == 0xd || *s1 ==0xa || *s1 == 0 ) {
                    break;
                }
                buf[i] = *s1++;
            }
            m_carinfo.Volume_Value = atoi(buf);
            XFS_SetVolume( m_carinfo.Volume_Value );
            XFS_WriteBuffer(xfs_auto, "祝您一路平安");
            MX25L128_CreatSaveTask();
        }
    }
    else if((u8*)strstr((const char *)ptr, "AT+NET=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+NET=");
        s1 += strlen("AT+NET=");
        if( *s1 == '?' ) {
            if( m_carinfo.NetAddrType == 0 ) {
                Debug_Printf(SET, "Addr Type=%s\r\nServer=%d.%d.%d.%d\r\nPort=%d",
                             m_carinfo.NetAddrType == 0 ? "0:IP" : "1:Domain",
                             m_carinfo.ServerName[0], m_carinfo.ServerName[1], m_carinfo.ServerName[2], m_carinfo.ServerName[3],
                             m_carinfo.NetPort );
            }
            else {
                Debug_Printf(SET, "Addr Type=%s\r\nServer=%s\r\nPort=%d",
                             m_carinfo.NetAddrType == 0 ? "0:IP" : "1:Domain",
                             m_carinfo.ServerName,
                             m_carinfo.NetPort );
            }
        }
        else {
            Debug_Printf(SET, "AT+NET=OK");
            s2 = strtok( s1, ";" );
            while( s2 != NULL ) {
                if(strstr((const char *)s2, "Type:") != NULL) {
                    memset(buf, 0, sizeof(buf));
                    memcpy(buf, s2 + strlen("Type:"), strlen(s2)-strlen("Type:"));
                    m_carinfo.NetAddrType = atoi(buf);
                }
                else if(strstr((const char *)s2, "Server:") != NULL) {
                    s2 += strlen("Server:");
                    strlcpy( (char*)m_carinfo.ServerName, s2, __SERVERNAME_LEN__ );
                }
                else if(strstr((const char *)s2, "Port:") != NULL) {
                    memset(buf, 0, sizeof(buf));
                    memcpy(buf, s2 + strlen("Port:"), strlen(s2)-strlen("Port:"));
                    m_carinfo.NetPort = atoi(buf);
                }
                s2 = strtok( NULL, ";" );
            }

            if( m_carinfo.NetAddrType == 0 ) {
                i = 0;
                s2 = strtok( (char*)m_carinfo.ServerName, "." );
                while( s2 != NULL ) {
                    memset(buf, 0, sizeof(buf));
                    strcpy(buf, s2);
                    m_carinfo.ServerName[i++] = atoi(buf);
                    s2 = strtok( NULL, "." );
                }
                memset( m_carinfo.ServerName + i, 0, sizeof(m_carinfo.ServerName) - i );
            }
            MX25L128_CreatSaveTask();
        }
    }
    else if((u8*)strstr((const char *)ptr, "AT+DBGNONE=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+DBGNONE=");
        s1 += strlen("AT+DBGNONE=");
        switch( *s1 ) {
        case '?':
            Debug_Printf(SET, "AT+DBGNONE=%d", m_var.Flag_Debug_Enable == RESET ? 0 : 1);
				    Debug_Printf( SET, "AT+DBGNONE=%x", m_var.Flag_Debug_Enable );
            return;
        case '0':
            m_var.Flag_Debug_Enable = RESET;
            break;
        case '1':
            m_var.Flag_Debug_Enable = SET;
            break;
        }
        Debug_Printf( SET, "AT+DBGNONE=%c OK", *s1 );
    }
    else if((u8*)strstr((const char *)ptr, "AT+DTUID=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+DTUID=");
        s1 += strlen("AT+DTUID=");
        if( *s1 == '?' ) {
            Debug_Printf(SET, "AT+DTUID=%s", m_carinfo.DTU_ID);
        }
        else {
            Debug_Printf(SET, "AT+DTUID=OK");
            memset(m_carinfo.DTU_ID, 0, sizeof(m_carinfo.DTU_ID));
            for(i=0; i<__DTU_ID_LEN__; i++) {
                if(*s1 == 0xd || *s1 ==0xa || *s1 == 0 ) {
                    break;
                }
                m_carinfo.DTU_ID[i] = *s1++;
            }
            MX25L128_CreatSaveTask();
        }
    }
    else if((u8*)strstr((const char *)src, "AT+TIME=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+TIME=");
        s1 += strlen("AT+TIME=");
        if( *s1 == '?' ) {
            Debug_Printf(SET, "系统时间: %04d-%02d-%02d %02d:%02d:%02d",
                         Sys_CalendarTime.w_year,
                         Sys_CalendarTime.w_month,
                         Sys_CalendarTime.w_day,
                         Sys_CalendarTime.hour,
                         Sys_CalendarTime.min,
                         Sys_CalendarTime.sec);
        }
        else {
            Debug_Printf(SET, "AT+TIME=OK");
            TimeSync( &Sys_CalendarTime, src + 8);
        }
    }
    else if((u8*)strstr((const char *)ptr, "AT+FLASH=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+FLASH=");
        s1 += strlen("AT+FLASH=");
        switch( *s1 ) {
        case 'E':
            if( *(s1+1) == 'R' && *(s1+2) == 'A' && *(s1+3) == 'S' && *(s1+4) == 'E' ) {
                Debug_Printf(SET, "AT+FLASH=ERASE OK");
                MX25L128_Sector_Erase(ADDR_ILLEGAL_CONTECT);
                MX25L128_Sector_Erase(ADDR_ILLEGAL_CONTECT+0x1000);
                MX25L128_Sector_Erase(ADDR_ILLEGAL_CONTECT+0x2000);
                MX25L128_Sector_Erase(ADDR_ILLEGAL_UPLOAD);
                MX25L128_Word_Write(ADDR_ILLEGAL_UPLOAD, 0);            //初始化车辆违章总数为0
                MX25L128_Word_Write(ADDR_ILLEGAL_UPLOAD+4, 0);          //初始化车辆未处理违章总数为0
            }
            break;
        case 'I': // 17.6.5 csYxj(728920175@qq.com)
            if( *(s1+1) == 'D' ) {
                s2 = buf;
                sprintf( s2, "违章编号:\r\n" );
                s2 += strlen( s2 );

                sprintf( s2, "未知=%d\r\n", IllegalUnknow );
                s2 += strlen( s2 );

                sprintf( s2, "违章停车=%d\r\n", ino_IllegalPark );
                s2 += strlen( s2 );

                sprintf( s2, "无证驾驶=%d\r\n", ino_NoDriveLic );
                s2 += strlen( s2 );

                sprintf( s2, "滥用远光=%d\r\n", IllegalUseFarLight );
                s2 += strlen( s2 );

                sprintf( s2, "未系安全带=%d\r\n", DriveNoSeatBelt );
                s2 += strlen( s2 );

                sprintf( s2, "违规使用安全带=%d\r\n", IllegalUseSeatBelt );
                s2 += strlen( s2 );

                sprintf( s2, "超速=%d\r\n", OverSpeed );
                s2 += strlen( s2 );

                sprintf( s2, "疲劳驾驶=%d\r\n", FatigueDrive );
                s2 += strlen( s2 );

                sprintf( s2, "大客车夜间违章运行=%d\r\n", BusIllegalRun );
                s2 += strlen( s2 );

                sprintf( s2, "违反限行=%d\r\n", IllegalRun );
                s2 += strlen( s2 );

                sprintf( s2, "违规使用喇叭=%d\r\n", IllegalUseTrumpet );
                s2 += strlen( s2 );

                sprintf( s2, "开车玩手机=%d\r\n", DrivePlayPhone_enum );
                s2 += strlen( s2 );

                sprintf( s2, "急停急走=%d", Acceleratione_enum );
                s2 += strlen( s2 );

                Debug_Printf( SET, buf );
            }
            break;
        case 'M': // 17.6.6 csYxj(728920175@qq.com)
            if( *(s1+1) == 'A' && *(s1+2) == 'K' && *(s1+3) == 'E' ) {
                s1 += 4;
                if( *s1 >= '0' && *s1 <= '9' ) {
                    memset( buf, 0, sizeof( buf) );
                    s2 = buf;
                    while(1) {
                        if( *s1 >= '0' && *s1 <= '9' ) {
                            *s2++ = *s1++;
                        }
                        else {
                            i = atoi( buf );
                            break;
                        }
                    }
                }
                if(i > 60)
                    i = 60;

                MakeViolation(i);
            }
            break;
        case 'R':
            if( *(s1+1) == 'E' && *(s1+2) == 'A' && *(s1+3) == 'D' ) {
                ViolenceTotal = MX25L128_Word_read(ADDR_ILLEGAL_UPLOAD);
                ViolenceUntreated = MX25L128_Word_read(ADDR_ILLEGAL_UPLOAD+4);

                Debug_Printf(SET, "违章总数:%d\r\n未上传违章数:%d", ViolenceTotal, ViolenceUntreated);
            }
            break;
        case 'S': // 17.6.5 csYxj(728920175@qq.com)
            if( *(s1+1) == 'H' && *(s1+2) == 'O' && *(s1+3) == 'W' ) {
                s1 += 4;
                if( *s1 >= '0' && *s1 <= '9' ) {
                    memset( buf, 0, sizeof( buf) );
                    s2 = buf;
                    while(1) {
                        if( *s1 >= '0' && *s1 <= '9' ) {
                            *s2++ = *s1++;
                        }
                        else {
                            i = atoi( buf );
                            break;
                        }
                    }
                    if( ViolenceTotal <= 0 ) {
                        Debug_Printf(SET, "AT+FLASH=SHOW[%d/%d] OK\r\n", i, ViolenceTotal );
                    }
                    else {
                        if( i < ViolenceTotal )
                            ill_Addr = ADDR_ILLEGAL_CONTECT + (ViolenceTotal - i - 1) * ill_size;
                        else {
                            ill_Addr = ADDR_ILLEGAL_CONTECT;
                            i = ViolenceTotal - 1;
                        }

                        MX25L128_BufferRead( (unsigned char*)&illegal1.id, ill_Addr, ill_size );
                        illegal_json_packet( (unsigned char*)buf, illegal1 );
                        Debug_Printf(SET, "AT+FLASH=SHOW[%d/%d] OK\r\n%s", i+1, ViolenceTotal, buf );
                    }
                }
            }
            break;
        }
    }
    else if((u8*)strstr((const char *)ptr, "AT+SN=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+SN=");
        s1 += strlen("AT+SN=");
        if( *s1 == '?' ) {
            Debug_Printf(SET, "车牌号[UTF-8]:%s\r\n车牌号[GBK]:%s", m_carinfo.Car_9SN, m_carinfo.Car_8SN );
        }
        else {
            Debug_Printf(SET, "AT+SN=OK");
            m_carinfo.Car_5SN[0] = HEX_decode_char(s1[0], s1[1]);
            m_carinfo.Car_5SN[1] = HEX_decode_char(s1[2], s1[3]);
            m_carinfo.Car_5SN[2] = HEX_decode_char(s1[4], s1[5]);
            m_carinfo.Car_5SN[3] = HEX_decode_char(s1[6], s1[7]);
            m_carinfo.Car_5SN[4] = HEX_decode_char(s1[8], s1[9]);

            memset( m_carinfo.Car_9SN, 0, sizeof(m_carinfo.Car_9SN ) );
            memset( m_carinfo.Car_8SN, 0, sizeof(m_carinfo.Car_8SN ) );
            decoder_carno( m_carinfo.Car_5SN, m_carinfo.Car_8SN, m_carinfo.Car_9SN );

            I2411E_ChangeCarID( m_carinfo.Car_9SN );

            MX25L128_CreatSaveTask();
        }
    }
    else if((u8*)strstr((const char *)ptr, "AT+TEL=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+TEL=");
        s1 += strlen("AT+TEL=");
        if( *s1 == '?' ) {
            s2 = buf;
            sprintf( s2, "手机号:\r\n" );
            s2 += strlen( s2 );
            for( i=0; i<5; i++ ) {
                if( m_carinfo.Family_Phone[i][0] != 'F' && m_carinfo.Family_Phone[i][0] != 0 ) {
                    sprintf( s2, "\t手机%d:%s\r\n", i+1, m_carinfo.Family_Phone[i] );
                    s2 += strlen( s2 );
                }
            }
            Debug_Printf( SET, buf );
        }
        else if( *s1 >= '0' && *s1 <= '9' ) {
            Debug_Printf(SET, "AT+TEL=OK");
            memset(m_carinfo.Family_Phone[0], 0, 5*sizeof(m_carinfo.Family_Phone[i]));
            i = 0;
            s2 = strtok( s1, ";" );
            while( s2 != NULL ) {
                memset( m_carinfo.Family_Phone[i], 'F', __PHONE_LENGTH__ );
                for( j=0; j< __PHONE_LENGTH__; j++ ) {
                    if(s2[j] == 0xd || s2[j] == 0x0a || s2[j] == 0)
                        break;
                    m_carinfo.Family_Phone[i][j] = s2[j];
                }

                i++;
                if(i > 4)
                    break;

                s2 = strtok( NULL, ";" );
            }
            for(; i<5; i++) {
                memset((char*)m_carinfo.Family_Phone[i], 'F', __PHONE_LENGTH__ ); 
            }
            MX25L128_CreatSaveTask();
        }
        else if( *s1 == 'D' && *(s1+1) == 'I' && *(s1+2) == 'A' && *(s1+3) == 'L' ) { // 17.6.5 csYxj(728920175@qq.com)
            Debug_Printf(SET, "AT+TEL=DIAL OK");
            GPRS_CmdNow = Call_Start;
        }
        else if( *s1 == 'S' && *(s1+1) == 'M' && *(s1+2) == 'S' ) { // 17.6.8 csYxj(728920175@qq.com)
            Debug_Printf(SET, "AT+TEL=SMS OK");
            GTW_BurglarAlarm.Flag_SMS = SET;
        }
    }
    else if((u8*)strstr((const char *)ptr, "AT+DRIVERID=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+DRIVERID=");
        s1 += strlen("AT+DRIVERID=");
        if( *s1 == '?' ) {
            s2 = buf;
            sprintf( s2, "驾驶证号码:\r\n" );
            s2 += strlen( s2 );
            for(i=0; i<5; i++ ) {
                if( strlen((char*)m_carinfo.Driver_ID[i]) > 0 ) {
                    sprintf( s2, "驾证%d:%s\r\n", i+1, m_carinfo.Driver_ID[i] );
                    s2 += strlen( s2 );
                }
            }
            Debug_Printf( SET, buf );
        }
        else {
            Debug_Printf(SET, "AT+DRIVERID=OK");
            memset(m_carinfo.Driver_ID[0], 0, 5*sizeof(m_carinfo.Driver_ID[0]));
            i = 0;
            s2 = strtok( s1, ";" );
            while( s2 != NULL ) {
                for( j=0; j< __DRIVER_TYPE_LEN__ + __DRIVERID_LEN__; j++ ) {
                    if(s2[j] == 0xd || s2[j] == 0x0a || s2[j] == 0)
                        break;
                    m_carinfo.Driver_ID[i][j] = s2[j];
                }

                i++;
                if(i > 4)
                    break;

                s2 = strtok( NULL, ";" );
            }
            MX25L128_CreatSaveTask();
        }
    }
    else if((u8*)strstr((const char *)ptr, "AT+AUTH=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+AUTH=");
        s1 += strlen("AT+AUTH=");
        if( *s1 == '?' ) {
            Debug_Printf(SET, "当前授权码:%s", m_carinfo.Authorize_Code );
        }
        else {
            Debug_Printf(SET, "AT+AUTH=OK" );
            memset(m_carinfo.Authorize_Code, 0, sizeof(m_carinfo.Authorize_Code));
            for(i=0; i<__AUTHORIZE_LEN__; i++) {
                if(*s1 == 0xd || *s1 ==0xa || *s1 == 0 ) {
                    break;
                }
                m_carinfo.Authorize_Code[i] = *s1++;
            }
            MX25L128_CreatSaveTask();
        }
    }
    else if((u8*)strstr((const char *)ptr, "AT+CARTYPE=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+CARTYPE=");
        s1 += strlen("AT+CARTYPE=");
        if( *s1 == '?' ) {
            Debug_Printf(SET,  "AT+CARTYPE=%02d", m_carinfo.Car_Type );
        }
        else {
            Debug_Printf(SET, "AT+CARTYPE=OK");
            memset( buf, 0, sizeof(buf) );
            for(i=0; i<3; i++) {
                if(*s1 == 0xd || *s1 ==0xa || *s1 == 0 ) {
                    break;
                }
                buf[i] = *s1++;
            }
            i = atoi(buf);
            if( i > 0 && i < 17 ) {
                m_carinfo.Car_Type = i;
            }
        }
    }
    else if((u8*)strstr((const char *)ptr, "AT+DRIVERTYPE=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+DRIVERTYPE=");
        s1 += strlen("AT+DRIVERTYPE=");
        if( *s1 == '?' ) {
            Debug_Printf(SET,  "AT+DRIVERTYPE=%02d", m_carinfo.Driver_Type );
        }
        else {
            Debug_Printf(SET, "AT+DRIVERTYPE=OK");
            memset( buf, 0, sizeof(buf) );
            for(i=0; i<3; i++) {
                if(*s1 == 0xd || *s1 ==0xa || *s1 == 0 ) {
                    break;
                }
                buf[i] = *s1++;
            }
            i = atoi(buf);
            if( i > 0 && i < 17 ) {
                m_carinfo.Driver_Type = i;
            }
        }
    }
    else if((u8*)strstr((const char *)ptr, "AT+SIM=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+SIM=");
        s1 += strlen("AT+SIM=");
        switch( *s1 ) {
        case '?':
            Debug_Printf(SET,
                         "AT+SIM=%d[%s]",
                         m_carinfo.Flag_Simulation & 0x01,
                         m_carinfo.Flag_Simulation == SET ? "仿真" : "实车" );
            return;
        case '0':
            m_carinfo.Flag_Simulation = RESET;
            break;
        case '1':
            m_carinfo.Flag_Simulation = SET;
            break;
        }
        MX25L128_CreatSaveTask();
        Debug_Printf(SET,
                     "AT+SIM=%c[%s] OK",
                     *s1,
                     m_carinfo.Flag_Simulation == SET ? "仿真" : "实车" );
    }
    else if((u8*)strstr((const char *)ptr, "AT+SIGN=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+SIGN=");
        s1 += strlen("AT+SIGN=");

        switch( *s1 ) {
        case '?':
            Debug_Printf(SET,
                         "AT+SIGN=%d[%s信号强度]",
                         m_carinfo.Flag_GSMSign & 0x01,
                         m_carinfo.Flag_GSMSign == SET ? "语音播报" : "无语音报" );
            return;
        case '0':
            m_carinfo.Flag_GSMSign = RESET;
            break;
        case '1':
            m_carinfo.Flag_GSMSign = SET;
            break;
        }
        MX25L128_CreatSaveTask();
        Debug_Printf(SET, "AT+SIGN=%c OK", *s1 );
    }
    else if((u8*)strstr((const char *)ptr, "AT+LAMP=") != NULL) {
        s1 = strstr((const char *)ptr, "AT+LAMP=");
        s1 += strlen("AT+LAMP=");

        switch( *s1 ) {
        case '?':
            Debug_Printf(SET,
                         "AT+LAMP=%d[%s]",
                         m_carinfo.Flag_LAMPInput & 0x01,
                         m_carinfo.Flag_LAMPInput == SET ? "直接" : "模块");
            return;
        case '0':
            m_carinfo.Flag_LAMPInput = RESET;
            break;

        case '1':
            m_carinfo.Flag_LAMPInput = SET;
            break;

        case '2':
            m_var.Flag_InitLAMPidNo = SET;
            return;
        }
        MX25L128_CreatSaveTask();
        Debug_Printf(SET,
                     "AT+LAMP=%c[%s] OK",
                     *s1,
                     m_carinfo.Flag_LAMPInput == SET ? "直接" : "模块" );
    }
    else if((u8*)strstr((const char *)ptr, "AT+I24=") != NULL) {
        s1  = strstr((const char *)ptr, "AT+I24=");
        s1 += strlen("AT+I24=");
        switch( *s1 ) {
        case '?':
            Debug_Printf(SET,
                         "AT+I24=%d[I2411E输出%s], Inspection=%s",
                         m_var.Flag_Init_I2411E & 0x1,
                         m_var.Flag_Init_I2411E == 0 ? "开" : "关",
                         m_carinfo.Flag_Inspection_Channel == 0 ? "WIFI" : "BlueTooth");
            return;
        case '0':
            m_var.Flag_Init_I2411E = 0;
            m_var.Flag_Init_M35 = 0xff;
            break;
        case '1': 
            m_var.Flag_Init_I2411E = 0xff;
            break;
        case '2':
            m_carinfo.Flag_Inspection_Channel = 0;
            break;
        case '3':
            m_carinfo.Flag_Inspection_Channel = 0xff;
            break;
        default:
            if( m_var.Flag_Init_I2411E == 0 ) {
                Usart_SendString( I2411E_UART, s1 );
            }
            return;
        }
        MX25L128_CreatSaveTask();
        Debug_Printf(SET,
                     "AT+I24=%c[I2411E输出%s] OK",
                     *s1,
                     m_var.Flag_Init_I2411E == 0 ? "开" : "关" );
    }
    else if((u8*)strstr((const char *)ptr, "AT+M35=") != NULL) {
        s1  = strstr((const char *)ptr, "AT+M35=");
        s1 += strlen("AT+M35=");
        switch( *s1 ) {
        case '0':
            m_var.Flag_Init_M35 = 0;
            m_var.Flag_Init_I2411E = 0xff;
            break;
        case '1':
            m_var.Flag_Init_M35 = 0xff;
            break;
        case '?':
            Debug_Printf(SET,
                         "AT+M35=%d[M35输出%s]",
                         m_var.Flag_Init_M35 & 0x1,
                         m_var.Flag_Init_M35 == 0 ? "开" : "关" );
            return;
        case 'D':
            if( *(s1+1) == 'I' && *(s1+2) == 'A' && *(s1+3) == 'L' ) { // 17.6.5 csYxj(728920175@qq.com)
                Debug_Printf(SET, "AT+M35=DIAL OK");
                GPRS_CmdNow = Call_Start;
            }
            return;
        case 'S':
            if( *(s1+1) == 'M' && *(s1+2) == 'S' ) { // 17.6.8 csYxj(728920175@qq.com)
                Debug_Printf(SET, "AT+M35=SMS OK");
                GTW_BurglarAlarm.Flag_SMS = SET;
            }
            return;
        default:
            if( m_var.Flag_Init_M35 == 0 ) {
                Usart_SendString( M35_UART, s1 );
            }
            return;
        }
        MX25L128_CreatSaveTask();
        Debug_Printf(SET,
                     "AT+M35=%c[M35输出%s] OK",
                     *s1,
                     m_var.Flag_Init_M35 == 0 ? "开" : "关" );
    }
    else if((u8*)strstr((const char *)ptr, "AT+RF433=") != NULL) {
        s1  = strstr((const char *)ptr, "AT+RF433=");
        s1 += strlen("AT+RF433=");
        switch( *s1 ) {
        case '0':
            m_var.Flag_Init_RF433 = 0;
            break;
        case '1':
            m_var.Flag_Init_RF433 = 0xff;
            break;
        case '?':
            Debug_Printf(SET,
                         "AT+RF433=%d[RF433输出%s]",
                         m_var.Flag_Init_RF433 & 0x1,
                         m_var.Flag_Init_RF433 == 0 ? "开" : "关" );
            return;
        case 'S':
            if( *(s1+1) == 'H' && *(s1+2) == 'O' && *(s1+3) == 'W' ) { // 17.6.8 csYxj(728920175@qq.com)
                Debug_Printf(SET,
                             "采集仪编号=(%02x%02x%02x%02x)[%02x,%02x]\r\n\t鸣笛%s[%s]\t远光%s[%s]\r\n\t市区%s[%s]\t高速%s[%s]\r\n\t无线%s[%s]\t服务区%s[%s]",
                             CheckerAnswer_ID.c[0], CheckerAnswer_ID.c[1], CheckerAnswer_ID.c[2], CheckerAnswer_ID.c[3], RF433_MsgMask, RF433_MsgData,
                             DataCollectorMsg.MsgWhistle == SET ? "禁止" : "允许",     GTW02_BoardCast_Msg.MsgWhistle == SET ? "有效" : "无效",
                             DataCollectorMsg.MsgHighBeams == SET ? "禁止" : "允许",   GTW02_BoardCast_Msg.MsgHighBeams == SET ? "有效" : "无效",
                             DataCollectorMsg.MsgCity == SET ? "禁止" : "允许",        GTW02_BoardCast_Msg.MsgCity == SET ? "有效" : "无效",
                             DataCollectorMsg.MsgHighway == SET ? "禁止" : "允许",     GTW02_BoardCast_Msg.MsgHighway == SET ? "有效" : "无效",
                             DataCollectorMsg.MsgWirelessSignal == SET ? "禁止" : "允许", GTW02_BoardCast_Msg.MsgWirelessSignal == SET ? "有效" : "无效",
                             DataCollectorMsg.MsgRestAreas == SET ? "禁止" : "允许",   GTW02_BoardCast_Msg.MsgRestAreas == SET ? "有效" : "无效");
            }
            return;
        }
        Debug_Printf(SET,
                     "AT+RF433=%c[RF433输出%s] OK",
                     *s1,
                     m_var.Flag_Init_RF433 == 0 ? "开" : "关");
    }
    else if((u8*)strstr((const char *)ptr, "AT+GPS=") != NULL) {
        Debug_Printf(SET, "AT+GPS=OK");
        s1 = strstr((const char *)ptr, "AT+GPS=");
        s1 += strlen("AT+GPS=");
        Usart_SendString(GPS_UART, s1);
    }
    else if((u8*)strstr((const char *)ptr, "AT+XFS=") != NULL ) {
        Debug_Printf(SET, "AT+XFS=OK");
        s1 = strstr((const char *)ptr, "AT+XFS=");
        s1 += strlen("AT+XFS=");
        Usart_SendString(XFS_UART, s1);
    }
    else if((u8*)strstr((const char *)ptr, "AT+STUDY") != NULL) {
        m_var.Flag_InputStudy = 0xff;
        Debug_Printf(SET, "AT+STUDY OK");
    }
#ifdef __HARDWARE_TEST__
    else if((u8*)strstr((const char *)ptr, "AT+DBGWIFI") != NULL) { //打开调试wifi模块
        Flag_FinishScan = RESET;
        Count_I2411E_SCAN = 4000;
        Usart_SendString(I2411E_UART,"AT+DISCOVERDEVICE=1,5,3\r");//搜索蓝牙
    }
    else if((u8*)strstr((const char *)ptr, "AT+DBGBLU") != NULL) { //打开调试蓝牙模块
        Flag_FinishScan = RESET;
        Count_I2411E_SCAN = 4000;
        Usart_SendString(I2411E_UART,"AT+WSCAN\r");//搜索wifi
    }
    else if((u8*)strstr((const char *)ptr, "AT+DBGRF") != NULL) { //打开调试RF4432模块
        Si4432_SendChangeLight(Si4432_TxBuf);//发送变光型号
    }
    else if((u8*)strstr((const char *)ptr, "AT+DBGM35") != NULL) {
        GPRS_CmdNow = M35_Init_Start;
    }
    else if((u8*)strstr((const char *)ptr, "AT+DBGGPSON") != NULL) {
        GPS_HardwareTestON = SET;//测试GPS模块
        Debug_Printf(SET, "AT+DBGGPSON OK");
    }
    else if((u8*)strstr((const char *)ptr, "AT+DBGGPSOFF") != NULL) {
        GPS_HardwareTestON = RESET;//测试GPS模块
        Debug_Printf(SET, "AT+DBGGPSOFF OK");
    }
    else if((u8*)strstr((const char *)ptr, "AT+DBGTTS") != NULL) {
        //打开调试语音模块
        XFS_WriteBuffer(xfs_auto, "语音测试");
    }
    else if((u8*)strstr((const char *)ptr, "AT+LEDON") != NULL) {
        ON_LED();
    }
    else if((u8*)strstr((const char *)ptr, "AT+LEDOFF") != NULL) {
        OFF_LED();
    }
#endif
}

