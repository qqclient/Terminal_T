/*******************************************************************************
* File Name          : gtw_setupapp.c
* Author             : ������
* Version            : V0.1
* Date               : 2017-03-19
* Description        : Instrument application support function
*******************************************************************************/

#include "gtw_Head.h"

/*******************************************************************************
* Function Name  : GTW_Broadcast
* Description    : �㲥��Ϣ
*******************************************************************************/
unsigned short GTW_Broadcast(unsigned char* src, unsigned char* dsr)
{
    unsigned char iMsg;

    iMsg = Asc2Byte((char*)src+22, 2);

    switch(iMsg) {
    case 1://����Զ��
        DataCollectorMsg.MsgHighBeams = SET;
        break;
    case 2://��������
        DataCollectorMsg.MsgWhistle = SET;
        break;
    case 3://����
        Flag_LimitSpeed = SET;
        break;
    case 4://����
        Flag_LimitRun = SET;
        break;
    case 5://�����Ϣ
    case 6://���������
    case 7://�������
    case 8://�������
    case 9://���뽼��
        break;
    }

    return 0;
}


/*******************************************************************************
* Function Name  : GTW_ReadTicket
* Description    : ��ȡ����
******************************************************************************/
unsigned short GTW_ReadTicket(unsigned char* src, unsigned char* dsr)
{
    unsigned short send_pack_len = 0;

    return send_pack_len;
}


/*******************************************************************************
* Function Name  : GTW_DataCollector
* Description    : �����ɼ�����Ϣ
******************************************************************************/
unsigned short GTW_DataCollector(unsigned char* src, unsigned char* dsr)
{
    unsigned short send_pack_len = 0,imsg = 0;

    // ��������Ϣ�Ƿ񷢸�����
    if( memcmp(src + HEAD_LENGTH, m_carinfo.Car_9SN, __CAR9SN_LEN__ ) != 0) {
        return 0; //���Ƿ�����������Ϣ
    }

    // ��ϢID
    imsg = MAKEWORD( src[1], src[2] );
    switch(imsg) {
    case 0x1:   // ��ȡ����
        send_pack_len = GTW_ReadTicket(src, dsr);
        break;

    case 0x2:   // �㲥��Ϣ
        send_pack_len = GTW_Broadcast(src, dsr);
        break;

    default:
        break;
    }

    return send_pack_len;
}


/*******************************************************************************
* Function Name  : GTW_CarTerminal
* Description    : ���������ն���Ϣ
******************************************************************************/
unsigned short GTW_CarTerminal(unsigned char* src, unsigned char* dsr)
{
    MsgHead_Struct Si4432RevMsg;

    // ��Ϣ��Ϣ�ֽ�
    Si4432RevMsg.MsgID =   MAKEWORD (src[1],src[2]);
    Si4432RevMsg.MsgSource = (MsgSource_Typedef)MAKEWORD(src[6],src[7]);

    if( Si4432RevMsg.MsgSource == CarTerminal ) {
        if(Si4432RevMsg.MsgID == 0x01) {
//                  if( isSubtend( Si4432_RxBuf[10], gpsx.direction) == 1 )
            // �յ��ĺ����뱾������Ա�
            Flag_ChangeLight = SET;
        }
    }
    return 0;
}

