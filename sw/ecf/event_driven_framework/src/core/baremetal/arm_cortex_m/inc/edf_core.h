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
#include <stdint.h>

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
 * @brief Indicates whether the CPU is currently executing in an ISR context.
 *
 * Evaluates to true if the Interrupt Program Status Register (IPSR) is
 * non-zero, meaning the processor is handling an interrupt.
 */
#define EDF_CORE_IS_IN_ISR() (EDF_core_getIPSR() != 0U)

/**
 * @brief Defines the local variable to store the scheduler lock status.
 *
 * @note Intended for use with macros that manage scheduler locking.
 */
#define EDF_CORE_SCHED_STATUS EDF_core_schedStatus_t sched_status;

/**
 * @brief Constant representing the unlocked scheduler state.
 */
#define EDF_CORE_SCHED_NOT_LOCKED 0xFFU

/**
 * @brief Locks the scheduler up to a specified ceiling priority.
 *
 * Prevents task preemption by active objects with priority
 * at or below the given ceiling.
 *
 * @param[in] ceiling Lock ceiling priority.
 */
#define EDF_CORE_SCHED_LOCK(ceiling)                  \
  do                                                  \
  {                                                   \
    if (EDF_CORE_IS_IN_ISR())                         \
    {                                                 \
      sched_status = EDF_CORE_SCHED_NOT_LOCKED;       \
    }                                                 \
    else                                              \
    {                                                 \
      sched_status = EDF_core_lockScheduler(ceiling); \
    }                                                 \
  } while (false)

/**
 * @brief Restores scheduler state after a previous lock.
 *
 * Re-enables task preemption if it was previously locked.
 */
#define EDF_CORE_SCHED_UNLOCK()                    \
  do                                               \
  {                                                \
    if (sched_status != EDF_CORE_SCHED_NOT_LOCKED) \
    {                                              \
      EDF_core_unlockScheduler(sched_status);      \
    }                                              \
  } while (false)

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
#define EDF_CORE_NOTIFY_EVENT(me)                               \
  do                                                            \
  {                                                             \
    EMF_bitmask_setBit(EDF_core.ready_set.bitmask, (me)->prio); \
    if (!EDF_CORE_IS_IN_ISR())                                  \
    {                                                           \
      if (EDF_core_schedule() != 0U)                            \
      {                                                         \
        EDF_core_activate();                                    \
      }                                                         \
    }                                                           \
  } while (false)

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
  uint8_t active_priority; /**< Priority of the currently running active
                                object. */
  uint8_t next_priority; /**< Priority of the next active object to execute. */
  uint8_t active_preemption_threshold; /**< Current preemption threshold. */
  uint8_t lock_ceiling;                /**< Current scheduler lock ceiling. */
} EDF_core_t;

/**
 * @brief Type representing the scheduler lock state.
 */
typedef uint8_t EDF_core_schedStatus_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/**
 * @brief EDF core instance.
 *
 * @note Declared global to allow port-specific implementations
 * and to be accessed from macros defined in this file.
 */
extern EDF_core_t EDF_core;

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @anchor internal_access_via_macros_only
 * @par Internal access via macros only
 *
 * The following functions are not meant to be called directly from external
 * modules. They are exposed only for documentation purposes and to allow
 * access through the public macros defined in this header.
 */

/**
 * @brief Raises the scheduler lock ceiling.
 *
 * Prevents preemption by active objects with a priority at or below
 * the specified ceiling.
 *
 * @param[in] ceiling New lock ceiling priority.
 * @return Previous scheduler lock state.
 */
EDF_core_schedStatus_t EDF_core_lockScheduler(uint8_t ceiling);

/**
 * @brief Restores the previous scheduler lock state.
 *
 * @param[in] prev_ceiling Previous scheduler lock state.
 */
void EDF_core_unlockScheduler(EDF_core_schedStatus_t prev_ceiling);

/**
 * @brief Determine the next active object eligible for execution.
 *
 * Evaluates the current set of ready active objects and selects
 * the highest-priority one that can preempt the currently running
 * context, considering both the scheduler lock and preemption threshold.
 *
 * @note This function shall be called within a critical section.
 *
 * @return The priority of the next active object to run,
 * or 0 if no activation is required.
 */
uint8_t EDF_core_schedule(void);

/**
 * @brief Evaluates scheduling after an active object completes execution.
 *
 * Determines if another active object should be activated based on
 * current priorities and preemption thresholds, following the
 * completion of the given active object's event processing.
 *
 * @note This function shall be called within a critical section.
 *
 * @param[in] act Pointer to the active object that has just run.
 * @param[in] pthre_in Preemption threshold before the current activation,
 * in other words, the preemption threshold of the AO preempted by the currently
 * running AO.
 * @return The priority of the next active object to run,
 * or 0 if no activation is required.
 */
uint8_t EDF_core_scheduleAfter(const EDF_activeObject_t* act, uint8_t pthre_in);

/**
 * @brief Activates ready active objects according to their priorities.
 *
 * Executes all active objects that are ready to run and whose priorities
 * allow preemption of the last context.
 *
 * @note This function shall be called within a critical section.
 */
void EDF_core_activate(void);

/**
 * @brief PendSV exception handler.
 *
 * Handles context switching. Prepares the transition from Handler mode to
 * Thread mode to execute @ref EDF_core_activate and then
 * @ref EDF_core_preemptedContextReturn.
 *
 * @note Implemented in assembly within @c edf_core.S.
 */
extern void PendSV_Handler(void);

/**
 * @brief Preempted context return helper.
 *
 * Generates an NMI to safely return to preempted execution context.
 *
 * @note Implemented in assembly within @c edf_core.S.
 */
extern void EDF_core_preemptedContextReturn(void);

/**
 * @brief NMI exception handler.
 *
 * Completes the return sequence to Thread mode, re-enabling
 * interrupts and restoring the preempted execution context.
 *
 * @note Implemented in assembly within @c edf_core.S.
 */
extern void NMI_Handler(void);

/**
 * @brief Disable FPU context stacking.
 *
 * @note Implemented in assembly within @c edf_core.S.
 */
extern void EDF_core_disableFPUContext(void);

/**
 * @brief Read the current IPSR value.
 *
 * Returns the content of the Interrupt Program Status Register (IPSR).
 *
 * @note Implemented in assembly within @c edf_core.S.
 *
 * @return  0 when the processor is executing in thread mode,
 * or a non-zero when running within an interrupt or exception context.
 */
extern uint32_t EDF_core_getIPSR(void);

/**
 * @brief Workaround for ARM Erratum 838869.
 *
 * @note Implemented in assembly within @c edf_core.S.
 */
extern void EDF_core_ARMErratum838869(void);

#endif /* EDF_CORE_H */
