/*******************************************************************************
 * @brief GPIO trace outputs.
 *
 * @copyright
 * Copyright (c) 2026 FBS93.
 * See the LICENSE file of this project for license details.
 * This notice shall be retained in all copies or substantial portions
 * of the software.
 *
 * @warning
 * This software is provided "as is", without any express or implied warranty.
 * The user assumes all responsibility for its use and any consequences.
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

#include <stdint.h>

#include "gpio_trace.h"
#include "stm32f103c8tx_registers.h"
#include "eaf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

#define GPIO_OUTPUT_PUSH_PULL_50MHZ 0x3U
#define GPIO_TRACE_PIN_MASK (0x7U << 12U)

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

EAF_DEFINE_THIS_FILE(__FILE__);

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

static uint32_t getPinMask(gpioTrace_channel_t channel)
{
  EAF_ASSERT((unsigned int)channel <= GPIO_TRACE_CHANNEL_HIGH_EXEC);

  return (1U << (12U + (unsigned int)channel));
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void gpioTrace_init(void)
{
  RCC_APB2ENR |= RCC_APB2ENR_IOPBEN;

  /* Preload the output latch low before switching PB12-PB14 to outputs. */
  GPIOB_BSRR = GPIO_TRACE_PIN_MASK << 16U;
  GPIOB_CRH &= ~(0xFFFU << 16U);
  GPIOB_CRH |= ((GPIO_OUTPUT_PUSH_PULL_50MHZ << 16U) |
                (GPIO_OUTPUT_PUSH_PULL_50MHZ << 20U) |
                (GPIO_OUTPUT_PUSH_PULL_50MHZ << 24U));
}

void gpioTrace_setActive(gpioTrace_channel_t channel)
{
  GPIOB_BSRR = getPinMask(channel);
}

void gpioTrace_setInactive(gpioTrace_channel_t channel)
{
  GPIOB_BSRR = getPinMask(channel) << 16U;
}
