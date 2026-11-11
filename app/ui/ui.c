#include "ui.h"
#include "st7789.h"

#include "font.h"
#include "image.h"

#include <stdbool.h>
#include <stdio.h>
static uint16_t bg_color_black = fill_color(0,0,0);

static uint16_t bg_color_time = fill_color(248,248,248);    //时间显示底色
static uint16_t bg_color_indoor = fill_color(136,217,234);  //室内显示底色
static uint16_t bg_color_outdoor = fill_color(253,238,195); //室外显示底色

// //页面进程
// void WelcomePage_Display(void);
// void ErrorPage_Display(void);
// void WIFIConnectingPage_Display(void);
// //主页面初始化
// void MainPage_Display_Init(void);
// //室内室外温度、天气显示
// void MainPage_Indoor_Refresh(float temperature, float humidity);
// void MainPage_Outdoor_Refresh(float temperature, const int weather_code);
// //WIFI状态显示
// void MainPage_WIFI_Refresh(bool state);
// //主页面时间、日期更新
// void MainPage_RTC_TimeRefresh(RTC_Time_t *time);
// void MainPage_RTC_DateRefresh(RTC_Time_t *time);
void WelcomePage_Display(void)
{
    ST7789_FillColor(0,0,ST7789_WIDTH-1,ST7789_HEIGHT-1,bg_color_black);
    ST7789_DrawImage(0,0,&image_welcome_page);
}
void ErrorPage_Display(void)
{
    ST7789_FillColor(0,0,ST7789_WIDTH-1,ST7789_HEIGHT-1,bg_color_black);
    ST7789_DrawImage(52,95,&image_error_page_pic);
    ST7789_DrawString(35,240,"WiFi Error",&font32_maple_bold,fill_color(255,0,0),fill_color(255,255,255));
}

void WIFIConnectingPage_Display(void)
{
    ST7789_FillColor(0,0,ST7789_WIDTH-1,ST7789_HEIGHT-1,bg_color_black);
    ST7789_DrawString(60,36,"Welcome!",&font32_maple_bold,fill_color(0,255,255),bg_color_black);
    ST7789_DrawImage(52,95,&image_wifi);
    ST7789_DrawString(84,220,"WiFi",&font32_maple_bold,fill_color(0,255,255),bg_color_black);
    ST7789_DrawString(32,253,"Connecting...",&font32_maple_bold,fill_color(255,255,0),bg_color_black);
}

void MainPage_Display_Init(void)
{
    ST7789_FillColor(0,0,ST7789_WIDTH-1,ST7789_HEIGHT-1,bg_color_black);
    do//底色填充
    {
        ST7789_FillColor(15,15,224,154,bg_color_time);//填充上半部分底色
        ST7789_FillColor(15,165,114,304,bg_color_indoor);//填充左下部分底色
        ST7789_FillColor(125,165,224,304,bg_color_outdoor);//填充右下部分底色
    } while (0);
    
    do//WiFi信息显示
    {
        ST7789_DrawImage(23,20,&gImage_wifi_pic);//显示wifi图标
        ST7789_DrawString(100,20,"[iPhone 17 Pro]",&font16_maple,fill_color(0,0,0),bg_color_time);//WiFi名称显示
    }while(0);

    do//时间部分显示
    {
        ST7789_DrawString(25,50,"--:--",&font76_maple_extrabold,fill_color(0,0,0),bg_color_time);//时间显示
        ST7789_DrawString(35,125,"----/--/-- ---",&font24_maple_bold,fill_color(0,0,0),bg_color_time);//日期显示
    }while(0);
    
    do//室内环境显示
    {
        
        ST7789_DrawString(17,167,"Indoor",&font24_maple_bold,fill_color(0,0,0),bg_color_indoor);//室内显示
        ST7789_DrawString(17,207,"T:",&font24_maple_bold,fill_color(0,0,0),bg_color_indoor);//温度显示
        ST7789_DrawString(41,207,"--.--",&font24_maple_bold,fill_color(0,0,0),bg_color_indoor);//温度显示
        ST7789_DrawString(101,207,"C",&font24_maple_bold,fill_color(0,0,0),bg_color_indoor);//温度显示

        ST7789_DrawString(17,247,"H:",&font24_maple_bold,fill_color(0,0,0),bg_color_indoor);//湿度显示
        ST7789_DrawString(41,247,"--.--",&font24_maple_bold,fill_color(0,0,0),bg_color_indoor);//湿度显示
        ST7789_DrawString(101,247,"%",&font24_maple_bold,fill_color(0,0,0),bg_color_indoor);//湿度显示
    } while (0);
    
    do//室外环境显示
    {
        
        ST7789_DrawString(127,167,"Outdoor",&font24_maple_bold,fill_color(0,0,0),bg_color_outdoor);//室外显示
        ST7789_DrawString(127,207,"T:",&font24_maple_bold,fill_color(0,0,0),bg_color_outdoor);//温度显示
        ST7789_DrawString(151,207,"--.--",&font24_maple_bold,fill_color(0,0,0),bg_color_outdoor);//温度显示
        ST7789_DrawString(211,207,"C",&font24_maple_bold,fill_color(0,0,0),bg_color_outdoor);//温度显示

        ST7789_DrawImage(139,239,&gImage_thermometer_pic);//显示温度计图标
        ST7789_DrawImage(166,240,&gImage_sunny_pic);//显示天气图标
    }while(0);

}
void MainPage_WIFI_Refresh(bool state)
{
    if(state == true)
    {
        ST7789_DrawImage(23,20,&gImage_wifi_pic);//显示wifi图标
        ST7789_DrawString(68,20,"    [iPhone 17 Pro]",&font16_maple,fill_color(0,0,0),bg_color_time);//WiFi名称显示
    }
    else
    {
        ST7789_DrawString(23,20,"  ",&font24_maple_bold,fill_color(0,0,0),bg_color_time);//WiFi名称显示
        ST7789_DrawString(68,20,"[WIFI Disconnected]",&font16_maple,fill_color(0,0,0),bg_color_time);//WiFi名称显示
    }
}
void MainPage_RTC_TimeRefresh(RTC_Time_t *time)
{
    char time_str[20];
    snprintf(time_str, sizeof(time_str), "%02u%c%02u",
            time->hour, (time->second%2 == 0) ? ':' : ' ', time->minute);
    ST7789_DrawString(25,50,time_str,&font76_maple_extrabold,fill_color(0,0,0),bg_color_time);//时间显示
}
void MainPage_RTC_DateRefresh(RTC_Time_t *time)
{
    static const char *weekday_str[8] = {"","Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
    uint8_t weekday = (time->weekday>=1 && time->weekday<=7) ? time->weekday : 0;

    char data_str[30];
    snprintf(data_str, sizeof(data_str), "%4u/%02u/%02u %s",
            time->year, time->month, time->day, weekday_str[weekday]);
    ST7789_DrawString(35,125,data_str,&font24_maple_bold,fill_color(0,0,0),bg_color_time);//日期显示
}
void MainPage_Indoor_Refresh(float temperature, float humidity)
{
    char tem_ptr[8] = {0};
    char hum_ptr[8] = {0};
    if(temperature > -10.0f && temperature < 100.0f)
    {
        snprintf(tem_ptr, sizeof(tem_ptr), "%.2f", temperature);
        ST7789_DrawString(41,207,tem_ptr,&font24_maple_bold,fill_color(0,0,0),bg_color_indoor);//温度显示
    }
    if(humidity > 0.0f && humidity < 100.0f)
    {
        snprintf(hum_ptr, sizeof(hum_ptr), "%.2f", humidity);
        ST7789_DrawString(41,247,hum_ptr,&font24_maple_bold,fill_color(0,0,0),bg_color_indoor);//湿度显示
    }
    else
    {
        ST7789_DrawString(41,247,"--.--",&font24_maple_bold,fill_color(0,0,0),bg_color_indoor);//错误显示
    }
}
void MainPage_Outdoor_Refresh(float temperature, const int weather_code)
{
    char tem_ptr[8] = {0};
    if(temperature > -30.0f && temperature < 100.0f)
    {
        snprintf(tem_ptr, sizeof(tem_ptr), "%.2f", temperature);
        ST7789_DrawString(151,207,tem_ptr,&font24_maple_bold,fill_color(0,0,0),bg_color_outdoor);//温度显示
    }
    else
    {
        ST7789_DrawString(151,207,"--.--",&font24_maple_bold,fill_color(0,0,0),bg_color_indoor);//温度显示
    }
    if(weather_code == -1)
    {
        ST7789_DrawString(166,240,"  ",&font54_maple_bold,fill_color(0,0,0),bg_color_outdoor);//天气图标显示
    }
    const image_t *weather_pic = NULL;
    if(weather_code == 0 || weather_code == 2 || weather_code == 38)
    {
        weather_pic = &gImage_sunny_pic;
    }
    else if(weather_code == 1 || weather_code == 3)
    {
        weather_pic = &gImage_moon_pic;
    }
    else if(weather_code == 32 || weather_code == 33)
    {
        weather_pic = &gImage_windy_pic;
    }
    else if(weather_code == 4 || weather_code == 9)
    {
        weather_pic = &gImage_overcast_pic;
    }
    else if(weather_code == 5 || weather_code == 6 || weather_code == 7 || weather_code == 8)
    {
        weather_pic = &gImage_cloudy_pic;
    }
    else if(weather_code == 10 || weather_code == 13 || weather_code == 14 || weather_code == 15 ||
            weather_code == 16 || weather_code == 17 || weather_code == 18 || weather_code == 19)
    {
        weather_pic = &gImage_rainy_pic;
    }
    else if(weather_code == 11)
    {
        weather_pic = &gImage_rainstorm_pic;
    }
    else if(weather_code == 20 || weather_code == 21 || weather_code == 22 ||
            weather_code == 23 || weather_code == 24 || weather_code == 25)
    {
        weather_pic = &gImage_snowy_pic;
    }
    else{
        weather_pic = &gImage_sunny_pic;
    }
    if(weather_pic != NULL)
    {
        ST7789_DrawImage(166,240,weather_pic);
    }
    else
    {
        ST7789_DrawString(166,240,"  ",&font54_maple_bold,fill_color(0,0,0),bg_color_outdoor);//天气图标显示
    }
}