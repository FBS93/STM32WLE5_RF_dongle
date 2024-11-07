#include <stdint.h>

#include "stm32wlxx_hal.h"

#include "uart_hal.h"
#include "fifo.h"

/* UART module critical section*/
#define UART_CRITICAL_SECTION_BEGIN() HAL_NVIC_DisableIRQ(USART2_IRQn);
#define UART_CRITICAL_SECTION_END() HAL_NVIC_EnableIRQ(USART2_IRQn);

/* Global handler shall be used to control the UART due to ST generated code architecture... */
extern UART_HandleTypeDef huart2;

/* Rx interrupt buffer */
static uint8_t u8_uartHAL_rxInterrupBuffer;

/* Rx FIFO */
static s_fifo_t s_uartHAL_rxFIFO;

/* Rx Transfer completed callback that shall be defined here due to ST generated code architecture... */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

  if (huart->Instance == USART2)
  {

    if (fifo_getAvailableSpace(&s_uartHAL_rxFIFO) >= 1u)
    {
      fifo_push(&s_uartHAL_rxFIFO, 1u, &u8_uartHAL_rxInterrupBuffer);
    }
    else
    {
      // FIFO overflow, error not managed
    }

    /* Configure UART in receive 1 Byte in interrupt mode */
    HAL_UART_Receive_IT(&huart2, &u8_uartHAL_rxInterrupBuffer, 1u);
  }
}

void uartHAL_init(void)
{
  /* Rx FIFO initialization */
  fifo_init(&s_uartHAL_rxFIFO);

  /* Configure UART in receive 1 Byte in interrupt mode */
  HAL_UART_Receive_IT(&huart2, &u8_uartHAL_rxInterrupBuffer, 1u);
}

void uartHAL_read(uint16_t *pu16_len, uint8_t *pu8_data)
{
  /* Enter critical section to protect the FIFO that is also used from UART Rx Interrupt */
  UART_CRITICAL_SECTION_BEGIN();

  /* Get all bytes from FIFO */
  *pu16_len = fifo_getOccupiedSpace(&s_uartHAL_rxFIFO);
  fifo_pop(&s_uartHAL_rxFIFO, *pu16_len, pu8_data);

  /* Exit critical section */
  UART_CRITICAL_SECTION_END();
}

void uartHAL_write(uint16_t u16_len, uint8_t *pu8_data)
{
  HAL_UART_Transmit(&huart2, pu8_data, u16_len, 1000u);
}
