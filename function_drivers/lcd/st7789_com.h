#ifndef ST7789_COM_H
#define ST7789_COM_H
#include <stdint.h>
//位选信号
void ST7789_CS_High(void);
void ST7789_CS_Low(void);
//数据/命令信号
void ST7789_DC_High(void);
void ST7789_DC_Low(void);
//复位信号
void ST7789_Reset_High(void);
void ST7789_Reset_Low(void);
//背光信号
void ST7789_BL_High(void);
void ST7789_BL_Low(void);

uint8_t st7789_write_byte(uint8_t byte);
uint8_t st7789_read_byte(uint8_t *byte);

uint8_t st7789_dma_init(void);
uint8_t st7789_dma_transmit(const uint16_t *DataAddr, uint32_t Size, uint8_t memory_increment);
#endif