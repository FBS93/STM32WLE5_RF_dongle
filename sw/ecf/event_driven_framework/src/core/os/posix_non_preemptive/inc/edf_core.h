/*******************************************************************************
 * @brief Event Driven Framework (EDF) core/port-specific.
 *
 * @copyright
 * Copyright (c) 2026 FBS93.
 * See the LICENSE file of this project for license details.
 * This notice shall be retained in all copies or substantial portions
 * of the software.
 *
 * @note
 * This file is a derivative work based on:
 * QP/C (c) Quantum Leaps, LLC.
 *
 * @warning
 * This software is provided "as is", without any express or implied warranty.
 * The user assumes all responsibility for its use and any consequences.
 ******************************************************************************/

#ifndef EDF_CORE_H
#define EDF_CORE_H

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
#include "edf_active_object.h"
#include "emf.h"

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/**
 * @brief Defines the variable storing the scheduler lock status.
 *
 * Not implemented in this port.
 */
#define EDF_CORE_SCHED_STATUS

/**
 * @brief Lock task scheduling.
 *
 * Not implemented in this port.
 *
 * @param[in] _ Unused parameter, kept for API compatibility.
 */
#define EDF_CORE_SCHED_LOCK(_) EMF_UTILS_UNUSED_PARAM(_)

/**
 * @brief Unlock task scheduling.
 *
 * Not implemented in this port.
 */
#define EDF_CORE_SCHED_UNLOCK() ((void)0)

/**
 * @brief Wait until an event is available for the active object.
 *
 * Not used in this port.
 *
 * @note This macro is called within a critical section.
 *
 * @param[in,out] me Unused parameter, kept for API compatibility.
 */
#define EDF_CORE_WAIT_FOR_EVENT(me) ((void)0)

/**
 * @brief Notify that a new event is available for the active object.
 *
 * @note Shall be called inside a critical section.
 *
 * @param[in,out] me Active object instance.
 */
#define EDF_CORE_NOTIFY_EVENT(me) \
  EMF_bitmask_setBit(EDF_core.ready_set.bitmask, (me)->prio);

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief EDF core structure.
 */
typedef struct
{
  EDF_activeObject_bitmask_t ready_set; /**< Bitmask of active objects ready to
                                             run. */
} EDF_core_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/**
 * @brief EDF core instance.
 *
 * @note Declared global to allow access from macros defined in this file.
 */
extern EDF_core_t EDF_core;

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

#endif /* EDF_CORE_H */
