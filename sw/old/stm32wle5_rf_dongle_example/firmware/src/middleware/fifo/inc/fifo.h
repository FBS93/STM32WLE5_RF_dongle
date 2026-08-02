#ifndef FIFO_H_
#define FIFO_H_

#include <stdint.h>
#include <stdbool.h>

// Define the size of the FIFO
#define FIFO_SIZE 1024

// Define the FIFO data structure
typedef struct
{
    uint8_t pu8_buffer[FIFO_SIZE];
    uint16_t u16_head;
    uint16_t u16_tail;
    uint16_t u16_count;
} s_fifo_t;

void fifo_init(s_fifo_t *ps_fifo);
bool fifo_isEmpty(s_fifo_t *ps_fifo);
bool fifo_isFull(s_fifo_t *ps_fifo);
uint16_t fifo_getAvailableSpace(s_fifo_t *ps_fifo);
uint16_t fifo_getOccupiedSpace(s_fifo_t *ps_fifo);
void fifo_push(s_fifo_t *ps_fifo, uint16_t u16_len, uint8_t *pu8_data);
void fifo_pop(s_fifo_t *ps_fifo, uint16_t u16_len, uint8_t *pu8_data);

#endif /* FIFO_H_ */
