#ifndef AHT20_COM_H
#define AHT20_COM_H
#include <stdint.h>
uint8_t aht20_write_hal(uint8_t slave_address, uint8_t *data, uint8_t len);
uint8_t aht20_read_hal(uint8_t slave_address, uint8_t *data, uint8_t len);
#endif