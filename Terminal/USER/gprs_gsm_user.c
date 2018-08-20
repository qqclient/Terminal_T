/******************** (C) COPYRIGHT soarsky ************************************
* File Name          : GPRS_GSM.c
* Author             :
* Version            :
* Date               :
* Description        : GSM/GPRSʹ��M35ģ��
*******************************************************************************/

#include "gprs_gsm_user.h"
#include "ElecDriverLic.h"
#include "gps_user.h"
#include "protocol.h"

u8 Test;
u8 M35_ProcessNow = 0;
u16 GPRS_LocationTime = 0;
u8 Flag_CallPhoneOK = RESET;//���ڱ�־�Ƿ��д�绰
u8 Flag_SendMsgOK= RESET;


//static char IllegalParkStr1[] = "���ƺ�XXXXXXXXX��Υͣ������,��ͽ�ͣ��Ϸ�ͣ��λ,����XXXX��XX��XX��XXʱXX��ǰͨ��Υ�´��������볷��Υ��;���ڴ�ʱ��ǰ�ݳ��뿪��ǰλ��500��һ��,���Զ������˴�Υ��.";
static char IllegalParkStr1_en[] = "Your car is being illegally parked, Please stop near the legal parking spaces!";
//static char IllegalParkStr2[] = "���ƺ�XXXXXXXXX��Υͣ������";
static char IllegalDriveStr[] = "Your car is being illegally driving";//"���ĳ����ڱ�Υ��ʹ��";
static char TmpTelephone[] = "00000000000";


/*******************************************************************************
* Function Name  : M35_Process
* Description    : ����M35ģ��Ĵ�绰,������,����GPRS����ҵ��
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void M35_Process(void)
{
    /*��������г�ʼ��----------------------------------------------------------------*/
    M35_Initialize();

    /*���Ͷ���Ϣ----------------------------------------------------------------------*/
    if((M35_GetStatus() == SET) &&                           //��ǰSIM������
            Flag_SendMsgOK != SET &&                             //��δִ��if����������
            (Flag_CallPolice == SET || Flag_IllegalPark == SET) )//�ж��Ƿ�Ҫ���з�����������Υͣ���Ͷ���
    {
        GPRS_CmdNow = SendMessage_Start;//����Ӣ�Ķ���
//        GPRS_CmdNow = M35_PDU_SMS1;//�������Ķ���
        memcpy(TmpTelephone,m_carinfo.Family_Phone[0], __TELPHONE_LEN__);//�������ֻ�����
        Flag_SendMsgOK = SET;
    }

    if(Flag_CallPolice == SET)
    {
        M35_SendMessage(TmpTelephone,IllegalDriveStr);
//        M35_SendMessage_PDU(TmpTelephone,IllegalParkStr2);
    }
    else if(Flag_IllegalPark == SET)//Υͣ
    {
        // ����Υͣ����
        M35_SendMessage(TmpTelephone, IllegalParkStr1_en);
    }

    /*����绰------------------------------------------------------------------------*/
    if((M35_GetStatus() == SET)  &&                           //��ǰSIM������
            Flag_CallPhoneOK != SET  &&                           //��δִ�д�绰
            ((Flag_CallPolice == SET) || Flag_IllegalPark == SET))//�ж��Ƿ�Ҫ���з�����������Υͣ��绰
    {
        GPRS_CmdNow = Call_Start;
        Flag_CallPhoneOK = SET;
        memcpy(TmpTelephone,m_carinfo.Family_Phone[0], __TELPHONE_LEN__);//�������ֻ�����
        if(Flag_CallPolice == SET) { //��������
            GPRS_LocationTime = 10;//���÷��ͳ���λ�õ�ʱ��
        }
        else if(Flag_IllegalPark == SET) { //Υͣ

        }
    }

    M35_Call(TmpTelephone);

    /*��ʱ����GPRS����-----------------------------------------------------------------*/
    if(GPRS_LocationTime == 0)
    {
        if(M35_GetStatus() == SET)//��ǰSIM������
        {
            if(Flag_CallPolice == SET)//��������
            {
                GPRS_LocationTime = 30;//10���ӷ���һ��λ����Ϣ
            } else
            {
                GPRS_LocationTime = 600;//10���Ӷ�ʱ����λ����Ϣ
            }
            GPRS_CmdNow = SendTcp_Start;
        }
    }

    M35_SendCarLocation('0');//��λ����Ϣͨ��TCP���͵���������

    M35_SendIllegalPark('0');//ͨ��GPRS����ȡ��Υ��ͣ��
}

/*******************************************************************************
* Function Name  : M35_SendCarStatus
* Description    : ͨ��GPRS������״̬���͵���̨������
* Input          : CarStatus����״̬
0:�ն˿���(�ϵ�) 1:����Ϩ��,���ն�δ�ص�   2:�����������    3:�ն˶�ʱ�ύ  4:�����Ƿ�����
* Output         : None
* Return         : None
******************************************************************************/
void M35_SendCarStatus(u8 CarStatus,u8 Additon)
{
    static u8 MsgNumber = 0;//��Ϣ��ˮ��
    u16 CheckCode = 0;
    u8 CarStatusBuffer[23] = {0x7E,0x00,0x01,0x00,0x00,37,0x00,0x01,MsgNumber>>8,MsgNumber};//��Ϣͷ
    MsgNumber++;
    if(GPRS_CmdNow < SendTcp_Start || GPRS_CmdNow > SendTcp_Finished)
    {
        return;
    }

    memcpy(CarStatusBuffer+10,m_carinfo.Car_9SN,__CAR9SN_LEN__);
    CarStatusBuffer[19] = CarStatus;
    CarStatusBuffer[20] = Additon;
    CarStatusBuffer[21] = CheckCode>>8;
    CarStatusBuffer[22] = (u8)CheckCode;

    M35_SendTcpData(CarStatusBuffer,sizeof(CarStatusBuffer));

}




