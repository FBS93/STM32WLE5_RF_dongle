/*******************************************************************************
 * @brief RF manager HSM activity implementation.
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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */
#include <stddef.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */
#include "eaf.h"
#include "edf.h"

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "events.h"
#include "rf_manager_activities.h"
#include "rf_manager_hal.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/** @brief Define static file name string for asserts. */
EAF_DEFINE_THIS_FILE(__FILE__);

/** @brief Immutable operation-complete indication. */
static const events_operationComplete_t operationCompleteEvent = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_OPERATION_COMPLETE)};

/** @brief Immutable CAD-detected indication. */
static const events_cadDetected_t cadDetectedEvent = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_CAD_DETECTED)};

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void rfManagerActivities_handleGetConfigRequest(
  rfManager_t* me,
  const events_getConfigRequest_t* e)
{
  events_getConfigResponse_t* response;

  EAF_ASSERT((me != NULL) && (e != NULL));

  response = (events_getConfigResponse_t*)EDF_event_initMutable(
    sizeof(*response), EVENTS_GET_CONFIG_RESPONSE);
  EAF_ASSERT(response != NULL);

  response->configuration = me->configuration;
  EDF_activeObject_publish(&response->super);
}

void rfManagerActivities_handleSetConfig(rfManager_t* me,
                                         const events_setConfig_t* e)
{
  rfTypes_configuration_t configuration;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  configuration = e->configuration;

  rfManagerHal_stop();
  rfManagerHal_setConfiguration(&configuration);
  me->configuration = configuration;
  me->configuration_valid = true;
  me->active_operation = RF_TYPES_OPERATION_IDLE;
}

void rfManagerActivities_handleCalibrate(rfManager_t* me,
                                         const events_calibrate_t* e)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  rfManagerHal_stop();
  me->active_operation = RF_TYPES_OPERATION_CALIBRATION;

  rfManagerHal_calibrate(e->image_calibration_lower_frequency_hz,
                         e->image_calibration_upper_frequency_hz);

  EDF_activeObject_publish(&operationCompleteEvent.super);
}

void rfManagerActivities_handleTxPacket(rfManager_t* me,
                                        const events_txPacket_t* e)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  if (me->configuration_valid)
  {
    rfManagerHal_stop();
    me->active_operation = RF_TYPES_OPERATION_PACKET_TX;

    rfManagerHal_transmit(
      &me->configuration, e->packet_data, e->payload_length);
  }
}

bool rfManagerActivities_handleRxStart(rfManager_t* me,
                                       const events_rxStart_t* e)
{
  bool accepted;

  EAF_ASSERT((me != NULL) && (e != NULL));

  accepted = me->configuration_valid &&
             ((me->configuration.packet_type == RF_TYPES_PACKET_TYPE_LORA) ||
              (me->configuration.packet_type == RF_TYPES_PACKET_TYPE_FSK));
  if (accepted)
  {
    rfManagerHal_stop();
    me->active_operation = RF_TYPES_OPERATION_RX;

    rfManagerHal_startReceive(&me->configuration);
  }

  return accepted;
}

bool rfManagerActivities_handleCadStart(rfManager_t* me,
                                        const events_cadStart_t* e)
{
  bool accepted;

  EAF_ASSERT((me != NULL) && (e != NULL));

  accepted = me->configuration_valid &&
             (me->configuration.packet_type == RF_TYPES_PACKET_TYPE_LORA);
  if (accepted)
  {
    rfManagerHal_stop();
    me->active_operation = RF_TYPES_OPERATION_CAD;

    rfManagerHal_startCad(&me->configuration);
  }

  return accepted;
}

void rfManagerActivities_handleTxContinuousWave(
  rfManager_t* me,
  const events_txContinuousWave_t* e)
{
  EAF_ASSERT((me != NULL) && (e != NULL));

  if (me->configuration_valid)
  {
    rfManagerHal_stop();
    me->active_operation = RF_TYPES_OPERATION_CONTINUOUS_WAVE_TX;

    rfManagerHal_startContinuousWave();
  }
}

bool rfManagerActivities_handleTxContinuousPreamble(
  rfManager_t* me,
  const events_txContinuousPreamble_t* e)
{
  bool accepted;

  EAF_ASSERT((me != NULL) && (e != NULL));

  accepted = me->configuration_valid &&
             ((me->configuration.packet_type == RF_TYPES_PACKET_TYPE_LORA) ||
              (me->configuration.packet_type == RF_TYPES_PACKET_TYPE_FSK) ||
              (me->configuration.packet_type == RF_TYPES_PACKET_TYPE_MSK));
  if (accepted)
  {
    rfManagerHal_stop();
    me->active_operation = RF_TYPES_OPERATION_CONTINUOUS_PREAMBLE_TX;

    rfManagerHal_startContinuousPreamble();
  }

  return accepted;
}

void rfManagerActivities_handleStop(rfManager_t* me, const events_stop_t* e)
{
  EAF_ASSERT((me != NULL) && (e != NULL));

  rfManagerHal_stop();
  me->active_operation = RF_TYPES_OPERATION_IDLE;
}

void rfManagerActivities_handleGetDiagnosticsRequest(
  rfManager_t* me,
  const events_getDiagnosticsRequest_t* e)
{
  events_getDiagnosticsResponse_t* response;

  EAF_ASSERT((me != NULL) && (e != NULL));

  response = (events_getDiagnosticsResponse_t*)EDF_event_initMutable(
    sizeof(*response), EVENTS_GET_DIAGNOSTICS_RESPONSE);
  EAF_ASSERT(response != NULL);

  response->active_rf_operation = me->active_operation;
  response->radio_errors = rfManagerHal_getErrors();
  EDF_activeObject_publish(&response->super);
}

void rfManagerActivities_handleOperationComplete(
  rfManager_t* me,
  const events_operationComplete_t* e)
{
  EAF_ASSERT((me != NULL) && (e != NULL));

  rfManagerHal_stop();
  me->active_operation = RF_TYPES_OPERATION_IDLE;
}

void rfManagerActivities_txDone(void* context)
{
  EAF_ASSERT(context != NULL);

  EDF_activeObject_publish(&operationCompleteEvent.super);
}

void rfManagerActivities_txTimeout(void* context)
{
  EAF_ASSERT(context != NULL);

  EDF_activeObject_publish(&operationCompleteEvent.super);
}

void rfManagerActivities_rxDone(void* context)
{
  rfManager_t* me;
  events_rxPacket_t* event;
  uint8_t rx_mode;

  EAF_ASSERT(context != NULL);

  me = (rfManager_t*)context;
  event =
    (events_rxPacket_t*)EDF_event_initMutable(sizeof(*event), EVENTS_RX_PACKET);
  EAF_ASSERT(event != NULL);

  event->received_packet_length_bytes =
    rfManagerHal_readReceivedPacket(event->packet_data,
                                    sizeof(event->packet_data),
                                    &me->configuration,
                                    &event->packet_status);
  EDF_activeObject_publish(&event->super);

  if (me->active_operation == RF_TYPES_OPERATION_CAD)
  {
    rfManagerHal_startCad(&me->configuration);
  }
  else
  {
    if (me->configuration.packet_type == RF_TYPES_PACKET_TYPE_LORA)
    {
      rx_mode = me->configuration.packet_type_configuration.lora.rx_mode;
    }
    else
    {
      rx_mode = me->configuration.packet_type_configuration.fsk.rx_mode;
    }

    if (rx_mode == 0U)
    {
      EDF_activeObject_publish(&operationCompleteEvent.super);
    }
  }
}

void rfManagerActivities_rxTimeout(void* context)
{
  rfManager_t* me;

  EAF_ASSERT(context != NULL);

  me = (rfManager_t*)context;
  if (me->active_operation == RF_TYPES_OPERATION_CAD)
  {
    rfManagerHal_startCad(&me->configuration);
  }
  else
  {
    EDF_activeObject_publish(&operationCompleteEvent.super);
  }
}

void rfManagerActivities_rxError(void* context)
{
  EAF_ASSERT(context != NULL);

  // A rejected packet does not complete the active RX or CAD operation.
}

void rfManagerActivities_cadDone(void* context, bool detected)
{
  rfManager_t* me;

  EAF_ASSERT(context != NULL);

  me = (rfManager_t*)context;
  if (detected)
  {
    EDF_activeObject_publish(&cadDetectedEvent.super);
    if (me->configuration.packet_type_configuration.lora.cad.exit_mode == 0U)
    {
      EDF_activeObject_publish(&operationCompleteEvent.super);
    }
  }
  else
  {
    rfManagerHal_startCad(&me->configuration);
  }
}
