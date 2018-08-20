/*******************************************************************************
* File Name          : gtw_lightapp.c
* Author             : csyxj@728920175@qq.com
* Version            : V0.1
* Date               : 2017-03-19
* Description        : Headlight module support function
*******************************************************************************/

#include "gtw_Head.h"


/*******************************************************************************
* Function Name  : Get_SendCarNo
* Description    :
******************************************************************************/
unsigned short Get_SendCarNo (unsigned char* dsr)
{
    unsigned char* s1 = dsr + HEAD_LENGTH;

#ifdef __DEBUG
    XFS_WriteBuffer( xfs_auto, "����ģ�鳵��" );
    if( m_var.Flag_Debug_Enable == SET ) {
        Debug_Printf( RESET, "���ó���%s=>ģ��", m_carinfo.Car_8SN );
    }
#endif

    // ���ƺ�[9Byte]:
    memcpy( s1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // ���ƺ�
    s1 += __CAR9SN_LEN__;

    return GTW_Packet(dsr, 1, dsr+HEAD_LENGTH, s1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}


/*******************************************************************************
* Function Name  : Get_ReadLamp
* Description    :
******************************************************************************/
unsigned short Get_ReadLamp(unsigned char* src, unsigned char* dsr)
{
    unsigned char *d1 = dsr + HEAD_LENGTH;

    // ���ƺ�[9Byte]:
    memcpy( d1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // ���ƺ�
    d1 += __CAR9SN_LEN__;

    *d1++ = (Car_InputCheck.Value_LowBeam == RESET ? '0' : '1');
    *d1++ = (Car_InputCheck.Value_HighBeam == RESET ? '0' : '1');

    return GTW_Packet(dsr, 2, dsr+HEAD_LENGTH, d1 - dsr - HEAD_LENGTH, LightModule );
}


/*******************************************************************************
* Function Name  : Get_ReadSN
* Description    : ��ȡ���ƺ���Ϣ
******************************************************************************/
unsigned short Get_ReadSN(unsigned char* src, unsigned char* dsr)
{
    unsigned char *d1 = dsr + HEAD_LENGTH;

    *d1++ = src[8];
    *d1++ = src[9];

    *d1++ = src[1];
    *d1++ = src[2];

    // ���ƺ�[9Byte]:
    memcpy( d1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // ���ƺ�
    d1 += __CAR9SN_LEN__;

    *d1++ = 0;//�ɹ�

#ifdef __DEBUG
    XFS_WriteBuffer(xfs_auto, "���ģ���ȡ���ƺ�");
#endif

    return GTW_Packet(dsr, 0x802, dsr+HEAD_LENGTH, d1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}


/*******************************************************************************
* Function Name  : Get_LightChange
* Description    : Զ����Ƽ��
******************************************************************************/
unsigned short Get_LightChange(unsigned char* src, unsigned char* dsr)
{
    unsigned char *d1 = dsr + HEAD_LENGTH;

    *d1++ = src[8];
    *d1++ = src[9];

    *d1++ = src[1];
    *d1++ = src[2];

    // ���ƺ�[9Byte]:
    memcpy( d1, m_carinfo.Car_9SN, __CAR9SN_LEN__ );    // ���ƺ�
    d1 += __CAR9SN_LEN__;

    *d1++ = 0;//�ɹ�

    if( (m_carinfo.Flag_LAMPInput & 0x1) == RESET ) {
        // RF433MHzģ����
        if(src[HEAD_LENGTH+__CAR9SN_LEN__] == '0') {
            Car_InputCheck.Value_LowBeam = RESET;
        }
        else {
            Car_InputCheck.Value_LowBeam = SET;
        }
        if(Car_InputCheck_Last.Value_LowBeam != Car_InputCheck.Value_LowBeam) {
#ifndef __DEBUG
            Car_InputCheck_Last.Value_LowBeam = Car_InputCheck.Value_LowBeam;
#endif
            Car_SetupCheck.Value_LowBeam = SET;
        }

        if(src[HEAD_LENGTH+__CAR9SN_LEN__+1] == '0') {
            Car_InputCheck.Value_HighBeam = RESET;
        }
        else {
            Car_InputCheck.Value_HighBeam = SET;
        }
        if(Car_InputCheck_Last.Value_HighBeam != Car_InputCheck.Value_HighBeam) {
#ifndef __DEBUG
            Car_InputCheck_Last.Value_HighBeam = Car_InputCheck.Value_HighBeam;
#endif
            Car_SetupCheck.Value_HighBeam = SET;
        }
    }

    return GTW_Packet(dsr, 0x802, dsr+HEAD_LENGTH,d1 - dsr - HEAD_LENGTH, m_carinfo.DTU_Type );
}


unsigned short GTW_LightAPP(unsigned char* src, unsigned char* dsr)
{
    unsigned short imsg;
    unsigned short send_pack_len = 0;

    // ��������Ϣ�Ƿ񷢸�����
    if( memcmp(src+ HEAD_LENGTH, m_carinfo.Car_9SN, __CAR9SN_LEN__ ) != 0) {
			
			Debug_Printf(SET, "���ƺ� %s  Rv: %s", m_carinfo.Car_9SN,src);
        return 0; //���Ƿ�����������Ϣ
    }

    imsg = MAKEWORD( src[1], src[2] );
#ifdef __DEBUG
    if(m_var.Flag_Debug_Enable == SET)
        Debug_Printf(SET, "�ƹ�����Ϣ=%04x", imsg);
#endif

    switch(imsg)
    {
    case 0x01://��ȡ���ƺ���Ϣ
        send_pack_len = Get_ReadSN(src,dsr);
        break;
    case 0x02://��Ʊ任��Ϣ
        send_pack_len = Get_LightChange(src,dsr);
        break;
    default :
        break;
    }
    return send_pack_len;
}

