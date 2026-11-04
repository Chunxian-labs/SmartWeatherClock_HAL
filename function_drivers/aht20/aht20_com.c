#include "aht20_com.h"
#include "i2c.h"

uint8_t aht20_write_hal(uint8_t slave_address, uint8_t *data, uint8_t len)
{
    if(data == NULL || len == 0) return 0;

    uint16_t dev_addr = slave_address << 1;
    if(HAL_I2C_Master_Transmit(&hi2c2,
                                dev_addr,
                                data,
                                len,
                                100) != HAL_OK)
                                {
                                    return 0;
                                }
    return 1;
}

uint8_t aht20_read_hal(uint8_t slave_address, uint8_t *data, uint8_t len)
{
    if(data == NULL || len == 0) return 0;

    uint16_t dev_addr = slave_address << 1;
    if(HAL_I2C_Master_Receive(&hi2c2,
                                dev_addr,
                                data,
                                len,
                                100) != HAL_OK)
                                {
                                    return 0;
                                }
    return 1;
}
