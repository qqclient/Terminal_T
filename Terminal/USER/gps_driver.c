/******************** (C) COPYRIGHT soarsky ************************************
* File Name          :
* Author             :
* Version            :
* Date               :
* Description        :待优化
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"

extern void GPS_DMA_Config(void);

nmea_msg         gpsx;      //GPS信息
nmea_utc_time    GPS_Time;  //北京时间

// Direction_Typedef 中定义的方向对应名称
unsigned char txtDirection[8][6] = { {"东"}, {"南"}, {"西"}, {"北"}, {"东南"}, {"西南"}, {"东北"}, {"西北"} };

/* Private variables ---------------------------------------------------------*/
/* DMA接收缓冲  */
//#define GPS_RBUFF_SIZE            512                   //串口接收缓冲区大小
#define HALF_GPS_RBUFF_SIZE       (GPS_RBUFF_SIZE/2)    //串口接收缓冲区一半
uint8_t GPS_RxBuff[GPS_RBUFF_SIZE];

/* Private functions ---------------------------------------------------------*/
volatile unsigned char   Flag_RevGpsFinished = RESET;//用于标志是否接受到一次GPS数据
volatile unsigned short  RevGpsCount = 0;//接受到GPS数据的长度

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
* Description    : GPS里程输出函数
* Input          :
* Output         : None
* Return         : 返回总里程的数据格式为float型，单位为km.
*******************************************************************************/
void total_distance(void)
{
    static volatile unsigned int SpeedMileTime = 0;
    static volatile unsigned int WriteTime = 0;

    if(gpsx.gpssta == 1 || gpsx.gpssta == 2) { // GPS定位成功

        if(Sys_TimeCount - SpeedMileTime >= 5 ) {
            SpeedMileTime = Sys_TimeCount;
            m_carinfo.distance += ( GetSpeed() * 1000 / 3600 ) * 5;  // 单位为 m/sec

            if( Sys_TimeCount - WriteTime >= 1 * 60 * 60 ) {
                WriteTime = Sys_TimeCount;
                MX25L128_CreatSaveTask();
            }
        }
    }
}

/*******************************************************************************
* Function Name  : main
* Description    : 程序入口
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

    if(Flag_RevGpsFinished == SET) {    //接收到一次数据了
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
            // 时间换算
            GMTconvert(&gpsx.utc, &GPS_Time, 8, 1);

            total_distance(); // 统计里程

            // 航向计算
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
                   // XFS_WriteBuffer(xfs_auto, "航向%s", txtDirection[gpsx.direction] );
                    if( m_var.Flag_Debug_Enable == SET )
                        Debug_Printf( SET, "航向%s", txtDirection[gpsx.direction] );
                }
#endif
            }

#ifdef __DEBUG
            // gpsx.gpssta=GPS状态:0=未定位;1=非差分定位;2=差分定位;6=正在估算.
            if( Sys_TimeCount - GPS_RunTime >= 5 && Sys_TimeCount - Sys_RunTime > 30 ) {
                GPS_RunTime = Sys_TimeCount;

                s1 = buf;
                sprintf( s1,
                         "时间:%04d-%02d-%02d %02d:%02d:%02d\r\n",
                         GPS_Time.year,
                         GPS_Time.month,
                         GPS_Time.date,
                         GPS_Time.hour,
                         GPS_Time.min,
                         GPS_Time.sec);
                s1 = buf + strlen(buf);

                sprintf( s1, "速度%d,", GetSpeed() );
                s1 = buf + strlen(buf);

                sprintf( s1, "(经度:%.7f,", (double)gpsx.longitude / 100000 );   //得到经度字符串
                s1 = buf + strlen(buf);

                sprintf( s1, "纬度:%.7f),", (double)gpsx.latitude / 100000 );   //得到纬度字符串
                s1 = buf + strlen(buf);

                sprintf( s1, "高度:%.1fm\r\n", (float)gpsx.altitude / 10 );  //得到高度字符串
                s1 = buf + strlen(buf);

                sprintf( s1, "定位星数:%02d,", gpsx.posslnum);        //用于定位的GPS卫星数
                s1 = buf + strlen(buf);

                sprintf( s1, "GPS 星数:%02d,", gpsx.svnum % 100);     //可见GPS卫星数
                s1 = buf + strlen(buf);

                sprintf( s1, "北斗星数:%02d", gpsx.beidou_svnum % 100); //可见北斗卫星数
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
                    //XFS_WriteBuffer(xfs_auto, "未定位");
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf(SET, "未定位");
                    gps_sta = 0;
                }
            }
            else if( gpsx.gpssta == 6 ) {
                if( gps_sta != 6 ) {
                    //XFS_WriteBuffer(xfs_auto, "正在定位");
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf(SET, "正在定位");
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
* Input          : 从buf里面得到第cx个逗号所在的位置
* Output         : None
* Return         : 返回值:0~0XFE,代表逗号所在位置的偏移.0XFF,代表不存在第cx个逗号
*******************************************************************************/
u8 NMEA_Comma_Pos(u8 *buf,u8 cx)
{
    u8 *p=buf;
    while(cx)
    {
        if(*buf=='*'||*buf<' '||*buf>'z')return 0XFF;//遇到'*'或者非法字符,则不存在第cx个逗号
        if(*buf==',')cx--;
        buf++;
    }
    return buf-p;
}


/*******************************************************************************
* Function Name  : NMEA_Pow
* Description    : m^n函数
* Input          : None
* Output         : None
* Return         : 返回值:m^n次方.
*******************************************************************************/
u32 NMEA_Pow(u8 m,u8 n)
{
    u32 result=1;
    while(n--)result*=m;
    return result;
}


/*******************************************************************************
* Function Name  : NMEA_Str2num
* Description    : str转换为数字,以','或者'*'结束
* Input          : buf:数字存储区      dx:小数点位数,返回给调用函数
* Output         : None
* Return         : 转换后的数值
*******************************************************************************/
int NMEA_Str2num(u8 *buf,u8*dx)
{
    u8 *p=buf;
    u32 ires=0,fres=0;
    u8 ilen=0,flen=0,i;
    u8 mask=0;
    int res;
    while(1) { //得到整数和小数的长度
        if(*p=='-') {
            mask|=0X02;    //是负数
            p++;
        }
        if(*p==','||(*p=='*'))break;//遇到结束了
        if(*p=='.') {
            mask|=0X01;    //遇到小数点了
            p++;
        }
        else if(*p>'9'||(*p<'0')) { //有非法字符
            ilen=0;
            flen=0;
            break;
        }
        if(mask&0X01)flen++;
        else ilen++;
        p++;
    }
    if(mask&0X02)buf++;	//去掉负号
    for(i=0; i<ilen; i++) {	//得到整数部分数据
        ires+=NMEA_Pow(10,ilen-1-i)*(buf[i]-'0');
    }
    if(flen>5)flen=5;	//最多取5位小数
    *dx=flen;	 		//小数点位数
    for(i=0; i<flen; i++) { //得到小数部分数据
        fres+=NMEA_Pow(10,flen-1-i)*(buf[ilen+1+i]-'0');
    }
    res=ires*NMEA_Pow(10,flen)+fres;
    if(mask&0X02)res=-res;
    return res;
}


/*******************************************************************************
* Function Name  : NMEA_GPGSV_Analysis
* Description    : 分析GPGSV信息
* Input          : gpsx:nmea信息结构体   buf:接收到的GPS数据缓冲区首地址
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
    len=p1[7]-'0';								//得到GPGSV的条数
    posx=NMEA_Comma_Pos(p1,3); 					//得到可见卫星总数
    if(posx!=0XFF)gpsx->svnum=NMEA_Str2num(p1+posx,&dx);
    for(i=0; i<len; i++) {
        p1=(u8*)strstr((const char *)p,"$GPGSV");
        for(j=0; j<4; j++) {
            posx=NMEA_Comma_Pos(p1,4+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].num=NMEA_Str2num(p1+posx,&dx);	//得到卫星编号
            else break;
            posx=NMEA_Comma_Pos(p1,5+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].eledeg=NMEA_Str2num(p1+posx,&dx);//得到卫星仰角
            else break;
            posx=NMEA_Comma_Pos(p1,6+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].azideg=NMEA_Str2num(p1+posx,&dx);//得到卫星方位角
            else break;
            posx=NMEA_Comma_Pos(p1,7+j*4);
            if(posx!=0XFF)gpsx->slmsg[slx].sn=NMEA_Str2num(p1+posx,&dx);	//得到卫星信噪比
            else break;
            slx++;
        }
        p=p1+1;//切换到下一个GPGSV信息
    }
}


/*******************************************************************************
* Function Name  : NMEA_BDGSV_Analysis
* Description    : 分析BDGSV信息
* Input          : gpsx:nmea信息结构体   buf:接收到的GPS数据缓冲区首地址
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
    len=p1[7]-'0';								//得到BDGSV的条数
    posx=NMEA_Comma_Pos(p1,3); 					//得到可见北斗卫星总数
    if(posx!=0XFF)gpsx->beidou_svnum=NMEA_Str2num(p1+posx,&dx);
    for(i=0; i<len; i++) {
        p1=(u8*)strstr((const char *)p,"$BDGSV");
        for(j=0; j<4; j++) {
            posx=NMEA_Comma_Pos(p1,4+j*4);
            if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_num=NMEA_Str2num(p1+posx,&dx);	//得到卫星编号
            else break;
            posx=NMEA_Comma_Pos(p1,5+j*4);
            if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_eledeg=NMEA_Str2num(p1+posx,&dx);//得到卫星仰角
            else break;
            posx=NMEA_Comma_Pos(p1,6+j*4);
            if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_azideg=NMEA_Str2num(p1+posx,&dx);//得到卫星方位角
            else break;
            posx=NMEA_Comma_Pos(p1,7+j*4);
            if(posx!=0XFF)gpsx->beidou_slmsg[slx].beidou_sn=NMEA_Str2num(p1+posx,&dx);	//得到卫星信噪比
            else break;
            slx++;
        }
        p=p1+1;//切换到下一个BDGSV信息
    }
}

/*******************************************************************************
* Function Name  : NMEA_GNGGA_Analysis
* Description    : 分析GNGGA信息
* Input          : gpsx:nmea信息结构体  buf:接收到的GPS数据缓冲区首地址
* Output         : None
* Return         : None
*******************************************************************************/
void NMEA_GNGGA_Analysis(nmea_msg *gpsx,u8 *buf)
{
    u8 *p1,dx;
    u8 posx;
    p1=(u8*)strstr((const char *)buf,"$GNGGA");
    posx=NMEA_Comma_Pos(p1,6);								//得到GPS状态
    if(posx!=0XFF)gpsx->gpssta=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,7);								//得到用于定位的卫星数
    if(posx!=0XFF)gpsx->posslnum=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,9);								//得到海拔高度
    if(posx!=0XFF)gpsx->altitude=NMEA_Str2num(p1+posx,&dx);
}


/*******************************************************************************
* Function Name  : NMEA_GNGSA_Analysis
* Description    : 分析GNGSA信息
* Input          : gpsx:nmea信息结构体  buf:接收到的GPS数据缓冲区首地址
* Output         : None
* Return         : None
*******************************************************************************/
void NMEA_GNGSA_Analysis(nmea_msg *gpsx,u8 *buf)
{
    u8 *p1,dx;
    u8 posx;
    u8 i;
    p1=(u8*)strstr((const char *)buf,"$GNGSA");
    posx=NMEA_Comma_Pos(p1,2);								//得到定位类型
    if(posx!=0XFF)gpsx->fixmode=NMEA_Str2num(p1+posx,&dx);
    for(i=0; i<12; i++) {									//得到定位卫星编号
        posx=NMEA_Comma_Pos(p1,3+i);
        if(posx!=0XFF)gpsx->possl[i]=NMEA_Str2num(p1+posx,&dx);
        else break;
    }
    posx=NMEA_Comma_Pos(p1,15);								//得到PDOP位置精度因子
    if(posx!=0XFF)gpsx->pdop=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,16);								//得到HDOP位置精度因子
    if(posx!=0XFF)gpsx->hdop=NMEA_Str2num(p1+posx,&dx);
    posx=NMEA_Comma_Pos(p1,17);								//得到VDOP位置精度因子
    if(posx!=0XFF)gpsx->vdop=NMEA_Str2num(p1+posx,&dx);
}


/*******************************************************************************
* Function Name  : NMEA_GNRMC_Analysis
* Description    : 分析GNRMC信息
* Input          : gpsx:nmea信息结构体   buf:接收到的GPS数据缓冲区首地址
* Output         : None
* Return         : None
*******************************************************************************/
void NMEA_GNRMC_Analysis(nmea_msg *gpsx,u8 *buf)
{
    u8 *p1,dx;
    u8 posx;
    u32 temp;
    float rs;
    p1=(u8*)strstr((const char *)buf,"$GNRMC");//"$GNRMC",经常有&和GNRMC分开的情况,故只判断GPRMC.
    posx=NMEA_Comma_Pos(p1,1);								//得到UTC时间
    if(posx!=0XFF) {
        temp=NMEA_Str2num(p1+posx,&dx)/NMEA_Pow(10,dx);	 	//得到UTC时间,去掉ms
        gpsx->utc.hour=temp/10000;
        gpsx->utc.min=(temp/100)%100;
        gpsx->utc.sec=temp%100;
    }
    posx=NMEA_Comma_Pos(p1,3);								//得到纬度
    if(posx!=0XFF) {
        temp=NMEA_Str2num(p1+posx,&dx);
        gpsx->latitude=temp/NMEA_Pow(10,dx+2);	//得到°
        rs=temp%NMEA_Pow(10,dx+2);				//得到'
        gpsx->latitude=gpsx->latitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为°
    }
    posx=NMEA_Comma_Pos(p1,4);								//南纬还是北纬
    if(posx!=0XFF)gpsx->nshemi=*(p1+posx);
    posx=NMEA_Comma_Pos(p1,5);								//得到经度
    if(posx!=0XFF) {
        temp=NMEA_Str2num(p1+posx,&dx);
        gpsx->longitude=temp/NMEA_Pow(10,dx+2);	//得到°
        rs=temp%NMEA_Pow(10,dx+2);				//得到'
        gpsx->longitude=gpsx->longitude*NMEA_Pow(10,5)+(rs*NMEA_Pow(10,5-dx))/60;//转换为°
    }
    posx=NMEA_Comma_Pos(p1,6);								//东经还是西经
    if(posx!=0XFF)gpsx->ewhemi=*(p1+posx);
    posx=NMEA_Comma_Pos(p1,9);								//得到UTC日期
    if(posx!=0XFF) {
        temp=NMEA_Str2num(p1+posx,&dx);		 				//得到UTC日期
        gpsx->utc.date=temp/10000;
        gpsx->utc.month=(temp/100)%100;
        gpsx->utc.year=2000+temp%100;
    }
}


/*******************************************************************************
* Function Name  : NMEA_GNVTG_Analysis
* Description    : 分析GNVTG信息
* Input          : gpsx:nmea信息结构体   buf:接收到的GPS数据缓冲区首地址
* Output         : None
* Return         : None
*******************************************************************************/
void NMEA_GNVTG_Analysis(nmea_msg *gpsx,u8 *buf)
{
    u8 *p1,dx;
    u8 posx;
    p1=(u8*)strstr((const char *)buf,"$GNVTG");
    posx=NMEA_Comma_Pos(p1,1);								//得到对地真航向
    if(posx!=0XFF) {
        gpsx->direction_angle=NMEA_Str2num(p1+posx,&dx);
    }
    posx=NMEA_Comma_Pos(p1,7);								//得到地面速率
    if(posx!=0XFF) {
        gpsx->speed=NMEA_Str2num(p1+posx,&dx);
        if(dx<3)gpsx->speed*=NMEA_Pow(10,3-dx);	 	 		//确保扩大1000倍
    }
}



/*******************************************************************************
* Function Name  : GPS_Analysis
* Description    : 提取NMEA-0183信息
* Input          : gpsx:nmea信息结构体   buf:接收到的GPS数据缓冲区首地址
* Output         : None
* Return         : None
*******************************************************************************/
void GPS_Analysis(nmea_msg *gpsx,u8 *buf)
{
    NMEA_GPGSV_Analysis(gpsx,buf);	//GPGSV解析
    NMEA_BDGSV_Analysis(gpsx,buf);	//BDGSV解析
    NMEA_GNGGA_Analysis(gpsx,buf);	//GNGGA解析
    NMEA_GNGSA_Analysis(gpsx,buf);	//GPNSA解析
    NMEA_GNRMC_Analysis(gpsx,buf);	//GPNMC解析
    NMEA_GNVTG_Analysis(gpsx,buf);	//GPNTG解析
}

/*******************************************************************************
* Function Name  : TIM4_Set
* Description    : 设置TIM4的开关
* Input          : sta:0，关闭;1,开启;
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_Set(u8 sta)
{
    if(sta) {
        TIM_SetCounter(TIM4, 0);    // 计数器清空
        TIM_Cmd(TIM4, ENABLE);      // 使能TIMx
    }
    else
        TIM_Cmd(TIM4, DISABLE);     // 关闭定时器4
}

#ifndef __GPS_DMA__
/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : 定时器4中断服务程序
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET) { //是更新中断
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update  );  //清除TIMx更新中断标志

        Flag_RevGpsFinished = SET;	//标记接收完成
        TIM4_Set(0);			//关闭TIM4
    }
}


/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : 定时器4中断服务程序
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPS_IRQHandler(void)
{
    unsigned char TempData;

    if(USART_GetITStatus(GPS_UART, USART_IT_RXNE) != RESET) {   // 接收到数据
        USART_ClearITPendingBit(GPS_UART,USART_IT_RXNE);        // 清除挂起标志位
        TempData = USART_ReceiveData( GPS_UART );
        if(RevGpsCount < GPS_RBUFF_SIZE) {  // 还可以接收数据
            TIM_SetCounter( TIM4, 0 );      // 计数器清空
            if(RevGpsCount == 0) {
                TIM4_Set(1);	 	        // 使能定时器4的中断
            }
            GPS_RxBuff[RevGpsCount++] = TempData;   // 记录接收到的值
        }
        else {
            Flag_RevGpsFinished = SET;			    // 强制标记接收完成
        }
    }
}

#else
/*******************************************************************************
* Function Name  : TIM4_IRQHandler
* Description    : 定时器4中断服务程序
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void GPS_DMA_IRQHandler(void)
{
    if(DMA_GetITStatus(GPS_DMA_IT_HT) )         /* DMA 半传输完成 */
    {
        Flag_RevGpsFinished = SET;                //设置半传输完成标志位
        DMA_ClearFlag(GPS_DMA_FLAG_HT);
    }
    else if(DMA_GetITStatus(GPS_DMA_IT_TC))     /* DMA 传输完成 */
    {
        Flag_RevGpsFinished = SET;                    //设置传输完成标志位
        DMA_ClearFlag(GPS_DMA_FLAG_TC);

    }
}

#endif
/*******************************************************************************
* Function Name  : IsLeapYear
* Description    : 判断是否为闰年
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
* Description    : 格林威志标准时间转换为北京时间
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  GMTconvert(nmea_utc_time *SourceTime, nmea_utc_time *ConvertTime, unsigned char GMT, unsigned char AREA)
{
    uint32_t YY, MM, DD, hh, mm, ss;   //年月日时分秒暂存变量
    if(GMT == 0)    return;              //如果处于0时区直接返回
    if(GMT > 12)    return;              //时区最大为12 超过则返回
    YY = SourceTime->year;                //获取年
    MM = SourceTime->month;                 //获取月
    DD = SourceTime->date;                 //获取日
    hh = SourceTime->hour;                //获取时
    mm = SourceTime->min;                 //获取分
    ss = SourceTime->sec;                 //获取秒

    if(AREA) {                       //东(+)时区处理
        if(hh + GMT < 24) {
            hh += GMT;  // 如果与格林尼治时间处于同一天则仅加小时即可
        }
        else {           // 如果已经晚于格林尼治时间1天则进行日期处理
            hh = hh + GMT - 24;    //先得出时间
            if(MM == 1 || MM == 3 || MM == 5 || MM == 7 || MM == 8 || MM == 10) { //大月份(12月单独处理)
                if(DD < 31) {
                    DD++;
                }
                else {
                    DD = 1;
                    MM++;
                }
            }
            else if(MM == 4 || MM == 6 || MM == 9 || MM == 11) {       //小月份2月单独处理)
                if(DD < 30) {
                    DD++;
                }
                else {
                    DD    =    1;
                    MM    ++;
                }
            }
            else if(MM == 2) { //处理2月份
                if((DD == 29) || (DD == 28 && IsLeapYear(YY) == 0)) {  //本来是闰年且是2月29日 或者不是闰年且是2月28日
                    DD    =    1;
                    MM    ++;
                }
                else {
                    DD++;
                }
            }
            else if(MM == 12) { //处理12月份
                if(DD < 31) {
                    DD++;
                }
                else {       //跨年最后一天
                    DD    =    1;
                    MM    =    1;
                    YY    ++;
                }
            }
        }
    }
    else {
        if(hh >= GMT) {
            hh    -=    GMT;    //如果与格林尼治时间处于同一天则仅减小时即可
        }
        else {                  //如果已经早于格林尼治时间1天则进行日期处理
            hh    =    hh + 24 - GMT;    //先得出时间
            if(MM == 2 || MM == 4 || MM == 6 || MM == 8 || MM == 9 || MM == 11) { //上月是大月份(1月单独处理)
                if(DD > 1) {
                    DD--;
                }
                else {
                    DD    =    31;
                    MM    --;
                }
            }
            else if(MM == 5 || MM == 7 || MM == 10 || MM == 12) {       //上月是小月份2月单独处理)
                if(DD > 1) {
                    DD--;
                }
                else {
                    DD    =    30;
                    MM    --;
                }
            }
            else if(MM == 3) { //处理上个月是2月份
                if((DD == 1) && IsLeapYear(YY) == 0) {               //不是闰年
                    DD    =    28;
                    MM    --;
                }
                else {
                    DD--;
                }
            }
            else if(MM == 1) { //处理1月份
                if(DD > 1) {
                    DD--;
                }
                else {       //新年第一天
                    DD    =    31;
                    MM    =    12;
                    YY    --;
                }
            }
        }
    }
    ConvertTime->year   =    YY;                //更新年
    ConvertTime->month  =    MM;                //更新月
    ConvertTime->date   =    DD;                //更新日
    ConvertTime->hour   =    hh;                //更新时
    ConvertTime->min    =    mm;                //更新分
    ConvertTime->sec    =    ss;                //更新秒

    if( (m_var.FLag_UpdateClock & 0x1) == RESET && YY > 0 && MM > 0 && DD > 0  ) {
        m_var.FLag_UpdateClock = 0xff;
        m_var.UpdateClock_TimeOver = 60 * 60;

        RTC_SetTime( YY, MM, DD, hh, mm, ss );

#ifdef __DEBUG
        if( m_var.Flag_Debug_Enable == SET ) {
            Debug_Printf(SET, "终端同步GPS时间");
        }
#endif
    }
}


/****************************************END OF FILE**********************************/
