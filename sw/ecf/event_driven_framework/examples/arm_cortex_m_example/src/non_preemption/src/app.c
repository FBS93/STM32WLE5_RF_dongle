/*******************************************************************************
 * @brief Dummy EDF application used by the ARM Cortex-M non-preemption example.
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

#include <stdint.h>

#include "app.h"
#include "gpio_trace.h"
#include "exec_timer.h"
#include "edf.h"
#include "eaf.h"
#include "emf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

#define APP_EVENT_BUFF_SIZE 16U
#define APP_PERIOD_TICKS 1000U
#define AO_LOW_PRIO 2U
#define AO_HIGH_PRIO 4U

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

typedef enum
{
  AO_LOW_TIMEOUT = EDF_HSM_USER_SIGNAL,
  AO_HIGH_TIMEOUT,
} app_eventSignal_t;

/**
 * @brief Worker configuration and queue; no private stack is needed.
 */
typedef struct
{
  EDF_activeObject_t super;
  EDF_event_ptr event_buff[APP_EVENT_BUFF_SIZE];
  gpioTrace_channel_t trace_channel;
  EDF_event_signal_t activation_sig;
  uint32_t release_tick;
  uint16_t execution_budget_ticks;
} workerAo_t;

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

EAF_DEFINE_THIS_FILE(__FILE__);

static const EDF_event_t lowEvent = EDF_EVENT_IMMUTABLE_INIT(AO_LOW_TIMEOUT);
static const EDF_event_t highEvent = EDF_EVENT_IMMUTABLE_INIT(AO_HIGH_TIMEOUT);
static EDF_activeObject_bitmask_t subscriberList[AO_HIGH_TIMEOUT];
static uint32_t phaseTick;

static workerAo_t aoLow = {
  .trace_channel = GPIO_TRACE_CHANNEL_LOW_EXEC,
  .activation_sig = AO_LOW_TIMEOUT,
  .release_tick = 100U,
  .execution_budget_ticks = 30000U,
};

static workerAo_t aoHigh = {
  .trace_channel = GPIO_TRACE_CHANNEL_HIGH_EXEC,
  .activation_sig = AO_HIGH_TIMEOUT,
  .release_tick = 200U,
  .execution_budget_ticks = 10000U,
};

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

static EDF_hsm_stateReturn_t readyState(workerAo_t* me, const EDF_event_t* e);

static EDF_hsm_stateReturn_t initialTransition(workerAo_t* me,
                                               const EDF_event_t* e)
{
  EMF_UTILS_UNUSED_PARAM(me);
  EMF_UTILS_UNUSED_PARAM(e);

  return EDF_HSM_RET_TRAN(readyState);
}

static EDF_hsm_stateReturn_t readyState(workerAo_t* me, const EDF_event_t* e)
{
  uint16_t start_tick;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  if (e->sig == me->activation_sig)
  {
    gpioTrace_setActive(me->trace_channel);
    start_tick = execTimer_getTick();

    /* Wall time includes ISRs; IRQs stay enabled during the workload. */
    while (execTimer_elapsedTicks(start_tick, execTimer_getTick()) <
           me->execution_budget_ticks)
    {
    }

    gpioTrace_setInactive(me->trace_channel);
    return EDF_HSM_RET_HANDLED();
  }

  return EDF_HSM_RET_SUPER(&EDF_hsm_top);
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
  EDF_activeObject_subscribe(EDF_AO_UPCAST(*me), me->activation_sig);
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void app_init(void)
{
  EDF_init();
  EDF_activeObject_pubSubInit(subscriberList,
                              EMF_UTILS_SIZEOF_ARRAY(subscriberList));
  phaseTick = 0U;

  initWorkerAo(&aoLow, AO_LOW_PRIO);
  initWorkerAo(&aoHigh, AO_HIGH_PRIO);
}

void app_onTick(void)
{
  /* Called from SysTick ISR; AOs cannot dispatch until the ISR returns. */
  gpioTrace_setInactive(GPIO_TRACE_CHANNEL_HIGH_RELEASE);

  ++phaseTick;
  if (phaseTick == APP_PERIOD_TICKS)
  {
    phaseTick = 0U;
  }

  if (phaseTick == aoLow.release_tick)
  {
    EDF_activeObject_publish(&lowEvent);
  }
  if (phaseTick == aoHigh.release_tick)
  {
    EDF_activeObject_publish(&highEvent);
    gpioTrace_setActive(GPIO_TRACE_CHANNEL_HIGH_RELEASE);
  }
}
