
#define MAXNUM_TTS				16		//TTS����  ���
#define TTS_LEN					180		//TTS���в�ų��� 

extern volatile unsigned short XFS_TimeCount;

void XFS_Process(void);
void XFS_Initialize(void);
void XFS_WriteBuffer(unsigned char nType, char *fmt,...);
void XFS_SetVolume(char Volume);
//void XFS_CreatVolumeTask(void);
