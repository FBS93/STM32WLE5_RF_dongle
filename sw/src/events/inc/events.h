/*******************************************************************************
 * @brief Project event signals and typed event parameters.
 *
 * The event parameter types preserve the semantics and numeric encodings of
 * SW_ARCH_DESIGN_1. These types are internal software objects and do not define
 * a serialized or packed wire layout.
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

#ifndef EVENTS_H
#define EVENTS_H

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
#include "edf_event.h"
#include "edf_hsm.h"

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

/**
 * @brief Project-specific event signals.
 */
typedef enum
{
  EVENTS_GET_CONFIG_REQUEST =
    EDF_HSM_USER_SIGNAL,            //!< Event type: events_getConfigRequest_t.
  EVENTS_SET_CONFIG,                //!< Event type: events_setConfig_t.
  EVENTS_CALIBRATE,                 //!< Event type: events_calibrate_t.
  EVENTS_TX_PACKET,                 //!< Event type: events_txPacket_t.
  EVENTS_RX_START,                  //!< Event type: events_rxStart_t.
  EVENTS_CAD_START,                 //!< Event type: events_cadStart_t.
  EVENTS_TX_CONTINUOUS_WAVE,        //!< Event type: events_txContinuousWave_t.
  EVENTS_TX_CONTINUOUS_PREAMBLE,    //!< Event type:
                                    //!< events_txContinuousPreamble_t.
  EVENTS_STOP,                      //!< Event type: events_stop_t.
  EVENTS_GET_DIAGNOSTICS_REQUEST,   //!< Event type:
                                    //!< events_getDiagnosticsRequest_t.
  EVENTS_GET_CONFIG_RESPONSE,       //!< Event type: events_getConfigResponse_t.
  EVENTS_GET_DIAGNOSTICS_RESPONSE,  //!< Event type:
                                    //!< events_getDiagnosticsResponse_t.
  EVENTS_RX_PACKET,                 //!< Event type: events_rxPacket_t.
  EVENTS_CAD_DETECTED,              //!< Event type: events_cadDetected_t.
  EVENTS_OPERATION_COMPLETE,        //!< Event type: events_operationComplete_t.
  EVENTS_HOST_DATA_AVAILABLE,       //!< Event type: events_hostDataAvailable_t.
  EVENTS_LAST_SIGNAL  //!< One past the last application event signal.
} events_signal_t;

/**
 * @brief GET_CONFIG_REQUEST event.
 */
typedef struct
{
  EDF_event_t super;  //!< EDF event base.
} events_getConfigRequest_t;

/**
 * @brief SET_CONFIG event.
 */
typedef struct
{
  EDF_event_t super;                      //!< EDF event base.
  rfTypes_configuration_t configuration;  //!< RF configuration.
} events_setConfig_t;

/**
 * @brief CALIBRATE event.
 */
typedef struct
{
  EDF_event_t super;                             //!< EDF event base.
  uint8_t image_calibration_lower_frequency_hz;  //!< Image calibration
                                                 //!< lowerFrequencyHz; 152..960
                                                 //!< MHz in 4 MHz units.
  uint8_t image_calibration_upper_frequency_hz;  //!< Image calibration
                                                 //!< upperFrequencyHz; 152..960
                                                 //!< MHz in 4 MHz units.
} events_calibrate_t;

/**
 * @brief TX_PACKET event.
 */
typedef struct
{
  EDF_event_t super;                                   //!< EDF event base.
  uint8_t packet_data[RF_TYPES_PACKET_DATA_CAPACITY];  //!< TX packet data.
} events_txPacket_t;

/**
 * @brief RX_START event.
 */
typedef struct
{
  EDF_event_t super;  //!< EDF event base.
} events_rxStart_t;

/**
 * @brief CAD_START event.
 */
typedef struct
{
  EDF_event_t super;  //!< EDF event base.
} events_cadStart_t;

/**
 * @brief TX_CONTINUOUS_WAVE event.
 */
typedef struct
{
  EDF_event_t super;  //!< EDF event base.
} events_txContinuousWave_t;

/**
 * @brief TX_CONTINUOUS_PREAMBLE event.
 */
typedef struct
{
  EDF_event_t super;  //!< EDF event base.
} events_txContinuousPreamble_t;

/**
 * @brief STOP event.
 */
typedef struct
{
  EDF_event_t super;  //!< EDF event base.
} events_stop_t;

/**
 * @brief GET_DIAGNOSTICS_REQUEST event.
 */
typedef struct
{
  EDF_event_t super;  //!< EDF event base.
} events_getDiagnosticsRequest_t;

/**
 * @brief GET_CONFIG_RESPONSE event.
 */
typedef struct
{
  EDF_event_t super;                      //!< EDF event base.
  rfTypes_configuration_t configuration;  //!< RF configuration.
} events_getConfigResponse_t;

/**
 * @brief GET_DIAGNOSTICS_RESPONSE event.
 */
typedef struct
{
  EDF_event_t super;                        //!< EDF event base.
  rfTypes_operation_t active_rf_operation;  //!< Current RF configuration
  uint16_t radio_errors;  //!< Radio errors bit field; zero means no error.
} events_getDiagnosticsResponse_t;

/**
 * @brief RX_PACKET event.
 */
typedef struct
{
  EDF_event_t super;                     //!< EDF event base.
  uint8_t received_packet_length_bytes;  //!< Received packetLengthBytes;
                                         //!< 0..255 bytes.
  uint8_t packet_data[RF_TYPES_PACKET_DATA_CAPACITY];  //!< RX packet data.
  rfTypes_packetStatus_t packet_status;  //!< RSSI plus LoRa SNR or FSK/GFSK
                                         //!< status flags.
} events_rxPacket_t;

/**
 * @brief CAD_DETECTED event.
 */
typedef struct
{
  EDF_event_t super;  //!< EDF event base.
} events_cadDetected_t;

/**
 * @brief OPERATION_COMPLETE event.
 */
typedef struct
{
  EDF_event_t super;  //!< EDF event base.
} events_operationComplete_t;

/**
 * @brief HOST_DATA_AVAILABLE event.
 */
typedef struct
{
  EDF_event_t super;  //!< EDF event base.
} events_hostDataAvailable_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

#endif /* EVENTS_H */
