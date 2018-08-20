#ifndef __MPUIIC_H
#define __MPUIIC_H

#define MPU_SCL_PIN     GPIO_Pin_8
#define MPU_SCL_PORT    GPIOB
#define MPU_SDA_PIN     GPIO_Pin_9
#define MPU_SDA_PORT    GPIOB
#define MPU6050_EN      GPIO_Pin_6
#define MPU6050_EN_PORT GPIOD
#define MPU_SCL_HIGH    GPIO_SetBits(MPU_SCL_PORT, MPU_SCL_PIN)
#define MPU_SCL_LOW     GPIO_ResetBits(MPU_SCL_PORT, MPU_SCL_PIN)
#define MPU_SDA_HIGH    GPIO_SetBits(MPU_SDA_PORT, MPU_SDA_PIN)
#define MPU_SDA_LOW     GPIO_ResetBits(MPU_SDA_PORT, MPU_SDA_PIN)
#define MPU_SDA_READ    GPIO_ReadInputDataBit(MPU_SDA_PORT, MPU_SDA_PIN)
#define MPU_RCC_APB2    RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOD
#define MPU6050_ON      GPIO_SetBits(MPU6050_EN_PORT, MPU6050_EN)
#define MPU6050_OFF     GPIO_ResetBits(MPU6050_EN_PORT, MPU6050_EN)

//IIC���в�������
void MPU_IIC_Delay(void);				//MPU IIC��ʱ����
void MPU_IIC_Init(void);                //��ʼ��IIC��IO��
void MPU_IIC_Start(void);				//����IIC��ʼ�ź�
void MPU_IIC_Stop(void);	  			//����IICֹͣ�ź�
void MPU_IIC_Send_Byte(unsigned char txd);			//IIC����һ���ֽ�
unsigned char MPU_IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
unsigned char MPU_IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void MPU_IIC_Ack(void);					//IIC����ACK�ź�
void MPU_IIC_NAck(void);				//IIC������ACK�ź�

void IMPU_IC_Write_One_Byte(unsigned char daddr,unsigned char addr,unsigned char data);
unsigned char MPU_IIC_Read_One_Byte(unsigned char daddr,unsigned char addr);


#endif

