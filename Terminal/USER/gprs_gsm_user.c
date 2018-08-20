/******************** (C) COPYRIGHT soarsky ************************************
* File Name          : GPRS_GSM.c
* Author             :
* Version            :
* Date               :
* Description        : GSM/GPRS使用M35模块
*******************************************************************************/

#include "gprs_gsm_user.h"
#include "ElecDriverLic.h"
#include "gps_user.h"
#include "protocol.h"

u8 Test;
u8 M35_ProcessNow = 0;
u16 GPRS_LocationTime = 0;
u8 Flag_CallPhoneOK = RESET;//用于标志是否有打电话
u8 Flag_SendMsgOK= RESET;


//static char IllegalParkStr1[] = "车牌号XXXXXXXXX因违停被抄牌,请就近停入合法停车位,并在XXXX年XX月XX日XX时XX分前通过违章处理功能申请撤销违章;或于此时间前驾车离开当前位置500米一上,可自动撤销此次违章.";
static char IllegalParkStr1_en[] = "Your car is being illegally parked, Please stop near the legal parking spaces!";
//static char IllegalParkStr2[] = "车牌号XXXXXXXXX因违停被抄牌";
static char IllegalDriveStr[] = "Your car is being illegally driving";//"您的车正在被违法使用";
static char TmpTelephone[] = "00000000000";


/*******************************************************************************
* Function Name  : M35_Process
* Description    : 处理M35模块的打电话,发短信,发送GPRS数据业务
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void M35_Process(void)
{
    /*开机后进行初始化----------------------------------------------------------------*/
    M35_Initialize();

    /*发送短消息----------------------------------------------------------------------*/
    if((M35_GetStatus() == SET) &&                           //当前SIM卡空闲
            Flag_SendMsgOK != SET &&                             //还未执行if语句里的内容
            (Flag_CallPolice == SET || Flag_IllegalPark == SET) )//判断是否要进行防盗报警或者违停发送短信
    {
        GPRS_CmdNow = SendMessage_Start;//发送英文短信
//        GPRS_CmdNow = M35_PDU_SMS1;//发送中文短信
        memcpy(TmpTelephone,m_carinfo.Family_Phone[0], __TELPHONE_LEN__);//拨打车主手机号码
        Flag_SendMsgOK = SET;
    }

    if(Flag_CallPolice == SET)
    {
        M35_SendMessage(TmpTelephone,IllegalDriveStr);
//        M35_SendMessage_PDU(TmpTelephone,IllegalParkStr2);
    }
    else if(Flag_IllegalPark == SET)//违停
    {
        // 发送违停短信
        M35_SendMessage(TmpTelephone, IllegalParkStr1_en);
    }

    /*拨打电话------------------------------------------------------------------------*/
    if((M35_GetStatus() == SET)  &&                           //当前SIM卡空闲
            Flag_CallPhoneOK != SET  &&                           //还未执行打电话
            ((Flag_CallPolice == SET) || Flag_IllegalPark == SET))//判断是否要进行防盗报警或者违停打电话
    {
        GPRS_CmdNow = Call_Start;
        Flag_CallPhoneOK = SET;
        memcpy(TmpTelephone,m_carinfo.Family_Phone[0], __TELPHONE_LEN__);//拨打车主手机号码
        if(Flag_CallPolice == SET) { //防盗报警
            GPRS_LocationTime = 10;//设置发送车辆位置的时间
        }
        else if(Flag_IllegalPark == SET) { //违停

        }
    }

    M35_Call(TmpTelephone);

    /*定时发送GPRS数据-----------------------------------------------------------------*/
    if(GPRS_LocationTime == 0)
    {
        if(M35_GetStatus() == SET)//当前SIM卡空闲
        {
            if(Flag_CallPolice == SET)//防盗报警
            {
                GPRS_LocationTime = 30;//10秒钟发送一次位置信息
            } else
            {
                GPRS_LocationTime = 600;//10分钟定时发送位置信息
            }
            GPRS_CmdNow = SendTcp_Start;
        }
    }

    M35_SendCarLocation('0');//将位置信息通过TCP发送到服务器端

    M35_SendIllegalPark('0');//通过GPRS发送取消违法停车
}

/*******************************************************************************
* Function Name  : M35_SendCarStatus
* Description    : 通过GPRS将车辆状态发送到后台服务器
* Input          : CarStatus车辆状态
0:终端开机(上电) 1:车辆熄火,但终端未关电   2:车辆点火启动    3:终端定时提交  4:车辆非法开门
* Output         : None
* Return         : None
******************************************************************************/
void M35_SendCarStatus(u8 CarStatus,u8 Additon)
{
    static u8 MsgNumber = 0;//消息流水号
    u16 CheckCode = 0;
    u8 CarStatusBuffer[23] = {0x7E,0x00,0x01,0x00,0x00,37,0x00,0x01,MsgNumber>>8,MsgNumber};//消息头
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




