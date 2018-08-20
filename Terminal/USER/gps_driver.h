#ifndef __GPS_DRIVER
#define __GPS_DRIVER


#include "stm32f10x.h"

typedef enum {
    East =  0,          // ��
    South = 1,          // ��
    West =  2,          // ��
    North = 3,          // ��
    Southeast = 4,      // ����
    Southwest = 5,      // ����
    Northeast = 6,      // ����
    Northwest = 7,      // ����
} Direction_Typedef;

//GPS NMEA-0183Э����Ҫ�����ṹ�嶨��
//������Ϣ
__packed typedef struct
{
    u8 num;		//���Ǳ��
    u8 eledeg;	//��������
    u16 azideg;	//���Ƿ�λ��
    u8 sn;		//�����
} nmea_slmsg;


//���� NMEA-0183Э����Ҫ�����ṹ�嶨��
//������Ϣ
__packed typedef struct
{
    u8 beidou_num;		//���Ǳ��
    u8 beidou_eledeg;	//��������
    u16 beidou_azideg;	//���Ƿ�λ��
    u8 beidou_sn;		//�����
} beidou_nmea_slmsg;

//UTCʱ����Ϣ
__packed typedef struct
{
    u16 year;	//���
    u8 month;	//�·�
    u8 date;	//����
    u8 hour; 	//Сʱ
    u8 min; 	//����
    u8 sec; 	//����
} nmea_utc_time;
extern nmea_utc_time    GPS_Time;//����ʱ��


//NMEA 0183 Э����������ݴ�Žṹ��
__packed typedef struct
{
    u8 svnum;					//�ɼ�GPS������
    u8 beidou_svnum;			//�ɼ�GPS������
    nmea_slmsg slmsg[12];		//���12��GPS����
    beidou_nmea_slmsg beidou_slmsg[12];//���������12�ű�������
    nmea_utc_time utc;			//UTCʱ��
    u32 latitude;				//γ�� ������100000��,ʵ��Ҫ����100000
    u8 nshemi;					//��γ/��γ,N:��γ;S:��γ
    u32 longitude;			    //���� ������100000��,ʵ��Ҫ����100000
    u8 ewhemi;					//����/����,E:����;W:����
    u8 gpssta;					//GPS״̬:0,δ��λ;1,�ǲ�ֶ�λ;2,��ֶ�λ;6,���ڹ���.
    u8 posslnum;				//���ڶ�λ��GPS������,0~12.
    u8 possl[12];				//���ڶ�λ�����Ǳ��
    u8 fixmode;					//��λ����:1,û�ж�λ;2,2D��λ;3,3D��λ
    u16 pdop;					//λ�þ������� 0~500,��Ӧʵ��ֵ0~50.0
    u16 hdop;					//ˮƽ�������� 0~500,��Ӧʵ��ֵ0~50.0
    u16 vdop;					//��ֱ�������� 0~500,��Ӧʵ��ֵ0~50.0
    int altitude;			 	//���θ߶�,�Ŵ���10��,ʵ�ʳ���10.��λ:0.1m
    u16 speed;					//��������,�Ŵ���1000��,ʵ�ʳ���10.��λ:0.001����/Сʱ
    int direction_angle;        //�Ե��汱����0~360��
    Direction_Typedef  direction;              //����
} nmea_msg;
extern nmea_msg gpsx;

#define GPS_RBUFF_SIZE            512                   //���ڽ��ջ�������С
extern uint8_t GPS_RxBuff[GPS_RBUFF_SIZE];


void GPS_RxBufferProcess(void);

#endif

