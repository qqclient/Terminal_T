/******************** (C) COPYRIGHT soarsky ************************************
* File Name          : io_detection.c
* Author             : csyxj@728920175@qq.com
* Version            : V0.1
* Date               : 2017-05-27
* Description        : 输入采集功能
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "gtw_Head.h"

extern CarInfo_Def m_carinfo;

Car_InputCheck_Typedef Car_InputCheck;
Car_InputCheck_Typedef Car_SetupCheck;
Car_InputCheck_Typedef Car_InputCheck_Last;

static void Read_HighBeam(unsigned char real);
static void Read_LowBeam(unsigned char real);
static void Read_Ignite(unsigned char real);
static void Read_Honk(unsigned char real);
static void Read_DriverBelt(unsigned char real);
static void Read_DeputyBelt(unsigned char real);
static void Read_DriverDoor(unsigned char real);
static void Read_SecondDoor(unsigned char real);
static void Read_IC1(void);
static void Read_DeputyBelt2(unsigned char real);
static void Read_C_Key(void);


   unsigned char wftlF,wftlH; 
   unsigned int  wftH[2];
	unsigned int  wftL[2];                  // 记录波形的高低电平时间
	unsigned char wfl=0,wfh=0;

/*******************************************************************************
* Function Name  : Study_CarStatus
* Description    : 学习车辆状态
*******************************************************************************/
void Study_CarStatus(void)
{
    unsigned char i;
    unsigned char Flag_Study = RESET;

    M35_TimeCount = 2000;

    for(i = 0; i < (KEY_COUNT+1); i++) {
        Read_Key(); // 检测标定按键
    }

    while(1) {
        Read_CarStatus( SET );
			  Car_InputCheck.Value_Ignite = RESET    ;  // 点火
        Read_Key();// 检测标定按键

        if( M35_TimeCount == 1800) {
            if( Car_InputCheck.Value_KEY == SET ) {
                Flag_Study = SET;
            }
            else {
                break;
            }
        }

        // 开关采集的端口状态标定
        if( Car_InputCheck.Value_KEY == SET && M35_TimeCount < 1000) {
            //第二步将读取出来的值重新赋值
            m_carinfo.Value_HighBeam    = Car_InputCheck.Value_HighBeam   ;  // 远光灯  //==SET表示打开,==RESET表示关闭
            m_carinfo.Value_LowBeam     = Car_InputCheck.Value_LowBeam    ;  // 近光灯
            m_carinfo.Value_Ignite      = Car_InputCheck.Value_Ignite     ;  // 点火
            m_carinfo.Value_Honk        = Car_InputCheck.Value_Honk       ;  // 喇叭
            m_carinfo.Value_DriverBelt  = Car_InputCheck.Value_DriverBelt ;  // 主安全带
            m_carinfo.Value_DeputyBelt  = Car_InputCheck.Value_DeputyBelt ;  // 副安全带1
            m_carinfo.Value_DeputyBelt2 = Car_InputCheck.Value_DeputyBelt2;  // 副安全带2
            m_carinfo.Value_DriverDoor  = Car_InputCheck.Value_DriverDoor ;  // 门开关1
            m_carinfo.Value_SecondDoor  = Car_InputCheck.Value_SecondDoor ;  // 门开关2

            MX25L128_SaveBase( SET );

#ifdef __DEBUG
            XFS_WriteBuffer(xfs_auto, "学习成功");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "学习成功");
#endif
            break;
        }

        if ( M35_TimeCount == 0 ) {
            break;
        }

        IWDG_ReloadCounter();

    }

    if(Flag_Study == SET ) {
        M35_TimeCount = 500;//
        while(1) {
            Read_CarStatus( RESET );
            IWDG_ReloadCounter();
            if ( M35_TimeCount == 0 ) {
                break;
            }
        }
    }

    M35_TimeCount = 0;
    GPIO_SetBits(LIGHT_PORT,LIGHT);       // 灭

    memset( (unsigned char*)&Car_SetupCheck, 0, (int)&Car_SetupCheck.Value_C_KEY - (int)&Car_SetupCheck );
}

void Input_Study(void)
{
    static unsigned char study_st = 0;
    static unsigned int  study_time = 0;
    unsigned char i = 0;
    
    switch(study_st) {
    case 0:
        if( m_var.Flag_InputStudy == 0xff ) {
            XFS_WriteBuffer(xfs_auto, "开始学习");
#ifdef __DEBUG
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf(SET, "开始学习");
#endif
            m_var.Flag_InputStudy = 0;
            study_st++;
        }
        break;
    case 1:
        XFS_WriteBuffer(xfs_auto, "请将车辆熄火,关好车门,关闭远近灯,松开安全带,15秒后开始读取状态");
#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "请将车辆熄火,关好车门,关闭远近灯,松开安全带,15秒后开始读取状态");
#endif
        study_time = Sys_TimeCount;
        study_st++;
        break;
    case 2:
        if( Sys_TimeCount - study_time >= 25 ) {
            study_st++;
        }
        break;
    case 3:
        for( i=0; i<KEY_COUNT+1; i++ )
            Read_CarStatus( SET );
    
        m_carinfo.Value_HighBeam    = Car_InputCheck.Value_HighBeam;
        m_carinfo.Value_LowBeam     = Car_InputCheck.Value_LowBeam;
        m_carinfo.Value_Ignite      = Car_InputCheck.Value_Ignite;
        m_carinfo.Value_Honk        = Car_InputCheck.Value_Honk;
        m_carinfo.Value_DriverBelt  = Car_InputCheck.Value_DriverBelt;
        m_carinfo.Value_DeputyBelt  = Car_InputCheck.Value_DeputyBelt;
        m_carinfo.Value_DeputyBelt2 = Car_InputCheck.Value_DeputyBelt2;
        m_carinfo.Value_DriverDoor  = Car_InputCheck.Value_DriverDoor;
        m_carinfo.Value_SecondDoor  = Car_InputCheck.Value_SecondDoor;
        
        MX25L128_SaveBase( SET );
        study_st++;
        break;
    case 4:
        XFS_WriteBuffer(xfs_auto, "学习成功");
#ifdef __DEBUG
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf(SET, "学习成功");
#endif
        memset( (unsigned char*)&Car_SetupCheck, 0,
                (int)&Car_SetupCheck.Value_C_KEY - (int)&Car_SetupCheck );
        
        study_st = 0;
        break;
    }
}

/*******************************************************************************
* Function Name  : Read_CarStatus
* Description    : 读取远光灯状态
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void Read_CarStatus(unsigned char real)
{
	if( real != SET ){  //不是学习状态下测试灯光，必须在点火后1S才能检测
		if( m_var.ReadBeamTimer > 2000 ){
				Read_HighBeam(real);    //远光灯
				Read_LowBeam(real);     //近光灯
		}
	}
	else{  //学习的时候不需要延时
		Read_HighBeam(real);    //远光灯
		Read_LowBeam(real);     //近光灯
	}
	if( real != SET )  //学习的时候不学习点火
			Read_Ignite(real);      //点火
    Read_Honk(real);        //喇叭
    Read_DriverBelt(real);  //主安全带
    Read_DeputyBelt(real);  //副安全带
    Read_DriverDoor(real);  //驾驶员一侧的门
    Read_SecondDoor(real);  //副驾驶员一侧的门  //临时作为车速是否大于零的按键判断
    Read_IC1();
    Read_DeputyBelt2(real);
    Read_C_Key();
    Read_Key();
}

/*******************************************************************************
* Function Name  : Read_HighBeam
* Description    : 读取远光灯状态
*******************************************************************************/
static void Read_HighBeam(unsigned char real)
{
    static int Key_Hi = 0;
    static int Key_Lo = 0;

    unsigned char key_value = GPIO_ReadInputDataBit(HIGH_BEAM_PORT, HIGH_BEAM);

	  //-------------------------------------------------
	 if( real == RESET ){    //熄火后不再检测远灯  real = SET 是学习，real = RESET 是平时检测
	  //if( Car_InputCheck.Value_Ignite == RESET )
		 if( GPIO_ReadInputDataBit(IGNITE_PORT, IGNITE) )
			return;
	 }
	  //---------------------------------------------------
    if( (m_carinfo.Flag_LAMPInput & 0x1) != RESET ) {
        if( key_value == ( real == RESET ? m_carinfo.Value_HighBeam & 0x01 : 0 )) {
            if(Key_Lo++ >= KEY_COUNT) {
                Car_InputCheck.Value_HighBeam = SET;
            }
            Key_Hi = 0;
        }
        else {
            if(Key_Hi++ >= KEY_COUNT) {
                Car_InputCheck.Value_HighBeam = RESET;
            }
            Key_Lo = 0;
        }

        if(Car_InputCheck_Last.Value_HighBeam != Car_InputCheck.Value_HighBeam) {
#ifndef __DEBUG
            Car_InputCheck_Last.Value_HighBeam = Car_InputCheck.Value_HighBeam;
#endif
            Car_SetupCheck.Value_HighBeam = SET;
        }
    }

#ifdef __DEBUG
#ifdef __KEY_VOICE__
    if(Car_InputCheck_Last.Value_HighBeam != Car_InputCheck.Value_HighBeam) {
        Car_InputCheck_Last.Value_HighBeam = Car_InputCheck.Value_HighBeam;
        XFS_WriteBuffer(xfs_auto, "远灯%s", Car_InputCheck.Value_HighBeam == SET ? "开" : "关");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf( SET, "远灯%s", Car_InputCheck.Value_HighBeam == SET ? "开" : "关");
    }
#endif
#endif
}


/*******************************************************************************
* Function Name  : Read_LowBeam
* Description    : 读取近光灯状态
*******************************************************************************/
static void Read_LowBeam(unsigned char real)
{
    static int Key_Hi = 0;
    static int Key_Lo = 0;

    unsigned char key_value = GPIO_ReadInputDataBit(LOW_BEAM_PORT, LOW_BEAM);

		  //-------------------------------------------------
	 if( real == RESET ){    //熄火后不再检测近灯  real = SET 是学习，real = RESET 是平时检测
	  //if( Car_InputCheck.Value_Ignite == RESET )
		 if( GPIO_ReadInputDataBit(IGNITE_PORT, IGNITE) )
			return;
	 }
	  //---------------------------------------------------
	
    if( (m_carinfo.Flag_LAMPInput & 0x1) != RESET ) {
        if( key_value == ( real == RESET ? m_carinfo.Value_LowBeam & 0x01 : 0 )) {
            if(Key_Lo++ >= KEY_COUNT) {
                Car_InputCheck.Value_LowBeam = SET;
            }
            Key_Hi = 0;
        }
        else {
            if(Key_Hi++ >= KEY_COUNT) {
                Car_InputCheck.Value_LowBeam = RESET;
            }
            Key_Lo = 0;
        }

        if(Car_InputCheck_Last.Value_LowBeam != Car_InputCheck.Value_LowBeam) {
#ifndef __DEBUG
            Car_InputCheck_Last.Value_LowBeam = Car_InputCheck.Value_LowBeam;
#endif
            Car_SetupCheck.Value_LowBeam = SET;
        }
    }

#ifdef __KEY_VOICE__
#ifdef __DEBUG
    if(Car_InputCheck_Last.Value_LowBeam != Car_InputCheck.Value_LowBeam) {
        Car_InputCheck_Last.Value_LowBeam = Car_InputCheck.Value_LowBeam;
        XFS_WriteBuffer(xfs_auto, "近灯%s", Car_InputCheck.Value_LowBeam == SET ? "开" : "关");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf( SET, "近灯%s", Car_InputCheck.Value_LowBeam == SET ? "开" : "关");
    }
#endif
#endif
}


/*******************************************************************************
* Function Name  : Read_Ignite
* Description    : 读取点火状态
*******************************************************************************/
static void Read_Ignite(unsigned char real)
{
    static int Key_Hi = 0;
    static int Key_Lo = 0;

    unsigned char key_value = GPIO_ReadInputDataBit(IGNITE_PORT, IGNITE);

    if( key_value == ( real == RESET ? m_carinfo.Value_Ignite & 1 : 0 )) {
        if(Key_Lo++ >= KEY_COUNT) {
            Car_InputCheck.Value_Ignite = SET;
        }
        Key_Hi = 0;
    }
    else {
        if(Key_Hi++ >= KEY_COUNT) {
            Car_InputCheck.Value_Ignite = RESET;
        }
        Key_Lo = 0;
    }

    if(Car_InputCheck_Last.Value_Ignite != Car_InputCheck.Value_Ignite) {
#ifndef __DEBUG
        Car_InputCheck_Last.Value_Ignite = Car_InputCheck.Value_Ignite;
#endif
        Car_SetupCheck.Value_Ignite = SET;
    }

#ifdef __KEY_VOICE__
#ifdef __DEBUG
    if(Car_InputCheck_Last.Value_Ignite != Car_InputCheck.Value_Ignite) {
        Car_InputCheck_Last.Value_Ignite = Car_InputCheck.Value_Ignite;
        XFS_WriteBuffer(xfs_auto, "%s", Car_InputCheck.Value_Ignite == SET ? "点火" : "熄火");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf( SET, "%s", Car_InputCheck.Value_Ignite == SET ? "点火" : "熄火");
    }
#endif
#endif
		
		if( Car_InputCheck.Value_Ignite != SET ){  //在熄火时灯光检测延时时间清零
			  //Debug_Printf( SET,"熄火清零");
				m_var.ReadBeamTimer = 0;
		}
		
}


/*******************************************************************************
* Function Name  : Read_Honk
* Description    : 喇叭检测
*******************************************************************************/
static void Read_Honk(unsigned char real)
{
    static int Key_Hi = 0;
    static int Key_Lo = 0;

    unsigned char key_value = GPIO_ReadInputDataBit(HONK_PORT, HONK);

    if( key_value == ( real == RESET ? m_carinfo.Value_Honk & 1 : 0 )) {
			
        /*if(Key_Lo++ >= KEY_COUNT) {
            Car_InputCheck.Value_Honk = SET;
        }
        Key_Hi = 0;*/
				
				//---------------------------------
						wftlH = 0;  //停止计时
					  if( wftH[0] >= 5 ) //0.5ms  //1Khz
						{
							wfh++;
						}
					  wftH[0]=0;
						
						wftlF = 1;  //开始计时
									//if(Key_Lo++ >= KEY_COUNT) {
						if( wftL[0] >= KEY_COUNT_KEY  ){
							 wfl = 0;
					     wfh = 0;
							 Car_InputCheck.Value_Honk = SET;
							 Debug_Printf( SET, "有波形开");
						}
						Key_Hi = 0;
				 //--------------------------------
    }
    else {
			
        /*if(Key_Hi++ >= KEY_COUNT) {
            Car_InputCheck.Value_Honk = RESET;
        }
        Key_Lo = 0;
				*/
				//----------------------------------
				  wftlF = 0;  //停止计时
					if( wftL[0] >= 5 ) //0.5ms  //1Khz
					{
						wfl++;
					}
					wftL[0]=0;
					
					wftlH = 1;  //开始计时
					//if(Key_Hi++ >= KEY_COUNT) {
					if( wftH[0] >= KEY_COUNT_KEY  ){
							wfl = 0;
							wfh = 0;
							Car_InputCheck.Value_Honk = RESET;
							//Debug_Printf( SET, "有波形关");
					}
					Key_Lo = 0;
				//----------------------------------
    }
		
		//-----------------------------
		if( wfl > 5 && wfh > 5	)  //有波形
				{
					wfl = 0;
					wfh = 0;
					//XFS_WriteBuffer(xfs_auto, "有波形");
					if( real == SET )
					{
						Car_InputCheck.Value_Honk = SET;
					}
					else{
					 if( (m_carinfo.Value_Honk & 0x01) == 0 ) {  //根据学习的状态来判断是高还是低
							Car_InputCheck.Value_Honk = SET;
					 }
					 else{
						  Car_InputCheck.Value_Honk = RESET;
					 }
				 }
				 Debug_Printf( SET, "有波形");
			}
		//-----------------------------

    if(Car_InputCheck_Last.Value_Honk != Car_InputCheck.Value_Honk) {
#ifndef __DEBUG
        Car_InputCheck_Last.Value_Honk = Car_InputCheck.Value_Honk;
#endif
        Car_SetupCheck.Value_Honk = SET;
    }

#ifdef __KEY_VOICE__
#ifdef __DEBUG
    if(Car_InputCheck_Last.Value_Honk != Car_InputCheck.Value_Honk) {
        Car_InputCheck_Last.Value_Honk = Car_InputCheck.Value_Honk;
        XFS_WriteBuffer(xfs_auto, "鸣笛%s", Car_InputCheck.Value_Honk == SET ? "按下" : "松开");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf( SET, "鸣笛%s", Car_InputCheck.Value_Honk == SET ? "按下" : "松开");
    }
#endif
#endif
}


/*******************************************************************************
* Function Name  : Read_DriverBelt
* Description    : 检测主安全带
*******************************************************************************/
static void Read_DriverBelt(unsigned char real)
{
    static int Key_Hi = 0;
    static int Key_Lo = 0;

    unsigned char key_value = GPIO_ReadInputDataBit(DRIVER_BELT_PORT,DRIVER_BELT);

    if( key_value == ( real == RESET ? m_carinfo.Value_DriverBelt & 1 : 0 )) {
        if(Key_Lo++ >= KEY_COUNT) {
            Car_InputCheck.Value_DriverBelt = SET;
        }
        Key_Hi = 0;
    }
    else {
        if(Key_Hi++ >= KEY_COUNT) {
            Car_InputCheck.Value_DriverBelt = RESET;
        }
        Key_Lo = 0;
    }

    if(Car_InputCheck_Last.Value_DriverBelt != Car_InputCheck.Value_DriverBelt) {
#ifndef __DEBUG
        Car_InputCheck_Last.Value_DriverBelt = Car_InputCheck.Value_DriverBelt;
#endif
        Car_SetupCheck.Value_DriverBelt = SET;
    }

#ifdef __KEY_VOICE__
#ifdef __DEBUG
    if(Car_InputCheck_Last.Value_DriverBelt != Car_InputCheck.Value_DriverBelt) {
        Car_InputCheck_Last.Value_DriverBelt = Car_InputCheck.Value_DriverBelt;
        XFS_WriteBuffer(xfs_auto, "%s安全带", Car_InputCheck.Value_DriverBelt == SET ? "系" : "松");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf( SET, "%s安全带", Car_InputCheck.Value_DriverBelt == SET ? "系" : "松");
    }
#endif
#endif
}


/*******************************************************************************
* Function Name  : Read_DeputyBelt
* Description    : 检测副安全带
*******************************************************************************/
static void Read_DeputyBelt(unsigned char real)
{
    static int Key_Hi = 0;
    static int Key_Lo = 0;

    unsigned char key_value = GPIO_ReadInputDataBit(DEPUTY_BELT_PORT, DEPUTY_BELT);

    if( key_value == ( real == RESET ? m_carinfo.Value_DeputyBelt & 0x01 : 0 )) {
        if(Key_Lo++ >= KEY_COUNT) {
            Car_InputCheck.Value_DeputyBelt = SET;
        }
        Key_Hi = 0;
    }
    else {
        if(Key_Hi++ >= KEY_COUNT) {
            Car_InputCheck.Value_DeputyBelt = RESET;
        }
        Key_Lo = 0;
    }

    if(Car_InputCheck_Last.Value_DeputyBelt != Car_InputCheck.Value_DeputyBelt) {
#ifndef __DEBUG
        Car_InputCheck_Last.Value_DeputyBelt = Car_InputCheck.Value_DeputyBelt;
#endif
        Car_SetupCheck.Value_DeputyBelt = SET;
    }

#ifdef __KEY_VOICE__
#ifdef __DEBUG
    if( (m_carinfo.Flag_Simulation & 0x01) == RESET ) {
        if(Car_InputCheck_Last.Value_DeputyBelt != Car_InputCheck.Value_DeputyBelt) {
            Car_InputCheck_Last.Value_DeputyBelt = Car_InputCheck.Value_DeputyBelt;
            XFS_WriteBuffer(xfs_auto, "%s副安全带", Car_InputCheck.Value_DeputyBelt == SET ? "系" : "松");
            if(m_var.Flag_Debug_Enable == SET)
                Debug_Printf( SET, "%s副安全带", Car_InputCheck.Value_DeputyBelt == SET ? "系" : "松");
        }
    }
#endif
#endif
}


/*******************************************************************************
* Function Name  : Read_DriverDoor
* Description    : 检测驾驶员一侧的门
*******************************************************************************/
static void Read_DriverDoor(unsigned char real)
{
    static int Key_Hi = 0;
    static int Key_Lo = 0;

    unsigned char key_value = GPIO_ReadInputDataBit(DRIVER_DOOR_PORT, DRIVER_DOOR);

    if( key_value == ( real == RESET ? m_carinfo.Value_DriverDoor & 0x01 : 0 )) {
        if(Key_Lo++ >= KEY_COUNT) {
            Car_InputCheck.Value_DriverDoor = SET;
        }
        Key_Hi = 0;
    }
    else {
        if(Key_Hi++ >= KEY_COUNT) {
            Car_InputCheck.Value_DriverDoor = RESET;
        }
        Key_Lo = 0;
    }

    if(Car_InputCheck_Last.Value_DriverDoor != Car_InputCheck.Value_DriverDoor) {
#ifndef __DEBUG
        Car_InputCheck_Last.Value_DriverDoor = Car_InputCheck.Value_DriverDoor;
#endif
        Car_SetupCheck.Value_DriverDoor = SET;
    }

#ifdef __KEY_VOICE__
#ifdef __DEBUG
    if(Car_InputCheck_Last.Value_DriverDoor != Car_InputCheck.Value_DriverDoor) {
        Car_InputCheck_Last.Value_DriverDoor = Car_InputCheck.Value_DriverDoor;
        XFS_WriteBuffer(xfs_auto, "%s门", Car_InputCheck.Value_DriverDoor == SET ? "开" : "关");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf( SET, "%s门", Car_InputCheck.Value_DriverDoor == SET ? "开" : "关");
    }
#endif
#endif
}


/*******************************************************************************
* Function Name  : Read_DriverDoor
* Description    : 检测驾驶员一侧的门
*******************************************************************************/
static void Read_SecondDoor(unsigned char real)
{
    static int Key_Hi = 0;
    static int Key_Lo = 0;

    unsigned char key_value = GPIO_ReadInputDataBit(SECOND_DOOR_PORT, SECOND_DOOR);

    if( key_value == ( real == RESET ? m_carinfo.Value_SecondDoor & 0x01 : 0 )) {
        if(Key_Lo++ >= KEY_COUNT) {
            Car_InputCheck.Value_SecondDoor = SET;
        }
        Key_Hi = 0;
    }
    else {
        if(Key_Hi++ >= KEY_COUNT) {
            Car_InputCheck.Value_SecondDoor = RESET;
        }
        Key_Lo = 0;
    }

    if(Car_InputCheck_Last.Value_SecondDoor != Car_InputCheck.Value_SecondDoor) {
#ifndef __DEBUG
        Car_InputCheck_Last.Value_SecondDoor = Car_InputCheck.Value_SecondDoor;
#endif
        Car_SetupCheck.Value_SecondDoor = SET;
    }

#ifdef __KEY_VOICE__
#ifdef __DEBUG
    if(Car_InputCheck_Last.Value_SecondDoor != Car_InputCheck.Value_SecondDoor) {
        Car_InputCheck_Last.Value_SecondDoor = Car_InputCheck.Value_SecondDoor;
        XFS_WriteBuffer(xfs_auto, "%s门2", Car_InputCheck.Value_SecondDoor == SET ? "开" : "关");
        if(m_var.Flag_Debug_Enable == SET)
            Debug_Printf( SET, "%s门2", Car_InputCheck.Value_SecondDoor == SET ? "开" : "关");
    }
#endif
#endif
}


/*******************************************************************************
* Function Name  : Read_DeputyBelt2
* Description    : 检测副安全带
*******************************************************************************/
static void Read_DeputyBelt2(unsigned char real)
{
    static int Key_Hi = 0;
    static int Key_Lo = 0;

    unsigned char key_value = GPIO_ReadInputDataBit(DEPUTY_BELT2_PORT, DEPUTY_BELT2);

    if( key_value == ( real == RESET ? m_carinfo.Value_DeputyBelt2 & 0x01 : 0 )) {
        if(Key_Lo++ >= KEY_COUNT) {
            Car_InputCheck.Value_DeputyBelt2 = SET;
        }
        Key_Hi = 0;
    }
    else {
        if(Key_Hi++ >= KEY_COUNT) {
            Car_InputCheck.Value_DeputyBelt2 = RESET;
        }
        Key_Lo = 0;
    }

    if(Car_InputCheck_Last.Value_DeputyBelt2 != Car_InputCheck.Value_DeputyBelt2) {
        Car_InputCheck_Last.Value_DeputyBelt2 = Car_InputCheck.Value_DeputyBelt2;
        Car_SetupCheck.Value_DeputyBelt2 = SET;
    }
}


/*******************************************************************************
* Function Name  :
* Description    :
*******************************************************************************/
static void Read_IC1(void)
{
    static int Key_Hi = 0;
    static int Key_Lo = 0;

    unsigned char key_value = GPIO_ReadInputDataBit(IC1_PORT, IC1);

    if( key_value == 0 ) {
        if(Key_Lo++ >= KEY_COUNT) {
            Car_InputCheck.Value_IC1 = SET;
        }
        Key_Hi = 0;
    }
    else {
        if(Key_Hi++ >= KEY_COUNT) {
            Car_InputCheck.Value_IC1 = RESET;
        }
        Key_Lo = 0;
    }

    if(Car_InputCheck_Last.Value_IC1 != Car_InputCheck.Value_IC1) {
        Car_InputCheck_Last.Value_IC1 = Car_InputCheck.Value_IC1;
        Car_SetupCheck.Value_IC1 = SET;
    }
}


/*******************************************************************************
* Function Name  : Read_KEY
* Description    : 学习按键
*******************************************************************************/
void Read_Key(void)
{
    static int Key_Hi = 0;
    static int Key_Lo = 0;

    unsigned char key_value = GPIO_ReadInputDataBit(KEY_PORT, KEY);

    if( key_value == 0 ) {
        if(Key_Lo++ >= KEY_COUNT) {
            Car_InputCheck.Value_KEY = SET;
        }
        Key_Hi = 0;
    }
    else {
        if(Key_Hi++ >= KEY_COUNT) {
            Car_InputCheck.Value_KEY = RESET;
        }
        Key_Lo = 0;
    }

    if(Car_InputCheck_Last.Value_KEY != Car_InputCheck.Value_KEY) {
        Car_InputCheck_Last.Value_KEY = Car_InputCheck.Value_KEY;
        Car_SetupCheck.Value_KEY = SET;
    }
}


/*******************************************************************************
* Function Name  : Read_C_Key
* Description    :
*******************************************************************************/
static void Read_C_Key(void)
{
    static int Key_Hi = 0;
    static int Key_Lo = 0;

    unsigned char key_value = GPIO_ReadInputDataBit(C_KEY_PORT, C_KEY);

    if( key_value == 0 ) {
        if(Key_Lo++ >= KEY_COUNT) {
            Car_InputCheck.Value_C_KEY = SET;
        }
        Key_Hi = 0;
    }
    else {
        if(Key_Hi++ >= KEY_COUNT) {
            Car_InputCheck.Value_C_KEY = RESET;
        }
        Key_Lo = 0;
    }

    if(Car_InputCheck_Last.Value_C_KEY != Car_InputCheck.Value_C_KEY) {
        Car_InputCheck_Last.Value_C_KEY = Car_InputCheck.Value_C_KEY;
        Car_SetupCheck.Value_C_KEY = SET;
    }
}

