#include "st7789.h"
#include "st7789_com.h"
#include "FreeRTOS.h"
#include "task.h"

#define MINUM_FONT_WIDTH    8
#define MINUM_FONT_HEIGHT   16
#define MAXMUN_FONT_WIDTH   38
#define MAXMUN_FONT_HEIGHT  76
static void ST7789_Reset(void)
{
    ST7789_Reset_Low();
    vTaskDelay(pdMS_TO_TICKS(1));//根据数据手册，延时时间需要大于10微秒
    ST7789_Reset_High();
    vTaskDelay(pdMS_TO_TICKS(120));//等待120ms之后才可以进行唤醒操作
}
static uint8_t ST7789_WriteRegister(uint8_t reg, uint8_t *data, uint8_t len)
{
    ST7789_CS_Low();//选中设备
    ST7789_DC_Low();//发送命令

    if(!st7789_write_byte(reg))
        return 0;
    ST7789_DC_High();//发送数据
    for(uint8_t i=0;i<len;i++)
    {
        if(!st7789_write_byte(data[i]))
        {
            ST7789_CS_High();//如果发送失败，释放设备选线
            return 0;
        }
    }
    ST7789_CS_High();//如果发送成功，释放设备选线
    return 1;
}
static uint8_t ST7789_RoomIsAvailable(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
    if(x1 >= ST7789_WIDTH || y1 >= ST7789_HEIGHT
    || x2 >= ST7789_WIDTH || y2 >= ST7789_HEIGHT)
    {
        return 0;
    }
    if(x1 > x2 || y1 > y2)
    {
        return 0;
    }
    return 1;
}
static uint8_t ST7789_GRAM_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2)
{
    if(!ST7789_WriteRegister(0x2A, (uint8_t[]){(x1>>8)&0xFF, x1&0xFF, (x2>>8)&0xFF, x2&0xFF}, 4))//行渲染
        return 0;
    if(!ST7789_WriteRegister(0x2B, (uint8_t[]){(y1>>8)&0xFF, y1&0xFF, (y2>>8)&0xFF, y2&0xFF}, 4))//列渲染
        return 0;
    if(!ST7789_WriteRegister(0x2C, NULL, 0))//写入数据使能
        return 0;
    return 1;
}
static uint8_t ST7789_Display_Init(void)
{
    ST7789_Reset();
    if(!ST7789_WriteRegister(0x11,NULL,0))
        return 0;
    vTaskDelay(pdMS_TO_TICKS(5));//在发送命令之前等待5ms
    if(!ST7789_WriteRegister(0x36, (uint8_t[]){0x00}, 1))
        return 0;
    if(!ST7789_WriteRegister(0x3A, (uint8_t[]){0x55}, 1))
        return 0;
    if(!ST7789_WriteRegister(0xB2,(uint8_t[]){0x0C,0x0C,0x00,0x33,0x33}, 5))
        return 0;
    if(!ST7789_WriteRegister(0xB7, (uint8_t[]){0x46}, 1))
        return 0;
    if(!ST7789_WriteRegister(0xBB, (uint8_t[]){0x1B}, 1))
        return 0;
    if(!ST7789_WriteRegister(0xC0, (uint8_t[]){0x2C}, 1))
        return 0;
    if(!ST7789_WriteRegister(0xC2, (uint8_t[]){0x01}, 1))
        return 0;
    if(!ST7789_WriteRegister(0xC4, (uint8_t[]){0x20}, 1))
        return 0;
    if(!ST7789_WriteRegister(0xC6, (uint8_t[]){0x0F}, 1))
        return 0;
    if(!ST7789_WriteRegister(0xD0, (uint8_t[]){0xA4,0xA1}, 2))
        return 0;
    if(!ST7789_WriteRegister(0xD6, (uint8_t[]){0xA1}, 1))
        return 0;
    if(!ST7789_WriteRegister(0xE0,
        (uint8_t[]){
            0xF0,0x00,0x06,0x04,0x05,0x05,0x31,
            0x44,0x48,0x36,0x12,0x12,0x2B,0x34
        }, 14))
        return 0;
    if(!ST7789_WriteRegister(0xE1,
        (uint8_t[]){
            0xF0,0x0B,0x0F,0x0F,0x0D,0x26,0x31,
            0x43,0x47,0x38,0x14,0x14,0x2C,0x32
        }, 14))
        return 0;
    if(!ST7789_WriteRegister(0x21, NULL, 0))
        return 0;
    if(!ST7789_WriteRegister(0x29, NULL, 0))
        return 0;

    ST7789_BL_High();//打开背光
    return 1;
}
static const uint8_t *ASCII_Get_Model(const char ch, const font_t *font)
{
    if(!ch || !font->ascii_model) //参数检查，如果字符为0或者字体模型为空，则返回空指针
    {
        return NULL;
    }
    uint16_t bytes_per_row = (font->size/2+7)/8; //计算每行占用的字节数,向上取整
    uint16_t bytes_per_char = bytes_per_row * font->size; //计算每个字符占用的总字节数

    if(font->ascii_map != NULL)//如果存在自定义映射表
    {
        const char *map_pointer = font->ascii_map;//获取映射表指针,指向映射表首字符
        while(*map_pointer != '\0')//遍历映射表，直到遇到结束符'\0'
        {
            if(*map_pointer == ch)//如果当前映射字符与目标字符匹配
            {
                uint16_t index = map_pointer - font->ascii_map; //计算索引位置
                return font->ascii_model + index * bytes_per_char; //返回对应的字模数据指针
            }
            map_pointer++; //移动到下一个映射字符
        }
        return font->ascii_model; //如果没有找到匹配的字符，返回默认字模数据指针
    }
    return font -> ascii_model + (ch - font->first_char) * bytes_per_char; 
    //如果没有自定义映射表，直接根据字符的ASCII值计算偏移量并返回字模数据指针
}
static uint8_t ST7789_Draw_Font(uint16_t x, uint16_t y,
                                uint16_t fwidth, uint16_t fheight,
                                const uint8_t *font_model, uint16_t color, 
                                uint16_t bgcolor)
{
    if(font_model == NULL)
        return 0; //如果字模数据为空，返回失败
    if(fwidth < MINUM_FONT_WIDTH || fwidth > MAXMUN_FONT_WIDTH || fheight < MINUM_FONT_HEIGHT || fheight > MAXMUN_FONT_HEIGHT)
        return 0; //如果字体宽度或高度超出范围，返回失败
    if(!ST7789_RoomIsAvailable(x, y, x + fwidth - 1, y + fheight - 1))
        return 0; //如果绘制区域超出屏幕范围，返回失败
    static uint16_t buffer[MAXMUN_FONT_HEIGHT * MAXMUN_FONT_WIDTH]; //定义一个静态缓冲区，用于存储字体渲染的像素数据
    uint16_t *buf_ptr = buffer; //定义一个指针，指向缓冲区的起始位置
    uint16_t bytes_per_row = (fwidth + 7) / 8; //计算每行占用的字节数，向上取整
    for(uint16_t row = 0; row < fheight; row++)
    {
        const uint8_t *row_data = font_model + row * bytes_per_row; //获取当前行的字模数据指针
        for(uint16_t col = 0; col < fwidth; col++)
        {
            uint8_t byte_index = col / 8; //计算当前列所在的字节索引
            uint8_t byte = row_data[byte_index]; //获取当前字节的值
            uint8_t bit_index = 7 - (col % 8); //计算当前列在字节中的位索引
            uint16_t pixel_color = 
                (byte & (1<<bit_index)) ? color : bgcolor; //根据字模数据决定像素颜色
            *buf_ptr++ = pixel_color; //将像素颜色写入缓冲区
        }
    }
    if(!ST7789_GRAM_Set(x, y, x + fwidth - 1, y + fheight - 1)) //设置GRAM区域
        return 0; //如果设置失败，返回失败
    ST7789_CS_Low();//选中设备
    ST7789_DC_High();//发送数据
    if(!st7789_dma_transmit(buffer, buf_ptr-buffer, 1)) //使用DMA传输缓冲区数据
    {
        ST7789_CS_High();//如果发送失败，释放设备选线
        return 0; //返回失败
    }
    ST7789_CS_High();//如果发送成功，释放设备选线
    return 1; //返回成功
}
static int UTF8_Char_Length(const char *str)
{
    if(str == NULL || *str == '\0') // 检查输入字符串是否为空或为空字符
        return -1; // 返回 -1 表示无效输入
    if((str[0] & 0x80) == 0x00)         // 0xxxxxxx, ASCII字符
        return 1;
    else if((str[0] & 0xE0) == 0xC0)    // 110xxxxx, 2字节UTF-8字符
        return 2;   
    else if((str[0] & 0xF0) == 0xE0)    // 1110xxxx, 3字节UTF-8字符
        return 3;
    else if((str[0] & 0xF8) == 0xF0)    // 11110xxx, 4字节UTF-8字符
        return 4;
    else
        return -1; // 非法的UTF-8编码
}
uint8_t ST7789_Init(void)
{
    if(!st7789_dma_init())
        return 0;
    if(!ST7789_Display_Init())
        return 0;
    return 1;
}
uint8_t ST7789_FillColor(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color)
{
    if(!ST7789_RoomIsAvailable(x1,y1,x2,y2))  //判断坐标是否有效
        return 0;
    if(!ST7789_GRAM_Set(x1,y1,x2,y2))  //渲染准备
        return 0;
    
    ST7789_CS_Low();//选中设备
    ST7789_DC_High();//发送数据
    uint32_t total_pixels = (x2-x1+1)*(y2-y1+1); //计算出数据总量
    if(!st7789_dma_transmit(&color, total_pixels, 0)) //使用DMA传输数据
    {
        ST7789_CS_High();//如果发送失败，释放设备选线
        return 0;
    }
    ST7789_CS_High();//如果发送成功，释放设备选线
    return 1;
}
uint8_t ST7789_DrawChar(uint16_t x, uint16_t y, char ch, 
                        const font_t *font, uint16_t color, uint16_t bgcolor)
{
    if(ch < 0x20 || ch > 0x7E) //检查字符是否在可显示范围内
        return 0; //如果不在范围内，返回失败
    if(font == NULL || font->ascii_model == NULL) //检查字体是否有效
        return 0; //如果字体无效，返回失败
    uint16_t fwidth = font->size / 2; //计算字体宽度
    uint16_t fheight = font->size; //计算字体高度
    if(!ST7789_RoomIsAvailable(x, y, x + fwidth - 1, y + fheight - 1)) //检查绘制区域是否有效
        return 0; //如果绘制区域无效，返回失败
    const uint8_t *font_model = ASCII_Get_Model(ch, font); //获取字符的字模数据
    if(font_model == NULL) //检查字模数据是否有效
        return 0; //如果字模数据无效，返回失败
    if(!ST7789_Draw_Font(x, y, fwidth, fheight, font_model, color, bgcolor)) //绘制字符
        return 0; //如果绘制失败，返回失败
    return 1; //返回成功
}
uint8_t ST7789_DrawString(uint16_t x, uint16_t y, char *str, 
                            const font_t *font, uint16_t color, uint16_t bgcolor)
{
    while(*str != '\0') //遍历字符串直到遇到结束符
    {
        int char_len = UTF8_Char_Length(str); //获取当前字符的UTF-8长度
        if(char_len == -1) //检查字符长度是否有效
            return 0; //如果无效，返回失败
        if(char_len == 1) //如果是ASCII字符
        {
            if(!ST7789_DrawChar(x, y, *str, font, color, bgcolor)) //绘制ASCII字符
                return 0; //如果绘制失败，返回失败
            str++;
            x += font->size / 2; //更新x坐标，移动到下一个字符位置
        }
        else //如果是多字节UTF-8字符
        {
            //这里可以添加对多字节UTF-8字符的处理逻辑，例如调用ST7789_DrawUnicode函数
            //目前暂不支持多字节UTF-8字符的绘制，直接返回失败
            return 0; 
        }
    }
    return 1; //返回成功
}

uint8_t ST7789_DrawImage(uint16_t x, uint16_t y, const image_t *image)
{
    if(x >= ST7789_WIDTH || y >= ST7789_HEIGHT)
        return 0; //如果起始坐标超出屏幕范围，返回失败
    if(image == NULL || image->image_data == NULL)
        return 0; //如果图像数据为空，返回失败
    uint32_t img_pixels = image->width * image->height; //计算图像总像素数
    if(!ST7789_RoomIsAvailable(x, y, x + image->width - 1, y + image->height - 1))
        return 0; //如果图像区域超出屏幕范围，返回失败
    if(!ST7789_GRAM_Set(x, y, x + image->width - 1, y + image->height - 1))
        return 0; //如果设置GRAM区域失败，返回失败
    ST7789_CS_Low();//选中设备
    ST7789_DC_High();//发送数据
    if(!st7789_dma_transmit((const uint16_t *)image->image_data, img_pixels, 1)) //使用DMA传输图像数据
    {
        ST7789_CS_High();//如果发送失败，释放设备选线
        return 0;
    }
    ST7789_CS_High();//如果发送成功，释放设备选线
    return 1; //返回成功
}