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

#ifndef EXEC_TIMER_H
#define EXEC_TIMER_H

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

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initialize the hardware timer used for blocking execution delays.
 */
void execTimer_init(void);

/**
 * @brief Read the current free-running timer tick.
 *
 * @return Current timer tick.
 */
uint16_t execTimer_getTick(void);

/**
 * @brief Convert a delay in microseconds into execution-timer ticks.
 *
 * @param[in] delay_us Delay in microseconds.
 * @return Delay expressed in execution-timer ticks.
 */
uint16_t execTimer_delayUsToTicks(uint32_t delay_us);

/**
 * @brief Compute elapsed execution-timer ticks between two samples.
 *
 * Supports the natural wraparound of the free-running 16-bit timer.
 *
 * @param[in] start_tick Earlier timer tick.
 * @param[in] end_tick Later timer tick.
 * @return Elapsed timer ticks.
 */
uint16_t execTimer_elapsedTicks(uint16_t start_tick, uint16_t end_tick);

#endif /* EXEC_TIMER_H */
