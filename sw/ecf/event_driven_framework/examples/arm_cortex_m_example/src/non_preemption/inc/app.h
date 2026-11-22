/*******************************************************************************
 * @brief Dummy EDF application used by the ARM Cortex-M non-preemption example.
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

#ifndef APP_H
#define APP_H

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initialize the dummy EDF application before starting SysTick.
 */
void app_init(void);

/**
 * @brief Advance the release schedule and publish due events from SysTick ISR.
 */
void app_onTick(void);

#endif /* APP_H */
