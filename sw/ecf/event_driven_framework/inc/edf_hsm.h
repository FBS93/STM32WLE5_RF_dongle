/*******************************************************************************
 * @brief Event Driven Framework (EDF) HSM.
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

#ifndef EDF_HSM_H
#define EDF_HSM_H

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */
#include <stdint.h>
#include <stdbool.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "edf_event.h"

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/**
 * @brief Cast to EDF_hsm_t base type.
 *
 * @param[in] hsm HSM instance.
 */
#define EDF_HSM_UPCAST(hsm) ((EDF_hsm_t*)(hsm))

/**
 * @brief Sets handled return value from state handler.
 *
 * Use when the event is fully handled by the current state.
 */
#define EDF_HSM_RET_HANDLED() ((EDF_hsm_stateReturn_t)RET_HANDLED)

/**
 * @brief Sets unhandled return value from state handler.
 *
 * Event handling is implemented but not handled;
 * so event will bubble up to superstate.
 */
#define EDF_HSM_RET_UNHANDLED() ((EDF_hsm_stateReturn_t)RET_UNHANDLED)

/**
 * @brief Sets the superstate and return value from state handler.
 *
 * This macro shall be used in the default case of the event-handling
 * switch of all application state handlers. Specifies the superstate
 * of the current state handler.
 *
 * @param[in] super Superstate handler.
 */
#define EDF_HSM_RET_SUPER(super)                                         \
  ((EDF_HSM_UPCAST(me))->temp_state = ((EDF_hsm_stateHandler_t)(super)), \
   (EDF_hsm_stateReturn_t)RET_SUPER)

/**
 * @brief Sets the transition target and return value from a state handler.
 *
 * This macro shall be used when a state handler executes a state
 * transition. Specifies the target state that the HSM should enter.
 *
 * @param[in] target Target state handler for the transition.
 */
#define EDF_HSM_RET_TRAN(target)                                          \
  ((EDF_HSM_UPCAST(me))->temp_state = ((EDF_hsm_stateHandler_t)(target)), \
   (EDF_hsm_stateReturn_t)RET_TRANSITION)

/**
 * @brief Internal framework reserved signals for HSM management.
 *
 * The order and numeric values of these signals are important
 * and tightly coupled to the HSM implementation.
 */
/// @{
/**
 * @brief Forces a state handler to return its superstate.
 *
 * This signal must never be handled explicitly. It always falls into
 * the default case of the state switch and returns
 * @ref EDF_HSM_RET_SUPER(super).
 */
#define EDF_HSM_EMPTY_SIGNAL ((EDF_event_signal_t)0U)

/**
 * @brief Entry action signal.
 *
 * A state may implement this handler to perform its entry action.
 * The only valid return is @ref EDF_HSM_RET_HANDLED(). If no entry action
 * exists,
 * no handling of the signal should be implemented.
 */
#define EDF_HSM_ENTRY_SIGNAL ((EDF_event_signal_t)1U)

/**
 * @brief Exit action signal.
 *
 * A state may implement this handler to perform its exit action.
 * The only valid return is @ref EDF_HSM_RET_HANDLED(). If no exit action
 * exists,
 * no handling of the signal should be implemented.
 */
#define EDF_HSM_EXIT_SIGNAL ((EDF_event_signal_t)2U)

/**
 * @brief Initial transition signal.
 *
 * Not intended to execute any action; the only valid return is
 * @ref EDF_HSM_RET_TRAN(target), or leave the handling of the signal
 * unimplemented.
 */
#define EDF_HSM_INIT_SIGNAL ((EDF_event_signal_t)3U)

/**
 * @brief First valid signal value available for user-defined events.
 *
 * All signals from this value upwards are reserved for application use.
 */
#define EDF_HSM_USER_SIGNAL ((EDF_event_signal_t)4U)
/// @}

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief Internal return codes from state/action handlers.
 *
 * @note For EDF library internal use only. Users must use the public macros:
 * - EDF_HSM_RET_HANDLED
 * - EDF_HSM_RET_UNHANDLED
 * - EDF_HSM_RET_SUPER
 * - EDF_HSM_RET_TRAN
 */
typedef enum
{
  RET_HANDLED,    /*!< Event handled. */
  RET_UNHANDLED,  /*!< Event not handled, propagate to superstate if any,
                       otherwise discard (top state). */
  RET_SUPER,      /*!< Explicit request to propagate event to superstate. */
  RET_TRANSITION, /*!< Transition taken. */
  RET_IGNORED     /*!< Reserved, used only in top state. */
} EDF_hsm_stateReturn_t;

/**
 * @brief State handler function type for EDF HSM.
 *
 * @note The parameter @p me is void* to support C-style "inheritance":
 * internally in the EDF library it is treated as EDF_hsm_t, while the user
 * can implement state handlers using their specific AO type.
 * The user AO's first member shall be EDF_hsm_t. This allows state handlers
 * to access the full AO structure while keeping the EDF HSM implementation
 * generic. When the EDF library calls the state handler of the user's AO HSM,
 * an implicit cast from EDF_hsm_t* to the user AO type is performed.
 *
 * @param[in] me Pointer to the HSM instance of the active object
 *               currently dispatching the event.
 * @param[in] e Pointer to the event being dispatched.
 * @return EDF_hsm_stateReturn_t value indicating how the event was handled.
 */
typedef EDF_hsm_stateReturn_t (*EDF_hsm_stateHandler_t)(void* me,
                                                        const EDF_event_t* e);

/**
 * @brief HSM structure.
 */
typedef struct
{
  EDF_hsm_stateHandler_t current_state; /*!< Currently active state handler. */
  EDF_hsm_stateHandler_t temp_state;    /*!< Temporary state handler; used to
                                             manage the HSM functionalities. */
} EDF_hsm_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes HSM.
 *
 * @param[in,out] me Pointer to the HSM instance.
 * @param[in] initial Initial state handler function.
 */
void EDF_hsm_init(EDF_hsm_t* me, EDF_hsm_stateHandler_t initial);

/**
 * @brief Starts HSM.
 *
 * @param[in,out] me Pointer to the HSM instance.
 * @param[in] e Pointer to the init event provided to the initial state.
 */
void EDF_hsm_start(EDF_hsm_t* me, const EDF_event_t* e);

/**
 * @brief Dispatches an event to the HSM.
 *
 * @param[in,out] me Pointer to the HSM instance.
 * @param[in] e Pointer to the event to dispatch.
 */
void EDF_hsm_dispatch(EDF_hsm_t* me, const EDF_event_t* e);

/**
 * @brief Returns the current active state of the HSM.
 *
 * @param[in] me Pointer to the HSM instance.
 * @return Current active state handler.
 */
EDF_hsm_stateHandler_t EDF_hsm_getCurrentState(const EDF_hsm_t* me);

/**
 * @brief Top state handler of the HSM.
 *
 * Represents the superstate that contains all other states of the HSM.
 *
 * @note Users should use this as the parent of their HSM’s superstates.
 *
 * @param[in] me Pointer to the HSM instance.
 * @param[in] e Pointer to the event being dispatched.
 * @return Always returns RET_IGNORED.
 */
EDF_hsm_stateReturn_t EDF_hsm_top(const EDF_hsm_t* me, const EDF_event_t* e);

#endif /* EDF_HSM_H */
