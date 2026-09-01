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
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "edf_core.h"
#include "edf.h"
#include "edf_fuzz.h"
#include "edf_framework.h"
#include "edf_active_object.h"
#include "ebf.h"
#include "eaf.h"
#include "emf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief Maximum stdin fuzz input length in bytes (1 MiB).
 */
#define EDF_FUZZ_INPUT_MAX_LEN 1048576U

/**
 * @brief Number of fatal signals handled by the coverage flush hook.
 */
#define EDF_COVERAGE_FLUSH_SIGNAL_COUNT 6U

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

/**
 * @brief Tracks whether the fatal coverage flush was already attempted.
 *
 * volatile_use: asynchronous_interaction
 */
static volatile sig_atomic_t llvmCoverageFlushAttempted;

/**
 * @brief Previously installed actions for fatal replay signals.
 *
 * These actions are restored before re-raising the signal so the original crash
 * or timeout reason is not hidden by the best-effort coverage flush hook.
 */
static struct sigaction
  coverageFlushPrevActions[EDF_COVERAGE_FLUSH_SIGNAL_COUNT];

/**
 * @brief Fatal signals intercepted during source-coverage replay.
 *
 * `SIGTERM` is included because replay tooling commonly terminates hangs with a
 * regular termination signal after the timeout budget expires.
 */
static const int coverageFlushSignals[EDF_COVERAGE_FLUSH_SIGNAL_COUNT] = {
  SIGABRT,
  SIGBUS,
  SIGFPE,
  SIGILL,
  SIGSEGV,
  SIGTERM,
};

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

EDF_core_t EDF_core;

/**
 * @brief LLVM profiling runtime flush entry point.
 *
 * This symbol is only provided when the executable is linked with LLVM source
 * coverage instrumentation. Keeping the declaration weak lets the fuzz port use
 * the same source code in normal fuzz builds without introducing a hard link
 * dependency or changing application code.
 */
extern int __llvm_profile_write_file(void) EBF_WEAK;

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/**
 * @brief Return the payload length associated with a fuzz descriptor.
 *
 * @param[in] descriptor Fuzz event descriptor.
 * @return Mutable payload length in bytes.
 */
static size_t getFuzzEventPayloadLen(
  const EDF_fuzz_eventDescriptor_t* descriptor);

/**
 * @brief Publish the fuzzed event selected by the current fuzz descriptor.
 *
 * @param[in] descriptor Fuzz event descriptor.
 * @param[in] payload Mutable event payload bytes.
 * @param[in] payload_len Available payload length.
 * @return true if the event was injected, otherwise false.
 */
static bool injectFuzzEvent(const EDF_fuzz_eventDescriptor_t* descriptor,
                            const uint8_t* payload,
                            size_t payload_len);

/**
 * @brief Best-effort LLVM profile flush for fatal replay termination.
 *
 * `cov-analysis` replays realistic stdin/CLI fuzz targets by executing the real
 * binary. When a replayed testcase crashes or a hang is terminated by timeout,
 * the process can die before LLVM persists the `.profraw` data needed by source
 * coverage reports. This port-level hook tries to flush that data without
 * adding synthetic coverage code.
 *
 * The hook is intentionally weak-linked: if the binary is not compiled with the
 * LLVM profiling runtime, `__llvm_profile_write_file` resolves to NULL and the
 * flush attempt becomes a no-op so normal fuzzing behavior stays unchanged.
 *
 * This flush is best-effort only. It is triggered from fatal-signal handling
 * during replay, so coverage persistence is attempted pragmatically rather than
 * guaranteed by an async-signal-safe contract.
 *
 * This function may be invoked from a signal handler context where
 * async-signal-safety is not guaranteed.
 */
static void maybeFlushLlvmCoverage(void);

/**
 * @brief Resolve the index of a fatal signal inside the flush table.
 *
 * @param[in] signum POSIX signal number.
 * @return Matching table index or EDF_COVERAGE_FLUSH_SIGNAL_COUNT.
 */
static size_t getCoverageFlushSignalIndex(int signum);

/**
 * @brief Flush coverage data and re-raise the intercepted fatal signal.
 *
 * @note
 * This handler intentionally performs non-async-signal-safe operations
 * (LLVM profile flushing via `__llvm_profile_write_file`).
 *
 * According to POSIX, calling such functions from a signal handler results
 * in undefined behavior. This trade-off is accepted in this fuzzing context
 * to maximize coverage preservation when crashes or timeouts terminate
 * the process.
 *
 * The implementation is best-effort and may not be reliable in all cases.
 *
 * @param[in] signum POSIX signal number.
 */
static void coverageFlushSignalHandler(int signum);

/**
 * @brief Install fatal-signal handlers used by coverage-aware replay.
 */
static void installCoverageFlushSignalHandlers(void);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static size_t getFuzzEventPayloadLen(
  const EDF_fuzz_eventDescriptor_t* descriptor)
{
  size_t payload_len;

  EAF_ASSERT(descriptor != NULL);

  payload_len = 0U;

  if (descriptor->is_mutable == false)
  {
    payload_len = 0U;
  }
  else
  {
    EAF_ASSERT(descriptor->mutable_event_size >= sizeof(EDF_event_t));

    payload_len = (size_t)descriptor->mutable_event_size - sizeof(EDF_event_t);
  }

  return payload_len;
}

static bool injectFuzzEvent(const EDF_fuzz_eventDescriptor_t* descriptor,
                            const uint8_t* payload,
                            size_t payload_len)
{
  EDF_event_t* event;
  uint8_t* event_payload;
  size_t expected_payload_len;
  bool is_injected;

  EAF_ASSERT(descriptor != NULL);

  is_injected = false;

  if (descriptor->is_mutable == false)
  {
    EAF_ASSERT(descriptor->immutable_event != NULL);
    EDF_activeObject_publish(descriptor->immutable_event);
    is_injected = true;
  }
  else
  {
    expected_payload_len = getFuzzEventPayloadLen(descriptor);

    if (payload_len >= expected_payload_len)
    {
      event =
        EDF_event_initMutable(descriptor->mutable_event_size, descriptor->sig);
      EAF_ASSERT(event != NULL);

      if (expected_payload_len > 0U)
      {
        event_payload = ((uint8_t*)event) + sizeof(EDF_event_t);
        (void)memcpy(event_payload, payload, expected_payload_len);
      }

      EDF_activeObject_publish(event);
      is_injected = true;
    }
  }

  return is_injected;
}

static void maybeFlushLlvmCoverage(void)
{
  if (llvmCoverageFlushAttempted == 0)
  {
    llvmCoverageFlushAttempted = 1;

    if (__llvm_profile_write_file != NULL)
    {
      (void)__llvm_profile_write_file();
    }
  }
}

static size_t getCoverageFlushSignalIndex(int signum)
{
  size_t idx;
  size_t signal_idx;

  signal_idx = EDF_COVERAGE_FLUSH_SIGNAL_COUNT;

  for (idx = 0U; idx < EDF_COVERAGE_FLUSH_SIGNAL_COUNT; ++idx)
  {
    if (coverageFlushSignals[idx] == signum)
    {
      signal_idx = idx;
      break;
    }
  }

  return signal_idx;
}

static void coverageFlushSignalHandler(int signum)
{
  size_t signal_idx;

  maybeFlushLlvmCoverage();

  signal_idx = getCoverageFlushSignalIndex(signum);
  if (signal_idx < EDF_COVERAGE_FLUSH_SIGNAL_COUNT)
  {
    (void)sigaction(signum, &coverageFlushPrevActions[signal_idx], NULL);
  }
  else
  {
    (void)signal(signum, SIG_DFL);
  }

  (void)raise(signum);
}

static void installCoverageFlushSignalHandlers(void)
{
  struct sigaction sig_act;
  size_t idx;

  (void)memset(&sig_act, 0, sizeof(sig_act));
  sig_act.sa_handler = &coverageFlushSignalHandler;
  (void)sigemptyset(&sig_act.sa_mask);

  for (idx = 0U; idx < EDF_COVERAGE_FLUSH_SIGNAL_COUNT; ++idx)
  {
    (void)sigaction(
      coverageFlushSignals[idx], &sig_act, &coverageFlushPrevActions[idx]);
  }
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void EAF_onError(const char* file, int line, int id)
{
  EMF_UTILS_UNUSED_PARAM(file);
  EMF_UTILS_UNUSED_PARAM(line);
  EMF_UTILS_UNUSED_PARAM(id);

  /**
   * In this fuzzing port, an assert/error means the current testcase
   * cannot continue executing normally. Therefore, terminates the process with
   * `exit(0)` so the testcase ends immediately in a controlled way without
   * being reported to AFL++ as a fatal crash.
   */
  exit(0);
}

void EDF_init(void)
{
  // Clear EDF_core instance.
  EMF_utils_clear(&EDF_core, sizeof(EDF_core));

  // Initialize EDF framework.
  EDF_framework_init();

  installCoverageFlushSignalHandlers();
}

int EDF_run(void)
{
  uint8_t prio;
  uint8_t last_prio;
  EDF_activeObject_t* ao;
  EDF_event_t* e;
  EDF_event_signal_t fuzz_selector;
  size_t fuzz_event_index;
  unsigned char fuzz_buf[EDF_FUZZ_INPUT_MAX_LEN];
  ssize_t fuzz_len;
  size_t offset;
  size_t payload_offset;
  size_t payload_len;
  const EDF_fuzz_eventDescriptor_t* descriptor;

  EDF_onStartup();

  // Publish-subscribe shall be initialized to enable event fuzzing.
  EAF_ASSERT(EDF_framework.subscr_registry.max_signal >= EDF_HSM_USER_SIGNAL);
  EAF_ASSERT(EDF_fuzz_eventTableLen > 0U);

  fuzz_len = read(STDIN_FILENO, fuzz_buf, sizeof(fuzz_buf));

  if (fuzz_len >= (ssize_t)sizeof(EDF_event_signal_t))
  {
    offset = 0U;

    while (offset + sizeof(EDF_event_signal_t) <= (size_t)fuzz_len)
    {
      // Read the next fuzz-table selector from the testcase stream.
      EMF_endian_readBuffLE(
        &fuzz_buf[offset], &fuzz_selector, (uint8_t)sizeof(EDF_event_signal_t));

      fuzz_event_index = (size_t)fuzz_selector % EDF_fuzz_eventTableLen;

      descriptor = &EDF_fuzz_eventTable[fuzz_event_index];

      payload_offset = offset + sizeof(EDF_event_signal_t);
      payload_len = getFuzzEventPayloadLen(descriptor);

      if ((size_t)fuzz_len - payload_offset < payload_len)
      {
        break;
      }

      (void)injectFuzzEvent(descriptor,
                            &fuzz_buf[payload_offset],
                            (size_t)fuzz_len - payload_offset);

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

          if (last_prio > 0U)
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
          if (last_prio > 0U)
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

          isRunning = false;

          EBF_CRITICAL_SECTION_ENTRY();
        }
      }

      EBF_CRITICAL_SECTION_EXIT();

      // Advance to the next testcase segment.
      offset += sizeof(EDF_event_signal_t) + payload_len;
    }
  }

  EDF_onShutdown();

  return 0;
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
 * @anchor ticker_thread
 * @par Ticker thread
 *
 * The ticker thread is responsible for periodically invoking
 * @ref EDF_timeEvent_tick() with the configured @p tick_rate values.
 *
 * @note The EDF framework does not create this thread automatically.
 * It is the user's responsibility to configure and start the ticker
 * thread with the appropriate scheduling attributes.
 */
