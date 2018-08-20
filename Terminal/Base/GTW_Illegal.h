
extern volatile unsigned char  Flag_LimitSpeed;     // 大客车限行信号
extern volatile unsigned char  Flag_LimitRun;       // 限行信号
extern volatile unsigned char  Flag_ChangeLight;    // 变光信号
extern volatile unsigned char  Flag_DrivePlayPhone; // 开车玩手机信号
extern volatile unsigned char  Flag_ApplyResult;    // 收到驾照申请标记
extern volatile unsigned char  Flag_NoDriverLic;    // 无证驾驶标志

void SafeBelt(void);
void FarLightInArea(void);
void FarLightInMeet(void);
void DriverVerify(void);
void PreventSteal(void);
void DrivePlayPhone(void);
void FatiguDriving(void);
void BusRunAtNight(void);
void VehicleLimitRun(void);
void LimitSpeed(void);
void LimitTrumpet(void);
void IllegalPark(void);
void MakeViolation(unsigned short count);

