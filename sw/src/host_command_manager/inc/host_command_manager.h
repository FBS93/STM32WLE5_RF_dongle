/*******************************************************************************
 * @brief Host command manager active object public interface.
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

#ifndef HOST_COMMAND_MANAGER_H
#define HOST_COMMAND_MANAGER_H

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
#include "edf_active_object.h"

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/** @brief Maximum COBS-encoded host request frame size, including delimiter. */
#define HOST_COMMAND_MANAGER_RX_FRAME_CAPACITY 264U

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief Host command manager active object.
 */
typedef struct
{
  EDF_activeObject_t super;  //!< EDF active-object base.
  uint8_t rx_frame[HOST_COMMAND_MANAGER_RX_FRAME_CAPACITY];
  //!< Persistent COBS-encoded frame assembly, including its delimiter.
  uint16_t rx_frame_length;  //!< Bytes currently assembled.
} hostCommandManager_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes a host command manager active object.
 *
 * @param[in,out] me Pointer to the active object instance.
 */
void hostCommandManager_init(hostCommandManager_t* me);

#endif /* HOST_COMMAND_MANAGER_H */
