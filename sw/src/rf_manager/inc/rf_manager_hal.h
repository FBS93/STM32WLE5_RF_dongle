/*******************************************************************************
 * @brief RF manager internal radio hardware abstraction interface.
 *
 * The module provides semantic radio configuration and operation control. It
 * reports asynchronous radio indications through caller-provided callbacks.
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

#ifndef RF_MANAGER_HAL_H
#define RF_MANAGER_HAL_H

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */
#include <stdbool.h>
#include <stdint.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "rf_types.h"

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/** @brief Asynchronous radio indication callbacks. */
typedef struct
{
  void (*tx_done)(void* context);     //!< Packet transmission completed.
  void (*tx_timeout)(void* context);  //!< Packet transmission timed out.
  void (*rx_done)(void* context);     //!< A packet was received.
  void (*rx_timeout)(void* context);  //!< Receive operation timed out.
  void (*rx_error)(void* context);    //!< Receive operation failed.
  void (*cad_done)(void* context,
                   bool detected);  //!< CAD scan completed.
} rfManagerHal_callbacks_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the radio abstraction and registers indication callbacks.
 *
 * @param[in] callbacks Pointer to callbacks retained by the HAL.
 * @param[in,out] context Caller context passed to each callback.
 */
void rfManagerHal_init(const rfManagerHal_callbacks_t* callbacks,
                       void* context);

/**
 * @brief Applies a complete semantic radio configuration.
 *
 * @param[in] configuration Pointer to the configuration to apply.
 */
void rfManagerHal_setConfiguration(
  const rfTypes_configuration_t* configuration);

/**
 * @brief Performs complete radio and image calibration.
 *
 * @param[in] lower_frequency Encoded lower image-calibration frequency.
 * @param[in] upper_frequency Encoded upper image-calibration frequency.
 */
void rfManagerHal_calibrate(uint8_t lower_frequency, uint8_t upper_frequency);

/**
 * @brief Starts packet transmission using the applied configuration.
 *
 * @param[in] data Pointer to the packet bytes.
 * @param[in] length Number of packet bytes.
 */
void rfManagerHal_transmit(const uint8_t* data, uint8_t length);

/** @brief Starts RX using the mode and timing in the applied configuration. */
void rfManagerHal_startReceive(void);

/** @brief Starts one CAD scan using the applied LoRa CAD configuration. */
void rfManagerHal_startCad(void);

/**
 * @brief Starts continuous-wave TX using the applied frequency and power.
 */
void rfManagerHal_startContinuousWave(void);

/** @brief Starts infinite-preamble TX using the applied configuration. */
void rfManagerHal_startContinuousPreamble(void);

/** @brief Stops the active radio operation and places the radio in standby. */
void rfManagerHal_stop(void);

/**
 * @brief Reads the last received packet and its status.
 *
 * @param[out] data Pointer to the destination packet buffer.
 * @param[in] capacity Destination buffer capacity in bytes.
 * @param[out] status Pointer to the packet-status destination.
 * @return Number of packet bytes read.
 */
uint8_t rfManagerHal_readReceivedPacket(uint8_t* data,
                                        uint8_t capacity,
                                        rfTypes_packetStatus_t* status);

/**
 * @brief Gets currently recorded radio device errors.
 *
 * @return Radio error flags in the system-defined encoding.
 */
uint16_t rfManagerHal_getErrors(void);

#endif /* RF_MANAGER_HAL_H */
