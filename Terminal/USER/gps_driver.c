/******************** (C) COPYRIGHT soarsky ************************************
* File Name          :
* Author             :
* Version            :
* Date               :
* Description        :���Ż�
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"

extern void GPS_DMA_Config(void);

nmea_msg         gpsx;      //GPS��Ϣ
nmea_utc_time    GPS_Time;  //����ʱ��

// Direction_Typedef �ж���ķ����Ӧ����
unsigned char txtDirection[8][6] = { {"��"}, {"��"}, {"��"}, {"��"}, {"����"}, {"����"}, {"����"}, {"����"} };

/* Private variables ---------------------------------------------------------*/
/* DMA���ջ���  */
//#define GPS_RBUFF_SIZE            512                   //���ڽ��ջ�������С
#define HALF_GPS_RBUFF_SIZE       (GPS_RBUFF_SIZE/2)    //���ڽ��ջ�����һ��
uint8_t GPS_RxBuff[GPS_RBUFF_SIZE];

/* Private functions ---------------------------------------------------------*/
volatile unsigned char   Flag_RevGpsFinished = RESET;//���ڱ�־�Ƿ���ܵ�һ��GPS����
volatile unsigned short  RevGpsCount = 0;//���ܵ�GPS���ݵĳ���

int  NMEA_Str2num(u8 *buf,u8*dx);
void GPS_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_BDGSV_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GNGGA_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GNGSA_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GNGSA_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GNRMC_Analysis(nmea_msg *gpsx,u8 *buf);
void NMEA_GNVTG_Analysis(nmea_msg *gpsx,u8 *buf);
u8* data_Little_endian(u8* data,u16 len);
void TIM4_Set(u8 sta);
u8 IsLeapYear(u8 iYear);
void  GMTconvert(nmea_utc_time *SourceTime, nmea_utc_time *ConvertTime, u8 GMT, u8 AREA);

#ifdef __HARDWARE_TEST__
volatile unsigned char GPS_HardwareTestON = RESET;
#endif

#ifdef __DEBUG
u8  gpsx_direction;
u32 gpsx_speed;
#endif



/*******************************************************************************
* Function Name  : total_distance
* Description    : GPS����������
* Input          :
* Output         : None
* Return         : ��������̵����ݸ�ʽΪfloat�ͣ���λΪkm.
*******************************************************************************/
void total_distance(void)
{
    static volatile unsigned int SpeedMileTime = 0;
    static volatile unsigned int WriteTime = 0;

    if(gpsx.gpssta == 1 || gpsx.gpssta == 2) { // GPS��λ�ɹ�

        if(Sys_TimeCount - SpeedMileTime >= 5 ) {
            SpeedMileTime = Sys_TimeCount;
            m_carinfo.distance += ( GetSpeed() * 1000 / 3600 ) * 5;  // ��λΪ m/sec

            if( Sys_TimeCount - WriteTime >= 1 * 60 * 60 ) {
                WriteTime = Sys_TimeCount;
                MX25L128_CreatSaveTask();
            }
        }
    }
}

/*******************************************************************************
* Function Name  : main
* Description    : �������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPS_RxBufferProcess(void)
{
    float tp;
    static unsigned int GPS_RunTime = 0;

#ifdef __DEBUG
    char* s1;
    char buf[256] = { 0 };
    static unsigned char gps_sta = 0xff;
    static unsigned char gps_direction = 0xff;
#endif


#ifdef __HARDWARE_TEST__
    static unsigned int GPS_LastTime = 0;

    if(Flag_RevGpsFinished == SET) {    //���յ�һ��������
        if(Sys_TimeCount - GPS_LastTime >= 5 && GPS_HardwareTestON == SET) {
            GPS_LastTime = Sys_TimeCount;
            Debug_Printf(SET, "\r\nGPS ERROR");
        }
    }
    else {
        GPS_LastTime = Sys_TimeCount;
        if( GPS_HardwareTestON == SET ) {
            Debug_Printf(SET, "\r\nGPS OK");
        }
    }
#endif

    if(Flag_RevGpsFinished == SET) {
        Flag_RevGpsFinished = RESET;

        GPS_RxBuff[RevGpsCount-1] = 0;
        GPS_Analysis( &gpsx, (u8*)GPS_RxBuff );

        RevGpsCount = 0;
        memset(GPS_RxBuff, 0, sizeof(GPS_RxBuff));

#ifdef __GPS_DMA__
        DMA_Cmd(GPS_DMA_CHANNEL, DISABLE);
        GPS_DMA_CHANNEL->CNDTR = GPS_RBUFF_SIZE;
        DMA_Cmd(GPS_DMA_CHANNEL, ENABLE);
#endif
        if( gpsx.gpssta == 1 || gpsx.gpssta == 2 ) {
            // ʱ�任��
            GMTconvert(&gpsx.utc, &GPS_Time, 8, 1);

            total_distance(); // ͳ�����

            // �������
            if( GetSpeed() >= SPEED_MIN ) {
                tp = gpsx.direction_angle / 100;

                if(tp >= 338 ||  tp <= 22)      gpsx.direction = North;
                else if(tp >= 23 && tp <= 67)   gpsx.direction = Northeast;
                else if(tp >= 68 && tp <= 121)  gpsx.direction = East;
                else if(tp >= 122 && tp <= 157) gpsx.direction = Southeast;
                else if(tp >= 158 && tp <= 202) gpsx.direction = South;
                else if(tp >= 203 && tp <= 247) gpsx.direction = Southwest;
                else if(tp >= 248 && tp <= 292) gpsx.direction = West;
                else if(tp >= 293 && tp <= 337) gpsx.direction = Northwest;
#ifdef __DEBUG
                if( gps_direction != gpsx.direction ) {
                    gps_direction = gpsx.direction;
                   // XFS_WriteBuffer(xfs_auto, "����%s", txtDirection[gpsx.direction] );
                    if( m_var.Flag_Debug_Enable == SET )
                        Debug_Printf( SET, "����%s", txtDirection[gpsx.direction] );
                }
#endif
            }

#ifdef __DEBUG
            // gpsx.gpssta=GPS״̬:0=δ��λ;1=�ǲ�ֶ�λ;2=��ֶ�λ;6=���ڹ���.
            if( Sys_TimeCount - GPS_RunTime >= 5 && Sys_TimeCount - Sys_RunTime > 30 ) {
                GPS_RunTime = Sys_TimeCount;

                s1 = buf;
                sprintf( s1,
                         "ʱ��:%04d-%02d-%02d %02d:%02d:%02d\r\n",
                         GPS_Time.year,
                         GPS_Time.month,
                         GPS_Time.date,
                         GPS_Time.hour,
                         GPS_Time.min,
                         GPS_Time.sec);
                s1 = buf + strlen(buf);

                sprintf( s1, "�ٶ�%d,", GetSpeed() );
                s1 = buf + strlen(buf);

                sprintf( s1, "(����:%.7f,", (double)gpsx.longitude / 100000 );   //�õ������ַ���
                s1 = buf + strlen(buf);

                sprintf( s1, "γ��:%.7f),", (double)gpsx.latitude / 100000 );   //�õ�γ���ַ���
                s1 = buf + strlen(buf);

                sprintf( s1, "�߶�:%.1fm\r\n", (float)gpsx.altitude / 10 );  //�õ��߶��ַ���
                s1 = buf + strlen(buf);

                sprintf( s1, "��λ����:%02d,", gpsx.posslnum);        //���ڶ�λ��GPS������
                s1 = buf + strlen(buf);

                sprintf( s1, "GPS ����:%02d,", gpsx.svnum % 100);     //�ɼ�GPS������
                s1 = buf + strlen(buf);

                sprintf( s1, "��������:%02d", gpsx.beidou_svnum % 100); //�ɼ�����������
                s1 = buf + strlen(buf);

                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf(SET, buf);
            }
#endif
        }
#ifdef __DEBUG
        if( Sys_TimeCount - GPS_RunTime >= 5 && Sys_TimeCount - Sys_RunTime > 30 ) {
            GPS_RunTime = Sys_TimeCount;
            if( gpsx.gpssta == 0 ) {
                if( gps_sta != 0 ) {
                    //XFS_WriteBuffer(xfs_auto, "δ��λ");
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf(SET, "δ��λ");
                    gps_sta = 0;
                }
            }
            else if( gpsx.gpssta == 6 ) {
                if( gps_sta != 6 ) {
                    //XFS_WriteBuffer(xfs_auto, "���ڶ�λ");
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf(SET, "���ڶ�λ");
                    gps_sta = 6;
                }
            }
        }
#endif
    }
}


/*******************************************************************************
* Function Name  : NMEA_Comma_Pos
* Description    :
* Input          : ��buf����õ���cx���������ڵ�λ��
* Output         : None
* Return         : ����ֵ:0~0XFE,����������λ�õ�ƫ��.0XFF,�������ڵ�cx������
*******************************************************************************/
u8 NMEA_Comma_Pos(u8 *buf,u8 cx)
{
    u8 *p=buf;
    while(cx)
    {
        if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//����'*'���߷Ƿ��ַ�,�򲻴��ڵ�cx������
        if(*buf==',')cx--;
        buf++;
    }
    return buf-p;
}


/*******************************************************************************
* Function Name  : NMEA_Pow
* Description    : m^n����
* Input          : None
* Output         : None
* Return         : ����ֵ:m^n�η�.
*******************************************************************************/
u32 NMEA_Pow(u8 m,u8 n)
{
    u32 result=1;
    while(n--)result*=m;
    return result;
}


/*******************************************************************************
* Function Name  : NMEA_Str2num
* Description    : strת��Ϊ����,��','����'*'����
* Input          : buf:���ִ洢��      dx:С����λ��,���ظ����ú���
* Output         : None
* Return         : ת�������ֵ
*******************************************************************************/
int NMEA_Str2num(u8 *buf,u8*dx)
{
    u8 *p=buf;
    u32 ires=0,fres=0;
    u8 ilen=0,flen=0,i;
    u8 mask=0;
    int res;
    while(1) { //�õ�������С���ĳ���
        if(*p=='-') {
            mask|=0X02;    //�Ǹ���
            p++;
        }
        if(*p==','||(*p=='*'))break;//����������
        if(*p=='.') {
            mask|=0X01;    //����С������
            p++;
        }
        else if(*p>'9'||(*p<'0')) { //�зǷ��ַ�
            ilen=0;
            flen=0;
            break;
        }
        if(mask&0X01)flen++;
        else ilen++;
        p++;
    }
    if(mask&0X02)buf++;	//ȥ������
    for(i=0; i<ilen; i++) {	//�õ�������������
        ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
    }
    if(flen>5)flen=5;	//���ȡ5λС��
    *dx=flen;	 		//С����λ��
    for(i=0; i<flen; i++) { //�õ�С����������
        fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
    }
    res=ires*NMEA_Pow(10,flen)+fres;
    if(mask&0X02)res=-res;
    return res;
}


/*******************************************************************************
* Function Name  : NMEA_GPGSV_Analysis
* Description    : ����GPGSV��Ϣ
* Input          : gpsx:nmea��Ϣ�ṹ��   buf:���յ���GPS���ݻ������׵�ַ
* Output         : None
* Return         : None
*******************************************************************************/
void NMEA_GPGSV_Analysis(nmea_msg *gpsx,u8 *buf)
{
    u8 *p,*p1,dx;
    u8 len,i,j,slx=0;
    u8 posx;
    p=buf;
    p1=(u8*)strstr((const char *)p,"$GPGSV");
    len=p1[7]-'0';								//�õ�GPGSV������
    posx=NMEA_Comma_Pos(p1,3); 					//�õ��ɼ���������
    if(posx!=0XFF)gpsx->svnum=NMEA_Str2num(p1+posx,&dx);
    for(i=0; i<len; i++) {
        p1=(u8*)strstr((const char *)p,"$GPGSV");
        for(j=0; j<4; j++) {
            posx=NMEA_Comma_Pos(p1,4+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].num=NMEA_Str2num(p1+posx,&dx);	//�õ����Ǳ��
            else break;
            posx=NMEA_Comma_Pos(p1,5+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//�õ���������
            else break;
            posx=NMEA_Comma_Pos(p1,6+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].azideg=NMEA_Str2num(p1+posx,&dx);//�õ����Ƿ�λ��
            else break;
            posx=NMEA_Comma_Pos(p1,7+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].sn=NMEA_Str2num(p1+posx,&dx);	//�õ����������
            else break;
            slx++;
        }
        p=p1+1;//�л�����һ��GPGSV��Ϣ
    }
}


/*******************************************************************************
* Function Name  : NMEA_BDGSV_Analysis
* Description    : ����BDGSV��Ϣ
* Input          : gpsx:nmea��Ϣ�ṹ��   buf:���յ���GPS���ݻ������׵�ַ
* Output         : None
* Return         : None
*******************************************************************************/
void NMEA_BDGSV_Analysis(nmea_msg *gpsx,u8 *buf)
{
    u8 *p,*p1,dx;
    u8 len,i,j,slx=0;
    u8 posx;
    p=buf;
    p1=(u8*)strstr((const char *)p,"$BDGSV");
    len=p1[7]-'0';								//�õ�BDGSV������
    posx=NMEA_Comma_Pos(p1,3); 					//�õ��ɼ�������������
    if(posx!=0XFF)gpsx->beidou_svnum=NMEA_Str2num(p1+posx,&dx);
    for(i=0; i<len; i++) {
        p1=(u8*)strstr((const char *)p,"$BDGSV");
        for(j=0; j<4; j++) {
            posx=NMEA_Comma_Pos(p1,4+j*4);
            if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_num=NMEA_Str2num(p1+posx,&dx);	//�õ����Ǳ��
            else break;
            posx=NMEA_Comma_Pos(p1,5+j*4);
            if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_eledeg=NMEA_Str2num(p1+posx,&dx);//�õ���������
            else break;
            posx=NMEA_Comma_Pos(p1,6+j*4);
            if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_azideg=NMEA_Str2num(p1+posx,&dx);//�õ����Ƿ�λ��
            else break;
            posx=NMEA_Comma_Pos(p1,7+j*4);
            if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_sn=NMEA_Str2num(p1+posx,&dx);	//�õ����������
            else break;
            slx++;
        }
        p=p1+1;//�л�����һ��BDGSV��Ϣ
    }
}

/*******************************************************************************
* Function Name  : NMEA_GNGGA_Analysis
* Description    : ����GNGGA��Ϣ
* Input          : gpsx:nmea��Ϣ�ṹ��  buf:���յ���GPS���ݻ������׵�ַ
* Output         : None
* Return         : None
*******************************************************************************/
void NMEA_GNGGA_Analysis(nmea_msg *gpsx,u8 *buf)
{
    u8 *p1,dx;
    u8 posx;
    p1=(u8*)strstr((const char *)buf,"$GNGGA");
    posx=NMEA_Comma_Pos(p1,6);								//�õ�GPS״̬
    if(posx!=0XFF)gpsx->gpssta=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,7);								//�õ����ڶ�λ��������
    if(posx!=0XFF)gpsx->posslnum=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,9);								//�õ����θ߶�
    if(posx!=0XFF)gpsx->altitude=NMEA_Str2num(p1+posx,&dx);
}


/*******************************************************************************
* Function Name  : NMEA_GNGSA_Analysis
* Description    : ����GNGSA��Ϣ
* Input          : gpsx:nmea��Ϣ�ṹ��  buf:���յ���GPS���ݻ������׵�ַ
* Output         : None
* Return         : None
*******************************************************************************/
void NMEA_GNGSA_Analysis(nmea_msg *gpsx,u8 *buf)
{
    u8 *p1,dx;
    u8 posx;
    u8 i;
    p1=(u8*)strstr((const char *)buf,"$GNGSA");
    posx=NMEA_Comma_Pos(p1,2);								//�õ���λ����
    if(posx!=0XFF)gpsx->fixmode=NMEA_Str2num(p1+posx,&dx);
    for(i=0; i<12; i++) {									//�õ���λ���Ǳ��
        posx=NMEA_Comma_Pos(p1,3+i);
        if(posx!=0XFF)gpsx->possl[i]=NMEA_Str2num(p1+posx,&dx);
        else break;
    }
    posx=NMEA_Comma_Pos(p1,15);								//�õ�PDOPλ�þ�������
    if(posx!=0XFF)gpsx->pdop=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,16);								//�õ�HDOPλ�þ�������
    if(posx!=0XFF)gpsx->hdop=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,17);								//�õ�VDOPλ�þ�������
    if(posx!=0XFF)gpsx->vdop=NMEA_Str2num(p1+posx,&dx);
}


/*******************************************************************************
* Function Name  : NMEA_GNRMC_Analysis
* Description    : ����GNRMC��Ϣ
* Input          : gpsx:nmea��Ϣ�ṹ��   buf:���յ���GPS���ݻ������׵�ַ
* Output         : None
* Return         : None
*******************************************************************************/
void NMEA_GNRMC_Analysis(nmea_msg *gpsx,u8 *buf)
{
    u8 *p1,dx;
    u8 posx;
    u32 temp;
    float rs;
    p1=(u8*)strstr((const char *)buf,"$GNRMC");//"$GNRMC",������&��GNRMC�ֿ������,��ֻ�ж�GPRMC.
    posx=NMEA_Comma_Pos(p1,1);								//�õ�UTCʱ��
    if(posx!=0XFF) {
        temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//�õ�UTCʱ��,ȥ��ms
        gpsx->utc.hour=temp/10000;
        gpsx->utc.min=(temp/100)%100;
        gpsx->utc.sec=temp%100;
    }
    posx=NMEA_Comma_Pos(p1,3);								//�õ�γ��
    if(posx!=0XFF) {
        temp=NMEA_Str2num(p1+posx,&dx);
        gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//�õ���
        rs=temp%NMEA_Pow(10,dx+2);				//�õ�'
        gpsx->latitude=gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//ת��Ϊ��
    }
    posx=NMEA_Comma_Pos(p1,4);								//��γ���Ǳ�γ
    if(posx!=0XFF)gpsx->nshemi=*(p1+posx);
    posx=NMEA_Comma_Pos(p1,5);								//�õ�����
    if(posx!=0XFF) {
        temp=NMEA_Str2num(p1+posx,&dx);
        gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//�õ���
        rs=temp%NMEA_Pow(10,dx+2);				//�õ�'
        gpsx->longitude=gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//ת��Ϊ��
    }
    posx=NMEA_Comma_Pos(p1,6);								//������������
    if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);
    posx=NMEA_Comma_Pos(p1,9);								//�õ�UTC����
    if(posx!=0XFF) {
        temp=NMEA_Str2num(p1+posx,&dx);		 				//�õ�UTC����
        gpsx->utc.date=temp/10000;
        gpsx->utc.month=(temp/100)%100;
        gpsx->utc.year=2000+temp%100;
    }
}


/*******************************************************************************
* Function Name  : NMEA_GNVTG_Analysis
* Description    : ����GNVTG��Ϣ
* Input          : gpsx:nmea��Ϣ�ṹ��   buf:���յ���GPS���ݻ������׵�ַ
* Output         : None
* Return         : None
*******************************************************************************/
void NMEA_GNVTG_Analysis(nmea_msg *gpsx,u8 *buf)
{
    u8 *p1,dx;
    u8 posx;
    p1=(u8*)strstr((const char *)buf,"$GNVTG");
    posx=NMEA_Comma_Pos(p1,1);								//�õ��Ե��溽��
    if(posx!=0XFF) {
        gpsx->direction_angle=NMEA_Str2num(p1+posx,&dx);
    }
    posx=NMEA_Comma_Pos(p1,7);								//�õ���������
    if(posx!=0XFF) {
        gpsx->speed=NMEA_Str2num(p1+posx,&dx);
        if(dx<3)gpsx->speed*=NMEA_Pow(10,3-dx);	 	 		//ȷ������1000��
    }
}



/*******************************************************************************
* Function Name  : GPS_Analysis
* Description    : ��ȡNMEA-0183��Ϣ
* Input          : gpsx:nmea��Ϣ�ṹ��   buf:���յ���GPS���ݻ������׵�ַ
* Output         : None
* Return         : None
*******************************************************************************/
void GPS_Analysis(nmea_msg *gpsx,u8 *buf)
{
    NMEA_GPGSV_Analysis(gpsx,buf);	//GPGSV����
    NMEA_BDGSV_Analysis(gpsx,buf);	//BDGSV����
    NMEA_GNGGA_Analysis(gpsx,buf);	//GNGGA����
    NMEA_GNGSA_Analysis(gpsx,buf);	//GPNSA����
    NMEA_GNRMC_Analysis(gpsx,buf);	//GPNMC����
    NMEA_GNVTG_Analysis(gpsx,buf);	//GPNTG����
}

/*******************************************************************************
* Function Name  : TIM4_Set
* Description    : ����TIM4�Ŀ���
* Input          : sta:0���ر�;1,����;
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_Set(u8 sta)
{
    if(sta) {
        TIM_SetCounter(TIM4, 0);    // ���������
        TIM_Cmd(TIM4, ENABLE);      // ʹ��TIMx
    }
    else
        TIM_Cmd(TIM4, DISABLE);     // �رն�ʱ��4
}

#ifndef __GPS_DMA__
/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : ��ʱ��4�жϷ������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) { //�Ǹ����ж�
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //���TIMx�����жϱ�־

        Flag_RevGpsFinished = SET;	//��ǽ������
        TIM4_Set(0);			//�ر�TIM4
    }
}


/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : ��ʱ��4�жϷ������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPS_IRQHandler(void)
{
    unsigned char TempData;

    if(USART_GetITStatus(GPS_UART, USART_IT_RXNE) != RESET) {   // ���յ�����
        USART_ClearITPendingBit(GPS_UART,USART_IT_RXNE);        // ��������־λ
        TempData = USART_ReceiveData( GPS_UART );
        if(RevGpsCount < GPS_RBUFF_SIZE) {  // �����Խ�������
            TIM_SetCounter( TIM4, 0 );      // ���������
            if(RevGpsCount == 0) {
                TIM4_Set(1);	 	        // ʹ�ܶ�ʱ��4���ж�
            }
            GPS_RxBuff[RevGpsCount++] = TempData;   // ��¼���յ���ֵ
        }
        else {
            Flag_RevGpsFinished = SET;			    // ǿ�Ʊ�ǽ������
        }
    }
}

#else
/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : ��ʱ��4�жϷ������
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPS_DMA_IRQHandler(void)
{
    if(DMA_GetITStatus(GPS_DMA_IT_HT) )         /* DMA �봫����� */
    {
        Flag_RevGpsFinished = SET;                //���ð봫����ɱ�־λ
        DMA_ClearFlag(GPS_DMA_FLAG_HT);
    }
    else if(DMA_GetITStatus(GPS_DMA_IT_TC))     /* DMA ������� */
    {
        Flag_RevGpsFinished = SET;                    //���ô�����ɱ�־λ
        DMA_ClearFlag(GPS_DMA_FLAG_TC);

    }
}

#endif
/*******************************************************************************
* Function Name  : IsLeapYear
* Description    : �ж��Ƿ�Ϊ����
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
unsigned char IsLeapYear(unsigned char iYear)
{
    unsigned short Year;
    Year    =    2000 + iYear;
    if((Year & 3) == 0) {
        return ((Year % 400 == 0) || (Year % 100 != 0));
    }
    return 0;
}

/*******************************************************************************
* Function Name  : GMTconvert
* Description    : ������־��׼ʱ��ת��Ϊ����ʱ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  GMTconvert(nmea_utc_time *SourceTime, nmea_utc_time *ConvertTime, unsigned char GMT, unsigned char AREA)
{
    uint32_t YY, MM, DD, hh, mm, ss;   //������ʱ�����ݴ����
    if(GMT == 0)    return;              //�������0ʱ��ֱ�ӷ���
    if(GMT > 12)    return;              //ʱ�����Ϊ12 �����򷵻�
    YY = SourceTime->year;                //��ȡ��
    MM = SourceTime->month;                 //��ȡ��
    DD = SourceTime->date;                 //��ȡ��
    hh = SourceTime->hour;                //��ȡʱ
    mm = SourceTime->min;                 //��ȡ��
    ss = SourceTime->sec;                 //��ȡ��

    if(AREA) {                       //��(+)ʱ������
        if(hh + GMT < 24) {
            hh += GMT;  // ������������ʱ�䴦��ͬһ�������Сʱ����
        }
        else {           // ����Ѿ����ڸ�������ʱ��1����������ڴ���
            hh = hh + GMT - 24;    //�ȵó�ʱ��
            if(MM == 1 || MM == 3 || MM == 5 || MM == 7 || MM == 8 || MM == 10) { //���·�(12�µ�������)
                if(DD < 31) {
                    DD++;
                }
                else {
                    DD = 1;
                    MM++;
                }
            }
            else if(MM == 4 || MM == 6 || MM == 9 || MM == 11) {       //С�·�2�µ�������)
                if(DD < 30) {
                    DD++;
                }
                else {
                    DD    =    1;
                    MM    ++;
                }
            }
            else if(MM == 2) { //����2�·�
                if((DD == 29) || (DD == 28 && IsLeapYear(YY) == 0)) {  //��������������2��29�� ���߲�����������2��28��
                    DD    =    1;
                    MM    ++;
                }
                else {
                    DD++;
                }
            }
            else if(MM == 12) { //����12�·�
                if(DD < 31) {
                    DD++;
                }
                else {       //�������һ��
                    DD    =    1;
                    MM    =    1;
                    YY    ++;
                }
            }
        }
    }
    else {
        if(hh >= GMT) {
            hh    -=    GMT;    //������������ʱ�䴦��ͬһ�������Сʱ����
        }
        else {                  //����Ѿ����ڸ�������ʱ��1����������ڴ���
            hh    =    hh + 24 - GMT;    //�ȵó�ʱ��
            if(MM == 2 || MM == 4 || MM == 6 || MM == 8 || MM == 9 || MM == 11) { //�����Ǵ��·�(1�µ�������)
                if(DD > 1) {
                    DD--;
                }
                else {
                    DD    =    31;
                    MM    --;
                }
            }
            else if(MM == 5 || MM == 7 || MM == 10 || MM == 12) {       //������С�·�2�µ�������)
                if(DD > 1) {
                    DD--;
                }
                else {
                    DD    =    30;
                    MM    --;
                }
            }
            else if(MM == 3) { //�����ϸ�����2�·�
                if((DD == 1) && IsLeapYear(YY) == 0) {               //��������
                    DD    =    28;
                    MM    --;
                }
                else {
                    DD--;
                }
            }
            else if(MM == 1) { //����1�·�
                if(DD > 1) {
                    DD--;
                }
                else {       //�����һ��
                    DD    =    31;
                    MM    =    12;
                    YY    --;
                }
            }
        }
    }
    ConvertTime->year   =    YY;                //������
    ConvertTime->month  =    MM;                //������
    ConvertTime->date   =    DD;                //������
    ConvertTime->hour   =    hh;                //����ʱ
    ConvertTime->min    =    mm;                //���·�
    ConvertTime->sec    =    ss;                //������

    if( (m_var.FLag_UpdateClock & 0x1) == RESET && YY > 0 && MM > 0 && DD > 0  ) {
        m_var.FLag_UpdateClock = 0xff;
        m_var.UpdateClock_TimeOver = 60 * 60;

        RTC_SetTime( YY, MM, DD, hh, mm, ss );

#ifdef __DEBUG
        if( m_var.Flag_Debug_Enable == SET ) {
            Debug_Printf(SET, "�ն�ͬ��GPSʱ��");
        }
#endif
    }
}


/****************************************END OF FILE**********************************/
