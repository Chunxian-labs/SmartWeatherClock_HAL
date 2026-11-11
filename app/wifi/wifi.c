#include "wifi.h"
#include "ui.h"
#include "esp32_at.h"
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"

void WIFI_Init(void)
{
    if(!ESP32_AT_Init())
	{
		printf("[AT]ESP32_AT_Init failed\r\n");
		goto err;
	}
	printf("[AT]ESP32_AT_Init success\r\n");
	if(!ESP32_AT_Wifi_Init())
	{
		printf("[WIFI]ESP_Wifi_Init failed\r\n");
		goto err;
	}
	printf("[WIFI]ESP_Wifi_Init success\r\n");
	return;
	
err:
	ErrorPage_Display();
	while(1)
	{
		printf("Error\r\n");
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
void WIFI_Connect(void)
{
	if(!ESP32_AT_Wifi_Connect("iPhone 17Pro","123456677",NULL))
	{
		printf("[WIFI]ESP_Wifi_Connect failed\r\n");
		goto err;
	}
	printf("[WIFI]ESP_Wifi_Connect success\r\n");
	if(!ESP_SNTP_Init())
	{
		printf("[SNTP]ESP_SNTP_Init failed\r\n");
		goto err;
	}
	printf("[SNTP]ESP_SNTP_Init success\r\n");

    return;

err:
	ErrorPage_Display();
	while(1)
	{
		printf("Error\r\n");
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}