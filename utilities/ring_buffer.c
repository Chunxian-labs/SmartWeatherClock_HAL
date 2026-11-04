#include "ring_buffer.h"
#include <stddef.h>
uint8_t Ring_Buffer_Init(ring_buffer_t *rb)
{
    if(rb == NULL)
    {
        return 0;
    }
    rb->head = 0;
    rb->tail = 0;
    return 1;
}

bool Ring_Buffer_IsEmpty(const ring_buffer_t *rb)
{
    if(rb == NULL)
        return false;
    return (rb->head == rb->tail);
}

bool Ring_Buffer_IsFull(const ring_buffer_t *rb)
{
    if(rb == NULL)
        return false;
    return (rb->tail == (rb->head+1) % RING_BUFFER_SIZE);
}

uint8_t Ring_Buffer_Write(ring_buffer_t *rb ,uint8_t data)
{
    if(rb == NULL)
        return 0;
    if(Ring_Buffer_IsFull(rb))
        return 0;
    rb->data[rb->head] = data;
    if(rb->head == RING_BUFFER_SIZE)
    {
        rb->head = 0;
    }
    return 1;
}

uint8_t Ring_Buffer_Read(ring_buffer_t *rb ,uint8_t *data)
{
    if(rb == NULL || data == NULL)
    {
        return 0;
    }
    if(Ring_Buffer_IsEmpty(rb))
    {
        return 0;
    }
    *data = rb->data[rb->tail];
    if(rb->tail == RING_BUFFER_SIZE)
    {
        rb->tail = 0;
    }
    return 1;
}

void Ring_Buffer_Clear(ring_buffer_t *rb)
{
    if(rb == NULL)
        return;
    rb->head = 0;
    rb->tail = 0;
}