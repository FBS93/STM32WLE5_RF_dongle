/*******************************************************************************
 * @brief RF manager HSM activities.
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

#ifndef RF_MANAGER_ACTIVITIES_H
#define RF_MANAGER_ACTIVITIES_H

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
#include "events.h"
#include "rf_manager.h"

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
 * @brief Handles a GET_CONFIG_REQUEST event.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleGetConfigRequest(
  rfManager_t* me,
  const events_getConfigRequest_t* e);

/**
 * @brief Handles a SET_CONFIG event.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleSetConfig(rfManager_t* me,
                                         const events_setConfig_t* e);

/**
 * @brief Handles a CALIBRATE event.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleCalibrate(rfManager_t* me,
                                         const events_calibrate_t* e);

/**
 * @brief Handles a TX_PACKET event.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleTxPacket(rfManager_t* me,
                                        const events_txPacket_t* e);

/**
 * @brief Handles an RX_START event.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleRxStart(rfManager_t* me,
                                       const events_rxStart_t* e);

/**
 * @brief Handles a CAD_START event.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleCadStart(rfManager_t* me,
                                        const events_cadStart_t* e);

/**
 * @brief Handles a TX_CONTINUOUS_WAVE event.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleTxContinuousWave(
  rfManager_t* me,
  const events_txContinuousWave_t* e);

/**
 * @brief Handles a TX_CONTINUOUS_PREAMBLE event.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleTxContinuousPreamble(
  rfManager_t* me,
  const events_txContinuousPreamble_t* e);

/**
 * @brief Handles a STOP event.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleStop(rfManager_t* me, const events_stop_t* e);

/**
 * @brief Handles a GET_DIAGNOSTICS_REQUEST event.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleGetDiagnosticsRequest(
  rfManager_t* me,
  const events_getDiagnosticsRequest_t* e);

/**
 * @brief Handles an OPERATION_COMPLETE event.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleOperationComplete(
  rfManager_t* me,
  const events_operationComplete_t* e);

#endif /* RF_MANAGER_ACTIVITIES_H */
