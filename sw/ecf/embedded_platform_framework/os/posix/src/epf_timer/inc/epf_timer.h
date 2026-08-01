/*******************************************************************************
 * @brief Platform-specific timer utilities.
 *
 * Provides thread-based timers supporting safe concurrent manipulation through
 * this API.
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

#ifndef EPF_TIMER_H
#define EPF_TIMER_H

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

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/**
 * @brief Converts milliseconds to nanoseconds.
 *
 * @param ms Time in milliseconds.
 * @return Equivalent time in nanoseconds as uint64_t.
 */
#define EPF_TIMER_MS_TO_NS(ms) ((uint64_t)(ms) * 1000000ULL)

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/
/**
 * @brief Callback type invoked on timer entry expiration.
 *
 * @note The callback executes in the timer thread context. Performing lengthy
 * or blocking operations may delay other timer entries or cause timing
 * deviations. Use this callback strictly as an ISR-like routine.
 */
typedef void (*EPF_timer_callback_t)(void);

/**
 * @brief Timer entry structure.
 */
typedef struct EPF_timer_entry_t
{
  struct EPF_timer_entry_t* next; /**< Link to the next timer in the list. */
  EPF_timer_callback_t cb; /**< User-defined callback executed on expiration. */
  uint64_t cnt;            /**< Down-counter until expiration (ticks). */
  uint64_t period; /**< Reload value for periodic timer, 0 for one-shot timer
                        (ticks). */
  bool is_linked;  /**< True if the timer is currently linked. */
} EPF_timer_entry_t;

/**
 * @brief Timer structure.
 */
typedef struct
{
  pthread_t ticker_tid;          /**< Ticker thread handler. */
  pthread_mutex_t mutex;         /**< Mutex protecting timer lists.
                                      See @ref timer_new_list. */
  volatile bool stop_ticker;     /**< Stop flag for ticker thread.
                                      volatile_use: context_interaction */
  EPF_timer_entry_t* armed_head; /**< Points to the list of armed timers. */
  EPF_timer_entry_t* new_head;   /**< Points to the list of newly armed timers.
                                      See @ref timer_new_list. */
} EPF_timer_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes a timer instance.
 *
 * @param me Pointer to the timer instance.
 * @param prio Ticker thread priority. Valid range: 1–99 (using SCHED_FIFO).
 */
void EPF_timer_init(EPF_timer_t* me, uint8_t prio);

/**
 * @brief Deinitializes a timer instance.
 *
 * @param me Pointer to the timer instance.
 */
void EPF_timer_deinit(EPF_timer_t* me);

/**
 * @brief Initializes a timer entry.
 *
 * @param[in,out] timer_entry Pointer to the timer entry instance.
 * @param cb Callback to invoke on timer expiration.
 */
void EPF_timeEvent_new(EPF_timer_entry_t* timer_entry, EPF_timer_callback_t cb);

/**
 * @brief Arms a timer entry.
 *
 * @param me Pointer to the timer instance.
 * @param[in,out] timer_entry Pointer to the timer entry instance.
 * @param start Time until first expiration (nano seconds).
 * @param period Periodic interval of timer expirations, 0 for one-shot timer
 * (nano seconds).
 */
void EPF_timer_arm(EPF_timer_t* me,
                   EPF_timer_entry_t* timer_entry,
                   uint64_t start,
                   uint64_t period);

/**
 * @brief Disarms a timer entry.
 *
 * @param me Pointer to the timer instance.
 * @param[in,out] timer_entry Pointer to the timer entry instance.
 * @return true if the timer was armed, false if it was already disarmed.
 */
bool EPF_timer_disarm(EPF_timer_t* me, EPF_timer_entry_t* timer_entry);

/**
 * @brief Rearms a timer entry with a new timeout.
 *
 * @param me Pointer to the timer instance.
 * @param[in,out] timer_entry Pointer to the timer entry instance.
 * @param[in] time Time until first expiration (nano seconds).
 * @return true if the timer was already armed, false if it was not.
 */
bool EPF_timer_rearm(EPF_timer_t* me,
                     EPF_timer_entry_t* timer_entry,
                     uint64_t time);

/**
 * @brief Gets the current counter value of a timer entry.
 *
 * @param me Pointer to the timer instance.
 * @param[in] timer_entry Pointer to the timer entry instance.
 * @return Current tick counter of the timer (nano seconds).
 */
uint64_t EPF_timer_currentCounter(EPF_timer_t* me,
                                  EPF_timer_entry_t* timer_entry);

#endif /* EPF_TIMER_H */
