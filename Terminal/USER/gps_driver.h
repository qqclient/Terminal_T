#ifndef __GPS_DRIVER
#define __GPS_DRIVER


#include "stm32f10x.h"

typedef enum {
    East =  0,          // 东
    South = 1,          // 南
    West =  2,          // 西
    North = 3,          // 北
    Southeast = 4,      // 东南
    Southwest = 5,      // 西南
    Northeast = 6,      // 东北
    Northwest = 7,      // 西北
} Direction_Typedef;

//GPS NMEA-0183协议重要参数结构体定义
//卫星信息
__packed typedef struct
{
    u8 num;		//卫星编号
    u8 eledeg;	//卫星仰角
    u16 azideg;	//卫星方位角
    u8 sn;		//信噪比
} nmea_slmsg;


//北斗 NMEA-0183协议重要参数结构体定义
//卫星信息
__packed typedef struct
{
    u8 beidou_num;		//卫星编号
    u8 beidou_eledeg;	//卫星仰角
    u16 beidou_azideg;	//卫星方位角
    u8 beidou_sn;		//信噪比
} beidou_nmea_slmsg;

//UTC时间信息
__packed typedef struct
{
    u16 year;	//年份
    u8 month;	//月份
    u8 date;	//日期
    u8 hour; 	//小时
    u8 min; 	//分钟
    u8 sec; 	//秒钟
} nmea_utc_time;
extern nmea_utc_time    GPS_Time;//北京时间


//NMEA 0183 协议解析后数据存放结构体
__packed typedef struct
{
    u8 svnum;					//可见GPS卫星数
    u8 beidou_svnum;			//可见GPS卫星数
    nmea_slmsg slmsg[12];		//最多12颗GPS卫星
    beidou_nmea_slmsg beidou_slmsg[12];//暂且算最多12颗北斗卫星
    nmea_utc_time utc;			//UTC时间
    u32 latitude;				//纬度 分扩大100000倍,实际要除以100000
    u8 nshemi;					//北纬/南纬,N:北纬;S:南纬
    u32 longitude;			    //经度 分扩大100000倍,实际要除以100000
    u8 ewhemi;					//东经/西经,E:东经;W:西经
    u8 gpssta;					//GPS状态:0,未定位;1,非差分定位;2,差分定位;6,正在估算.
    u8 posslnum;				//用于定位的GPS卫星数,0~12.
    u8 possl[12];				//用于定位的卫星编号
    u8 fixmode;					//定位类型:1,没有定位;2,2D定位;3,3D定位
    u16 pdop;					//位置精度因子 0~500,对应实际值0~50.0
    u16 hdop;					//水平精度因子 0~500,对应实际值0~50.0
    u16 vdop;					//垂直精度因子 0~500,对应实际值0~50.0
    int altitude;			 	//海拔高度,放大了10倍,实际除以10.单位:0.1m
    u16 speed;					//地面速率,放大了1000倍,实际除以10.单位:0.001公里/小时
    int direction_angle;        //对地真北航向，0~360度
    Direction_Typedef  direction;              //方向
} nmea_msg;
extern nmea_msg gpsx;

#define GPS_RBUFF_SIZE            512                   //串口接收缓冲区大小
extern uint8_t GPS_RxBuff[GPS_RBUFF_SIZE];


void GPS_RxBufferProcess(void);

#endif

