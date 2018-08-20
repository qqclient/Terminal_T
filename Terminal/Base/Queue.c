/*******************************************************************************
* File Name          : Queue.c
* Author             : csyxj@728920175@qq.com
* Version            : V0.1
* Date               : 2017-03-19
* Description        : FIFO����
*******************************************************************************/
#include "gtw_Head.h"

/*******************************************************************************
* ��������: QueueCreate
* ��������: ���г�ʼ��,�����ճ�ʼ�����ȷ����ڴ�ռ�;
*******************************************************************************/
ArrQueue* QueueCreate( int length )
{
    ArrQueue* pQueue = (ArrQueue *)malloc( sizeof(ArrQueue) );

    if( 0 == length ) {
        return(NULL);
    }

    pQueue->size = length;      // ��ʼ������������
    pQueue->length = 0;
    pQueue->rear = 0;
    pQueue->front = 0;
    pQueue->q_memory = NULL;
    pQueue->q_memory = malloc( pQueue->size * sizeof(QueueObject) );
    if ( NULL == pQueue->q_memory ) {
        free( pQueue );
        pQueue = NULL;
        return(pQueue);
    }

    return(pQueue);
}


/*******************************************************************************
* ��������: QueueDestroy
* ��������: ���ٶ���;
*******************************************************************************/
void QueueDestroy( ArrQueue* pQueue )
{
    if( NULL != pQueue ) {
        free( pQueue->q_memory );
        free( pQueue );

        pQueue = NULL;
    }
}


/*******************************************************************************
* ��������: QueueClear
* ��������: ��ն���;
*******************************************************************************/
void QueueClear( ArrQueue* pQueue )
{
    pQueue->rear = 0;    // �ƶ�ͷβָ��
    pQueue->front = 0;
    pQueue->length = 0;
}


/*******************************************************************************
* ��������: QueuePush
* ��������: ��������Ԫ��,��βָ��rear��1��
*******************************************************************************/
void QueuePush( ArrQueue* pQueue, QueueObject item )
{
    if( NULL == pQueue )
        return;

    pQueue->q_memory[ pQueue->rear ] = item;   // �������
    pQueue->rear++;         // ��β��ַ��1
    pQueue->length++;       // Ԫ�ظ���+1

    if( pQueue->rear >= pQueue->size ) {
        pQueue->rear = 0;   // ��β��ַ�ػ������׵�ַ
    }
}


/*******************************************************************************
* ��������: QueuePop
* ��������: �Ӷ���ȡ��Ԫ�أ���ͷָ��front��1��������Ϊ��ʱ����FALSE�������ɹ�����TRUE��
*******************************************************************************/
QueueObject QueuePop( ArrQueue* pQueue )
{
    QueueObject pItem;

    if( NULL == pQueue )
        return 0;

    pItem = pQueue->q_memory[ pQueue->front ]; // ���ݳ���
    pQueue->front++;        // ��ͷ��ַ+1
    pQueue->length--;       // Ԫ�ظ���-1

    if(pQueue->front >= pQueue->size ) {
        pQueue->front = 0;// ��ͷ��ַ�ػ������׵�ַ
    }

    return(pItem);
}


/*******************************************************************************
* ��������: QueueLength
* ��������: ȡ������Ԫ�ظ���;
*******************************************************************************/
int QueueLength( ArrQueue* pQueue )
{
    if( NULL ==  pQueue )
        return 0;

    return(pQueue->length);
}


/*******************************************************************************
* ��������: QueueSize
* ��������: ��ȡ�����ܴ�С;
*******************************************************************************/
int QueueSize( ArrQueue* pQueue )
{
    return(pQueue->size);
}


/******************************************************************************
* ��������: UART_SendByInt
* ��������: ��������
******************************************************************************/
void UART_SendByInt( USART_TypeDef * pUSARTx, ArrQueue* pQueue )
{
    if (USART_GetITStatus( pUSARTx, USART_IT_TXE ) != RESET ) {
        if( pQueue->length > 0 ) {
            // Write one byte to the transmit data register
            USART_SendData( pUSARTx, QueuePop( pQueue ) );
            pQueue->state = 1;
        }
        else {
            USART_ITConfig( pUSARTx, USART_IT_TXE, DISABLE );
            USART_ITConfig( pUSARTx, USART_IT_TC, ENABLE);
        }
    }
    else if (USART_GetITStatus( pUSARTx, USART_IT_TC ) != RESET ) {
        USART_ClearITPendingBit(pUSARTx, USART_IT_TC);

        if( pQueue->length > 0 ) {
            // Write one byte to the transmit data register
            USART_SendData( pUSARTx, QueuePop( pQueue ) );
            pQueue->state = 1;
        }
        else {
            USART_ITConfig( pUSARTx, USART_IT_TC, DISABLE );
            pQueue->state = 0;
        }
    }
}

/******************************************************************************
* ��������: UART_SendByIntStart
* ��������: �����жϷ�ʽ����UART����
******************************************************************************/
void UART_SendByIntStart( USART_TypeDef * pUSARTx, ArrQueue* pQueue )
{
    if( NULL != pQueue ) {
        if( pQueue->length > 0 && pQueue->state == 0 ) {
            pQueue->state = 1;
            USART_ClearFlag(pUSARTx, USART_FLAG_TC);            // ���������ɱ�־λ,������ܻᶪʧ��1���ֽڵ�����.
            USART_ITConfig( pUSARTx, USART_IT_TXE, ENABLE );    // �����жϷ���
        }
    }

}
