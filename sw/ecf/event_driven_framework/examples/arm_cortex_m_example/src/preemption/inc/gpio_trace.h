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
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */

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

/**
 * @brief Logic analyzer trace channels.
 */
typedef enum
{
  GPIO_TRACE_CHANNEL_LOW = 0, /**< AO low-priority trace output. */
  GPIO_TRACE_CHANNEL_MID,     /**< AO medium-priority trace output. */
  GPIO_TRACE_CHANNEL_HIGH,    /**< AO high-priority trace output. */
} gpioTrace_channel_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initialize the GPIO trace outputs.
 */
void gpioTrace_init(void);

/**
 * @brief Set one trace channel active.
 *
 * @param[in] channel Trace channel to set high.
 */
void gpioTrace_setActive(gpioTrace_channel_t channel);

/**
 * @brief Set one trace channel inactive.
 *
 * @param[in] channel Trace channel to set low.
 */
void gpioTrace_setInactive(gpioTrace_channel_t channel);

#endif /* GPIO_TRACE_H */
