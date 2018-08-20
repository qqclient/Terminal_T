/******************** (C) COPYRIGHT soarsky ********************
* File Name          : radio.c
* Author             :
* Version            :
* Date               :
* Description        : 无线模块使用的是si4432芯片
            ----------------------------------------------
*数据格式：|Preamble|SYNC|HEADER ADDR|PK LENGTH|DATA|CRC |
            ----------------------------------------------

*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"

#define SI4432_DATARATE_128K
#ifdef SI4432_DATARATE_128K
#define TIMESLOT	3
#endif
#ifdef SI4432_DATARATE_100K
#define TIMESLOT	3
#endif
#ifdef SI4432_DATARATE_38K4
#define TIMESLOT	8
#endif
#ifdef SI4432_DATARATE_9K6
#define TIMESLOT	30
#endif


/* Global variables ---------------------------------------------------------*/
volatile unsigned char  Si4432_SendTimeOut;
volatile unsigned char  Fla_ReadTicketCmd = RESET;
volatile unsigned char  Flag_InspectCmd = RESET;
volatile unsigned char  Flag_SendEPN = ERROR;
volatile unsigned char  CheckerAnswer_index = 0;
volatile unsigned int   CheckerAnswer_timer;//应答延时
volatile unsigned short Si4432_TimeSendChangeLight = 0;
volatile unsigned short Si4432_TimeSendSn = 0;

unsigned short  S_Timer[8];  //计算出发送延时时间

/***************本地宏定义*************/

/***************本地函数声明*************/
static void Si4432_Send_EPlateNum(unsigned char * src);
//static void Si4432_RxBufferProcess(void);

/***************本地变量定义*************/
static unsigned char CheckerAnswer_CMDno;       // 稽查器发来的稽查命令序号
i2b_Def CheckerAnswer_ID;                       // 采集仪ID后四位

Msg_Sign_DataCollector_Struct DataCollectorMsg; // 数据采集仪稽查命令的标志信息
Msg_Sign_DataCollector_Struct GTW02_BoardCast_Msg;
unsigned char RF433_MsgMask = 0;
unsigned char RF433_MsgData = 0;


Direction_Speed_Group_Struct Dir_SGroup;

/*******************************************************************************
* Function Name  : Si4432_Process
* Description    : Si4432无线接受数据,发送数据处理函数.只有在车辆处于行驶状态时才调用该函数
******************************************************************************/
void Si4432_CheckLight(void)
{
    static u8  SendChangeLight_Step = 0;
    static u32 Send_StartTime = 0;

    if( Flag_ChangeLight == SET ) {  // 收到变光信号不用发变光信号
        SendChangeLight_Step = 0;
	
        return;
    }

    switch(SendChangeLight_Step) {
		
    case 0:
        if(Car_InputCheck.Value_HighBeam == SET) { //远光灯是打开
            Si4432_TimeSendChangeLight = 200;
            SendChangeLight_Step = 1;
        }
        break;

    case 1:
        if(Si4432_TimeSendChangeLight == 0) {
            if(Car_InputCheck.Value_HighBeam == SET) { //远光灯打开
                SendChangeLight_Step = 0;
            }
            else if(Car_InputCheck.Value_HighBeam == RESET) { //远光灯关闭
                Send_StartTime = Sys_TimeCount;
#ifdef __DEBUG
                XFS_WriteBuffer(xfs_auto, "发送变光");
                if(m_var.Flag_Debug_Enable == SET)
                    Debug_Printf( SET, "发送变光");
#endif
                SendChangeLight_Step = 2;
            }
        }
        break;

    case 2:
        Si4432_SendChangeLight(Si4432_TxBuf);//发送变光信号
		Debug_Printf( SET, "发送变光信号 %d", Si4432_TimeSendChangeLight);
		if( CheckerAnswer_CMDno > 7 )//发送变光等待时间出错！数组坐标溢出！！！ 2017-9-21
			CheckerAnswer_CMDno = 7;
        //Si4432_TimeSendChangeLight = S_Timer[CheckerAnswer_CMDno] + rand() % 10;  //每次发送变光信号的间隔时间
		Si4432_TimeSendChangeLight = 400;   //400ms 间隔发送一次
        SendChangeLight_Step = 3;
        break;

    case 3:
        if(Sys_TimeCount - Send_StartTime > 4) { //4S   变光信号持续发送三秒
            SendChangeLight_Step = 4;
        }
        else {
            if(Si4432_TimeSendChangeLight == 0) {
                SendChangeLight_Step = 2;
            }
        }
		
        break;

    case 4:
        if(Si4432_TimeSendChangeLight == 0) {
            SendChangeLight_Step = 0;
        }
		//Debug_Printf( SET, "发送变光信号 %d", Si4432_TimeSendChangeLight);
        break;

    default:
        SendChangeLight_Step = 0;
        break;
    }
}


/*******************************************************************************
* Function Name  : Si4432_Send_EPlateNum
* Description    : 发送电子车牌号码
******************************************************************************/
void Si4432_SendBuffer(unsigned char* src, unsigned char len)
{
    u8 i;
    Si4432_SetTxMode();//设置为发送模式

    // 将需要发射的数据载入缓冲区
    Si4432_WriteRegister(0x3e, len); // 包长度
    rst_Si4432_SEL;
    Si4432_WriteByte(0x7f | 0x80);//写入寄存器地址
    for(i = 0; i< len; i++) {
        Si4432_WriteByte(src[i]);
    }
    set_Si4432_SEL;

    Si4432_WaitTxFinished();//等待发送数据完成
}


/*******************************************************************************
* Function Name  : Si4432_Send_EPlateNum
* Description    : 发送电子车牌号码
******************************************************************************/
static void Si4432_Send_EPlateNum(unsigned char* src)
{
    unsigned char* s1 = src;
    unsigned short crc16;

    unsigned char msg_word;//信息字                         //上传采集仪消息
    //0x00: 正常
    //0x01: 有违章为上传
    //0x02: 无证驾驶
    //0x03: 无证运营
    //0x04: 被盗抢车
    //0x05: 黑车
    *s1++ = 0x7f;
    memcpy( s1, m_carinfo.Car_5SN, __CAR5SN_LEN__ );
    s1 += __CAR5SN_LEN__;

    *s1++ = m_carinfo.Car_Type;
    *s1++ = msg_word;

    *s1++ = HIBYTE( crc16 );
    *s1++ = LOBYTE( crc16 );

    Si4432_SendBuffer(src, s1 - src);
}


/*******************************************************************************
* Function Name  : Get_Illegal_Untreated
* Description    : 标记 将 已经上传的违章  从   未上传违章列表  中  删除
******************************************************************************/
void  Get_Illegal_Untreated(unsigned int ill_num )
{
    unsigned short  i;
    unsigned short  j;
    unsigned int*   pill_upload;
    unsigned char   ill_remove;

    pill_upload = (unsigned int*)malloc(ViolenceUntreated * sizeof(int)); // 读未上传罚单记录编号
    MX25L128_BufferRead((unsigned char*)pill_upload, ADDR_ILLEGAL_UPLOAD + 8, ViolenceUntreated * sizeof(int) );

    ill_remove = 0;

    // 检查该编号是否在列表中
    for(j=0; j<ViolenceUntreated; j++) {
        if( pill_upload[j] == ill_num) {// 找到可以删除的编号
            pill_upload[j] = 0xffff; // 设置移除标记
            ill_remove++;// 删除总数增1
            break;
        }
    }

    if( ill_remove > 0 ) {
        for( i=0; i<ViolenceUntreated; i++ ) {
            if( pill_upload[i] == 0xffff ) {	        // 移走标记了的记录编号
                for(j=i; j<ViolenceUntreated; j++) {	// 向前移动一条记录
                    pill_upload[j] = pill_upload[j+1];
                }
                ViolenceUntreated--;
            }
        }

        //修改标记后的未上传违章数
        MX25L128_Word_Write(ADDR_ILLEGAL_UPLOAD + 4, ViolenceUntreated);

        //重新写入待上传违章记录编号
        MX25L128_BufferWrite((unsigned char*)pill_upload, ADDR_ILLEGAL_UPLOAD + 8, ViolenceUntreated * sizeof(int));
    }

    free(pill_upload);
}


/*******************************************************************************
* Function Name  : Si4432_Send_IllegalNum
* Description    : 发送违章内容
******************************************************************************/
void Si4432_Send_IllegalNum(unsigned char* src)
{
    unsigned char* s1 = src;
    unsigned short crc16;
    unsigned short msg_id=0x0802;//消息id
    Illegal_Def     illegal1;
    unsigned int    ill_Addr;
    volatile unsigned int*   pill_upload;
    unsigned char   ill_size;
    f2b_Def rev_ill_longi;
    f2b_Def rev_ill_lati;
    i2b_Def rev_ill_id;

    ill_size = (int)&illegal1.ino - (int)&illegal1.id + sizeof(illegal1.ino);
    pill_upload = (unsigned int*)malloc(  sizeof(int) );
    MX25L128_BufferRead((unsigned char*)pill_upload,
                        ADDR_ILLEGAL_UPLOAD + 8 + ( ViolenceUntreated - 1  ) * sizeof(int),
                        sizeof(int) );

    if( pill_upload[0] < (1+(0xffffff-0x300000)/ill_size) ) {
        ill_Addr = ADDR_ILLEGAL_CONTECT + ( pill_upload[0]-1) * ill_size;
        MX25L128_BufferRead( (unsigned char*)&illegal1.id, ill_Addr, ill_size );    // 读罚单

        *s1++ = 0x7f;
        *s1++ = HIBYTE(msg_id);           //消息id
        *s1++ = LOBYTE(msg_id);

        memcpy( s1, m_carinfo.Car_5SN, __CAR5SN_LEN__ ); //车牌号
        s1 += __CAR5SN_LEN__;

        *s1++ =  0x02;             //命令类型 0x02：发送罚单内容

        rev_ill_id.x = (u32)(illegal1.id);
        *s1++ =  rev_ill_id.c[3];     
        *s1++ =  rev_ill_id.c[2];     
        *s1++ =  rev_ill_id.c[1];     
        *s1++ =  rev_ill_id.c[0];     
        *s1++ =  illegal1.ptype;      

        memcpy(s1, illegal1.carno, __CAR9SN_LEN__);
        s1 += __CAR9SN_LEN__;

        Str2BcdTime(illegal1.stime,s1,14);
        s1 += 7;

        Str2BcdTime(illegal1.etime,s1,14); 
        s1 += 7;

        *s1++ =  illegal1.driver_type;   //准驾车型

        Str2BcdDriverNo(illegal1.driver_no, s1);
        s1 += 9;

        rev_ill_longi.x = illegal1.longitude;
        *s1++ = rev_ill_longi.c[0];
        *s1++ = rev_ill_longi.c[1];
        *s1++ = rev_ill_longi.c[2];
        *s1++ = rev_ill_longi.c[3];
        *s1++ =  illegal1.ewhemi;

        rev_ill_lati.x = illegal1.latitude;
        *s1++ = rev_ill_lati.c[0];
        *s1++ = rev_ill_lati.c[1];
        *s1++ = rev_ill_lati.c[2];
        *s1++ = rev_ill_lati.c[3];
        *s1++ =  illegal1.nshemi;

        *s1++ =  illegal1.ino;          //违章代号

        *s1++ = HIBYTE( crc16 );
        *s1++ = LOBYTE( crc16 );        //校验字

        Si4432_SendBuffer(src, s1 - src);
    }
}


/*******************************************************************************
* Function Name  : Si4432_SendSnToLightMode
* Description    : 发送车牌
******************************************************************************/
void Si4432_SendSnToLightMode(void)
{
    static u32 last_time = 0;
    unsigned char send_len;
    static unsigned char send_sn_sta = 0;

    if(last_time == 0 ) {
        last_time = Sys_TimeCount;
        return;
    }

    switch(send_sn_sta) {
    case 0:
        if( m_var.Flag_InitLAMPidNo == SET ) {
            m_var.Flag_InitLAMPidNo = RESET;
            last_time = Sys_TimeCount;//当前发送时间
            send_sn_sta = 1;
        }
        break;
    case 1:
        send_len = Get_SendCarNo(Si4432_TxBuf);
        Si4432_SendBuffer(Si4432_TxBuf, send_len);
        Si4432_TimeSendSn = 50;
        send_sn_sta = 2;
        break;
    case 2:
        if(Sys_TimeCount - last_time > 2) {
            last_time = Sys_TimeCount;//当前发送时间
            send_sn_sta = 3;
        }
        else if(Si4432_TimeSendSn == 0) {
            send_sn_sta = 1;
        }
        break;
    case 3:
        if(Sys_TimeCount - last_time > 5) {
            send_sn_sta = 0;
        }
        break;
    default:
        send_sn_sta = 0;
        break;
    }
}


/*******************************************************************************
* Function Name  : Si4432_SendChangeLight
* Description    : 发送变光信号
******************************************************************************/
void Si4432_SendChangeLight(unsigned char* src)
{
    unsigned char pack_len;
    unsigned char *s = src;

    s += HEAD_LENGTH;

    *s++ = 1;
    *s++ = gpsx.direction;

    GetLocation( s ); 
    s += 24;

    pack_len = GTW_Packet( src, 1, src+HEAD_LENGTH, s - src - HEAD_LENGTH, m_carinfo.DTU_Type );

    Si4432_SendBuffer(src, pack_len);
}


/*******************************************************************************
* Function Name  : Si4432_RxBufferProcess
* Description    : 稽查命令的标志信息设置
******************************************************************************/
void Msg_Set_Sign_DataColl(u8 *src)
{
    GTW02_BoardCast_Msg.MsgWhistle    = ReadDataBit(*src, GPIO_Pin_7);        // D0: 鸣笛设置0=无效;1=有效
    GTW02_BoardCast_Msg.MsgHighBeams  = ReadDataBit(*src, GPIO_Pin_6);        // D1: 远光设置0=无效;1=有效
    GTW02_BoardCast_Msg.MsgCity       = ReadDataBit(*src, GPIO_Pin_5);        // D2: 市区设置0=无效;1=有效
    GTW02_BoardCast_Msg.MsgHighway    = ReadDataBit(*src, GPIO_Pin_4);        // D3: 高速设置0=无效;1=有效
    GTW02_BoardCast_Msg.MsgWirelessSignal = ReadDataBit(*src, GPIO_Pin_3);    // D4: 无线信号设置0=无效;1=有效
    GTW02_BoardCast_Msg.MsgRestAreas  = ReadDataBit(*src, GPIO_Pin_2);        // D5: 服务区设置0=无效;1=有效

    if( GTW02_BoardCast_Msg.MsgWhistle == SET ) {
        DataCollectorMsg.MsgWhistle     = ReadDataBit(*(src+1), GPIO_Pin_7);            // 0=允许鸣笛;1=禁止鸣笛
    }
    if( GTW02_BoardCast_Msg.MsgHighBeams == SET ) {
        DataCollectorMsg.MsgHighBeams   = ReadDataBit(*(src+1), GPIO_Pin_6);            // 0=允许远光;1=禁止远光
    }
    if( GTW02_BoardCast_Msg.MsgCity == SET ) {
        DataCollectorMsg.MsgCity        = ReadDataBit(*(src+1), GPIO_Pin_5);            // 0=非市区;1=市区
    }
    if( GTW02_BoardCast_Msg.MsgHighway == SET ) {
        DataCollectorMsg.MsgHighway     = ReadDataBit(*(src+1), GPIO_Pin_4);    	    // 0=非高速;1=高速
    }
    if( GTW02_BoardCast_Msg.MsgWirelessSignal == SET ) {
        DataCollectorMsg.MsgWirelessSignal = ReadDataBit(*(src+1), GPIO_Pin_3);         // 0=允许无线信号;1=禁用无线信号
    }
    if( GTW02_BoardCast_Msg.MsgRestAreas == SET ) {
        DataCollectorMsg.MsgRestAreas   = ReadDataBit(*(src+1), GPIO_Pin_2);            // 0=非服务区; 1=服务区
    }
#ifdef __DEBUG
    if(m_var.Flag_Debug_Enable == SET && m_var.Flag_Init_RF433 == 0 )
        Debug_Printf(SET,
                     "\t广播=[%02x, %02x]\r\n\t鸣笛%s[%s]\t远光%s[%s]\r\n\t市区%s[%s]\t高速%s[%s]\r\n\t无线%s[%s]\t服务区%s[%s]",
                     *src, *(src+1),
                     DataCollectorMsg.MsgWhistle == SET ? "禁止" : "允许",     GTW02_BoardCast_Msg.MsgWhistle == SET ? "有效" : "无效",
                     DataCollectorMsg.MsgHighBeams == SET ? "禁止" : "允许",   GTW02_BoardCast_Msg.MsgHighBeams == SET ? "有效" : "无效",
                     DataCollectorMsg.MsgCity == SET ? "禁止" : "允许",        GTW02_BoardCast_Msg.MsgCity == SET ? "有效" : "无效",
                     DataCollectorMsg.MsgHighway == SET ? "禁止" : "允许",     GTW02_BoardCast_Msg.MsgHighway == SET ? "有效" : "无效",
                     DataCollectorMsg.MsgWirelessSignal == SET ? "禁止" : "允许", GTW02_BoardCast_Msg.MsgWirelessSignal == SET ? "有效" : "无效",
                     DataCollectorMsg.MsgRestAreas == SET ? "禁止" : "允许",   GTW02_BoardCast_Msg.MsgRestAreas == SET ? "有效" : "无效");
#endif
}


/*******************************************************************************
* Function Name  : Si4432_RxBufferProcess
* Description    : 将接收到的数据进行协议解析
******************************************************************************/
void Si4432_RxBufferProcess(void)
{
#define RF433_HEAD_LEN 9
    unsigned char  i;
    unsigned char  rev_len;
    unsigned char  speed_group;
    unsigned short pack_len = 64;
    unsigned short iMsg = 0;
    unsigned short send_count;
    i2b_Def        rev_ill_num;
    i2b_Def        rev_speed_group;
    i2b_Def        TerCarType;      // CarTypeDef;车辆类型定义中的  本车实际车型

    rev_len = Si4432_ReadBuffer();

    if(rev_len >  0 ) {
        if( Si4432_RxBuf[0] == 0x7E) {  //协仪
#ifdef __DEBUG
     
        Debug_Printf( SET, "收到SI432信号" );
#endif
            send_count = GTW_Analysis(Si4432_RxBuf, rev_len, Si4432_TxBuf, &pack_len);  //数据解包
            if( send_count > 0 ) {
                for( i=0; i<send_count / pack_len; i++ ) {
                    Si4432_SendBuffer( Si4432_TxBuf, pack_len );
                }
                if( send_count % pack_len > 0 ) {
                    Si4432_SendBuffer( Si4432_TxBuf, send_count % pack_len );
                }
            }
        }
        else if (Si4432_RxBuf[0] == 0x7F ) {
            if( (Si4432_RxBuf[1] ^ Si4432_RxBuf[2]) == 0xFF) {
                CheckerAnswer_CMDno = Si4432_RxBuf[1]; 

                Msg_Set_Sign_DataColl(&Si4432_RxBuf[3]);

                memcpy( rev_speed_group.c, Si4432_RxBuf+5, sizeof(rev_speed_group.c) ); // 17.6.7 csYxj(728920175@qq.com)

                for(i = 0; i<10; i++) {
                    if( memcmp(m_carinfo.Car_5SN, Si4432_RxBuf + RF433_HEAD_LEN + __CAR5SN_LEN__ * i, __CAR5SN_LEN__) == 0 ) {
                        CheckerAnswer_index = 4;
                        CheckerAnswer_ID.x = rev_speed_group.x;
                    }
                    if( (RF433_HEAD_LEN + __CAR5SN_LEN__ * i) >= rev_len )
                        break;
                }

                if( CheckerAnswer_index != 4 && rev_speed_group.x != CheckerAnswer_ID.x && rev_speed_group.x != 0 ) {
                    CheckerAnswer_CMDno = Si4432_RxBuf[1];
                    Flag_InspectCmd = SET;
#ifdef __DEBUG
                    // XFS_WriteBuffer(xfs_auto, "收到采集仪编号");
                    if(m_var.Flag_Debug_Enable == SET && m_var.Flag_Init_RF433 == 0 )
                        Debug_Printf(SET,
                                     "采集仪编号(%02x%02x%02x%02x),广播(%02x,%02x)",
                                     rev_speed_group.c[0],
                                     rev_speed_group.c[1],
                                     rev_speed_group.c[2],
                                     rev_speed_group.c[3],
                                     RF433_MsgMask,
                                     RF433_MsgData);
#endif
                }
            }
            else {
                iMsg = MAKEWORD(Si4432_RxBuf[1], Si4432_RxBuf[2]);
                switch( iMsg ) {
                case 1: {
                    Msg_Set_Sign_DataColl(&Si4432_RxBuf[3]);

                    speed_group = Si4432_RxBuf[5];

                    for(i = 0; i < speed_group; i++) {
                        rev_speed_group.c[3] = (Si4432_RxBuf[i*6+6]);
                        rev_speed_group.c[2] = (Si4432_RxBuf[i*6+6+1]);
                        rev_speed_group.c[1] = (Si4432_RxBuf[i*6+6+2]);
                        rev_speed_group.c[0] = (Si4432_RxBuf[i*6+6+3]);

                        if( ReadData32Bit( rev_speed_group.x, (1 << (TerCarType.x - 1)) ) == SET) {
                            Flag_LimitSpeed = 1;
                            Dir_SGroup.Restrict_Direction = (Si4432_RxBuf[i*6+6+4]);    //方向限制
                            Dir_SGroup.Max_Speed = (Si4432_RxBuf[i*6+6+5]);             //最高车速限制
                        }
                        else if( ReadData32Bit( rev_speed_group.x, (1 << (TerCarType.x - 1)) ) == RESET) {
                            Flag_LimitSpeed = 0;
                        }
                    }
                    break;
                }
                case 2: {  
                    if(Si4432_RxBuf[8] == 0x01)
                    {
                        if(ViolenceUntreated > 0)    //违章总数大于0，则发送电子车牌
                        {
                            Si4432_Send_EPlateNum(Si4432_RxBuf);//发送电子车牌
#ifdef __DEBUG
                            XFS_WriteBuffer(xfs_auto, "发送电子车牌");
#endif
                        }
                    }
                    else if(Si4432_RxBuf[8] == 0x02) //0x02：确定返回违单内容
                    {
                        if(memcmp(&Si4432_RxBuf[3], m_carinfo.Car_5SN, __CAR5SN_LEN__) == 0) {
                            rev_ill_num.c[3] = Si4432_RxBuf[9];
                            rev_ill_num.c[2] = Si4432_RxBuf[10];
                            rev_ill_num.c[1] = Si4432_RxBuf[11];
                            rev_ill_num.c[0] = Si4432_RxBuf[12];
                            if( rev_ill_num.x > 0) {
                                Get_Illegal_Untreated(rev_ill_num.x); //标记 将 已经上传的违章  从   未上传违章列表  中  删除
                            }

                            if(ViolenceUntreated > 0) {
                                Si4432_Send_IllegalNum( Si4432_TxBuf);       //发送违章内容
                            }
                        }
                    }
                    break;
                }
                }
            }
        }
        memset(Si4432_RxBuf, 0, sizeof(Si4432_RxBuf));
    }
}


/*******************************************************************************
* Function Name  : Si4432_CheckerAnswer
* Description    :
******************************************************************************/
extern unsigned int Sys_RunTime;
void Si4432_CheckerAnswer(void)
{
    switch(CheckerAnswer_index) {
    case 0: {
        if(Flag_InspectCmd == SET && CheckerAnswer_timer == 0) {
            Flag_InspectCmd = RESET;
            CheckerAnswer_CMDno = (CheckerAnswer_CMDno % 8);
            CheckerAnswer_timer = S_Timer[CheckerAnswer_CMDno] + rand() % 10;
            CheckerAnswer_index = 2;
        }
        else {
            Flag_InspectCmd = RESET;
        }
        break;
    }

    case 1: {
#ifdef __DEBUG
        XFS_WriteBuffer(xfs_auto, "%d微秒后发送车牌", CheckerAnswer_timer);
        Debug_Printf(SET, "%d微秒后发送车牌", CheckerAnswer_timer);
#endif
        CheckerAnswer_index = 2;
        break;
    }

    case 2: {
        if(CheckerAnswer_timer == 0) {
            Si4432_Send_EPlateNum(Si4432_RxBuf);//发送电子车牌
            CheckerAnswer_index = 0;
        }
        break;
    }

    case 3: {
        if(CheckerAnswer_timer == 0) {
            CheckerAnswer_index = 0;//跳转到Default
        }
        break;
    }

    case 4: {
        CheckerAnswer_timer = 100000;//设置10S保护时间
        CheckerAnswer_index = 0;//跳转到Default
        break;
    }

    default:
        break;
    }
}

/*******************************************************************************
* Function Name  : Si4432_SendTimer
* Description    : 取出对应的位数数字，用来计算时间
******************************************************************************/
void Si4432_SendTimer(unsigned char* TmpSN)
{
    int i;
    unsigned int TmpSNData;

    TmpSNData = TmpSN[1];		//只取车号的低4字节
    TmpSNData <<= 8;
    TmpSNData += TmpSN[2];
    TmpSNData <<= 8;
    TmpSNData += TmpSN[3];
    TmpSNData <<= 8;
    TmpSNData += TmpSN[4];
    S_Timer[0] = (unsigned char)(TmpSNData%10);
    S_Timer[1] = (unsigned char)((TmpSNData%100)/10);
    S_Timer[2] = (unsigned char)((TmpSNData%1000)/100);
    S_Timer[3] = (unsigned char)((TmpSNData%10000)/1000);
    S_Timer[4] = (unsigned char)((TmpSNData%100000)/10000);
    S_Timer[5] = (unsigned char)((TmpSNData%1000000)/100000);
    S_Timer[6] = (unsigned char)((TmpSNData%10000000)/1000000);
    S_Timer[7] = (unsigned char)((TmpSNData%100000000)/10000000);

    for(i=0; i<8; i++) {
        S_Timer[i] = S_Timer[i] * 35;
    }
}

