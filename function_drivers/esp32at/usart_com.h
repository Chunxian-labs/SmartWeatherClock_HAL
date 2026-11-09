#ifndef USART_COM_H
#define USART_COM_H
#include <stdbool.h>
void USART2_Initcom(void);

bool USART2_Write_Byte(uint8_t data);

uint8_t USART2_Read_Byte(uint8_t *data, uint32_t timeout);

void USART2_Clear_Receive_Buffer(void);
#endif