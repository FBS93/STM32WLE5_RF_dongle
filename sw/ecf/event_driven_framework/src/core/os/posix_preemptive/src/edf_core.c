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

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <limits.h>

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

/**
 * @brief Offset applied to calculate the priority of Active Object threads.
 *
 * Ensures that Active Objects run below the main EDF thread and
 * the ticker thread in the SCHED_FIFO priority hierarchy.
 */
#define ACTIVE_OBJECT_THREAD_PRIO_OFFSET 2U

/**
 * @brief Offset applied to calculate the priority of the EDF main thread.
 *
 * Ensures the main EDF thread runs just below the ticker thread
 * in the SCHED_FIFO priority hierarchy.
 */
#define EDF_MAIN_THREAD_PRIO_OFFSET 1U

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
 * @brief Mutex used to synchronize the active object threads and EDF
 * initialization.
 */
static pthread_mutex_t startupMutex;

/**
 * @brief Flag indicating whether the EDF framework is running.
 *
 * volatile_use: context_interaction
 */
static volatile bool isRunning;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

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

/**
 * @brief Thread routine for an Active Object.
 *
 * @param[in] arg Pointer to the active object instance.
 * @return Always returns NULL (unused, as threads are detached).
 */
static void* activeObjectThread(void* arg);

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

static void* activeObjectThread(void* arg)
{
  EDF_activeObject_t* act;
  EDF_event_t* e;

  act = (EDF_activeObject_t*)arg;

  // Block this thread until the startup mutex is unlocked from EDF_run().
  (void)pthread_mutex_lock(&startupMutex);
  (void)pthread_mutex_unlock(&startupMutex);

  // For-ever.
  while (1)
  {
    // Typecast to discard const qualifier.
    e = (EDF_event_t*)EDF_activeObject_get(act);
    EDF_hsm_dispatch(&act->super, e);
    EDF_event_gc(e);
  }

  return NULL;
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void EDF_init(void)
{
  struct sigaction sig_act;

  /**
   * @todo Optional hard real-time optimization.
   *
   * mlockall(MCL_CURRENT | MCL_FUTURE) locks all current and future
   * memory pages of this process into RAM, preventing them from being
   * swapped to disk. This can eliminate unpredictable page faults and
   * improve timing determinism in hard real-time systems.
   *
   * Considerations before enabling:
   *  - Requires root privileges or CAP_IPC_LOCK capability.
   *  - Limited by the RLIMIT_MEMLOCK resource limit (ulimit -l).
   *  - May consume significant RAM and reduce overall system flexibility.
   *  - Not strictly necessary for soft real-time applications.
   *
   * If enabled, handle errors carefully: failure to lock memory does not
   * necessarily mean the application cannot run, but timing may degrade.
   * Use only if strict timing guarantees are required and the platform
   * environment supports it.
   *
   * Example code:
   *
   * @code
   * #include <sys/mman.h> // for mlockall()
   *
   * if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
   *   perror("mlockall failed");
   *   // optional: decide if you want to exit or continue without locked memory
   * }
   * @endcode
   */

  /**
   * Initialize the startup mutex with default attributes
   * (non-recursive, same as PTHREAD_MUTEX_INITIALIZER).
   */
  (void)pthread_mutex_init(&startupMutex, NULL);

  /**
   * Lock the startup mutex to block any active objects started before
   * calling @ref EDF_run().
   */
  (void)pthread_mutex_lock(&startupMutex);

  // Initialize EDF framework.
  EDF_framework_init();

  // set SIGINT (Ctrl-C) signal callback
  (void)memset(&sig_act, 0, sizeof(sig_act));
  sig_act.sa_handler = &sigIntHandler;
  (void)sigaction(SIGINT, &sig_act, NULL);
}

int EDF_run(void)
{
  struct sched_param sparam;

  EDF_onStartup();  // User-specific startup callback.

  // See @ref pthread_priority_scope.
  sparam.sched_priority =
    sched_get_priority_max(SCHED_FIFO) - EDF_MAIN_THREAD_PRIO_OFFSET;

  // See @ref scheduler_fifo_policy.
  (void)pthread_setschedparam(pthread_self(), SCHED_FIFO, &sparam);

  /**
   * Exit the startup critical section to unblock any active objects
   * started before calling @ref EDF_run().
   */
  (void)pthread_mutex_unlock(&startupMutex);

  isRunning = true;
  while (isRunning)
  {
    // Do nothing.
  }

  EDF_onShutdown();  // User-specific shutdown callback.

  (void)pthread_mutex_destroy(&startupMutex);

  return 0;  // Return success
}

void EDF_stop(void)
{
  /**
   * Calling this function terminates the main EDF thread,
   * which causes all Active Object threads to end abruptly.
   * The user is responsible for invoking @ref EDF_stop() only
   * when the AO states are safe for an immediate shutdown.
   */
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
  pthread_attr_t attr;
  int err;
  pthread_t thread;
  struct sched_param sched_param;

  // p-threads allocate stack internally.
  EMF_UTILS_UNUSED_PARAM(stack_storage);

  EAF_ASSERT(me != NULL);

  me->prio = (uint8_t)(prio & 0xFFU);  // Base priority.
  me->pthre = 0U;  // Preemption-threshold (not used in this port).

  // Ensure priority is within the valid SCHED_FIFO range.
  EAF_ASSERT((me->prio >= sched_get_priority_min(SCHED_FIFO)) &&
             (me->prio <= sched_get_priority_max(SCHED_FIFO)));

  EDF_eventQueue_init(&me->e_queue, q_storage, q_len);

  // Initialize the condition variable used for event notification.
  (void)pthread_cond_init(&me->e_cond, NULL);

  EDF_activeObject_register(me);

  // Execute top-most initial transition of the HSM.
  EDF_hsm_start(&me->super, e);

  // Configure the Active Object thread scheduling policy and attributes.
  // See @ref scheduler_fifo_policy.
  (void)pthread_attr_init(&attr);
  (void)pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  (void)pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  (void)pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  // Configure the Active Object thread priority.
  // See @ref pthread_priority_scope.
  sched_param.sched_priority =
    me->prio + (sched_get_priority_max(SCHED_FIFO) - EDF_MAX_ACTIVE_OBJECT -
                ACTIVE_OBJECT_THREAD_PRIO_OFFSET);
  (void)pthread_attr_setschedparam(&attr, &sched_param);

  // Configure the Active Object thread stack size.
  (void)pthread_attr_setstacksize(
    &attr,
    ((stack_size < (int_fast16_t)PTHREAD_STACK_MIN) ? PTHREAD_STACK_MIN :
                                                      stack_size));

  // Create the Active Object thread.
  err = pthread_create(&thread, &attr, &activeObjectThread, me);

  /**
   * If this assertion fails, creating the pthread with the SCHED_FIFO
   * policy was unsuccessful. Most likely, the application does not
   * have superuser privileges.
   */
  EAF_ASSERT(err == 0);

  // Release thread attribute object resources.
  (void)pthread_attr_destroy(&attr);
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

/**
 * @anchor scheduler_fifo_policy
 * @par Scheduler FIFO policy
 *
 * On POSIX, the scheduler policy closest to real-time is @c SCHED_FIFO.
 * This policy is only available with superuser privileges.
 *
 * @ref EDF_run() attempts to set this policy and maximize its priority,
 * so that ticking occurs in the most timely manner (as close to an
 * interrupt as possible).
 *
 * However, this call may fail, most likely due to insufficient privileges.
 */

/**
 * @anchor pthread_priority_scope
 * @par Pthread priority scope
 *
 * According to the POSIX man pages (@c pthread_attr_setschedpolicy),
 * the only supported value for scheduling scope is @c PTHREAD_SCOPE_SYSTEM.
 *
 * This means that threads compete for CPU time with all processes
 * running on the machine and thread priorities are interpreted
 * relative to other system processes.
 *
 * If thread priorities are set high enough, no other process (or
 * thread within one) can preempt CPU control.
 *
 * The EDF framework limits the number of priority levels to
 * @ref EDF_MAX_ACTIVE_OBJECT. To accommodate real-time needs, this port:
 * - Reserves the **highest available pthread priority** exclusively
 * for the ticker thread, which periodically invokes
 * @ref EDF_timeEvent_tick(). See @ref ticker_thread_priority.
 * - Reserves the **second-highest priority** for the main EDF
 * thread created when @ref EDF_run() is called.
 * - The remaining priorities are distributed among active objects,
 * while the three lowest priorities are left unused to avoid
 * starvation of system/background tasks.
 */

/**
 * @anchor ticker_thread_priority
 * @par Ticker thread priority
 *
 * The ticker thread is responsible for periodically invoking
 * @ref EDF_timeEvent_tick() with the configured @p tick_rate values.
 *
 * - For reliable and deterministic processing of time events, this
 * thread should run at the **maximum available priority** under the
 * @c SCHED_FIFO scheduling policy.
 * - The maximum priority can be obtained at runtime with:
 * @c sched_get_priority_max(SCHED_FIFO).
 * - Running with @c SCHED_FIFO at maximum priority requires:
 *  - Root privileges or the @c CAP_SYS_NICE capability.
 *  - A system with POSIX real-time scheduling support.
 *
 * Example code:
 *
 * @code
 * pthread_t tickThread;
 * pthread_attr_t attr;
 * struct sched_param sparam;
 *
 * pthread_attr_init(&attr);
 * pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
 *
 * sparam.sched_priority = sched_get_priority_max(SCHED_FIFO);
 * pthread_attr_setschedparam(&attr, &sparam);
 *
 * pthread_create(&tickThread, &attr, tickThreadFunc, NULL);
 * @endcode
 *
 * @warning The EDF framework does not create this thread automatically.
 * It is the user's responsibility to configure and start the ticker
 * thread with the appropriate scheduling attributes.
 */
