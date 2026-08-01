/*******************************************************************************
 * @brief Blocking execution-time timer.
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

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "exec_timer.h"
#include "stm32f103c8tx_registers.h"
#include "eaf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief Timer tick period used by the execution delay timer.
 */
#define EXEC_TIMER_TICK_US 10U

/**
 * @brief Prescaler value for 8 MHz / 80 = 100 kHz.
 */
#define EXEC_TIMER_PRESCALER 79U

/**
 * @brief Maximum auto-reload value used for the free-running timer.
 */
#define EXEC_TIMER_MAX_ARR 0xFFFFU

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

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void execTimer_init(void)
{
  RCC_APB1ENR |= RCC_APB1ENR_TIM2EN;

  TIM2_CR1 = 0U;
  TIM2_PSC = EXEC_TIMER_PRESCALER;
  TIM2_ARR = EXEC_TIMER_MAX_ARR;
  TIM2_EGR = TIM_EGR_UG;
  TIM2_CNT = 0U;
  TIM2_CR1 = TIM_CR1_CEN;
}

uint16_t execTimer_getTick(void)
{
  return (uint16_t)TIM2_CNT;
}

uint16_t execTimer_delayUsToTicks(uint32_t delay_us)
{
  uint16_t delay_ticks;
  uint32_t rounded_ticks;

  EAF_ASSERT(delay_us > 0U);

  rounded_ticks = (delay_us + (EXEC_TIMER_TICK_US - 1U)) / EXEC_TIMER_TICK_US;

  EAF_ASSERT(rounded_ticks <= EXEC_TIMER_MAX_ARR);

  delay_ticks = (uint16_t)rounded_ticks;

  EAF_ASSERT(delay_ticks > 0U);

  return delay_ticks;
}

uint16_t execTimer_elapsedTicks(uint16_t start_tick, uint16_t end_tick)
{
  return (uint16_t)(end_tick - start_tick);
}
