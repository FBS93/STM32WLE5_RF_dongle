/*******************************************************************************
 * @brief RF manager active object public interface.
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

#ifndef RF_MANAGER_H
#define RF_MANAGER_H

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
#include "edf.h"

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

/** @brief RF manager active object and its persistent state. */
typedef struct
{
  EDF_activeObject_t super;               //!< EDF active-object base.
  rfTypes_configuration_t configuration;  //!< Current stored configuration.
  bool configuration_valid;              //!< Whether configuration established.
  rfTypes_operation_t active_operation;  //!< Host-visible RF operation.
} rfManager_t;

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
void rfManager_init(rfManager_t* me);

/**
 * @brief Initializes and activates the active-object-owned HAL and resources,
 * enabling event-producing sources at the end of EDF_onStartup() in accordance
 * with the EDF startup contract.
 */
void rfManager_startup(void);

#endif /* RF_MANAGER_H */
