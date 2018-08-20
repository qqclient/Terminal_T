#include "mpu6050.h"
#include "utility.h"
#include "gtw_Head.h"


unsigned int    MPU_TimeCount = 0;
Illegal_Def     AccelerationeNomal;//������
mpu_msg_typedef mpu_msg;


//�жϼ��ٶȱ仯
//����ֵ:0,��������;  1=������;  2=������;
//    ����,�������
unsigned char Process_Get_Accelerometer(short getax, short getay, short getaz)
{
    float setR;
    long  DATATEMP[4];
    float direct[3] = { 0.0001, 0.0001, 0.0001 };

    DATATEMP[0] = (long)getax;
    DATATEMP[1] = (long)getay;
    DATATEMP[2] = (long)getaz;

    direct[0] = (float)getax / 16384;
    direct[1] = (float)getay / 16384;
    direct[2] = (float)getaz / 16384;

    DATATEMP[3] = DATATEMP[0] * DATATEMP[0] + DATATEMP[1] * DATATEMP[1] + DATATEMP[2] * DATATEMP[2];

    setR = sqrt(DATATEMP[3]) / 16384;        //   0.9--1.35

    if( ( direct[0] > LIMITSPEEDF || direct[1] > LIMITSPEEDF ) && setR > LIMITSPEED ) {
        return 1;
    }
    else if((direct[0] < LIMITSPEEDB || direct[1] < LIMITSPEEDB) && setR > LIMITSPEED ) {
        return 2;
    }
    else {
        return 0;
    }
}


void MPU_Process(void)
{
#ifdef __USE_MPU6050__
    static unsigned char  mpu_sta = 1;
    static unsigned char  mpu_R;
    static unsigned char  JiaspeedNum;
    static unsigned char  JianspeedNum;

    switch(mpu_sta) {
    case 1:
        if( mpu_dmp_init() == RESET ) {
            mpu_sta = 2;
        }
        break;

    case 2:
    {
        if( mpu_dmp_get_data( &mpu_msg.pitch, &mpu_msg.roll, &mpu_msg.yaw ) == 0 ) {
            MPU_Get_Accelerometer( &mpu_msg.aacx, &mpu_msg.aacy, &mpu_msg.aacz );	// �õ����ٶȴ���������
//            MPU_Get_Gyroscope( &mpu_msg.gyrox, &mpu_msg.gyroy, &mpu_msg.gyroz );	// �õ�����������
            mpu_R = Process_Get_Accelerometer( mpu_msg.aacx, mpu_msg.aacy, mpu_msg.aacz );

            if(mpu_R == 1) {
                JiaspeedNum++;
                if(JiaspeedNum >= 8) {
                    JiaspeedNum = 0;
#ifdef __DEBUG
                    XFS_WriteBuffer(xfs_auto, "������");
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf(SET,
                                     "������:\r\n������%.7f,�����%.7f,�����:%.7f\r\nx���ٶ�%d,y���ٶ�%d,z���ٶ�:%d,R���ٶ�:%d",
                                     mpu_msg.pitch,
                                     mpu_msg.roll,
                                     mpu_msg.yaw,
                                     mpu_msg.aacx,
                                     mpu_msg.aacy,
                                     mpu_msg.aacz,
                                     mpu_R );
#endif
                    //���Υ�¼�¼�洢
                    TimeToCharArray(Sys_CalendarTime,AccelerationeNomal.stime);  // ��¼Υ�¿�ʼʱ��
                    TimeToCharArray(Sys_CalendarTime,AccelerationeNomal.etime);  // ��¼Υ�½���ʱ��
                    AccelerationeNomal.ino = Acceleratione_enum;                 // Υ������
                    MX25L128_Illegal_Write(&AccelerationeNomal);                 // ��Υ������д��Flash��

                }
            }
            else if(mpu_R == 2) {
                JianspeedNum++;
                if(JianspeedNum >= 8)
                {
                    JianspeedNum = 0;
#ifdef __DEBUG
                    XFS_WriteBuffer(xfs_auto, "��ɲ��");
                    if(m_var.Flag_Debug_Enable == SET)
                        Debug_Printf(SET,
                                     "��ɲ��:\r\n������%.7f,�����%.7f,�����:%.7f\r\nx���ٶ�%d,y���ٶ�%d,z���ٶ�:%d,R���ٶ�:%d",
                                     mpu_msg.pitch,
                                     mpu_msg.roll,
                                     mpu_msg.yaw,
                                     mpu_msg.aacx,
                                     mpu_msg.aacy,
                                     mpu_msg.aacz,
                                     mpu_R );
#endif
                    //���Υ�¼�¼�洢
                    TimeToCharArray(Sys_CalendarTime,AccelerationeNomal.stime);  //��¼Υ�¿�ʼʱ��
                    TimeToCharArray(Sys_CalendarTime,AccelerationeNomal.etime);  //��¼Υ�½���ʱ��
                    AccelerationeNomal.ino = Acceleratione_enum;                 //Υ������
                    MX25L128_Illegal_Write(&AccelerationeNomal);                 //��Υ������д��Flash��
                }
            }
            else {
                JiaspeedNum  = 0;
                JianspeedNum = 0;
            }
        }
        break;
    }

    default:
        mpu_sta = 1;
        break;
    }
#endif
}



//��ʼ��MPU6050
//����ֵ:0,�ɹ�
//    ����,�������
unsigned char MPU_Init(void)
{
    u8 res;
    MPU_IIC_Init();//��ʼ��IIC����
    MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X80);	//��λMPU6050
    Delay_ms(100);
    MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X00);	//����MPU6050
    //MPU_Set_Gyro_Fsr(3);					//�����Ǵ�����,��2000dps
    MPU_Set_Accel_Fsr(0);					//���ٶȴ�����,��2g
    MPU_Set_Rate(50);						//���ò�����50Hz
    MPU_Write_Byte(MPU_INT_EN_REG,0X00);	//�ر������ж�
    MPU_Write_Byte(MPU_USER_CTRL_REG,0X00);	//I2C��ģʽ�ر�
    MPU_Write_Byte(MPU_FIFO_EN_REG,0X00);	//�ر�FIFO
    MPU_Write_Byte(MPU_INTBP_CFG_REG,0X80);	//INT���ŵ͵�ƽ��Ч
    res = MPU_Read_Byte(MPU_DEVICE_ID_REG);
    if(res==MPU_ADDR)//����ID��ȷ
    {
        MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X01);	//����CLKSEL,PLL X��Ϊ�ο�
        MPU_Write_Byte(MPU_PWR_MGMT2_REG,0X00);	//���ٶ��������Ƕ�����
        MPU_Set_Rate(50);						//���ò�����Ϊ50Hz
    } 
    else
        return 1;
    
    return 0;
}


//����MPU6050�����Ǵ����������̷�Χ
//fsr:0,��250dps;1,��500dps;2,��1000dps;3,��2000dps
//����ֵ:0,���óɹ�
//    ����,����ʧ��
//u8 MPU_Set_Gyro_Fsr(u8 fsr)
//{
//    return MPU_Write_Byte(MPU_GYRO_CFG_REG,fsr<<3);//���������������̷�Χ
//}


//����MPU6050���ٶȴ����������̷�Χ
//fsr:0,��2g;1,��4g;2,��8g;3,��16g
//����ֵ:0,���óɹ�
//    ����,����ʧ��
u8 MPU_Set_Accel_Fsr(u8 fsr)
{
    return MPU_Write_Byte(MPU_ACCEL_CFG_REG,fsr<<3);//���ü��ٶȴ����������̷�Χ
}


//����MPU6050�����ֵ�ͨ�˲���
//lpf:���ֵ�ͨ�˲�Ƶ��(Hz)
//����ֵ:0,���óɹ�
//    ����,����ʧ��
u8 MPU_Set_LPF(u16 lpf)
{
    u8 data=0;
    if(lpf>=188)data=1;
    else if(lpf>=98)data=2;
    else if(lpf>=42)data=3;
    else if(lpf>=20)data=4;
    else if(lpf>=10)data=5;
    else data=6;
    return MPU_Write_Byte(MPU_CFG_REG,data);//�������ֵ�ͨ�˲���
}


//����MPU6050�Ĳ�����(�ٶ�Fs=1KHz)
//rate:4~1000(Hz)
//����ֵ:0,���óɹ�
//    ����,����ʧ��
u8 MPU_Set_Rate(u16 rate)
{
    u8 data;
    if(rate>1000)rate=1000;
    if(rate<4)rate=4;
    data=1000/rate-1;
    data=MPU_Write_Byte(MPU_SAMPLE_RATE_REG,data);	//�������ֵ�ͨ�˲���
    return MPU_Set_LPF(rate/2);	//�Զ�����LPFΪ�����ʵ�һ��
}


////�õ��¶�ֵ
////����ֵ:�¶�ֵ(������100��)
//short MPU_Get_Temperature(void)
//{
//    u8 buf[2];
//    short raw;
//    float temp;
//    MPU_Read_Len(MPU_ADDR,MPU_TEMP_OUTH_REG,2,buf);
//    raw=((u16)buf[0]<<8)|buf[1];
//    temp=36.53+((double)raw)/340;
//    return temp*100;;
//}


////�õ�������ֵ(ԭʼֵ)
////gx,gy,gz:������x,y,z���ԭʼ����(������)
////����ֵ:0,�ɹ�
////    ����,�������
//u8 MPU_Get_Gyroscope(short *gx,short *gy,short *gz)
//{
//    u8 buf[6],res;
//    res=MPU_Read_Len(MPU_ADDR,MPU_GYRO_XOUTH_REG,6,buf);
//    if(res==0)
//    {
//        *gx=((u16)buf[0]<<8)|buf[1];
//        *gy=((u16)buf[2]<<8)|buf[3];
//        *gz=((u16)buf[4]<<8)|buf[5];
//    }
//    return res;;
//}


//�õ����ٶ�ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
u8 MPU_Get_Accelerometer(short *ax,short *ay,short *az)
{
    u8 buf[6],res;
    res=MPU_Read_Len(MPU_ADDR,MPU_ACCEL_XOUTH_REG,6,buf);
    if(res==0)
    {
        *ax=((u16)buf[0]<<8)|buf[1];
        *ay=((u16)buf[2]<<8)|buf[3];
        *az=((u16)buf[4]<<8)|buf[5];
    }
    return res;;
}


//IIC����д
//addr:������ַ
//reg:�Ĵ�����ַ
//len:д�볤��
//buf:������
//����ֵ:0,����
//    ����,�������
u8 MPU_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
    u8 i;
    MPU_IIC_Start();
    MPU_IIC_Send_Byte((addr<<1)|0);//����������ַ+д����
    if(MPU_IIC_Wait_Ack())	//�ȴ�Ӧ��
    {
        MPU_IIC_Stop();
        return 1;
    }
    MPU_IIC_Send_Byte(reg);	//д�Ĵ�����ַ
    MPU_IIC_Wait_Ack();		//�ȴ�Ӧ��
    for(i=0; i<len; i++)
    {
        MPU_IIC_Send_Byte(buf[i]);	//��������
        if(MPU_IIC_Wait_Ack())		//�ȴ�ACK
        {
            MPU_IIC_Stop();
            return 1;
        }
    }
    MPU_IIC_Stop();
    return 0;
}


//IIC������
//addr:������ַ
//reg:Ҫ��ȡ�ļĴ�����ַ
//len:Ҫ��ȡ�ĳ���
//buf:��ȡ�������ݴ洢��
//����ֵ:0,����
//    ����,�������
u8 MPU_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
    MPU_IIC_Start();
    MPU_IIC_Send_Byte((addr<<1)|0);//����������ַ+д����
    if(MPU_IIC_Wait_Ack())	//�ȴ�Ӧ��
    {
        MPU_IIC_Stop();
        return 1;
    }
    MPU_IIC_Send_Byte(reg);	//д�Ĵ�����ַ
    MPU_IIC_Wait_Ack();		//�ȴ�Ӧ��
    MPU_IIC_Start();
    MPU_IIC_Send_Byte((addr<<1)|1);//����������ַ+������
    MPU_IIC_Wait_Ack();		//�ȴ�Ӧ��
    while(len)
    {
        if(len==1)*buf=MPU_IIC_Read_Byte(0);//������,����nACK
        else *buf=MPU_IIC_Read_Byte(1);		//������,����ACK
        len--;
        buf++;
    }
    MPU_IIC_Stop();	//����һ��ֹͣ����
    return 0;
}


//IICдһ���ֽ�
//reg:�Ĵ�����ַ
//data:����
//����ֵ:0,����
//    ����,�������
u8 MPU_Write_Byte(u8 reg,u8 data)
{
    MPU_IIC_Start();
    MPU_IIC_Send_Byte((MPU_ADDR<<1)|0);//����������ַ+д����
    if(MPU_IIC_Wait_Ack())	//�ȴ�Ӧ��
    {
        MPU_IIC_Stop();
        return 1;
    }
    MPU_IIC_Send_Byte(reg);	//д�Ĵ�����ַ
    MPU_IIC_Wait_Ack();		//�ȴ�Ӧ��
    MPU_IIC_Send_Byte(data);//��������
    if(MPU_IIC_Wait_Ack())	//�ȴ�ACK
    {
        MPU_IIC_Stop();
        return 1;
    }
    MPU_IIC_Stop();
    return 0;
}


//IIC��һ���ֽ�
//reg:�Ĵ�����ַ
//����ֵ:����������
u8 MPU_Read_Byte(u8 reg)
{
    u8 res;
    MPU_IIC_Start();
    MPU_IIC_Send_Byte((MPU_ADDR<<1)|0);//����������ַ+д����
    MPU_IIC_Wait_Ack();		//�ȴ�Ӧ��
    MPU_IIC_Send_Byte(reg);	//д�Ĵ�����ַ
    MPU_IIC_Wait_Ack();		//�ȴ�Ӧ��
    MPU_IIC_Start();
    MPU_IIC_Send_Byte((MPU_ADDR<<1)|1);//����������ַ+������
    MPU_IIC_Wait_Ack();		//�ȴ�Ӧ��
    res=MPU_IIC_Read_Byte(0);//��ȡ����,����nACK
    MPU_IIC_Stop();			//����һ��ֹͣ����
    return res;
}


