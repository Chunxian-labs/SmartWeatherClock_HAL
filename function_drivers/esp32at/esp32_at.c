#include "esp32_at.h"
#include "usart_com.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
static const AT_ACK_MATCH_t at_ack_match_table[] =
{
    {AT_ACK_NONE, ""},
    {AT_ACK_OK, "OK\r\n"},
    {AT_ACK_ERROR, "ERROR\r\n"},
    {AT_ACK_BUSY, "busy p…\r\n"},
    {AT_ACK_READY, "ready\r\n"},
};  //AT指令应答匹配表
static char rxbuff[1024];//缓冲数组，用于接收ESP32的应答字符串
static uint32_t response_full_count = 0; //接收数组溢出计数器
static uint32_t at_timeout_count = 0;    //超时计数器
static uint32_t parse_fail_count = 0;    //解析失败计数器
/* 通信基础层 */
/* ======================================================================= */
//匹配ESP32的应答字符串，返回对应的枚举状态码
static AT_ACK_t AT_Match_ACK(const char *string)
{
    for(uint32_t i=0; i<ARRAY_SIZE(at_ack_match_table); i++)
    {
        if(strcmp(string, at_ack_match_table[i].string) == 0)
        {
            return at_ack_match_table[i].ack; //返回匹配到的枚举状态码
        }
    }
    return AT_ACK_NONE; //如果没有匹配到，返回AT_ACK_NONE枚举状态码
}
//主控芯片通过USART2发送数据到ESP32
static bool ESP_AT_USART_Write(const char *data)
{
    while(data != NULL && *data != '\0') //当指针不为空且字符串没有结束时
    {
        if(!USART2_Write_Byte((uint8_t)*data++))//持续按顺序发送字符，地址自增
            return false;
    }
    if(!USART2_Write_Byte('\r'))
        return false;
    if(!USART2_Write_Byte('\n'))
        return false;
    return true;
}
//等待ESP32的应答，返回对应的枚举状态码
static AT_ACK_t ESP_AT_GetResponse(uint32_t timeout)
{
    uint32_t rxlen = 0;  //已经接收的字节数
    const char *line = rxbuff; //line 指向"当前这一行"的起始位置
    rxbuff[0] = '\0'; //初始化缓冲数组为空字符串

    TickType_t start = xTaskGetTickCount();//获取当前的时间tick
    while(rxlen < sizeof(rxbuff)-1) // 留 1 字节给 '\0'，防止写满
    {
        TickType_t elapsed_ticks = xTaskGetTickCount() - start; //计算当前时间与开始时间的tick差值
        uint32_t elapsed_ms = elapsed_ticks * portTICK_PERIOD_MS; //将tick转换为毫秒
        if(elapsed_ms >= timeout) //如果超时
        {
            at_timeout_count++;
            return AT_ACK_NONE; //返回AT_ACK_NONE枚举状态码
        }
        uint32_t remaining_ms = timeout - elapsed_ms; //计算剩余时间,真正给串口接收的超时时间
        if(!USART2_Read_Byte((uint8_t *)&rxbuff[rxlen],remaining_ms))
        {
            at_timeout_count++;
            return AT_ACK_NONE; //返回AT_ACK_NONE枚举状态码
        }
        rxlen++; //接收一个字节，字节数加1
        if(rxbuff[rxlen-1] == '\n')
        {
            rxbuff[rxlen] = '\0';  //如果收到换行符，添加结束符，组成完整的字符串
            AT_ACK_t ACK = AT_Match_ACK(line); //匹配ESP32的应答字符串，返回对应的枚举状态码
            if(ACK != AT_ACK_NONE)
            {
                return ACK; //返回匹配到的枚举状态码
            }
            line = rxbuff + rxlen; //将line 指向"下一行"的起始位置
        }
    }
    response_full_count++;
    rxbuff[0] = '\0';
    return AT_ACK_NONE; //如果没有匹配到，返回AT_ACK_NONE枚举状态码
}
//发送AT指令
static bool ESP_Send_CMD(const char *cmd, uint32_t timeout)
{
#if ESP_AT_DEBUG
    printf("ESP_Send_CMD: %s\n", cmd);
#endif
    USART2_Clear_Receive_Buffer(); //在发送新命令之前先清空环形缓冲区
    if(!ESP_AT_USART_Write(cmd)) //发送AT指令
    {
        return false;
    }
    //执行到这一步说明发送成功
    AT_ACK_t ACK = ESP_AT_GetResponse(timeout); //等待ESP32的应答，返回对应的枚举状态码
#if ESP_AT_DEBUG
    printf("ESP_Response: %d\n", ACK);
#endif
    return ACK == AT_ACK_OK; //如果返回OK，返回true，否则返回false
}
//等待ESP32的引导完成
static bool ESP_AT_Wait_Boot(uint32_t timeout)
{
    for(uint32_t t=0;t<timeout;t+=100)
    {
        if(ESP_Send_CMD("AT",100)) //等待回复OK
        {
            return true; //如果在超时时间内返回了OK，返回true,引导完成
        }
    }
    return false; //如果没有在超时时间内返回OK，返回false,引导失败
}
//等待ESP32就绪
static bool ESP_Wait_Ready(uint32_t timeout)
{
    if(ESP_AT_GetResponse(timeout) == AT_ACK_READY)
    {
        return true;
    }
    return false;
}
static const char *ESP_GetResponseBuffer(void)
{
    return rxbuff;
}
/* 业务应用层 */
/* ======================================================================= */
//ESP32初始化
bool ESP32_AT_Init(void)
{
    if(!ESP_AT_Wait_Boot(3000)) 
        return false; //如果没有在超时时间内返回OK，返回false,引导失败
    if(!ESP_Send_CMD("AT",100)) //100ms
        return false; //如果没有在超时时间内返回OK，返回false,通信建立失败
    if(!ESP_Send_CMD("AT+RESTORE", 2000)) //2s
        return false; //如果没有在超时时间内返回OK，返回false,复位失败
    if(!ESP_Wait_Ready(5000))  //5s
        return false; //如果没有在超时时间内返回OK，返回false,就绪失败

    return true; //如果所有操作都成功，返回true,初始化成功
}

/*WIFI 模块 */
//WIFI初始化
bool ESP32_AT_Wifi_Init(void)
{
    return ESP_Send_CMD("AT+CWMODE=1", 2000); //将ESP32的Wi-Fi工作模式设置为Station站点模式,可连接外部wifi
}
//连接WiFi网络
//ssid：WiFi名称字符串指针，传入 WiFi 热点名字，例如 "MyHomeWiFi"
//password：WiFi密码字符串指针，例如 "12345678"
//mac：可选，指定要连接 AP 的 BSSID（MAC 地址），可以传NULL表示不指定 MAC，只按 ssid 连接
bool ESP32_AT_Wifi_Connect(const char *ssid, const char *password, const char *mac)
{
    if(ssid == NULL || password == NULL)
        return false; //WIFI名字，密码指针不能为空，为空直接返回失败
    char cmd[128]; //局部栈上字符缓冲区，用来拼装AT指令文本
    int len = snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    if(mac != NULL)
    {
        snprintf(cmd+len, sizeof(cmd) - len, ",\"%s\"", mac);
    }
    return ESP_Send_CMD(cmd, 5000); //发送WIFI连接请求，超时5s
}
//解析从 AT+CWSTATE? 指令的返回文本，WIFI状态，SSID，是否已连接
    // AT+CWSTATE?
    // +CWSTATE:2,"iPhone 17Pro"
    // OK
static bool Parse_CWSTATE_Responce(const char *responce,ESP32_WIFI_Info_t *wifi_info)
{
    if(wifi_info == NULL)
        return false; //WIFI信息指针不能为空，为空直接返回失败
    int WIFI_State;
    //找到"+CWSTATE:"字符串,并将其指向"+CWSTATE:"字符串的第一个字符
    responce = strstr(responce, "+CWSTATE:");
    if(responce == NULL)
        return false; //如果没有找到"+CWSTATE:",返回false,解析失败
    if(sscanf(responce,"%*[^:]:%d,\"%63[^\"]\"", &WIFI_State, wifi_info->ssid) != 2)
        return false;
    wifi_info->isConnected = (WIFI_State == 2);
    return true;
}
//解析从 AT+CWJAP? 指令的返回文本，WIFI的SSID，MAC, 信道，信号强度
    // AT+CWJAP?
    // +CWJAP:"iPhone 17Pro","ba:21:89:6f:b8:c4",2,-45,0,1,3,0,1
    // OK
static bool Parse_CWJAP_Responce(const char *responce,ESP32_WIFI_Info_t *wifi_info)
{
    if(wifi_info == NULL)
        return false; //WIFI信息指针不能为空，为空直接返回失败
    responce = strstr(responce, "+CWJAP:");
    if(responce == NULL)
        return false;
    if(sscanf(responce,
            "%*[^:]:\"%63[^\"]\",\"%18[^\"]\",%d,%d",
        wifi_info->ssid,
        wifi_info->bssid,
        &wifi_info->channel,
        &wifi_info->rssi) != 4)
        return false;
    return true;
}
//获取WIFI信息
bool ESP_WIFI_GetInfo(ESP32_WIFI_Info_t *wifi_info)
{
    if(wifi_info == NULL)
        return false; //WIFI信息指针不能为空，为空直接返回失败
    if(!ESP_Send_CMD("AT+CWSTATE?", 2000))//查询WIFI连接状态
        return false; //如果没有在超时时间内返回OK，返回false,获取失败
    if(!Parse_CWSTATE_Responce(ESP_GetResponseBuffer(), wifi_info))
    {
        parse_fail_count++;
        return false; //如果没有解析成功，返回false,解析失败
    }
    if(wifi_info->isConnected == true)
    {
        if(!ESP_Send_CMD("AT+CWJAP?", 2000))//如果WIFI连接成功，查询WIFI信息
            return false; //如果没有在超时时间内返回OK，返回false,获取失败
        if(!Parse_CWJAP_Responce(ESP_GetResponseBuffer(), wifi_info))
        {
            parse_fail_count++;
            return false; //如果没有解析成功，返回false,解析失败
        }
        return true;
    }
    return true;
}
//检查WIFI是否连接
bool ESP_WIFI_IsConnected(void)
{
    ESP32_WIFI_Info_t wifi_info;
    if(ESP_WIFI_GetInfo(&wifi_info))
    {
        return wifi_info.isConnected;
    }
    return false;
}
/* 时间模块 - SNTP简单网络时间协议 */
//SNTP初始化
bool ESP_SNTP_Init(void)
{
    if(!ESP_Send_CMD("AT+CIPSNTPCFG=1,8", 2000))
        return false; //如果没有在超时时间内返回OK，返回false,初始化失败
    return true; //如果初始化成功，返回true
}
//月份数字化
static uint8_t Month_To_Number(const char *month)
{
    if(month == NULL)
        return 0;
    const char* month_str[12] = 
        {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    for(uint8_t i = 0; i < 12; i++)
    {
        if(strcmp(month, month_str[i]) == 0)//如果输入的月份字符和数组中的月份字符相等
        {
            return i+1;
        }
    }
    return 0;
}
//日期数字化
static uint8_t Weekday_To_Number(const char *weekday)
{
    if(weekday == NULL)
        return 0;
    const char* weekday_str[7] =
        {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    for(uint8_t i = 0; i < 7; i++)
    {
        if(strcmp(weekday,weekday_str[i]) == 0)
        {
            return i+1;
        }
    }
    return 0;
}
//解析时间
    // AT+CIPSNTPTIME?
    // +CIPSNTPTIME:Sun Jul 27 14:07:19 2025
    // OK
static bool Parse_CIPSNTPCFG_Responce(const char *responce, ESP32_SNTP_Time_t *TIME)
{
    if(responce == NULL || TIME == NULL)
        return false;
    responce = strstr(responce, "+CIPSNTPTIME:");
    if(responce == NULL)
        return false;
    char weekday_str[4] = {0};
    char month_str[4] = {0};
    int day = 0, hour = 0, minute = 0, second = 0, year = 0;
    if(sscanf(responce, "+CIPSNTPTIME:%3s %3s %d %d:%d:%d %d",
        weekday_str, month_str, &day, &hour, &minute, &second, &year) != 7)
        return false;
    TIME->day       = (uint8_t)day;
    TIME->hour      = (uint8_t)hour;
    TIME->minute    = (uint8_t)minute;
    TIME->second    = (uint8_t)second;
    TIME->year      = (uint16_t)year;
    TIME->weekday   = Weekday_To_Number(weekday_str);
    TIME->month     = Month_To_Number(month_str);
    return true;
}
//获取时间
bool ESP_Get_RealTime(ESP32_SNTP_Time_t *TIME)
{
    if(TIME == NULL)
        return false;
    if(!ESP_Send_CMD("AT+CIPSNTPTIME?", 2000))
        return false;
    if(!Parse_CIPSNTPCFG_Responce(ESP_GetResponseBuffer(), TIME))
    {
        parse_fail_count++;
        return false;
    }
    return true;
}
//HTTP访问
const char *ESP_HTTP_GetResponce(const char *url)
{
    if(url == NULL)
        return NULL;
    static char txbuff[512];
    snprintf(txbuff, sizeof(txbuff),"AT+HTTPCLIENT=2,1,\"%s\",,,2",url);
    bool result = ESP_Send_CMD(txbuff, 5000);
    return result ? ESP_GetResponseBuffer() : NULL;
}