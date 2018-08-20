
#define I2411E_RxBUF_LEN		2048		    // 接收缓冲区大小
#define I2411E_TxBUF_LEN		2048   			// 发送缓冲区大小

extern unsigned char gtw_RxBuf[ ];    // 接收缓冲区
extern unsigned char gtw_TxBuf[ ];    // 发送缓冲区


extern unsigned short GTW_Packet(unsigned char* dsr, unsigned short nMsg_id,
                                 unsigned char* src, unsigned short len,
                                 unsigned short obj);

extern unsigned short int GTW_Analysis(unsigned char* src, unsigned short len,
                                       unsigned char* dsr, unsigned short* packet_size);

extern void GTW_ReceiveData( unsigned char dt, unsigned short recv_object, unsigned int recv_handle);
