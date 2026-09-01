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
#include "host_command_manager.h"

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
 * @param[in,out] me Pointer to the host command manager receiving data
 * notifications.
 */
void hostCommandManagerHal_init(hostCommandManager_t* me);

/**
 * @brief Gets the number of host bytes available to read.
 *
 * @return Number of available bytes.
 */
uint32_t hostCommandManagerHal_getAvailableBytes(void);

/**
 * @brief Reads and consumes exactly the requested number of bytes received
 * from the host.
 *
 * @param[out] data Pointer to a destination buffer of at least @c length bytes.
 * @param[in] length Number of bytes to read.
 */
void hostCommandManagerHal_read(uint8_t* data, uint32_t length);

/**
 * @brief Writes bytes to the host.
 *
 * @param[in] data Pointer to the bytes to write.
 * @param[in] length Number of bytes to write.
 */
void hostCommandManagerHal_write(const uint8_t* data, uint32_t length);

#endif /* HOST_COMMAND_MANAGER_HAL_H */
