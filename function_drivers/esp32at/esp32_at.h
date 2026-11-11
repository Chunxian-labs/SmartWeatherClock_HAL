#ifndef ESP32_AT_H
#define ESP32_AT_H
#include <stdint.h>
#include <stdbool.h>
#define ARRAY_SIZE(arr)  (sizeof(arr) / sizeof(arr[0]))  //获取数组元素个数
#define ESP_AT_DEBUG     0  //是否开启调试打印

typedef enum
{
    AT_ACK_NONE = 0,
    AT_ACK_OK,
    AT_ACK_ERROR,
    AT_ACK_BUSY,
    AT_ACK_READY,
}AT_ACK_t;             //AT指令应答状态
typedef struct
{
    AT_ACK_t ack;       //ack为枚举类型，用于存储AT指令应答状态
    const char *string; //string为字符串指针，用于存储AT指令应答字符串
}AT_ACK_MATCH_t;       //AT指令应答匹配结构体类型
typedef struct
{
    char ssid[64];  //wifi名称
    char bssid[18]; //MAC地址
    int channel;    //信道
    int rssi;       //信号强度
    bool isConnected; //是否已连接
}ESP32_WIFI_Info_t;
typedef struct
{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t weekday;
}ESP32_SNTP_Time_t;
bool ESP32_AT_Init(void);

/*WIFI 模块 */
bool ESP32_AT_Wifi_Init(void);
bool ESP32_AT_Wifi_Connect(const char *ssid, const char *password, const char *mac);
bool ESP_WIFI_GetInfo(ESP32_WIFI_Info_t *wifi_info);
bool ESP_WIFI_IsConnected(void);
/* 时间模块 - SNTP简单网络时间协议 */
bool ESP_SNTP_Init(void);
bool ESP_Get_RealTime(ESP32_SNTP_Time_t *TIME);
//HTTP访问
const char *ESP_HTTP_GetResponce(const char *url);

#endif