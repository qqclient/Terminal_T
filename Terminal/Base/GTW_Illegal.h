
extern volatile unsigned char  Flag_LimitSpeed;     // ��ͳ������ź�
extern volatile unsigned char  Flag_LimitRun;       // �����ź�
extern volatile unsigned char  Flag_ChangeLight;    // ����ź�
extern volatile unsigned char  Flag_DrivePlayPhone; // �������ֻ��ź�
extern volatile unsigned char  Flag_ApplyResult;    // �յ�����������
extern volatile unsigned char  Flag_NoDriverLic;    // ��֤��ʻ��־

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

