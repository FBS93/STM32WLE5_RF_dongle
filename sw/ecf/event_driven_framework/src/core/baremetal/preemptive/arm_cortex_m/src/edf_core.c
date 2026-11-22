/*******************************************************************************
 * @brief Event Driven Framework (EDF) core/port-specific implementation.
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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */
#include <stdint.h>
#include <stddef.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "edf.h"
#include "edf_core.h"
#include "edf_framework.h"
#include "edf_active_object.h"
#include "ebf.h"
#include "eaf.h"
#include "emf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief System Handler Priority Registers
 *
 * Array-like register block (e.g. use SCB_SYSPRI[2]).
 *
 * volatile_use: hardware_interaction
 */
#define SCB_SYSPRI ((uint32_t volatile*)0xE000ED18U)

/**
 * @brief Coprocessor Access Control Register.
 *
 * volatile_use: hardware_interaction
 */
#define SCB_CPACR *((uint32_t volatile*)0xE000ED88U)

/**
 * @brief Floating-Point Context Control Register.
 *
 * volatile_use: hardware_interaction
 */
#define FPU_FPCCR *((uint32_t volatile*)0xE000EF34U)

/**
 * @brief Interrupt Control and State Register.
 *
 * volatile_use: hardware_interaction
 */
#define SCB_ICSR *((uint32_t volatile*)0xE000ED04U)

/**
 * @brief PendSV priority position within SCB_SYSPRI[2] register.
 */
#define SCB_SYSPRI_PENDSV_PRI 16U

/**
 * @brief CP10 full-access position within SCB_CPACR register.
 */
#define SCB_CPACR_CP10_FULL_ACCESS 20U

/**
 * @brief CP11 full-access position within SCB_CPACR register.
 */
#define SCB_CPACR_CP11_FULL_ACCESS 22U

/**
 * @brief PendSV set-pending position within SCB_ICSR register.
 */
#define SCB_ICSR_PENDSVSET 28U

/**
 * @brief Automatic State Preservation Enable position within FPU_FPCCR
 * register.
 */
#define FPU_FPCCR_ASPEN 31U

/**
 * @brief Lazy State Preservation Enable position within FPU_FPCCR register.
 */
#define FPU_FPCCR_LSPEN 30U

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/**
 * @brief Define static file name string for asserts.
 */
EAF_DEFINE_THIS_FILE(__FILE__);

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

EDF_core_t EDF_core;

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/**
 * @brief EDF core initialization.
 */
void EDF_core_init(void);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

void EDF_core_init(void)
{
  // Set PendSV to priority 0xFF (lowest).
  SCB_SYSPRI[2] = (SCB_SYSPRI[2] | (0xFFU << SCB_SYSPRI_PENDSV_PRI));

#ifdef __ARM_FP  // If FPU available...
  // Make sure that the FPU is enabled by setting CP10 & CP11 full access.
  SCB_CPACR |=
    (3UL << SCB_CPACR_CP10_FULL_ACCESS) | (3UL << SCB_CPACR_CP11_FULL_ACCESS);

  // FPU automatic state preservation (ASPEN) lazy stacking (LSPEN).
  FPU_FPCCR |= (1U << FPU_FPCCR_LSPEN) | (1U << FPU_FPCCR_ASPEN);
#endif
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

EDF_core_schedStatus_t EDF_core_lockScheduler(uint8_t ceiling)
{
  EDF_core_schedStatus_t prev_ceiling;

  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(!EDF_CORE_IS_IN_ISR());

  // Assume scheduler NOT locked.
  prev_ceiling = (EDF_core_schedStatus_t)EDF_CORE_SCHED_NOT_LOCKED;

  // Raising the lock ceiling?
  if (ceiling > EDF_core.lock_ceiling)
  {
    // Get previous status of the lock.
    prev_ceiling = (EDF_core_schedStatus_t)EDF_core.lock_ceiling;

    // Set new status of the lock.
    EDF_core.lock_ceiling = ceiling;
  }

  EBF_CRITICAL_SECTION_EXIT();

  return prev_ceiling;
}

void EDF_core_unlockScheduler(EDF_core_schedStatus_t prev_ceiling)
{
  // Has the scheduler been actually locked by the last
  // EDF_core_lockScheduler()?
  if (prev_ceiling != EDF_CORE_SCHED_NOT_LOCKED)
  {
    EBF_CRITICAL_SECTION_ENTRY();

    EAF_ASSERT_IN_CRITICAL_SECTION(!EDF_CORE_IS_IN_ISR());
    EAF_ASSERT_IN_CRITICAL_SECTION(EDF_core.lock_ceiling > prev_ceiling);

    // Restore the previous lock ceiling.
    EDF_core.lock_ceiling = prev_ceiling;

    // Check if any AOs should be run after unlocking the scheduler.
    // Preemption needed?
    if (EDF_core_schedule() != 0U)
    {
      EDF_core_activate();  // Activate any unlocked AOs.
    }

    EBF_CRITICAL_SECTION_EXIT();
  }
}

uint8_t EDF_core_schedule(void)
{
  uint8_t prio;

  /**
   * @note This function is called within a critical section.
   */

  prio = 0U;  // Assume no activation needed.
  if (EMF_bitmask_isAnySet(EDF_core.ready_set.bitmask,
                           sizeof(EDF_core.ready_set)))
  {
    // Find the highest-prio AO with non-empty event queue.
    prio = EMF_bitmask_findMax(EDF_core.ready_set.bitmask,
                               sizeof(EDF_core.ready_set));

    // Is the AO's priority below the active preemption-threshold?
    if (prio <= EDF_core.active_preemption_threshold)
    {
      prio = 0U;  // No activation needed.
    }
    else
    {
      // Is the AO's priority below the lock-ceiling?
      if (prio <= EDF_core.lock_ceiling)
      {
        prio = 0U;  // No activation needed.
      }
      else
      {
        EDF_core.next_priority = prio;  // Next AO to run.
      }
    }
  }

  return prio;
}

uint8_t EDF_core_scheduleAfter(const EDF_activeObject_t* act, uint8_t pthre_in)
{
  uint8_t prio;

  /**
   * @note This function is called within a critical section.
   */

  prio = act->prio;

  // Empty queue?
  if (act->e_queue.front_e == NULL)
  {
    EMF_bitmask_clearBit(EDF_core.ready_set.bitmask, prio);
  }

  if (!EMF_bitmask_isAnySet(EDF_core.ready_set.bitmask,
                            sizeof(EDF_core.ready_set)))
  {
    prio = 0U;  // No activation needed.
  }
  else
  {
    // Find new highest-prio AO ready to run.
    prio = EMF_bitmask_findMax(EDF_core.ready_set.bitmask,
                               sizeof(EDF_core.ready_set));

    // Is the new priority below the initial preemption-threshold?
    if (prio <= pthre_in)
    {
      prio = 0U;  // No activation needed.
    }
    else
    {
      // Is the AO's priority below the lock preemption-threshold?
      if (prio <= EDF_core.lock_ceiling)
      {
        prio = 0U;  // No activation needed.
      }
    }
  }

  return prio;
}

void EDF_core_activate(void)
{
  uint8_t next_prio;
  uint8_t prio_in;
  uint8_t last_prio;
  uint8_t pthre_in;
  uint8_t next_pthre;
  EDF_activeObject_t* ao;
  EDF_event_t* e;

  /**
   * @note This function is called within a critical section.
   */

  EAF_ASSERT_IN_CRITICAL_SECTION(EDF_core.active_priority <=
                                 EDF_MAX_ACTIVE_OBJECT);
  EAF_ASSERT_IN_CRITICAL_SECTION(
    (EDF_core.next_priority > 0U) &&
    (EDF_core.next_priority <= EDF_MAX_ACTIVE_OBJECT));

  prio_in = EDF_core.active_priority;  // Save initial priority.
  next_prio = EDF_core.next_priority;  // Next priority to run.
  EDF_core.next_priority = 0U;         // Clear for the next time.
  last_prio = prio_in;

  pthre_in = 0U;  // Assume preempting the idle thread.
  if (prio_in != 0U)
  {
    // Preempting NOT the idle thread
    ao = EDF_framework.ao_registry[prio_in];

    // The AO must be registered.
    EAF_ASSERT_IN_CRITICAL_SECTION(ao != NULL);

    // Save initial preemption threshold.
    pthre_in = ao->pthre;
  }

  /**
   * Loop until no more ready-to-run AOs of higher preemption threshold than the
   * initial.
   */
  do
  {
    // Get next active object to be executed.
    ao = EDF_framework.ao_registry[next_prio];

    // The AO must be registered.
    EAF_ASSERT_IN_CRITICAL_SECTION(ao != NULL);

    // Get next preemption threshold.
    next_pthre = ao->pthre;

    // Set new active priority and preemption-threshold.
    EDF_core.active_priority = next_prio;
    EDF_core.active_preemption_threshold = next_pthre;

    // Changing context?
    if (next_prio != last_prio)
    {
      EDF_onContextSwitch(EDF_framework.ao_registry[last_prio], ao);
      last_prio = next_prio;  // Update last priority.
    }

    // Exit critical section to execute an active object.
    EBF_CRITICAL_SECTION_EXIT();

    // Get event for the AO.
    e = (EDF_event_t*)EDF_activeObject_get(
      ao);  // Typecast to discard const qualifier.

    // Dispatch event.
    EDF_hsm_dispatch(&ao->super, e);
    EDF_event_gc(e);

    // Entry critical section again after active object execution.
    EBF_CRITICAL_SECTION_ENTRY();

    // Determine the next highest-priority AO ready to run.
    next_prio =
      EDF_core_scheduleAfter(ao, pthre_in);  // Get next priority, if any.

  } while (next_prio != 0U);

  // Restore the active priority and preemption-threshold.
  EDF_core.active_priority = prio_in;
  EDF_core.active_preemption_threshold = pthre_in;

  // Resuming an active object?
  if (prio_in != 0U)
  {
    EDF_onContextSwitch(EDF_framework.ao_registry[last_prio],
                        EDF_framework.ao_registry[prio_in]);
  }
  else
  {
    // Resuming priority == 0 --> idle
    EDF_onContextSwitch(EDF_framework.ao_registry[last_prio], NULL);
  }
}

void EDF_init(void)
{
  EBF_CRITICAL_SECTION_ENTRY();

  // Clear EDF_core instance.
  EMF_utils_clear(&EDF_core, sizeof(EDF_core));

  // Setup the scheduler as initially locked and not running.
  EDF_core.lock_ceiling = (EDF_MAX_ACTIVE_OBJECT + 1U);  // Scheduler locked.

  EBF_CRITICAL_SECTION_EXIT();

  EDF_framework_init();
  EDF_core_init();
}

int EDF_run(void)
{
  EDF_onStartup();  // User-specific startup callback.

  EBF_CRITICAL_SECTION_ENTRY();

#ifdef __ARM_FP  // If FPU available...
  EDF_core_disableFPUContext();
#endif

  EDF_core.lock_ceiling = 0U;  // Unlock the scheduler.

  // Officially switch to the idle context.
  EDF_onContextSwitch(NULL, NULL);

  // Activate AOs to process events posted so far.
  if (EDF_core_schedule() != 0U)
  {
    EDF_core_activate();
  }

  EBF_CRITICAL_SECTION_EXIT();

  // Idle loop...
  while (1)
  {
    EDF_onIdle();  // User-specific idle callback.
  }

  return 0;
}

void EDF_stop(void)
{
  EDF_onShutdown();  // User-specific shutdown callback.
}

void EDF_activeObject_start(EDF_activeObject_t* me,
                            EDF_activeObject_prio_t prio,
                            EDF_event_ptr* q_storage,
                            EDF_eventQueue_cnt_t q_len,
                            void* stack_storage,
                            uint_fast16_t stack_size,
                            const EDF_event_t* e)
{
  EMF_UTILS_UNUSED_PARAM(stack_storage);  // Not needed in this core.
  EMF_UTILS_UNUSED_PARAM(stack_size);     // Not needed in this core.

  me->prio = (uint8_t)(prio & 0xFFU);  // Base priority.
  me->pthre = (uint8_t)(prio >> 8U);   // preemption-threshold

  EDF_activeObject_register(me);
  EDF_eventQueue_init(&me->e_queue, q_storage, q_len);
  EDF_hsm_start(&me->super,
                e);  // Execute top-most initial transition of the HSM.

  EBF_CRITICAL_SECTION_ENTRY();

  // Check if this AO needs to be scheduled.
  if (EDF_core_schedule() != 0U)
  {
    EDF_core_activate();
  }

  EBF_CRITICAL_SECTION_EXIT();
}

void EDF_activeObject_setAttr(EDF_activeObject_t* me,
                              uint32_t attr1,
                              const void* attr2)
{
  EMF_UTILS_UNUSED_PARAM(me);
  EMF_UTILS_UNUSED_PARAM(attr1);
  EMF_UTILS_UNUSED_PARAM(attr2);

  // Not used.
}

EBF_WEAK void EDF_onStartup(void)
{
}

EBF_WEAK void EDF_onShutdown(void)
{
}

void EDF_ISRExit(void)
{
  EBF_CRITICAL_SECTION_ENTRY();

  // Any active object to run?
  if (EDF_core_schedule() != 0U)
  {
    // Trigger PendSV exception (context switch request)
    SCB_ICSR = (1U << SCB_ICSR_PENDSVSET);
  }
  EBF_CRITICAL_SECTION_EXIT();

#if (__ARM_ARCH > 6)
  EDF_core_ARMErratum838869();
#endif
}

EBF_WEAK void EDF_onContextSwitch(EDF_activeObject_t* prev,
                                  EDF_activeObject_t* next)
{
  EMF_UTILS_UNUSED_PARAM(prev);
  EMF_UTILS_UNUSED_PARAM(next);
}

EBF_WEAK void EDF_onIdle(void)
{
}
