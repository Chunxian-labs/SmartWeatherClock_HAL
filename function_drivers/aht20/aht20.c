#include "aht20.h"
#include "aht20_com.h"
#include "FreeRTOS.h"
#include "task.h"
#define CHECK_STATUS_CMD     0X71
#define SLAVE_ADDR_7BIT      0X38

static uint8_t Initialization_Cmd[3] = {0XBE, 0X08, 0X00};
static uint8_t Measure_Cmd[3] = {0XAC, 0X33, 0X00};
static uint8_t aht20_isready(void);
static uint8_t aht20_read_status(uint8_t *status);

uint8_t AHT20_Init(void)
{
    vTaskDelay(pdMS_TO_TICKS(40));//上电等待40ms
    if(!aht20_isready()) 
    {
        if(!aht20_write_hal(SLAVE_ADDR_7BIT, Initialization_Cmd, 3))
            return 0;
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    return aht20_isready();
}
uint8_t AHT20_Measurement_Start(void)
{
    return aht20_write_hal(SLAVE_ADDR_7BIT, Measure_Cmd, 3);
}
uint8_t AHT20_Measurement_Delay(void)
{
    vTaskDelay(pdMS_TO_TICKS(80));
    for(uint8_t t=0;t<20;t++)
    {
        uint8_t status;
        if(!aht20_read_status(&status))
            return 0;
        if((status & 0x80) == 0)
            return 1;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    return 0;
}
uint8_t AHT20_GetMeasuredData(float *temperature, float *humidity)
{
    uint8_t data[6] = {0};
    if(temperature == NULL || humidity == NULL)
        return 0;
    if(!AHT20_Measurement_Delay())
        return 0;
    if(!aht20_read_hal(SLAVE_ADDR_7BIT, data, 6))
        return 0;
    if((data[0] & 0x88) != 0x08)
        return 0;
    //计算温度和湿度
    uint32_t raw_humidity = (((uint32_t)(data[1] << 12)) |
                            (uint32_t)(data[2] << 4) |
                            (uint32_t)((data[3] & 0xF0) >> 4));
    uint32_t raw_temperature = ((uint32_t)((data[3] & 0x0F) << 16) | 
                                (uint32_t)data[4] << 8 |
                                (uint32_t)data[5]);

    *humidity = (float)raw_humidity*100.0f/1024.0f/1024.0f;
    *temperature = (float)raw_temperature*200.0f/1024.0f/1024.0f-50.0f; 

    return 1;
}
static uint8_t aht20_read_status(uint8_t *status)
{
    uint8_t cmd = CHECK_STATUS_CMD;
    if(!aht20_write_hal(SLAVE_ADDR_7BIT, &cmd, 1)) 
        return 0;
    if(!aht20_read_hal(SLAVE_ADDR_7BIT, status, 1)) 
        return 0;

    return 1;
}
static uint8_t aht20_isready(void)
{
    uint8_t status;
    if(!aht20_read_status(&status)) 
        return 0;
    //如果第4位为1，说明就绪
    return (status & 0X08) == 0x08;
}
