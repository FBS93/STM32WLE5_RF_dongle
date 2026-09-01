/*******************************************************************************
 * @brief Host command manager HSM activities.
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

#ifndef HOST_COMMAND_MANAGER_ACTIVITIES_H
#define HOST_COMMAND_MANAGER_ACTIVITIES_H

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
 * @brief Handles a HOST_DATA_AVAILABLE event.
 *
 * @param[in,out] me Pointer to the host command manager instance.
 * @param[in] e Pointer to the event.
 */
void hostCommandManagerActivities_handleHostDataAvailable(
  hostCommandManager_t* me,
  const events_hostDataAvailable_t* e);

/**
 * @brief Handles a GET_CONFIG_RESPONSE event.
 *
 * @param[in,out] me Pointer to the host command manager instance.
 * @param[in] e Pointer to the event.
 */
void hostCommandManagerActivities_handleGetConfigResponse(
  hostCommandManager_t* me,
  const events_getConfigResponse_t* e);

/**
 * @brief Handles a GET_DIAGNOSTICS_RESPONSE event.
 *
 * @param[in,out] me Pointer to the host command manager instance.
 * @param[in] e Pointer to the event.
 */
void hostCommandManagerActivities_handleGetDiagnosticsResponse(
  hostCommandManager_t* me,
  const events_getDiagnosticsResponse_t* e);

/**
 * @brief Handles an RX_PACKET event.
 *
 * @param[in,out] me Pointer to the host command manager instance.
 * @param[in] e Pointer to the event.
 */
void hostCommandManagerActivities_handleRxPacket(hostCommandManager_t* me,
                                                 const events_rxPacket_t* e);

/**
 * @brief Handles a CAD_DETECTED event.
 *
 * @param[in,out] me Pointer to the host command manager instance.
 * @param[in] e Pointer to the event.
 */
void hostCommandManagerActivities_handleCadDetected(
  hostCommandManager_t* me,
  const events_cadDetected_t* e);

#endif /* HOST_COMMAND_MANAGER_ACTIVITIES_H */
