#ifndef UI_H
#define UI_H
#include <stdbool.h>
#include "rtc.h"
//页面进程
void WelcomePage_Display(void);
void ErrorPage_Display(void);
void WIFIConnectingPage_Display(void);
//主页面初始化
void MainPage_Display_Init(void);
//室内室外温度、天气显示
void MainPage_Indoor_Refresh(float temperature, float humidity);
void MainPage_Outdoor_Refresh(float temperature, const int weather_code);
//WIFI状态显示
void MainPage_WIFI_Refresh(bool state);
//主页面时间、日期更新
void MainPage_RTC_TimeRefresh(RTC_Time_t *time);
void MainPage_RTC_DateRefresh(RTC_Time_t *time);
#endif