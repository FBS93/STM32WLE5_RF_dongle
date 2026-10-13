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
#include <stdbool.h>

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
 * @brief Publishes the stored configuration, including before first SET_CONFIG.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleGetConfigRequest(
  rfManager_t* me,
  const events_getConfigRequest_t* e);

/**
 * @brief Stops RF, applies and stores the configuration, and marks it valid.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleSetConfig(rfManager_t* me,
                                         const events_setConfig_t* e);

/**
 * @brief Stops RF, performs calibration, and publishes operation completion.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleCalibrate(rfManager_t* me,
                                         const events_calibrate_t* e);

/**
 * @brief Starts packet TX with valid configuration, replacing active RF.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleTxPacket(rfManager_t* me,
                                        const events_txPacket_t* e);

/**
 * @brief Starts RX with valid LoRa or FSK configuration, replacing active RF.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 * @return @c true when RX was started; otherwise @c false.
 */
bool rfManagerActivities_handleRxStart(rfManager_t* me,
                                       const events_rxStart_t* e);

/**
 * @brief Replaces the active operation with CAD for a valid LoRa configuration.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 * @return @c true when CAD was started; otherwise @c false.
 */
bool rfManagerActivities_handleCadStart(rfManager_t* me,
                                        const events_cadStart_t* e);

/**
 * @brief Replaces active RF with continuous-wave TX if configuration is valid.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleTxContinuousWave(
  rfManager_t* me,
  const events_txContinuousWave_t* e);

/**
 * @brief Starts preamble TX for valid LoRa, FSK, or MSK, replacing active RF.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 * @return @c true when preamble transmission was started; otherwise @c false.
 */
bool rfManagerActivities_handleTxContinuousPreamble(
  rfManager_t* me,
  const events_txContinuousPreamble_t* e);

/**
 * @brief Stops the radio and marks the host-visible operation idle.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleStop(rfManager_t* me, const events_stop_t* e);

/**
 * @brief Publishes the active operation and radio errors without stopping RF.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleGetDiagnosticsRequest(
  rfManager_t* me,
  const events_getDiagnosticsRequest_t* e);

/**
 * @brief Stops completed RF hardware and marks the host-visible operation idle.
 *
 * @param[in,out] me Pointer to the RF manager instance.
 * @param[in] e Pointer to the event.
 */
void rfManagerActivities_handleOperationComplete(
  rfManager_t* me,
  const events_operationComplete_t* e);

/**
 * @brief Publishes operation completion after a radio TX-complete indication.
 *
 * @param[in] context Non-null registered caller context; contents are unused.
 */
void rfManagerActivities_txDone(void* context);

/**
 * @brief Publishes operation completion after a radio TX timeout.
 *
 * @param[in] context Non-null registered caller context; contents are unused.
 */
void rfManagerActivities_txTimeout(void* context);

/**
 * @brief Publishes the packet and resumes CAD or duty-cycle RX as needed.
 *
 * Single RX publishes operation completion; continuous RX remains listening.
 *
 * @param[in] context RF manager instance to inspect.
 */
void rfManagerActivities_rxDone(void* context);

/**
 * @brief Resumes CAD or duty-cycle RX on timeout; completes other RX modes.
 *
 * @param[in] context RF manager instance to inspect.
 */
void rfManagerActivities_rxTimeout(void* context);

/**
 * @brief Discards a rejected packet without completing RX or CAD.
 *
 * Rearms duty-cycle RX when it is the active operation.
 *
 * @param[in] context RF manager instance to inspect.
 */
void rfManagerActivities_rxError(void* context);

/**
 * @brief Repeats clear CAD scans; reports detection and optional completion.
 *
 * Detection completes CAD only when its configured exit mode does not enter RX.
 *
 * @param[in] context RF manager instance providing the CAD exit configuration.
 * @param[in] detected True when the radio detected channel activity.
 */
void rfManagerActivities_cadDone(void* context, bool detected);

#endif /* RF_MANAGER_ACTIVITIES_H */
