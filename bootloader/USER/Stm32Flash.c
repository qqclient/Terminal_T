/******************** (C) COPYRIGHT soarsky ********************
* File Name          :
* Author             :
* Version            :
* Date               :
* Description        :
����ַ:0x8040000
1k = 0x400
128 = 0x80
128k = 0x20000
*******************************************************************************/
#include "Stm32Flash.h"


u16 STMFLASH_BUF[SECTOR_SIZE/2];//���ڱ���һ������������
u16 iapbuf[SECTOR_SIZE/2];//���ڱ���u16����������


/*******************************************************************************
* ������  : STMFLASH_WriteAppBin
* ����    : ��������ְ�����,һ�����Ĵ�С�պ�1k,����һ����д��Flash
* ����    : appxaddr:Ӧ�ó������ʼ��ַ  appbuf:Ӧ�ó���CODE.  appsize:Ӧ�ó����С(�ֽ�).
* ����ֵ  :
*******************************************************************************/
void STMFLASH_WriteAppBin(u32 AppAddr,u8 *AppBuf,u16 AppSize)
{
    u16 t;
    u16 i=0;
    u16 temp;
    u32 fwaddr=AppAddr;//��ǰд��ĵ�ַ
    u8  *dfu = AppBuf;
    for(t=0; t<AppSize; t+=2) //��8bit������ת��Ϊ16bit������
    {
        temp=(u16)dfu[1]<<8;
        temp+=(u16)dfu[0];
        dfu+=2;//ƫ��2���ֽ�
        iapbuf[i++]=temp;
    }

    STMFLASH_Write(fwaddr,iapbuf,SECTOR_SIZE/2+SECTOR_SIZE%2);

    fwaddr+=SECTOR_SIZE;//ƫ��1024
}


/*******************************************************************************
* ������  : STMFLASH_ReadHalfWord
* ����    : ��ȡָ����ַ�İ���(16λ����)
* ����    : faddr ����ַ(�˵�ַ����Ϊ2�ı���!!)
* ����ֵ  : ��Ӧ����.
*******************************************************************************/
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
    return *(vu16*)faddr;
}

/*******************************************************************************
* ������  : STMFLASH_Read
* ����    : ��ָ����ַ��ʼ����ָ�����ȵ�����
* ����    : ReadAddr:��ʼ��ַ  pBuffer:����ָ��  NumToWrite:����(16λ)��
* ����ֵ  :
*******************************************************************************/
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)
{
    u16 i;
    for(i=0; i<NumToRead; i++)
    {
        pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
        ReadAddr+=2;//ƫ��2���ֽ�.
    }
}


/*******************************************************************************
* ������  : STMFLASH_Write_NoCheck
* ����    : ������д��
* ����    : WriteAddr:��ʼ��ַ   pBuffer:����ָ��  NumToWrite:����(16λ)��
* ����ֵ  :
*******************************************************************************/
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{
    u16 i;
    if(WriteAddr%2 != 0)
    {
        return;
    }
    for(i=0; i<NumToWrite; i++)
    {
        FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
        WriteAddr+=2;//��ַ����2.
    }
}




/*******************************************************************************
* ������  : STMFLASH_Write
* ����    : ��ָ����ַ��ʼд��ָ�����ȵ�����
* ����    : WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)  pBuffer:����ָ��  NumToWrite:����(16λ)��
* ����ֵ  :
*******************************************************************************/
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{
    u32 SecPos;	   //������ַ
    u16 SecOff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
    u16 SecRemain; //������ʣ���ַ(16λ�ּ���)
    u16 i;
    u32 OffAddr;   //ȥ��0X08000000��ĵ�ַ
    if(WriteAddr<FLASH_BASE || (WriteAddr>=(FLASH_BASE+1024*STM32_FLASH_SIZE)) || WriteAddr%2 != 0)
    {
        return;//�Ƿ���ַ
    }

    OffAddr=WriteAddr-FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
    SecPos=OffAddr/SECTOR_SIZE;			  //������ַ  0~64 for STM32F101CBT6
    SecOff=(OffAddr%SECTOR_SIZE)/2;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
    SecRemain=SECTOR_SIZE/2-SecOff;		//����ʣ��ռ��С
    if(NumToWrite<=SecRemain)
    {
        SecRemain = NumToWrite;//�����ڸ�������Χ
    }
    while(1)
    {
        STMFLASH_Read( WriteAddr, STMFLASH_BUF, SECTOR_SIZE/2);//������������������
        for(i=0; i<SecRemain; i++) //У������
        {
            if(STMFLASH_BUF[SecOff+i]!=0XFFFF)
            {
                break;//��Ҫ����
            }
        }
        if(i<SecRemain)//��Ҫ����
        {
            FLASH_ErasePage(WriteAddr);//�����������
            for(i=0; i<SecRemain; i++) //����
            {
                STMFLASH_BUF[i+SecOff] = pBuffer[i];
            }
            STMFLASH_Write_NoCheck(SecPos*SECTOR_SIZE+FLASH_BASE,STMFLASH_BUF,SECTOR_SIZE/2);//д����������
        }
        else
        {
            STMFLASH_Write_NoCheck(WriteAddr,pBuffer,SecRemain);//д�Ѿ������˵�,ֱ��д������ʣ������.
        }
        if(NumToWrite==SecRemain)
        {
            break;//д�������
        } else//д��δ����
        {
            SecPos++;				//������ַ��1
            SecOff=0;				//ƫ��λ��Ϊ0
            pBuffer+=SecRemain;  	//ָ��ƫ��
            WriteAddr+=SecRemain;	//д��ַƫ��
            NumToWrite-=SecRemain;	//�ֽ�(16λ)���ݼ�
            if(NumToWrite>(SECTOR_SIZE/2))
            {
                SecRemain=SECTOR_SIZE/2;//��һ����������д����
            }
            else
            {
                SecRemain=NumToWrite;//��һ����������д����
            }
        }
    }

}



/*******************END OF FILE******************************************************************************/
