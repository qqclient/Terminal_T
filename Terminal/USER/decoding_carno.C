/******************** (C) COPYRIGHT soarsky ************************************
* File Name          : decoding_carno.c
* Author             :
* Version            :
* Date               :
* Description        :
*******************************************************************************/
#include "gtw_Head.h"

const unsigned char CARNO2CHAR[] = { '0', '1','2', '3', '4', '5', '6', '7','8','9','A', 'B', 'C', 'D','E','F','G','H','I','J','K','L', 'M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
const unsigned char CARNO2PROVICE[] = "���澩���弽ԥ���ɺ���³������Ӷ���ʽ����¼�������ش�����̨";
const unsigned char PROVICEUTF8[] =  {
    0xE7,0xB2,0xA4,		//��
    0xE6,0xB9,0x98,		//��
    0xE4,0xBA,0xAC,		//��
    0xE6,0xB4,0xA5,		//��
    0xE6,0xB2,0xAA,		//��
    0xE6,0xB8,0x9D,		//��
    0xE5,0x86,0x80,		//��
    0xE8,0xB1,0xAB,		//ԥ
    0xE4,0xBA,0x91,		//��
    0xE8,0xBE,0xBD,		//��
    0xE9,0xBB,0x91,		//��
    0xE7,0x9A,0x96,		//��
    0xE9,0xB2,0x81,		//³
    0xE6,0x96,0xB0,		//��
    0xE8,0x8B,0x8F,		//��
    0xE6,0xB5,0x99,		//��
    0xE8,0xB5,0xA3,		//��
    0xE9,0x84,0x82,		//��
    0xE6,0xA1,0x82,		//��
    0xE7,0x94,0x98,		//��
    0xE6,0x99,0x8B,		//��
    0xE8,0x92,0x99,		//��
    0xE9,0x99,0x95,		//��
    0xE5,0x90,0x89,		//��
    0xE9,0x97,0xBD,		//��
    0xE8,0xB4,0xB5,		//��
    0xE9,0x9D,0x92,		//��
    0xE8,0x97,0x8F,		//��
    0xE5,0xB7,0x9D,		//��
    0xE5,0xAE,0x81,		//��
    0xE7,0x90,0xBC,		//��
    0xE5,0x8F,0xB0      //̨
};


/*******************************************************************************
* Function Name  : decoding_carno
* Description    : ����ѹ���ĳ��ƺ�
* Input          : SNѹ����ĳ��ƺ�
* Output         : car_sn8=GBK�복�ƺ�; car_sn9=UTF-8�복�ƺ�
* Return         :
******************************************************************************/
void decoder_carno(unsigned char* car_sn5, unsigned char* car_sn8, unsigned char* car_sn9)
{
    unsigned char decoded_carno[8];		//��:��AGT286
    unsigned char decoded_carno_UTF8[9];//��:��AGT286 (����ΪUTF8����,3�ֽ�,�����ֻ�����)

    unsigned char i;
    unsigned long j;
    unsigned long k;
    unsigned char m;
    unsigned long n;

    //��ʼ������
    for (i = 0; i < 9; i++) {
        decoded_carno[i] = 0x00;
    }

    for (i = 0; i < 10; i++) {
        decoded_carno_UTF8[i] = 0x00;
    }

    i = car_sn5[0]; //sn1;
    i &= 0x0F;
    n = i;
    n <<= 24;
    k = n;

    m = (k % 9);
    m <<= 2;	//����ֽ�(���ֽ�) �� 36 ������

    n /= 9;
    k *= 7;
    k += n;	//����ֽ�(���ֽ�) �� 36 �ĵ���

    j = car_sn5[1]; //sn2;
    j <<= 8;
    j += car_sn5[2]; //sn3;
    j <<= 8;
    j += car_sn5[3]; //sn4;
    j <<= 8;
    j += car_sn5[4]; //sn5;

    //------------------------------------------------------------
    i = (unsigned char)(j % 36);
    j = (j / 36);
    j += k;

    i += m;
    if(i >= 36) {
        i -= 36;
        j ++;
    }

    decoded_carno[7] = CARNO2CHAR[i];

    //------------------------------------------------------------
    i = (unsigned char)(j % 36);
    decoded_carno[6] = CARNO2CHAR[i];
    j = (j / 36);

    i = (unsigned char)(j % 36);
    decoded_carno[5] = CARNO2CHAR[i];
    j = (j / 36);

    i = (unsigned char)(j % 36);
    decoded_carno[4] = CARNO2CHAR[i];
    j = (j / 36);

    i = (unsigned char)(j % 36);
    decoded_carno[3] = CARNO2CHAR[i];
    j = (j / 36);

    i = (unsigned char)(j % 26);
    decoded_carno[2] = 'A';
    decoded_carno[2] += i;
    i = (unsigned char)(j / 26);
    i %= 32;

    decoded_carno[0] = CARNO2PROVICE[2*i];
    decoded_carno[1] = CARNO2PROVICE[2*i + 1];

    decoded_carno_UTF8[0] = PROVICEUTF8[3*i];
    decoded_carno_UTF8[1] = PROVICEUTF8[3*i + 1];
    decoded_carno_UTF8[2] = PROVICEUTF8[3*i + 2];

    //ת����UTF-8����
    for (i = 2; i < 9; i++) {
        decoded_carno_UTF8[i + 1] = decoded_carno[i];
    }

    memcpy(car_sn8, decoded_carno, __CAR8SN_LEN__);
    memcpy(car_sn9, decoded_carno_UTF8, __CAR9SN_LEN__);

    i = 0;
    if( (decoded_carno[7] >= 'A')&&(decoded_carno[7] <= 'Z') )
        i++;
    if( (decoded_carno[6] >= 'A')&&(decoded_carno[6] <= 'Z') )
        i++;
    if( (decoded_carno[5] >= 'A')&&(decoded_carno[5] <= 'Z') )
        i++;
    if( (decoded_carno[4] >= 'A')&&(decoded_carno[4] <= 'Z') )
        i++;
    if( (decoded_carno[3] >= 'A')&&(decoded_carno[3] <= 'Z') )
        i++;
}


/*******************************************************************************
* Function Name  : HEX_decode_char
* Description    : ASCIIת�����ַ�
*******************************************************************************/
unsigned char HEX_decode_char(unsigned char _H, unsigned char _L)
{
    unsigned char tmp;

    if(_H <= '9')
        tmp = _H - '0';
    else
        tmp = _H - 'A' + 10;
    tmp <<= 4;

    if(_L <= '9')
        tmp += _L - '0';
    else
        tmp += _L - 'A' + 10;

    return tmp;
}
