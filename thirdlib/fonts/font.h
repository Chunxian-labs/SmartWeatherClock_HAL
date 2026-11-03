#ifndef FONT_H
#define FONT_H
#include <stdint.h>
typedef struct
{
    uint8_t first_char;  // 字体的第一个字符
    uint16_t size;        // 字体大小
    const char *ascii_map;  // 字体的 ASCII 映射表
    const uint8_t *ascii_model; //字体数模指针
}font_t;

extern const font_t font16_maple;
extern const font_t font20_maple_bold;
extern const font_t font24_maple_bold;
extern const font_t font32_maple_bold;
extern const font_t font54_maple_bold;
extern const font_t font76_maple_extrabold;
#endif // FONT_H