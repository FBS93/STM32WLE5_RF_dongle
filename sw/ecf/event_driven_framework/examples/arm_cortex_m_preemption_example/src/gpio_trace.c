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

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */
#include <stdint.h>
#include <stdbool.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "gpio_trace.h"
#include "stm32f103c8tx_registers.h"
#include "eaf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief Output configuration nibble for push-pull 50 MHz GPIO.
 */
#define GPIO_OUTPUT_PUSH_PULL_50MHZ 0x3U

/**
 * @brief PB12 bit mask.
 */
#define GPIO_TRACE_LOW_PIN_MASK (1U << 12U)

/**
 * @brief PB13 bit mask.
 */
#define GPIO_TRACE_MID_PIN_MASK (1U << 13U)

/**
 * @brief PB14 bit mask.
 */
#define GPIO_TRACE_HIGH_PIN_MASK (1U << 14U)

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/**
 * @brief Define static file name string for asserts.
 */
EAF_DEFINE_THIS_FILE(__FILE__);

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/**
 * @brief Resolve one trace channel to its GPIO bit mask.
 *
 * @param[in] channel Trace channel.
 * @return GPIO bit mask.
 */
static uint32_t getPinMask(gpioTrace_channel_t channel);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static uint32_t getPinMask(gpioTrace_channel_t channel)
{
  uint32_t pin_mask;

  switch (channel)
  {
    case GPIO_TRACE_CHANNEL_LOW:
    {
      pin_mask = GPIO_TRACE_LOW_PIN_MASK;
      break;
    }
    case GPIO_TRACE_CHANNEL_MID:
    {
      pin_mask = GPIO_TRACE_MID_PIN_MASK;
      break;
    }
    case GPIO_TRACE_CHANNEL_HIGH:
    {
      pin_mask = GPIO_TRACE_HIGH_PIN_MASK;
      break;
    }
    default:
    {
      EAF_ASSERT(false);
      pin_mask = 0U;
      break;
    }
  }

  return pin_mask;
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void gpioTrace_init(void)
{
  RCC_APB2ENR |= RCC_APB2ENR_IOPBEN;

  GPIOB_CRH &= ~((0xFU << 16U) | (0xFU << 20U) | (0xFU << 24U));
  GPIOB_CRH |= ((GPIO_OUTPUT_PUSH_PULL_50MHZ << 16U) |
                (GPIO_OUTPUT_PUSH_PULL_50MHZ << 20U) |
                (GPIO_OUTPUT_PUSH_PULL_50MHZ << 24U));

  gpioTrace_setInactive(GPIO_TRACE_CHANNEL_LOW);
  gpioTrace_setInactive(GPIO_TRACE_CHANNEL_MID);
  gpioTrace_setInactive(GPIO_TRACE_CHANNEL_HIGH);
}

void gpioTrace_setActive(gpioTrace_channel_t channel)
{
  GPIOB_BSRR = getPinMask(channel);
}

void gpioTrace_setInactive(gpioTrace_channel_t channel)
{
  GPIOB_BSRR = (getPinMask(channel) << 16U);
}
