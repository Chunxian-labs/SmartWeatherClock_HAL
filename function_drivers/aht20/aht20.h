#ifndef AHT20_H
#define AHT20_H
#include <stdint.h>
uint8_t AHT20_Init(void);
uint8_t AHT20_Measurement_Start(void);

uint8_t AHT20_Measurement_Delay(void);

uint8_t AHT20_GetMeasuredData(float *temperature, float *humidity);

#endif