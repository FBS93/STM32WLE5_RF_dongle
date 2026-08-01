/*******************************************************************************
 * @brief Time manager active object used for test validation.
 *
 * Provides time management and time events used by the tests to verify timing
 * accuracy, deterministic expiration counters and correct delivery of time
 * events through the publish–subscribe mechanism.
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

#ifndef TIME_MGR_H
#define TIME_MGR_H

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
#include "edf.h"

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/**
 * @brief Index of 1ms tick rate.
 */
#define TICK_RATE_IDX_1_MS 0U

/**
 * @brief Size of timeMgr event buffer
 *
 * The buffer must be large enough to hold the maximum number of consecutive
 * posted events in the worst-case scenario without preemption. This ensures
 * the test can also be used in non-preemptive core/port configurations.
 */
#define EVENT_BUFF_SIZE 50U

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief timeMgr active object structure.
 *
 * @note Global for test access purposes.
 */
typedef struct
{
  EDF_activeObject_t super;                  /**< Base type. */
  EDF_event_ptr event_buff[EVENT_BUFF_SIZE]; /**< Event buffer. */
  uint32_t periodic_5ms_cnt;       /**< Counter of periodic 5ms time event
                                        expirations. */
  uint32_t periodic_100ms_cnt;     /**< Counter of periodic 100ms time event
                                        expirations. */
  uint32_t periodic_5ms_100ms_cnt; /**< Counter of periodic 5ms-100ms time event
                                        expirations. */
  uint32_t one_shot_100ms_cnt;     /**< Counter of one-shot 100ms time event
                                        expirations. */
} timeMgr_t;

/**
 * @brief Event signals.
 */
typedef enum
{
  PERIODIC_TIMER_5_MS = EDF_HSM_USER_SIGNAL, /**< Periodic 5ms time event
                                                  signal. */
  PERIODIC_TIMER_100_MS,      /**< Periodic 100ms time event signal. */
  PERIODIC_TIMER_5_MS_100_MS, /**< Periodic 5ms-100ms time event signal. */
  ONE_SHOT_TIMER_100_MS,      /**< One-shot 100ms time event signal. */
  VERIFY,                     /**< Verification event. */
  LAST_EVENT_SIGNAL,          /**< Reference for the last valid signal. */
} timeMgr_eventSignal_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/**
 * @brief timeMgr active object instance.
 *
 * @note Global for test access purposes.
 */
extern timeMgr_t timeMgr;

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the timeMgr active object and timers.
 */
void timeMgr_init(void);

/**
 * @brief Clear all timer expiration counters.
 */
void timeMgr_clearExpirationCnt(void);

/**
 * @brief Arm periodic timer of 5ms.
 */
void timeMgr_armPeriodicTimer5ms(void);

/**
 * @brief Disarm periodic timer of 5ms.
 */
void timeMgr_disarmPeriodicTimer5ms(void);

/**
 * @brief Arm periodic timer of 100ms.
 */
void timeMgr_armPeriodicTimer100ms(void);

/**
 * @brief Disarm periodic timer of 100ms.
 */
void timeMgr_disarmPeriodicTimer100ms(void);

/**
 * @brief Arm periodic timer; first expiration  at 5ms, then every 100ms.
 */
void timeMgr_armPeriodicTimer5ms100ms(void);

/**
 * @brief Disarm periodic timer; first expiration  at 5ms, then every 100ms.
 */
void timeMgr_disarmPeriodicTimer5ms100ms(void);

/**
 * @brief Arm one-shot timer of 100ms.
 */
void timeMgr_armOneShotTimer100ms(void);

/**
 * @brief Disarm one-shot timer of 100ms.
 */
void timeMgr_disarmOneShotTimer100ms(void);

#endif /* TIME_MGR_H */
