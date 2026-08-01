/*******************************************************************************
 * @brief Dummy EDF application used by the ARM Cortex-M preemption example.
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
#include "app.h"
#include "gpio_trace.h"
#include "exec_timer.h"
#include "tick_timer.h"
#include "edf.h"
#include "eaf.h"
#include "emf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief EDF tick-rate index used by the application.
 */
#define APP_TICK_RATE_IDX_1_MS 0U

/**
 * @brief Event queue length for each active object.
 */
#define APP_EVENT_BUFF_SIZE 16U

/**
 * @brief Low-priority active object priority.
 */
#define AO_LOW_PRIO 2U

/**
 * @brief Medium-priority active object priority.
 */
#define AO_MID_PRIO 3U

/**
 * @brief High-priority active object priority.
 */
#define AO_HIGH_PRIO 4U

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/**
 * @brief Event signals used by the dummy application.
 */
typedef enum
{
  AO_LOW_TIMEOUT = EDF_HSM_USER_SIGNAL, /**< Low-priority activation signal. */
  AO_MID_TIMEOUT,  /**< Medium-priority activation signal. */
  AO_HIGH_TIMEOUT, /**< High-priority activation signal. */
} app_eventSignal_t;

/**
 * @brief Generic worker active object used by the dummy application.
 */
typedef struct
{
  EDF_activeObject_t super;                      /**< Base type. */
  EDF_event_ptr event_buff[APP_EVENT_BUFF_SIZE]; /**< Queue. */
  EDF_timeEvent_t activation_time_event; /**< Periodic activation time event. */
  gpioTrace_channel_t trace_channel;     /**< GPIO channel used for tracing. */
  EDF_event_signal_t activation_sig;     /**< Self activation signal. */
  uint32_t first_delay_ms;    /**< Initial release offset in milliseconds. */
  uint32_t period_ms;         /**< Period in milliseconds. */
  uint32_t execution_time_us; /**< Blocking execution time in microseconds. */
  uint16_t execution_budget_ticks; /**< Execution budget in timer ticks. */
  uint16_t executed_ticks; /**< Accumulated execution time in timer ticks. */
  uint16_t resume_tick;    /**< Tick at which the current execution slice
                                 started. */
  bool execution_active;   /**< True while the AO trace pulse is active. */
  bool execution_counting; /**< True while execution time is being counted. */
} workerAo_t;

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/**
 * @brief Define static file name string for asserts.
 */
EAF_DEFINE_THIS_FILE(__FILE__);

/**
 * @brief Low-priority worker.
 */
static workerAo_t aoLow = {
  .trace_channel = GPIO_TRACE_CHANNEL_LOW,
  .activation_sig = AO_LOW_TIMEOUT,
  .first_delay_ms = 10U,
  .period_ms = 1000U,
  .execution_time_us = 500000U,
};

/**
 * @brief Medium-priority worker.
 */
static workerAo_t aoMid = {
  .trace_channel = GPIO_TRACE_CHANNEL_MID,
  .activation_sig = AO_MID_TIMEOUT,
  .first_delay_ms = 20U,
  .period_ms = 500U,
  .execution_time_us = 150000U,
};

/**
 * @brief High-priority worker.
 */
static workerAo_t aoHigh = {
  .trace_channel = GPIO_TRACE_CHANNEL_HIGH,
  .activation_sig = AO_HIGH_TIMEOUT,
  .first_delay_ms = 30U,
  .period_ms = 100U,
  .execution_time_us = 10000U,
};

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
 * @brief Initial transition of a worker active object.
 *
 * @param[in] me Pointer to the worker active object.
 * @param[in] e Pointer to the event.
 * @return State return code.
 */
static EDF_hsm_stateReturn_t initialTransition(workerAo_t* me,
                                               const EDF_event_t* e);

/**
 * @brief Ready state of a worker active object.
 *
 * @param[in] me Pointer to the worker active object.
 * @param[in] e Pointer to the event.
 * @return State return code.
 */
static EDF_hsm_stateReturn_t readyState(workerAo_t* me, const EDF_event_t* e);

/**
 * @brief Initialize one worker active object and its time event.
 *
 * @param[in,out] me Pointer to the worker active object.
 * @param[in] prio EDF active object priority.
 */
static void initWorkerAo(workerAo_t* me, EDF_activeObject_prio_t prio);

/**
 * @brief Resolve a worker active object from its EDF base pointer.
 *
 * @param[in] ao EDF active object pointer.
 * @return Matching worker active object, or NULL if it is not one of the demo
 *         workers.
 */
static workerAo_t* workerFromAo(EDF_activeObject_t* ao);

/**
 * @brief Update one worker accumulated execution budget until now.
 *
 * @param[in,out] me Pointer to the worker active object.
 */
static void updateExecutedTicks(workerAo_t* me);

/**
 * @brief Return the current counted execution time of one worker.
 *
 * @param[in] me Pointer to the worker active object.
 * @return Counted execution time in timer ticks.
 */
static uint16_t currentExecutedTicks(const workerAo_t* me);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static EDF_hsm_stateReturn_t initialTransition(workerAo_t* me,
                                               const EDF_event_t* e)
{
  EMF_UTILS_UNUSED_PARAM(me);
  EMF_UTILS_UNUSED_PARAM(e);

  return EDF_HSM_RET_TRAN(readyState);
}

static EDF_hsm_stateReturn_t readyState(workerAo_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  if (e->sig == me->activation_sig)
  {
    me->executed_ticks = 0U;
    me->resume_tick = execTimer_getTick();
    me->execution_active = true;
    me->execution_counting = true;
    gpioTrace_setActive(me->trace_channel);

    while (currentExecutedTicks(me) < me->execution_budget_ticks)
    {
    }

    updateExecutedTicks(me);
    me->execution_counting = false;
    me->execution_active = false;
    gpioTrace_setInactive(me->trace_channel);
    state = EDF_HSM_RET_HANDLED();
  }
  else
  {
    state = EDF_HSM_RET_SUPER(&EDF_hsm_top);
  }

  return state;
}

static void initWorkerAo(workerAo_t* me, EDF_activeObject_prio_t prio)
{
  EAF_ASSERT(me != NULL);

  EDF_activeObject_init(EDF_AO_UPCAST(*me),
                        (EDF_hsm_stateHandler_t)initialTransition);
  EDF_activeObject_start(EDF_AO_UPCAST(*me),
                         prio,
                         me->event_buff,
                         APP_EVENT_BUFF_SIZE,
                         NULL,
                         0U,
                         NULL);

  EDF_timeEvent_new(&me->activation_time_event,
                    EDF_AO_UPCAST(*me),
                    me->activation_sig,
                    APP_TICK_RATE_IDX_1_MS);
  me->execution_budget_ticks = execTimer_delayUsToTicks(me->execution_time_us);
  EDF_timeEvent_arm(&me->activation_time_event,
                    (EDF_timeEvent_cnt_t)me->first_delay_ms,
                    (EDF_timeEvent_cnt_t)me->period_ms);
}

static workerAo_t* workerFromAo(EDF_activeObject_t* ao)
{
  workerAo_t* worker;

  if (ao == EDF_AO_UPCAST(aoLow))
  {
    worker = &aoLow;
  }
  else if (ao == EDF_AO_UPCAST(aoMid))
  {
    worker = &aoMid;
  }
  else if (ao == EDF_AO_UPCAST(aoHigh))
  {
    worker = &aoHigh;
  }
  else
  {
    worker = NULL;
  }

  return worker;
}

static void updateExecutedTicks(workerAo_t* me)
{
  uint16_t now_tick;

  EAF_ASSERT(me != NULL);

  if (me->execution_counting)
  {
    now_tick = execTimer_getTick();
    me->executed_ticks =
      (uint16_t)(me->executed_ticks +
                 execTimer_elapsedTicks(me->resume_tick, now_tick));
    me->resume_tick = now_tick;
  }
}

static uint16_t currentExecutedTicks(const workerAo_t* me)
{
  uint16_t executed_ticks;

  EAF_ASSERT(me != NULL);

  executed_ticks = me->executed_ticks;

  if (me->execution_counting)
  {
    executed_ticks =
      (uint16_t)(executed_ticks +
                 execTimer_elapsedTicks(me->resume_tick, execTimer_getTick()));
  }

  return executed_ticks;
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void app_init(void)
{
  EDF_init();

  initWorkerAo(&aoLow, AO_LOW_PRIO);
  initWorkerAo(&aoMid, AO_MID_PRIO);
  initWorkerAo(&aoHigh, AO_HIGH_PRIO);
}

void EDF_onStartup(void)
{
  gpioTrace_init();
  execTimer_init();
  tickTimer_init();
}

void EDF_onContextSwitch(EDF_activeObject_t* prev, EDF_activeObject_t* next)
{
  workerAo_t* prev_worker;
  workerAo_t* next_worker;

  prev_worker = workerFromAo(prev);
  next_worker = workerFromAo(next);

  if ((prev_worker != NULL) && prev_worker->execution_active)
  {
    updateExecutedTicks(prev_worker);
    prev_worker->execution_counting = false;
  }

  if ((next_worker != NULL) && next_worker->execution_active)
  {
    next_worker->resume_tick = execTimer_getTick();
    next_worker->execution_counting = true;
  }
}
