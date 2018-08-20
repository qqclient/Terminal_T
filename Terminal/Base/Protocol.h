
#define I2411E_RxBUF_LEN		2048		    // ���ջ�������С
#define I2411E_TxBUF_LEN		2048   			// ���ͻ�������С

extern unsigned char gtw_RxBuf[ ];    // ���ջ�����
extern unsigned char gtw_TxBuf[ ];    // ���ͻ�����


extern unsigned short GTW_Packet(unsigned char* dsr, unsigned short nMsg_id,
                                 unsigned char* src, unsigned short len,
                                 unsigned short obj);

extern unsigned short int GTW_Analysis(unsigned char* src, unsigned short len,
                                       unsigned char* dsr, unsigned short* packet_size);

extern void GTW_ReceiveData( unsigned char dt, unsigned short recv_object, unsigned int recv_handle);
