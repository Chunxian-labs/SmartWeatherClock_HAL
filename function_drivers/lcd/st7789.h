#ifndef ST7789_H
#define ST7789_H
#include <stdint.h>
#include "font.h"
#include "image.h"
#define ST7789_WIDTH 240
#define ST7789_HEIGHT 320 
uint8_t ST7789_Init(void);

uint8_t ST7789_FillColor(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color);

uint8_t ST7789_DrawChar(uint16_t x, uint16_t y, char ch, 
                        const font_t *font, uint16_t color, uint16_t bgcolor);

uint8_t ST7789_DrawString(uint16_t x, uint16_t y, char *str, 
                            const font_t *font, uint16_t color, uint16_t bgcolor);


uint8_t ST7789_DrawImage(uint16_t x, uint16_t y, const image_t *image);

#endif