#include "st7789_com.h"
#include "gpio.h"
#include "spi.h"
#include "FreeRTOS.h"
#include "stm32f4xx_hal_spi.h"
#include "task.h"
#include "semphr.h"
static SemaphoreHandle_t lcd_dma_semaphore = NULL;  // DMA 完成/错误的同步信号量（二值）
static SemaphoreHandle_t lcd_spi_mutex = NULL;      // SPI 互斥信号量
static volatile uint8_t lcd_dma_failed = 0;          // 错误标志：ISR 置 1，任务读；volatile 防止被优化
//位选信号
void ST7789_CS_High(void)
{
    HAL_GPIO_WritePin(LCD_CS_Pin_GPIO_Port, LCD_CS_Pin_Pin, GPIO_PIN_SET);
}
void ST7789_CS_Low(void)
{
    HAL_GPIO_WritePin(LCD_CS_Pin_GPIO_Port, LCD_CS_Pin_Pin, GPIO_PIN_RESET);
}
//数据/命令信号
void ST7789_DC_High(void)
{
    HAL_GPIO_WritePin(LCD_DC_Pin_GPIO_Port, LCD_DC_Pin_Pin, GPIO_PIN_SET);
}
void ST7789_DC_Low(void)
{
    HAL_GPIO_WritePin(LCD_DC_Pin_GPIO_Port, LCD_DC_Pin_Pin, GPIO_PIN_RESET);
}
//复位信号
void ST7789_Reset_High(void)
{
    HAL_GPIO_WritePin(LCD_RESET_Pin_GPIO_Port, LCD_RESET_Pin_Pin, GPIO_PIN_SET);
}
void ST7789_Reset_Low(void)
{
    HAL_GPIO_WritePin(LCD_RESET_Pin_GPIO_Port, LCD_RESET_Pin_Pin, GPIO_PIN_RESET);
}
//背光信号
void ST7789_BL_High(void)
{
    HAL_GPIO_WritePin(LCD_BL_Pin_GPIO_Port, LCD_BL_Pin_Pin, GPIO_PIN_SET);
}
void ST7789_BL_Low(void)
{
    HAL_GPIO_WritePin(LCD_BL_Pin_GPIO_Port, LCD_BL_Pin_Pin, GPIO_PIN_RESET);
}

uint8_t st7789_write_byte(uint8_t byte)
{
    if(HAL_SPI_Transmit(&hspi2, &byte, 1, 100) != HAL_OK)
    {
        return 0;
    }
    return 1;
}

uint8_t st7789_read_byte(uint8_t *byte)
{
    if(HAL_SPI_Receive(&hspi2, byte, 1, 100) != HAL_OK)
    {
        return 0;
    }
    return 1;
}

uint8_t st7789_dma_init(void)
{
    if(lcd_dma_semaphore == NULL)
    {
        lcd_dma_semaphore = xSemaphoreCreateBinary();  //创建二值信号量，初始为空
    }
    if(lcd_spi_mutex == NULL)
    {
        lcd_spi_mutex = xSemaphoreCreateMutex();  //创建互斥信号量
    }
    return (lcd_dma_semaphore != NULL) && (lcd_spi_mutex != NULL);
}
/* HAL SPI 发送完成回调：由 HAL_DMA_IRQHandler 在 TC 中断里调用 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if(hspi == &hspi2 && lcd_dma_semaphore != NULL)
    {
        BaseType_t task_woken = pdFALSE;
        xSemaphoreGiveFromISR(lcd_dma_semaphore, &task_woken);  //释放信号量
        portYIELD_FROM_ISR(task_woken);  //如果需要切换任务，则进行
    }
}
/* HAL SPI 错误回调：由 HAL 在 TE/DME/FE 等错误中断里调用 */
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi)
{
    if(hspi == &hspi2 && lcd_dma_semaphore != NULL)
    {
        BaseType_t task_woken = pdFALSE;
        lcd_dma_failed = 1;  //设置错误标志
        xSemaphoreGiveFromISR(lcd_dma_semaphore, &task_woken);  //释放信号量
        portYIELD_FROM_ISR(task_woken);  //如果需要切换任务，则进行
    }
}

uint8_t st7789_dma_transmit(const uint16_t *DataAddr, uint32_t Size, uint8_t memory_increment)
{
    uint8_t result = 0;
    //DMA 按半字(16bit)搬运，源地址必须 2 字节对齐
    if(DataAddr == NULL || Size == 0 || ((uintptr_t)DataAddr & 0x01) != 0 || lcd_dma_semaphore == NULL)
    {
        return 0;  //参数错误
    }
    /* ---------- 运行环境检查：只能在普通任务上下文调用 ---------- */
    if (__get_IPSR() != 0U ||                       // 当前在中断服务程序里 → 不允许（会阻塞等信号量）
        __get_PRIMASK() != 0U ||                    // 全局中断被关（PRIMASK=1）→ 不允许
        __get_BASEPRI() != 0U ||                    // BASEPRI 屏蔽了部分中断 → 不允许
        xTaskGetSchedulerState() != taskSCHEDULER_RUNNING) // 调度器没运行 → 不能用阻塞 API
    {
        return 0;                                   // 环境不满足，返回失败
    }
    if(xSemaphoreTake(lcd_spi_mutex, pdMS_TO_TICKS(1000)) != pdTRUE)
    {
        return 0;  //获取互斥信号量失败
    }
    if(HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY)
    {
        xSemaphoreGive(lcd_spi_mutex);
        return 0;  //SPI 总线忙
    }

    /* 调用前必须保证上一次 SPI 传输已经结束。 */
    __HAL_SPI_DISABLE(&hspi2);  //禁用 SPI，配置 DMA
    hspi2.Init.DataSize = SPI_DATASIZE_16BIT;  //数据宽度 16bit
    SET_BIT(hspi2.Instance->CR1, SPI_CR1_DFF);  //设置数据宽度为 16bit
    /* 此时DMA尚未启动， 可以修改地址递增方式 */
    hspi2.hdmatx->Init.MemInc = memory_increment ? DMA_MINC_ENABLE : DMA_MINC_DISABLE;  //内存地址递增模式

    if(HAL_DMA_Init(hspi2.hdmatx) != HAL_OK)
    {
        goto cleanup;  //DMA 初始化失败
    }
    while(Size > 0)
    {
        uint16_t chunk_size = (Size > 65535) ? 65535 : Size;  //每次传输最大 65535 个半字

        (void)xSemaphoreTake(lcd_dma_semaphore, 0);  //清空信号量，防止上次残留
        lcd_dma_failed = 0;  //清除错误标志

        if(HAL_SPI_Transmit_DMA(&hspi2, (uint8_t *)DataAddr, chunk_size) != HAL_OK)
        {
            goto abort_transfer;  //DMA 启动失败
        }
        // 完成→HAL_SPI_TxCpltCallback Give；
        // 出错→HAL_SPI_ErrorCallback 置 failed 后 Give
        BaseType_t completed = xSemaphoreTake(lcd_dma_semaphore, pdMS_TO_TICKS(500));  //等待 DMA 完成或错误
        if(completed == pdFALSE || lcd_dma_failed != 0)
        {
            goto abort_transfer;  //等待超时或发生错误
        }
        Size -= chunk_size;
        if(memory_increment)
        {
            DataAddr += chunk_size;  //递增地址
        }
    }
    result = 1;  //传输成功
    goto cleanup;

abort_transfer:
    /*
     * 使用同步 Abort，返回前停止 DMA，
     * 避免 DMA 继续读取调用者的局部变量。
     */
    (void)HAL_SPI_Abort(&hspi2);
cleanup:
    /* ---------- 恢复 8bit 数据宽度，回到默认状态 ---------- */
    __HAL_SPI_DISABLE(&hspi2);                      // 改 DFF 前必须先关 SPI
    CLEAR_BIT(hspi2.Instance->CR1, SPI_CR1_DFF);    // DFF=0，恢复 8bit
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;        // 同步 HAL 句柄记录

    (void)xSemaphoreTake(lcd_dma_semaphore, 0);           // 清除可能残留的 Give，保证下次调用从干净状态开始

    xSemaphoreGive(lcd_spi_mutex);       // 释放互斥信号量
    return result;                                  // 0=失败，1=成功
}