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
#include "edf.h"

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
 * @brief Public host protocol command, response, and report identifiers.
 */
typedef enum
{
  HOST_COMMAND_MANAGER_GET_CONFIG_REQUEST =
    0x0000U,                                  //!< Configuration request.
  HOST_COMMAND_MANAGER_SET_CONFIG = 0x0001U,  //!< Apply configuration.
  HOST_COMMAND_MANAGER_CALIBRATE = 0x0002U,   //!< Radio calibration.
  HOST_COMMAND_MANAGER_TX_PACKET = 0x0003U,   //!< Packet TX request.
  HOST_COMMAND_MANAGER_RX_START = 0x0004U,    //!< Start packet RX.
  HOST_COMMAND_MANAGER_CAD_START = 0x0005U,   //!< Start LoRa CAD.
  HOST_COMMAND_MANAGER_TX_CONTINUOUS_WAVE = 0x0006U,  //!< Continuous-wave TX.
  HOST_COMMAND_MANAGER_TX_CONTINUOUS_PREAMBLE =
    0x0007U,                            //!< Infinite-preamble TX.
  HOST_COMMAND_MANAGER_STOP = 0x0008U,  //!< Stop the active RF operation.
  HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST =
    0x0009U,  //!< Diagnostics request.
  HOST_COMMAND_MANAGER_GET_CONFIG_RESPONSE =
    0x8001U,  //!< Configuration response.
  HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_RESPONSE =
    0x8002U,                                   //!< Diagnostics response.
  HOST_COMMAND_MANAGER_RX_PACKET = 0x8003U,    //!< Received packet report.
  HOST_COMMAND_MANAGER_CAD_DETECTED = 0x8004U  //!< Channel activity report.
} hostCommandManager_commandId_t;

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
 * @brief Initializes the active-object state and HSM, starts/registers it with
 * EDF using the architecture-defined priority and queue, and subscribes it to
 * the architecture-defined events without enabling asynchronous event sources.
 *
 * @param[in,out] me Pointer to the active object instance.
 */
void hostCommandManager_init(hostCommandManager_t* me);

/**
 * @brief Initializes and activates the active-object-owned HAL and resources,
 * enabling event-producing sources at the end of EDF_onStartup() in accordance
 * with the EDF startup contract.
 */
void hostCommandManager_startup(void);

#endif /* HOST_COMMAND_MANAGER_H */
