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

#ifndef GPIO_TRACE_H
#define GPIO_TRACE_H

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief Logic analyzer trace channels, mapped to PB12-PB14.
 */
typedef enum
{
  GPIO_TRACE_CHANNEL_LOW_EXEC = 0,
  GPIO_TRACE_CHANNEL_HIGH_RELEASE,
  GPIO_TRACE_CHANNEL_HIGH_EXEC,
} gpioTrace_channel_t;

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/** @brief Initialize all three trace outputs low. */
void gpioTrace_init(void);

/** @brief Set the selected trace channel high. */
void gpioTrace_setActive(gpioTrace_channel_t channel);

/** @brief Set the selected trace channel low. */
void gpioTrace_setInactive(gpioTrace_channel_t channel);

#endif /* GPIO_TRACE_H */
