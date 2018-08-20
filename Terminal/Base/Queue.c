/*******************************************************************************
* File Name          : Queue.c
* Author             : csyxj@728920175@qq.com
* Version            : V0.1
* Date               : 2017-03-19
* Description        : FIFO队列
*******************************************************************************/
#include "gtw_Head.h"

/*******************************************************************************
* 函数名称: QueueCreate
* 函数描述: 队列初始化,并按照初始化长度分配内存空间;
*******************************************************************************/
ArrQueue* QueueCreate( int length )
{
    ArrQueue* pQueue = (ArrQueue *)malloc( sizeof(ArrQueue) );

    if( 0 == length ) {
        return(NULL);
    }

    pQueue->size = length;      // 初始化缓冲区长度
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
* 函数名称: QueueDestroy
* 函数描述: 销毁队列;
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
* 函数名称: QueueClear
* 函数描述: 清空队列;
*******************************************************************************/
void QueueClear( ArrQueue* pQueue )
{
    pQueue->rear = 0;    // 移动头尾指针
    pQueue->front = 0;
    pQueue->length = 0;
}


/*******************************************************************************
* 函数名称: QueuePush
* 函数描述: 向队列添加元素,队尾指针rear加1。
*******************************************************************************/
void QueuePush( ArrQueue* pQueue, QueueObject item )
{
    if( NULL == pQueue )
        return;

    pQueue->q_memory[ pQueue->rear ] = item;   // 数据入队
    pQueue->rear++;         // 队尾地址＋1
    pQueue->length++;       // 元素个数+1

    if( pQueue->rear >= pQueue->size ) {
        pQueue->rear = 0;   // 队尾地址回缓冲区首地址
    }
}


/*******************************************************************************
* 函数名称: QueuePop
* 函数描述: 从队列取出元素，队头指针front加1。当队列为空时返回FALSE，操作成功返回TRUE。
*******************************************************************************/
QueueObject QueuePop( ArrQueue* pQueue )
{
    QueueObject pItem;

    if( NULL == pQueue )
        return 0;

    pItem = pQueue->q_memory[ pQueue->front ]; // 数据出队
    pQueue->front++;        // 队头地址+1
    pQueue->length--;       // 元素个数-1

    if(pQueue->front >= pQueue->size ) {
        pQueue->front = 0;// 队头地址回缓冲区首地址
    }

    return(pItem);
}


/*******************************************************************************
* 函数名称: QueueLength
* 函数描述: 取队列中元素个数;
*******************************************************************************/
int QueueLength( ArrQueue* pQueue )
{
    if( NULL ==  pQueue )
        return 0;

    return(pQueue->length);
}


/*******************************************************************************
* 函数名称: QueueSize
* 函数描述: 获取队列总大小;
*******************************************************************************/
int QueueSize( ArrQueue* pQueue )
{
    return(pQueue->size);
}


/******************************************************************************
* 函数名称: UART_SendByInt
* 函数描述: 启动发送
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
* 函数名称: UART_SendByIntStart
* 函数描述: 启动中断方式发送UART数据
******************************************************************************/
void UART_SendByIntStart( USART_TypeDef * pUSARTx, ArrQueue* pQueue )
{
    if( NULL != pQueue ) {
        if( pQueue->length > 0 && pQueue->state == 0 ) {
            pQueue->state = 1;
            USART_ClearFlag(pUSARTx, USART_FLAG_TC);            // 清除传输完成标志位,否则可能会丢失第1个字节的数据.
            USART_ITConfig( pUSARTx, USART_IT_TXE, ENABLE );    // 启动中断发送
        }
    }

}
