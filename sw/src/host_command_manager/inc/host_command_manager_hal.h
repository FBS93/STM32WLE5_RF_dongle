/*******************************************************************************
 * @brief Host command manager internal UART hardware abstraction interface.
 *
 * The module abstracts USART2, owns its RX and TX FIFOs, and provides
 * host-data received notification for the host command manager software unit.
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

#ifndef HOST_COMMAND_MANAGER_HAL_H
#define HOST_COMMAND_MANAGER_HAL_H

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
 * @brief Initializes the internal USART2 hardware abstraction.
 *
 */
void hostCommandManagerHal_init(void);

/**
 * @brief Reads available host bytes without blocking, up to @c capacity.
 *
 * Read until zero is returned. Observing an already empty RX FIFO atomically
 * rearms the target ISR notification; consuming the last byte does not.
 *
 * @param[out] data Destination buffer of at least @c capacity bytes.
 * @param[in] capacity Maximum number of bytes to read; greater than zero.
 * @return Number of bytes consumed, or zero when the RX FIFO was empty.
 */
uint32_t hostCommandManagerHal_read(uint8_t* data, uint32_t capacity);

/**
 * @brief Writes bytes to the host.
 *
 * @param[in] data Pointer to the bytes to write.
 * @param[in] length Number of bytes to write.
 */
void hostCommandManagerHal_write(const uint8_t* data, uint32_t length);

#endif /* HOST_COMMAND_MANAGER_HAL_H */
