#ifndef RING_BUFFER_H
#define RING_BUFFER_H
#define RING_BUFFER_SIZE 128
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t data[RING_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
}ring_buffer_t;
uint8_t Ring_Buffer_Init(ring_buffer_t *rb);

bool Ring_Buffer_IsEmpty(const ring_buffer_t *rb);

bool Ring_Buffer_IsFull(const ring_buffer_t *rb);

uint8_t Ring_Buffer_Write(ring_buffer_t *rb ,uint8_t data);

uint8_t Ring_Buffer_Read(ring_buffer_t *rb ,uint8_t *data);

void Ring_Buffer_Clear(ring_buffer_t *rb);

#endif
