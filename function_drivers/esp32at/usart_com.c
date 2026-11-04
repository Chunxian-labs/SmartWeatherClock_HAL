#include "usart.h"
#include "usart_com.h"
#include "ring_buffer.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stddef.h>

static uint8_t usart2_rx_byte;                   //接收暂存变量,接收中断每收到一个字节，写入该变量，该变量随中断更新
static ring_buffer_t usart2_rx_rb;             //接收环形缓冲区,接收中断将接收暂存变量的字节写入该缓冲形缓冲区，该缓冲形缓冲区随中断更新
volatile uint32_t usart2_rx_overflow_count = 0;  //丢包计数，用于debug
uint8_t USART2_Ring_Buffer_Init(ring_buffer_t *rb)
{
    if(rb == NULL)
    {
        return 0;
    }
    return Ring_Buffer_Init(rb);
}
void USART2_Start_Receive(void)   //上电/初始化时调用一次
{
    HAL_UART_Receive_IT(&huart2, &usart2_rx_byte, 1);
}
bool USART2_Write_Byte(uint8_t data)
{
    return (HAL_UART_Transmit(&huart2, &data, 1, 100) == HAL_OK);//发送一个字节，超时100ms,阻塞等待
}
uint8_t USART2_Read_Byte(uint8_t *data, uint32_t timeout)
{
    if(data == NULL)
        return 0;
    TickType_t start = xTaskGetTickCount(); //获取当前时间戳
    while(Ring_Buffer_IsEmpty(&usart2_rx_rb))
    {
        if(xTaskGetTickCount() - start >= pdMS_TO_TICKS(timeout)) //超时判断，如果超时返回失败
        {
            return 0;
        }
        vTaskDelay(1); //延时1ms，避免占用CPU资源
    }
    return Ring_Buffer_Read(&usart2_rx_rb, data); //从接收环形缓冲区读取一个字节
}

//接收中断服务函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart == &huart2)
    {
        if(!Ring_Buffer_Write(&usart2_rx_rb, usart2_rx_byte))//中断内部，向环形缓冲区写入一个字节
        {
            usart2_rx_overflow_count++;
        }
        USART2_Start_Receive();
    }
}

/* Called by the newlib _write implementation in syscalls.c. */
int __io_putchar(int ch)
{
    uint8_t data = (uint8_t)ch;

    if (HAL_UART_Transmit(&huart1, &data, 1, 100) != HAL_OK)
        return EOF;

    return (int)data;
}