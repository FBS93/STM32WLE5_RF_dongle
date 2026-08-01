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
#include <pthread.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "ebf.h"
#include "eaf.h"

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/**
 * @brief Defines the variable storing the scheduler lock status.
 *
 * Not implemented in this port. See @ref scheduler_locking_not_implemented.
 */
#define EDF_CORE_SCHED_STATUS

/**
 * @brief Lock task scheduling.
 *
 * Not implemented in this port. See @ref scheduler_locking_not_implemented.
 *
 * @param[in] _ Unused parameter, kept for API compatibility.
 */
#define EDF_CORE_SCHED_LOCK(_) EMF_UTILS_UNUSED_PARAM(_)

/**
 * @brief Unlock task scheduling.
 *
 * Not implemented in this port. See @ref scheduler_locking_not_implemented.
 */
#define EDF_CORE_SCHED_UNLOCK() ((void)0)

/**
 * @anchor scheduler_locking_not_implemented
 * @par Scheduler locking not implemented
 *
 * Scheduler locking (used inside @ref EDF_activeObject_publish()) is NOT
 * implemented in this port. This means that event multicasting is NOT atomic,
 * so thread preemption CAN happen during that time, especially when a
 * low-priority thread publishes events to higher-priority threads. This can
 * lead to (occasionally) unexpected event sequences.
 */

/**
 * @brief Wait until an event is available for the active object.
 *
 * @note This macro is called within a critical section.
 *
 * @param[in,out] me Active object instance.
 */
#define EDF_CORE_WAIT_FOR_EVENT(me)                              \
  do                                                             \
  {                                                              \
    while ((me)->e_queue.front_e == NULL)                        \
    {                                                            \
      EAF_ASSERT_IN_CRITICAL_SECTION(EBF_critSecNestCnt == 1);   \
      --EBF_critSecNestCnt;                                      \
      (void)pthread_cond_wait(&(me)->e_cond, &EBF_critSecMutex); \
      EAF_ASSERT_IN_CRITICAL_SECTION(EBF_critSecNestCnt == 0);   \
      ++EBF_critSecNestCnt;                                      \
    }                                                            \
  } while (false)

/**
 * @brief Notify that a new event is available for the active object.
 *
 * @note Shall be called inside a critical section.
 *
 * @param[in,out] me Active object instance.
 */
#define EDF_CORE_NOTIFY_EVENT(me) (void)pthread_cond_signal(&(me)->e_cond)

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

#endif /* EDF_CORE_H */
