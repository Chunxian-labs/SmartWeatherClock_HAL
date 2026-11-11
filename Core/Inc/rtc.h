/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.h
  * @brief   This file contains all the function prototypes for
  *          the rtc.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_H__
#define __RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdint.h>
/* USER CODE END Includes */

extern RTC_HandleTypeDef hrtc;

/* USER CODE BEGIN Private defines */
typedef struct
{
  uint16_t year;
  uint8_t month;
  uint8_t day;

  uint8_t hour;
  uint8_t minute;
  uint8_t second;

  uint8_t weekday;
}RTC_Time_t;
/* USER CODE END Private defines */

void MX_RTC_Init(void);

/* USER CODE BEGIN Prototypes */
/**
 * @brief 设置 RTC 日期和时间。
 *
 * @param time 待设置的时间，不能为空。
 *
 * @return HAL_OK：成功。
 *         HAL_ERROR：参数非法、尚未初始化或调用环境错误等。
 *         HAL_TIMEOUT：等待互斥锁或 HAL 硬件操作超时。
 *         HAL_BUSY：HAL 硬件操作返回忙状态。
 */
HAL_StatusTypeDef RTC_Set_Time(const RTC_Time_t *time);
/**
 * @brief 读取 RTC 日期和时间。
 *
 * @param time 接收结果的结构体，不能为空。
 *
 * @return HAL_OK 表示读取成功，其余状态表示失败。
 *
 * 只有返回 HAL_OK 时，输出结构体才会被更新。
 */
HAL_StatusTypeDef RTC_Get_Time(RTC_Time_t *time);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __RTC_H__ */

