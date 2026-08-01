/*******************************************************************************
 * @brief EDF fuzz example module.
 *
 * Provides a tiny dummy EDF application used only to demonstrate the host
 * fuzzing flow end-to-end.
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

#ifndef FUZZ_EXAMPLE_H
#define FUZZ_EXAMPLE_H

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */
#include <stdint.h>

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
 * @brief Event signals accepted by the fuzz example application.
 */
typedef enum
{
  FUZZ_EXAMPLE_PING_SIG = EDF_HSM_USER_SIGNAL,  //!< Ping event.
  FUZZ_EXAMPLE_START_SIG,                       //!< Start event.
  FUZZ_EXAMPLE_FINISH_SIG,                      //!< Finish event.
  FUZZ_EXAMPLE_RESET_SIG,                       //!< Reset event.
  FUZZ_EXAMPLE_SET_SIG,                         //!< Mutable setup event.
  FUZZ_EXAMPLE_STEP_SIG,                        //!< Mutable step event.
  FUZZ_EXAMPLE_LAST_SIG                         //!< Sentinel signal value.
} fuzzExample_eventSignal_t;

/**
 * @brief Mutable setup event accepted by the example application.
 */
typedef struct
{
  EDF_event_t super;     //!< EDF event base type.
  uint8_t slot;          //!< Slot to accept.
  uint8_t gate;          //!< Value gate used to enter collecting state.
  uint8_t target_count;  //!< Small target used by finish logic.
} fuzzExample_setEvent_t;

/**
 * @brief Mutable step event accepted by the example application.
 */
typedef struct
{
  EDF_event_t super;  //!< EDF event base type.
  uint8_t slot;       //!< Slot carried by the step.
  uint8_t value;      //!< Value carried by the step.
  uint8_t flags;      //!< Step-processing flags.
} fuzzExample_stepEvent_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initialize the fuzz example application and start its active object.
 */
void fuzzExample_init(void);

#endif /* FUZZ_EXAMPLE_H */
