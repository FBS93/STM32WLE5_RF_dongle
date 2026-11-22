/*******************************************************************************
 * @brief Common 1 ms SysTick configuration for the ARM Cortex-M examples.
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

#include "tick_timer.h"
#include "stm32f103c8tx_registers.h"

/* 8000 core cycles per tick at an 8 MHz core clock. */
#define SYSTICK_RELOAD_1_MS 7999U

void tickTimer_init(void)
{
  SYST_CSR = 0U;
  SYST_RVR = SYSTICK_RELOAD_1_MS;
  SYST_CVR = 0U;
  SYST_CSR = SYST_CSR_CLKSOURCE | SYST_CSR_TICKINT | SYST_CSR_ENABLE;
}
