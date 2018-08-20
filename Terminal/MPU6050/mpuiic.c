#include "mpuiic.h"
#include "utility.h"

//MPU IIC ��ʱ����
void MPU_IIC_Delay(void)
{
    Delay_us(2);
}


//SDA����Ϊ�����ʽ
void MPU_SDA_OUT(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = MPU_SDA_PIN;	 // �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
    GPIO_Init(MPU_SDA_PORT, &GPIO_InitStructure);					 //�����趨������ʼ��GPIO
}

//SDA����Ϊ���뷽ʽ
void MPU_SDA_IN(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = MPU_SDA_PIN;	 // �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 //�������
    GPIO_Init(MPU_SDA_PORT, &GPIO_InitStructure);					 //�����趨������ʼ��GPIO
}

//��ʼ��IIC
void MPU_IIC_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(MPU_RCC_APB2,ENABLE);//��ʹ������IO PORTCʱ��

    GPIO_InitStructure.GPIO_Pin = MPU_SCL_PIN;	 // �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
    GPIO_Init(MPU_SCL_PORT, &GPIO_InitStructure);					 //�����趨������ʼ��GPIO
    
    GPIO_InitStructure.GPIO_Pin = MPU6050_EN ;	 // �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
    GPIO_Init(MPU6050_EN_PORT, &GPIO_InitStructure);	    
    MPU6050_ON;
    

    MPU_SDA_OUT();

    MPU_SDA_HIGH;
    MPU_SCL_HIGH;
}

//����IIC��ʼ�ź�
void MPU_IIC_Start(void)
{
    MPU_SDA_OUT();     //sda�����
    MPU_SDA_HIGH;
    MPU_SCL_HIGH;
    MPU_IIC_Delay();
    MPU_SDA_LOW;//START:when CLK is high,DATA change form high to low
    MPU_IIC_Delay();
    MPU_SCL_LOW;//ǯסI2C���ߣ�׼�����ͻ��������
}

//����IICֹͣ�ź�
void MPU_IIC_Stop(void)
{
    MPU_SDA_OUT();//sda�����
    MPU_SCL_LOW;
    MPU_SDA_LOW;//STOP:when CLK is high DATA change form low to high
    MPU_IIC_Delay();
    MPU_SCL_HIGH;
    MPU_SDA_HIGH;//����I2C���߽����ź�
    MPU_IIC_Delay();
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
unsigned char MPU_IIC_Wait_Ack(void)
{
    unsigned char ucErrTime=0;
    MPU_SDA_IN();      //SDA����Ϊ����
    MPU_SDA_HIGH;
    MPU_IIC_Delay();
    MPU_SCL_HIGH;
    MPU_IIC_Delay();
    while(MPU_SDA_READ)
    {
        ucErrTime++;
        if(ucErrTime>250)
        {
            MPU_IIC_Stop();
            return 1;
        }
    }
    MPU_SCL_LOW;//ʱ�����0
    return 0;
}
//����ACKӦ��
void MPU_IIC_Ack(void)
{
    MPU_SCL_LOW;
    MPU_SDA_OUT();
    MPU_SDA_LOW;
    MPU_IIC_Delay();
    MPU_SCL_HIGH;
    MPU_IIC_Delay();
    MPU_SCL_LOW;
}
//������ACKӦ��
void MPU_IIC_NAck(void)
{
    MPU_SCL_LOW;
    MPU_SDA_OUT();
    MPU_SDA_HIGH;
    MPU_IIC_Delay();
    MPU_SCL_HIGH;
    MPU_IIC_Delay();
    MPU_SCL_LOW;
}
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��
void MPU_IIC_Send_Byte(unsigned char txd)
{
    unsigned char t;
    MPU_SDA_OUT();
    MPU_SCL_LOW;//����ʱ�ӿ�ʼ���ݴ���
    for(t=0; t<8; t++)
    {
        if(txd&0x80) {
            MPU_SDA_HIGH;
        }
        else {
            MPU_SDA_LOW;
        }

        txd<<=1;
        MPU_SCL_HIGH;
        MPU_IIC_Delay();
        MPU_SCL_LOW;
        MPU_IIC_Delay();
    }
}
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
unsigned char MPU_IIC_Read_Byte(unsigned char ack)
{
    unsigned char i,receive=0;
    MPU_SDA_IN();//SDA����Ϊ����
    for(i=0; i<8; i++ )
    {
        MPU_SCL_LOW;
        MPU_IIC_Delay();
        MPU_SCL_HIGH;
        receive<<=1;

        if(MPU_SDA_READ)
            receive++;

        MPU_IIC_Delay();
    }
    if (!ack)
        MPU_IIC_NAck();//����nACK
    else
        MPU_IIC_Ack(); //����ACK
    return receive;
}


















