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
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "edf_core.h"
#include "edf.h"
#include "edf_framework.h"
#include "edf_active_object.h"
#include "ebf.h"
#include "eaf.h"
#include "emf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

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

/**
 * @brief Flag indicating whether the EDF framework is running.
 *
 * volatile_use: context_interaction
 */
static volatile bool isRunning;

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
 * @brief Signal handler for SIGINT (Ctrl+C).
 *
 * This handler is registered via sigaction() to catch the SIGINT signal,
 * which is typically generated when the user presses Ctrl+C in the terminal.
 * Instead of terminating the process immediately, it perform a controlled
 * application exit.
 *
 * @param signum Signal number that triggered the handler.
 */
static void sigIntHandler(int signum);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static void sigIntHandler(int signum)
{
  /**
   * Unused because we have only registered SIGINT, so the value will
   * always be SIGINT (2), which corresponds to Ctrl+C.
   */
  EMF_UTILS_UNUSED_PARAM(signum);

  EDF_onShutdown();  // User-specific shutdown callback.
  exit(-1);
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void EDF_init(void)
{
  struct sigaction sig_act;

  // Clear EDF_core instance.
  EMF_utils_clear(&EDF_core, sizeof(EDF_core));

  // Initialize EDF framework.
  EDF_framework_init();

  // set SIGINT (Ctrl-C) signal callback
  EMF_utils_clear(&sig_act, sizeof(sig_act));
  sig_act.sa_handler = &sigIntHandler;
  (void)sigaction(SIGINT, &sig_act, NULL);
}

int EDF_run(void)
{
  uint8_t prio;
  uint8_t last_prio;
  EDF_activeObject_t* ao;
  EDF_event_t* e;

  EDF_onStartup();  // User-specific startup callback.

  EBF_CRITICAL_SECTION_ENTRY();

  last_prio = 0;
  isRunning = true;
  while (isRunning)
  {
    // Find new highest-prio AO ready to run.
    prio = EMF_bitmask_findMax(EDF_core.ready_set.bitmask,
                               sizeof(EDF_core.ready_set));

    if (prio > 0)
    {
      ao = EDF_framework.ao_registry[prio];

      if (last_prio > 0)
      {
        EDF_onContextSwitch(EDF_framework.ao_registry[last_prio], ao);
      }
      else
      {
        EDF_onContextSwitch(NULL, ao);
      }

      last_prio = prio;  // Update last priority.

      EBF_CRITICAL_SECTION_EXIT();

      // Typecast to discard const qualifier.
      e = (EDF_event_t*)EDF_activeObject_get(ao);

      // Dispatch event.
      EDF_hsm_dispatch(&ao->super, e);
      EDF_event_gc(e);

      EBF_CRITICAL_SECTION_ENTRY();

      // Empty queue?
      if (ao->e_queue.front_e == NULL)
      {
        EMF_bitmask_clearBit(EDF_core.ready_set.bitmask, prio);
      }
    }
    else
    {
      // No active object ready to run --> idle.
      if (last_prio > 0)
      {
        EDF_onContextSwitch(EDF_framework.ao_registry[last_prio], NULL);
      }
      else
      {
        EDF_onContextSwitch(NULL, NULL);
      }

      last_prio = 0;  // Update last priority.

      EBF_CRITICAL_SECTION_EXIT();

      EDF_onIdle();  // User-specific idle callback.

      EBF_CRITICAL_SECTION_ENTRY();
    }
  }

  EDF_onShutdown();  // User-specific shutdown callback.

  return 0;  // Return success
}

void EDF_stop(void)
{
  isRunning = false;  // Terminate the main EDF thread
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

  EAF_ASSERT(me != NULL);

  me->prio = (uint8_t)(prio & 0xFFU);  // Base priority.
  me->pthre = (uint8_t)(prio >> 8U);   // preemption-threshold

  EDF_activeObject_register(me);
  EDF_eventQueue_init(&me->e_queue, q_storage, q_len);

  // Execute top-most initial transition of the HSM.
  EDF_hsm_start(&me->super, e);
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

EBF_WEAK void EDF_onContextSwitch(EDF_activeObject_t* prev,
                                  EDF_activeObject_t* next)
{
  EMF_UTILS_UNUSED_PARAM(prev);
  EMF_UTILS_UNUSED_PARAM(next);
}

EBF_WEAK void EDF_onIdle(void)
{
}

/**
 * @anchor ticker_thread_posix_non_preemptive
 * @par Ticker thread
 *
 * The ticker thread is responsible for periodically invoking
 * @ref EDF_timeEvent_tick() with the configured @p tick_rate values.
 *
 * @note The EDF framework does not create this thread automatically.
 * It is the user's responsibility to configure and start the ticker
 * thread with the appropriate scheduling attributes.
 */
