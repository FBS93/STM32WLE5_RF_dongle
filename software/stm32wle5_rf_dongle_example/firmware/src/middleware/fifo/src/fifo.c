#include <stdint.h>
#include <stdbool.h>

#include "fifo.h"

// Initialize the FIFO buffer
void fifo_init(s_fifo_t *ps_fifo)
{
  ps_fifo->u16_head = 0;
  ps_fifo->u16_tail = 0;
  ps_fifo->u16_count = 0;
}

// Check if the FIFO buffer is empty
bool fifo_isEmpty(s_fifo_t *ps_fifo)
{
  return (ps_fifo->u16_count == 0);
}

// Check if the FIFO buffer is full
bool fifo_isFull(s_fifo_t *ps_fifo)
{
  return (ps_fifo->u16_count == FIFO_SIZE);
}

// Check how much space is available in the FIFO buffer
uint16_t fifo_getAvailableSpace(s_fifo_t *ps_fifo)
{
  return (FIFO_SIZE - ps_fifo->u16_count);
}

uint16_t fifo_getOccupiedSpace(s_fifo_t *ps_fifo)
{
  return ps_fifo->u16_count;
}

// Push data into the FIFO buffer
void fifo_push(s_fifo_t *ps_fifo, uint16_t u16_len, uint8_t *pu8_data)
{
  if (fifo_getAvailableSpace(ps_fifo) >= u16_len)
  {

    for (uint16_t u16_i = 0; u16_i < u16_len; u16_i++)
    {
      ps_fifo->pu8_buffer[ps_fifo->u16_head] = pu8_data[u16_i];
      ps_fifo->u16_head = (ps_fifo->u16_head + 1) % FIFO_SIZE;
    }

    ps_fifo->u16_count += u16_len;
  }
}

// Pop data from the FIFO buffer
void fifo_pop(s_fifo_t *ps_fifo, uint16_t u16_len, uint8_t *pu8_data)
{

  if (ps_fifo->u16_count >= u16_len)
  {

    for (uint16_t u16_i = 0; u16_i < u16_len; u16_i++)
    {
      pu8_data[u16_i] = ps_fifo->pu8_buffer[ps_fifo->u16_tail];
      ps_fifo->u16_tail = (ps_fifo->u16_tail + 1) % FIFO_SIZE;
    }

    ps_fifo->u16_count -= u16_len;
  }
}
