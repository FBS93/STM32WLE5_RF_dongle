#ifndef UART_HAL_H_
#define UART_HAL_H_

#include <stdint.h>

void uartHAL_init(void);
void uartHAL_read(uint16_t *pu16_len, uint8_t *pu8_data);
void uartHAL_write(uint16_t u16_len, uint8_t *pu8_data);

#endif /* UART_HAL_H_ */
