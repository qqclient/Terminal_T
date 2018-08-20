
#include "stm32f10x.h"


// 定义操作的类型
#define QueueObject unsigned char

//队列结构
typedef struct {
    unsigned char* q_memory;// 队列缓冲
    int rear;               // 队尾
    int front;              // 队头
    int length;             // 队列个数
    int size;               // 队列总个数
    unsigned char state;
} ArrQueue;

ArrQueue* QueueCreate( int length );
void QueueDestroy( ArrQueue* pQueue );
void QueueClear( ArrQueue* pQueue );
void QueuePush( ArrQueue* pQueue, QueueObject item );
QueueObject QueuePop( ArrQueue* pQueue );
int  QueueLength( ArrQueue* pQueue );
int  QueueSize( ArrQueue* pQueue );

void UART_SendByInt( USART_TypeDef * pUSARTx, ArrQueue* pQueue );
void UART_SendByIntStart( USART_TypeDef * pUSARTx, ArrQueue* pQueue );
