/*******************************************************************************
 * @brief Event Driven Framework (EDF) time event.
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

#ifndef EDF_TIME_EVENT_H
#define EDF_TIME_EVENT_H

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
#include "edf_active_object.h"

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief Time event queue counter type.
 */
#if (EDF_TIME_EVENT_MAX_TICK == 1U)
typedef uint8_t EDF_timeEvent_cnt_t;
#elif (EDF_TIME_EVENT_MAX_TICK == 2U)
typedef uint16_t EDF_timeEvent_cnt_t;
#elif (EDF_TIME_EVENT_MAX_TICK == 4U)
typedef uint32_t EDF_timeEvent_cnt_t;
#endif

/**
 * @brief Time event structure.
 */
typedef struct EDF_timeEvent_t
{
  EDF_event_t super;            /**< Base type. */
  struct EDF_timeEvent_t* next; /**< Link to the next time event in the list. */
  EDF_activeObject_t* ao;       /**< Associated active object. */
  EDF_timeEvent_cnt_t cnt;      /**< Down-counter until expiration (ticks). */
  EDF_timeEvent_cnt_t period;   /**< Reload value for periodic timer, 0 for
                                     one-shot timer (ticks). */
  uint8_t tick_rate;            /**< Tick rate index. */
  bool is_linked; /**< True if the time event is currently linked. */
} EDF_timeEvent_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes a time event.
 *
 * @param[in,out] me Pointer to the time event instance.
 * @param[in] ao Pointer to the associated active object.
 * @param[in] sig Event signal.
 * @param[in] tick_rate Tick rate index.
 */
void EDF_timeEvent_new(EDF_timeEvent_t* me,
                       EDF_activeObject_t* ao,
                       EDF_event_signal_t sig,
                       uint8_t tick_rate);

/**
 * @brief Arms a time event.
 *
 * @param[in,out] me Pointer to the time event instance.
 * @param[in] start Time until first expiration (ticks).
 * @param[in] period Periodic interval of timer expirations, 0 for one-shot
 * timer (ticks).
 */
void EDF_timeEvent_arm(EDF_timeEvent_t* me,
                       EDF_timeEvent_cnt_t start,
                       EDF_timeEvent_cnt_t period);

/**
 * @brief Disarms a time event.
 *
 * @param[in,out] me Pointer to the time event instance.
 * @return true if the time event was armed, false if it was already disarmed.
 */
bool EDF_timeEvent_disarm(EDF_timeEvent_t* me);

/**
 * @brief Rearms a time event with a new timeout.
 *
 * @param[in,out] me Pointer to the time event instance.
 * @param[in] time Time until expiration (ticks).
 * @return true if the time event was already armed, false if it was not.
 */
bool EDF_timeEvent_rearm(EDF_timeEvent_t* me, EDF_timeEvent_cnt_t time);

/**
 * @brief Gets the current counter value of a time event.
 *
 * @param[in] me Pointer to the time event instance.
 * @return Current tick counter of the time event (ticks).
 */
EDF_timeEvent_cnt_t EDF_timeEvent_currentCounter(const EDF_timeEvent_t* me);

/**
 * @brief Processes time events for a given tick rate.
 *
 * @note This function shall be called periodically at the frequency
 * corresponding to the given tick rate.
 *
 * @param[in] tick_rate Tick rate index.
 */
void EDF_timeEvent_tick(uint8_t tick_rate);

#endif /* EDF_TIME_EVENT_H */
