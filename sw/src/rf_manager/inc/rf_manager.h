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
/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */
#include "edf_active_object.h"

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
  EDF_activeObject_t super;                 //!< EDF active-object base.
  rfTypes_configuration_t configuration;    //!< Current stored configuration.
  rfTypes_operation_t active_operation;     //!< Host-visible RF operation.
} rfManager_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes an RF manager active object.
 *
 * @param[in,out] me Pointer to the active object instance.
 */
void rfManager_init(rfManager_t* me);

#endif /* RF_MANAGER_H */
