/*******************************************************************************
 * @brief Simple active object with an HSM used for test validation.
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

#ifndef HSM_H
#define HSM_H

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

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief hsm active object structure.
 *
 * @note Global for test access purposes.
 */
typedef struct
{
  EDF_activeObject_t super; /**< Base type. */
  /**
   * Indicates whether the last transition is a self-transition
   * or a child-to-parent transition followed by parent's init to child.
   */
  bool self_transition;
} hsm_t;

/**
 * @brief Event signals.
 */
typedef enum
{
  TRANSITION = EDF_HSM_USER_SIGNAL, /**< Generates a new transition in the
                                         HSM. */
  LAST_EVENT_SIGNAL,                /**< Reference for the last valid signal. */
} hsm_eventSignal_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/**
 * @brief hsm active object instance.
 *
 * @note Global for test access purposes.
 */
extern hsm_t hsm;

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the hsm active object.
 */
void hsm_init(void);

/**
 * @defgroup HSM_States HSM States.
 * @brief HSM state handlers used for test validation.
 *
 * Each function implements a state of the hierarchical state machine.
 *
 * @note Global for test access purposes.
 * @{
 */
EDF_hsm_stateReturn_t initialTransition(hsm_t* me, const EDF_event_t* e);
EDF_hsm_stateReturn_t S1(hsm_t* me, const EDF_event_t* e);
EDF_hsm_stateReturn_t S11(hsm_t* me, const EDF_event_t* e);
EDF_hsm_stateReturn_t S111(hsm_t* me, const EDF_event_t* e);
EDF_hsm_stateReturn_t S1111(hsm_t* me, const EDF_event_t* e);
EDF_hsm_stateReturn_t S11111(hsm_t* me, const EDF_event_t* e);
EDF_hsm_stateReturn_t S2(hsm_t* me, const EDF_event_t* e);
EDF_hsm_stateReturn_t S21(hsm_t* me, const EDF_event_t* e);
EDF_hsm_stateReturn_t S211(hsm_t* me, const EDF_event_t* e);
EDF_hsm_stateReturn_t S2111(hsm_t* me, const EDF_event_t* e);
EDF_hsm_stateReturn_t S2112(hsm_t* me, const EDF_event_t* e);
EDF_hsm_stateReturn_t S3(hsm_t* me, const EDF_event_t* e);
EDF_hsm_stateReturn_t S31(hsm_t* me, const EDF_event_t* e);
EDF_hsm_stateReturn_t S311(hsm_t* me, const EDF_event_t* e);
EDF_hsm_stateReturn_t S4(hsm_t* me, const EDF_event_t* e);
EDF_hsm_stateReturn_t S41(hsm_t* me, const EDF_event_t* e);
/** @} */  // HSM_States

#endif /* HSM_H */
