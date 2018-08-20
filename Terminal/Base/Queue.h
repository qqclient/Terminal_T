
#include "stm32f10x.h"


// �������������
#define QueueObject unsigned char

//���нṹ
typedef struct {
    unsigned char* q_memory;// ���л���
    int rear;               // ��β
    int front;              // ��ͷ
    int length;             // ���и���
    int size;               // �����ܸ���
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
