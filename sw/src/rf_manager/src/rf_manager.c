/*******************************************************************************
 * @brief RF manager active-object implementation.
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
#include "emf.h"

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "events.h"
#include "rf_manager.h"
#include "rf_manager_activities.h"
#include "rf_manager_hal.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/** @brief Architecture-defined active-object priority. */
#define ACTIVE_OBJECT_PRIORITY EDF_AO_PRIO(3U, 3U)
/** @brief Active-object event queue capacity. */
#define EVENT_QUEUE_STORAGE_SIZE 5U

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/** @brief Define static file name string for asserts. */
EAF_DEFINE_THIS_FILE(__FILE__);

/** @brief Active-object event queue storage. */
static EDF_event_ptr eventQueueStorage[EVENT_QUEUE_STORAGE_SIZE];
/** @brief Event used to execute the top-most HSM initial transition. */
static const EDF_event_t initializationEvent =
  EDF_EVENT_IMMUTABLE_INIT(EDF_HSM_INIT_SIGNAL);
/** @brief RF manager instance retained for startup and radio callbacks. */
static rfManager_t* instance;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/** @brief Performs the initial transition to idle. */
static EDF_hsm_stateReturn_t initialTransition(rfManager_t* me,
                                               const EDF_event_t* e);
/** @brief Handles events in the idle state. */
static EDF_hsm_stateReturn_t idle(rfManager_t* me, const EDF_event_t* e);
/** @brief Handles events in the TX state. */
static EDF_hsm_stateReturn_t tx(rfManager_t* me, const EDF_event_t* e);
/** @brief Handles events in the RX state. */
static EDF_hsm_stateReturn_t rx(rfManager_t* me, const EDF_event_t* e);
/** @brief Handles events in the CAD state. */
static EDF_hsm_stateReturn_t cad(rfManager_t* me, const EDF_event_t* e);
/** @brief Handles events in the calibration state. */
static EDF_hsm_stateReturn_t calibration(rfManager_t* me, const EDF_event_t* e);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static EDF_hsm_stateReturn_t initialTransition(rfManager_t* me,
                                               const EDF_event_t* e)
{
  EAF_ASSERT((me != NULL) && (e != NULL));

  return EDF_HSM_RET_TRAN(idle);
}

static EDF_hsm_stateReturn_t idle(rfManager_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t result;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case EVENTS_GET_CONFIG_REQUEST:
      rfManagerActivities_handleGetConfigRequest(
        me, (const events_getConfigRequest_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    case EVENTS_SET_CONFIG:
      rfManagerActivities_handleSetConfig(me, (const events_setConfig_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    case EVENTS_CALIBRATE:
      rfManagerActivities_handleCalibrate(me, (const events_calibrate_t*)e);
      result = EDF_HSM_RET_TRAN(calibration);
      break;
    case EVENTS_TX_PACKET:
      rfManagerActivities_handleTxPacket(me, (const events_txPacket_t*)e);
      result = EDF_HSM_RET_TRAN(tx);
      break;
    case EVENTS_RX_START:
      if (rfManagerActivities_handleRxStart(me, (const events_rxStart_t*)e))
      {
        result = EDF_HSM_RET_TRAN(rx);
      }
      else
      {
        result = EDF_HSM_RET_HANDLED();
      }
      break;
    case EVENTS_CAD_START:
      if (rfManagerActivities_handleCadStart(me, (const events_cadStart_t*)e))
      {
        result = EDF_HSM_RET_TRAN(cad);
      }
      else
      {
        result = EDF_HSM_RET_HANDLED();
      }
      break;
    case EVENTS_TX_CONTINUOUS_WAVE:
      rfManagerActivities_handleTxContinuousWave(
        me, (const events_txContinuousWave_t*)e);
      result = EDF_HSM_RET_TRAN(tx);
      break;
    case EVENTS_TX_CONTINUOUS_PREAMBLE:
      if (rfManagerActivities_handleTxContinuousPreamble(
            me, (const events_txContinuousPreamble_t*)e))
      {
        result = EDF_HSM_RET_TRAN(tx);
      }
      else
      {
        result = EDF_HSM_RET_HANDLED();
      }
      break;
    case EVENTS_STOP:
      rfManagerActivities_handleStop(me, (const events_stop_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    case EVENTS_GET_DIAGNOSTICS_REQUEST:
      rfManagerActivities_handleGetDiagnosticsRequest(
        me, (const events_getDiagnosticsRequest_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    default:
      result = EDF_HSM_RET_SUPER(EDF_hsm_top);
      break;
  }

  return result;
}

static EDF_hsm_stateReturn_t tx(rfManager_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t result;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case EVENTS_GET_CONFIG_REQUEST:
      rfManagerActivities_handleGetConfigRequest(
        me, (const events_getConfigRequest_t*)e);
      result = EDF_HSM_RET_TRAN(tx);
      break;
    case EVENTS_SET_CONFIG:
      rfManagerActivities_handleSetConfig(me, (const events_setConfig_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    case EVENTS_CALIBRATE:
      rfManagerActivities_handleCalibrate(me, (const events_calibrate_t*)e);
      result = EDF_HSM_RET_TRAN(calibration);
      break;
    case EVENTS_TX_PACKET:
      rfManagerActivities_handleTxPacket(me, (const events_txPacket_t*)e);
      result = EDF_HSM_RET_TRAN(tx);
      break;
    case EVENTS_RX_START:
      if (rfManagerActivities_handleRxStart(me, (const events_rxStart_t*)e))
      {
        result = EDF_HSM_RET_TRAN(rx);
      }
      else
      {
        result = EDF_HSM_RET_HANDLED();
      }
      break;
    case EVENTS_CAD_START:
      if (rfManagerActivities_handleCadStart(me, (const events_cadStart_t*)e))
      {
        result = EDF_HSM_RET_TRAN(cad);
      }
      else
      {
        result = EDF_HSM_RET_HANDLED();
      }
      break;
    case EVENTS_TX_CONTINUOUS_WAVE:
      rfManagerActivities_handleTxContinuousWave(
        me, (const events_txContinuousWave_t*)e);
      result = EDF_HSM_RET_TRAN(tx);
      break;
    case EVENTS_TX_CONTINUOUS_PREAMBLE:
      if (rfManagerActivities_handleTxContinuousPreamble(
            me, (const events_txContinuousPreamble_t*)e))
      {
        result = EDF_HSM_RET_TRAN(tx);
      }
      else
      {
        result = EDF_HSM_RET_HANDLED();
      }
      break;
    case EVENTS_STOP:
      rfManagerActivities_handleStop(me, (const events_stop_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    case EVENTS_GET_DIAGNOSTICS_REQUEST:
      rfManagerActivities_handleGetDiagnosticsRequest(
        me, (const events_getDiagnosticsRequest_t*)e);
      result = EDF_HSM_RET_TRAN(tx);
      break;
    case EVENTS_OPERATION_COMPLETE:
      rfManagerActivities_handleOperationComplete(
        me, (const events_operationComplete_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    default:
      result = EDF_HSM_RET_SUPER(EDF_hsm_top);
      break;
  }

  return result;
}

static EDF_hsm_stateReturn_t rx(rfManager_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t result;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case EVENTS_GET_CONFIG_REQUEST:
      rfManagerActivities_handleGetConfigRequest(
        me, (const events_getConfigRequest_t*)e);
      result = EDF_HSM_RET_TRAN(rx);
      break;
    case EVENTS_SET_CONFIG:
      rfManagerActivities_handleSetConfig(me, (const events_setConfig_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    case EVENTS_CALIBRATE:
      rfManagerActivities_handleCalibrate(me, (const events_calibrate_t*)e);
      result = EDF_HSM_RET_TRAN(calibration);
      break;
    case EVENTS_TX_PACKET:
      rfManagerActivities_handleTxPacket(me, (const events_txPacket_t*)e);
      result = EDF_HSM_RET_TRAN(tx);
      break;
    case EVENTS_RX_START:
      if (rfManagerActivities_handleRxStart(me, (const events_rxStart_t*)e))
      {
        result = EDF_HSM_RET_TRAN(rx);
      }
      else
      {
        result = EDF_HSM_RET_HANDLED();
      }
      break;
    case EVENTS_CAD_START:
      if (rfManagerActivities_handleCadStart(me, (const events_cadStart_t*)e))
      {
        result = EDF_HSM_RET_TRAN(cad);
      }
      else
      {
        result = EDF_HSM_RET_HANDLED();
      }
      break;
    case EVENTS_TX_CONTINUOUS_WAVE:
      rfManagerActivities_handleTxContinuousWave(
        me, (const events_txContinuousWave_t*)e);
      result = EDF_HSM_RET_TRAN(tx);
      break;
    case EVENTS_TX_CONTINUOUS_PREAMBLE:
      if (rfManagerActivities_handleTxContinuousPreamble(
            me, (const events_txContinuousPreamble_t*)e))
      {
        result = EDF_HSM_RET_TRAN(tx);
      }
      else
      {
        result = EDF_HSM_RET_HANDLED();
      }
      break;
    case EVENTS_STOP:
      rfManagerActivities_handleStop(me, (const events_stop_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    case EVENTS_GET_DIAGNOSTICS_REQUEST:
      rfManagerActivities_handleGetDiagnosticsRequest(
        me, (const events_getDiagnosticsRequest_t*)e);
      result = EDF_HSM_RET_TRAN(rx);
      break;
    case EVENTS_OPERATION_COMPLETE:
      rfManagerActivities_handleOperationComplete(
        me, (const events_operationComplete_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    default:
      result = EDF_HSM_RET_SUPER(EDF_hsm_top);
      break;
  }

  return result;
}

static EDF_hsm_stateReturn_t cad(rfManager_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t result;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case EVENTS_GET_CONFIG_REQUEST:
      rfManagerActivities_handleGetConfigRequest(
        me, (const events_getConfigRequest_t*)e);
      result = EDF_HSM_RET_TRAN(cad);
      break;
    case EVENTS_SET_CONFIG:
      rfManagerActivities_handleSetConfig(me, (const events_setConfig_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    case EVENTS_CALIBRATE:
      rfManagerActivities_handleCalibrate(me, (const events_calibrate_t*)e);
      result = EDF_HSM_RET_TRAN(calibration);
      break;
    case EVENTS_TX_PACKET:
      rfManagerActivities_handleTxPacket(me, (const events_txPacket_t*)e);
      result = EDF_HSM_RET_TRAN(tx);
      break;
    case EVENTS_RX_START:
      if (rfManagerActivities_handleRxStart(me, (const events_rxStart_t*)e))
      {
        result = EDF_HSM_RET_TRAN(rx);
      }
      else
      {
        result = EDF_HSM_RET_HANDLED();
      }
      break;
    case EVENTS_CAD_START:
      if (rfManagerActivities_handleCadStart(me, (const events_cadStart_t*)e))
      {
        result = EDF_HSM_RET_TRAN(cad);
      }
      else
      {
        result = EDF_HSM_RET_HANDLED();
      }
      break;
    case EVENTS_TX_CONTINUOUS_WAVE:
      rfManagerActivities_handleTxContinuousWave(
        me, (const events_txContinuousWave_t*)e);
      result = EDF_HSM_RET_TRAN(tx);
      break;
    case EVENTS_TX_CONTINUOUS_PREAMBLE:
      if (rfManagerActivities_handleTxContinuousPreamble(
            me, (const events_txContinuousPreamble_t*)e))
      {
        result = EDF_HSM_RET_TRAN(tx);
      }
      else
      {
        result = EDF_HSM_RET_HANDLED();
      }
      break;
    case EVENTS_STOP:
      rfManagerActivities_handleStop(me, (const events_stop_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    case EVENTS_GET_DIAGNOSTICS_REQUEST:
      rfManagerActivities_handleGetDiagnosticsRequest(
        me, (const events_getDiagnosticsRequest_t*)e);
      result = EDF_HSM_RET_TRAN(cad);
      break;
    case EVENTS_OPERATION_COMPLETE:
      rfManagerActivities_handleOperationComplete(
        me, (const events_operationComplete_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    default:
      result = EDF_HSM_RET_SUPER(EDF_hsm_top);
      break;
  }

  return result;
}

static EDF_hsm_stateReturn_t calibration(rfManager_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t result;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case EVENTS_GET_CONFIG_REQUEST:
      rfManagerActivities_handleGetConfigRequest(
        me, (const events_getConfigRequest_t*)e);
      result = EDF_HSM_RET_TRAN(calibration);
      break;
    case EVENTS_SET_CONFIG:
      rfManagerActivities_handleSetConfig(me, (const events_setConfig_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    case EVENTS_CALIBRATE:
      rfManagerActivities_handleCalibrate(me, (const events_calibrate_t*)e);
      result = EDF_HSM_RET_TRAN(calibration);
      break;
    case EVENTS_TX_PACKET:
      rfManagerActivities_handleTxPacket(me, (const events_txPacket_t*)e);
      result = EDF_HSM_RET_TRAN(tx);
      break;
    case EVENTS_RX_START:
      if (rfManagerActivities_handleRxStart(me, (const events_rxStart_t*)e))
      {
        result = EDF_HSM_RET_TRAN(rx);
      }
      else
      {
        result = EDF_HSM_RET_HANDLED();
      }
      break;
    case EVENTS_CAD_START:
      if (rfManagerActivities_handleCadStart(me, (const events_cadStart_t*)e))
      {
        result = EDF_HSM_RET_TRAN(cad);
      }
      else
      {
        result = EDF_HSM_RET_HANDLED();
      }
      break;
    case EVENTS_TX_CONTINUOUS_WAVE:
      rfManagerActivities_handleTxContinuousWave(
        me, (const events_txContinuousWave_t*)e);
      result = EDF_HSM_RET_TRAN(tx);
      break;
    case EVENTS_TX_CONTINUOUS_PREAMBLE:
      if (rfManagerActivities_handleTxContinuousPreamble(
            me, (const events_txContinuousPreamble_t*)e))
      {
        result = EDF_HSM_RET_TRAN(tx);
      }
      else
      {
        result = EDF_HSM_RET_HANDLED();
      }
      break;
    case EVENTS_STOP:
      rfManagerActivities_handleStop(me, (const events_stop_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    case EVENTS_GET_DIAGNOSTICS_REQUEST:
      rfManagerActivities_handleGetDiagnosticsRequest(
        me, (const events_getDiagnosticsRequest_t*)e);
      result = EDF_HSM_RET_TRAN(calibration);
      break;
    case EVENTS_OPERATION_COMPLETE:
      rfManagerActivities_handleOperationComplete(
        me, (const events_operationComplete_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    default:
      result = EDF_HSM_RET_SUPER(EDF_hsm_top);
      break;
  }

  return result;
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void rfManager_init(rfManager_t* me)
{
  static const events_signal_t subscriptions[] = {
    EVENTS_GET_CONFIG_REQUEST,
    EVENTS_SET_CONFIG,
    EVENTS_CALIBRATE,
    EVENTS_TX_PACKET,
    EVENTS_RX_START,
    EVENTS_CAD_START,
    EVENTS_TX_CONTINUOUS_WAVE,
    EVENTS_TX_CONTINUOUS_PREAMBLE,
    EVENTS_STOP,
    EVENTS_GET_DIAGNOSTICS_REQUEST,
    EVENTS_OPERATION_COMPLETE};
  size_t index;

  EAF_ASSERT(me != NULL);

  EMF_utils_clear(me, sizeof(*me));
  me->active_operation = RF_TYPES_OPERATION_IDLE;
  instance = me;
  EDF_activeObject_init(EDF_AO_UPCAST(*me),
                        (EDF_hsm_stateHandler_t)initialTransition);
  EDF_activeObject_start(EDF_AO_UPCAST(*me),
                         ACTIVE_OBJECT_PRIORITY,
                         eventQueueStorage,
                         EVENT_QUEUE_STORAGE_SIZE,
                         NULL,
                         0U,
                         &initializationEvent);
  for (index = 0U; index < (sizeof(subscriptions) / sizeof(subscriptions[0]));
       ++index)
  {
    EDF_activeObject_subscribe(EDF_AO_UPCAST(*me),
                               (EDF_event_signal_t)subscriptions[index]);
  }
}

void rfManager_startup(void)
{
  static const rfManagerHal_callbacks_t callbacks = {
    .tx_done = rfManagerActivities_txDone,
    .tx_timeout = rfManagerActivities_txTimeout,
    .rx_done = rfManagerActivities_rxDone,
    .rx_timeout = rfManagerActivities_rxTimeout,
    .rx_error = rfManagerActivities_rxError,
    .cad_done = rfManagerActivities_cadDone};

  EAF_ASSERT(instance != NULL);

  rfManagerHal_init(&callbacks, instance);
}
