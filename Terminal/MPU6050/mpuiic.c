#include "mpuiic.h"
#include "utility.h"

//MPU IIC 延时函数
void MPU_IIC_Delay(void)
{
    Delay_us(2);
}


//SDA设置为输出方式
void MPU_SDA_OUT(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = MPU_SDA_PIN;	 // 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(MPU_SDA_PORT, &GPIO_InitStructure);					 //根据设定参数初始化GPIO
}

//SDA设置为输入方式
void MPU_SDA_IN(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = MPU_SDA_PIN;	 // 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 //推挽输出
    GPIO_Init(MPU_SDA_PORT, &GPIO_InitStructure);					 //根据设定参数初始化GPIO
}

//初始化IIC
void MPU_IIC_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(MPU_RCC_APB2,ENABLE);//先使能外设IO PORTC时钟

    GPIO_InitStructure.GPIO_Pin = MPU_SCL_PIN;	 // 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(MPU_SCL_PORT, &GPIO_InitStructure);					 //根据设定参数初始化GPIO
    
    GPIO_InitStructure.GPIO_Pin = MPU6050_EN ;	 // 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
    GPIO_Init(MPU6050_EN_PORT, &GPIO_InitStructure);	    
    MPU6050_ON;
    

    MPU_SDA_OUT();

    MPU_SDA_HIGH;
    MPU_SCL_HIGH;
}

//产生IIC起始信号
void MPU_IIC_Start(void)
{
    MPU_SDA_OUT();     //sda线输出
    MPU_SDA_HIGH;
    MPU_SCL_HIGH;
    MPU_IIC_Delay();
    MPU_SDA_LOW;//START:when CLK is high,DATA change form high to low
    MPU_IIC_Delay();
    MPU_SCL_LOW;//钳住I2C总线，准备发送或接收数据
}

//产生IIC停止信号
void MPU_IIC_Stop(void)
{
    MPU_SDA_OUT();//sda线输出
    MPU_SCL_LOW;
    MPU_SDA_LOW;//STOP:when CLK is high DATA change form low to high
    MPU_IIC_Delay();
    MPU_SCL_HIGH;
    MPU_SDA_HIGH;//发送I2C总线结束信号
    MPU_IIC_Delay();
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
unsigned char MPU_IIC_Wait_Ack(void)
{
    unsigned char ucErrTime=0;
    MPU_SDA_IN();      //SDA设置为输入
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
    MPU_SCL_LOW;//时钟输出0
    return 0;
}
//产生ACK应答
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
//不产生ACK应答
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
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答
void MPU_IIC_Send_Byte(unsigned char txd)
{
    unsigned char t;
    MPU_SDA_OUT();
    MPU_SCL_LOW;//拉低时钟开始数据传输
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
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK
unsigned char MPU_IIC_Read_Byte(unsigned char ack)
{
    unsigned char i,receive=0;
    MPU_SDA_IN();//SDA设置为输入
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
        MPU_IIC_NAck();//发送nACK
    else
        MPU_IIC_Ack(); //发送ACK
    return receive;
}


















