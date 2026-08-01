/*******************************************************************************
 * @brief Active objects used for test validation.
 *
 * Active objects with event queues and basic publish/subscribe interaction.
 * Used by the tests to verify event routing and delivery behavior through
 * simple and isolated actors.
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

#ifndef ACTIVE_OBJECT_H
#define ACTIVE_OBJECT_H

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
#include "edf.h"

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/**
 * @brief Define the number of active objects.
 *
 * @note Global for test access purposes.
 */
#define ACTIVE_OBJECT_N_TOTAL 4U

/**
 * @brief Deterministic mapping between active object priority and subscribed
 * signal.
 *
 * Each active object (except the last) subscribes to exactly one signal. The
 * relationship is defined such that active object priority `prio` (starting
 * from 2) maps to signal `S1 + (prio - 2)`. The last active object subscribes
 * to all signals.
 */
#define ACTIVE_OBJECT_PRIO_TO_SIGNAL(prio) \
  ((activeObject_eventSignal_t)(S1 + ((prio) - 2U)))

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief Active object structure.
 *
 * @note Global for test access purposes.
 */
typedef struct
{
  EDF_activeObject_t super; /**< Base type. */
  bool event_received; /**< True if an event is received, false otherwise. */
} activeObject_t;

/**
 * @brief Event signals.
 */
typedef enum
{
  S1 = EDF_HSM_USER_SIGNAL, /**< Signal S1. */
  S2,                       /**< Signal S2. */
  S3,                       /**< Signal S3. */
  LAST_EVENT_SIGNAL,        /**< Reference for the last valid signal. */
} activeObject_eventSignal_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/**
 * @brief Active object instance.
 *
 * @note Global for test access purposes.
 */
extern activeObject_t activeObject[ACTIVE_OBJECT_N_TOTAL];

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes all the active objects.
 */
void activeObject_init(void);

/**
 * @brief Clear event received flag of all active objects.
 */
void activeObject_resetEventReceivedFlag(void);

#endif /* ACTIVE_OBJECT_H */
