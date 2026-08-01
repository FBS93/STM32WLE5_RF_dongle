/*******************************************************************************
 * @brief Time manager active object implementation, used for test validation.
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

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "time_mgr.h"
#include "edf.h"
#include "eaf.h"
#include "emf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief timeMgr active object priority.
 */
#define TIME_MGR_AO_PRIO 2U

/**
 * @brief Value 1ms. Use to calculate ticks.
 */
#define TICK_RATE_1_MS 1U

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/**
 * @brief Time events structure.
 */
typedef struct
{
  EDF_timeEvent_t periodic_5ms;       /**< Periodic 5ms time event. */
  EDF_timeEvent_t periodic_100ms;     /**< Periodic 100ms time event. */
  EDF_timeEvent_t periodic_5ms_100ms; /**< Periodic 5ms-100ms time event. */
  EDF_timeEvent_t one_shot_100ms;     /**< One-shot 100ms time event. */
} timeEvent_t;

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/**
 * @brief Define static file name string for asserts.
 */
EAF_DEFINE_THIS_FILE(__FILE__);

/**
 * @brief Subscriber lists used by the publish–subscribe mechanism.
 */
static EDF_activeObject_bitmask_t subscriberList[LAST_EVENT_SIGNAL];

/**
 * @brief Time event instance.
 */
static timeEvent_t timeEvent;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

timeMgr_t timeMgr;

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/**
 * @brief Initial transition of the timeMgr HSM.
 *
 * @param[in] me Pointer to the timeMgr instance.
 * @param[in] e Pointer to the event.
 */
static EDF_hsm_stateReturn_t initialTransition(timeMgr_t* me,
                                               const EDF_event_t* e);

/**
 * @brief readyState state of the timeMgr HSM.
 *s
 * @param[in] me Pointer to the timeMgr instance.
 * @param[in] e Pointer to the event.
 */
static EDF_hsm_stateReturn_t readyState(timeMgr_t* me, const EDF_event_t* e);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static EDF_hsm_stateReturn_t initialTransition(timeMgr_t* me,
                                               const EDF_event_t* e)
{
  EMF_UTILS_UNUSED_PARAM(me);
  EMF_UTILS_UNUSED_PARAM(e);

  return EDF_HSM_RET_TRAN(readyState);
}

static EDF_hsm_stateReturn_t readyState(timeMgr_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case PERIODIC_TIMER_5_MS:
    {
      timeMgr.periodic_5ms_cnt++;
      state = EDF_HSM_RET_HANDLED();
      break;
    }
    case PERIODIC_TIMER_100_MS:
    {
      timeMgr.periodic_100ms_cnt++;
      state = EDF_HSM_RET_HANDLED();
      break;
    }
    case PERIODIC_TIMER_5_MS_100_MS:
    {
      timeMgr.periodic_5ms_100ms_cnt++;
      state = EDF_HSM_RET_HANDLED();
      break;
    }
    case ONE_SHOT_TIMER_100_MS:
    {
      timeMgr.one_shot_100ms_cnt++;
      state = EDF_HSM_RET_HANDLED();
      break;
    }
    default:
    {
      state = EDF_HSM_RET_SUPER(&EDF_hsm_top);
      break;
    }
  }

  return state;
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void timeMgr_init(void)
{
  // Init EDF.
  EDF_init();

  // Init EDF publish and subscribe functionality.
  EDF_activeObject_pubSubInit(subscriberList,
                              (EDF_event_signal_t)LAST_EVENT_SIGNAL);

  // Init hsm active object.
  EDF_activeObject_init(EDF_AO_UPCAST(timeMgr),
                        (EDF_hsm_stateHandler_t)initialTransition);

  // Start active object.
  EDF_activeObject_start(EDF_AO_UPCAST(timeMgr),
                         (EDF_activeObject_prio_t)TIME_MGR_AO_PRIO,
                         timeMgr.event_buff,
                         EVENT_BUFF_SIZE,
                         NULL,
                         0,
                         NULL);

  // Create time events.
  EDF_timeEvent_new(&timeEvent.periodic_5ms,
                    EDF_AO_UPCAST(timeMgr),
                    PERIODIC_TIMER_5_MS,
                    TICK_RATE_IDX_1_MS);
  EDF_timeEvent_new(&timeEvent.periodic_100ms,
                    EDF_AO_UPCAST(timeMgr),
                    PERIODIC_TIMER_100_MS,
                    TICK_RATE_IDX_1_MS);
  EDF_timeEvent_new(&timeEvent.periodic_5ms_100ms,
                    EDF_AO_UPCAST(timeMgr),
                    PERIODIC_TIMER_5_MS_100_MS,
                    TICK_RATE_IDX_1_MS);
  EDF_timeEvent_new(&timeEvent.one_shot_100ms,
                    EDF_AO_UPCAST(timeMgr),
                    ONE_SHOT_TIMER_100_MS,
                    TICK_RATE_IDX_1_MS);
}

void timeMgr_clearExpirationCnt(void)
{
  timeMgr.periodic_5ms_cnt = 0;
  timeMgr.periodic_100ms_cnt = 0;
  timeMgr.periodic_5ms_100ms_cnt = 0;
  timeMgr.one_shot_100ms_cnt = 0;
}

void timeMgr_armPeriodicTimer5ms(void)
{
  EDF_timeEvent_arm(&timeEvent.periodic_5ms,
                    (EDF_timeEvent_cnt_t)(5U / TICK_RATE_1_MS),
                    (EDF_timeEvent_cnt_t)(5U / TICK_RATE_1_MS));
}

void timeMgr_disarmPeriodicTimer5ms(void)
{
  EDF_timeEvent_disarm(&timeEvent.periodic_5ms);
}

void timeMgr_armPeriodicTimer100ms(void)
{
  EDF_timeEvent_arm(&timeEvent.periodic_100ms,
                    (EDF_timeEvent_cnt_t)(100U / TICK_RATE_1_MS),
                    (EDF_timeEvent_cnt_t)(100U / TICK_RATE_1_MS));
}

void timeMgr_disarmPeriodicTimer100ms(void)
{
  EDF_timeEvent_disarm(&timeEvent.periodic_100ms);
}

void timeMgr_armPeriodicTimer5ms100ms(void)
{
  EDF_timeEvent_arm(&timeEvent.periodic_5ms_100ms,
                    (EDF_timeEvent_cnt_t)(5U / TICK_RATE_1_MS),
                    (EDF_timeEvent_cnt_t)(100U / TICK_RATE_1_MS));
}

void timeMgr_disarmPeriodicTimer5ms100ms(void)
{
  EDF_timeEvent_disarm(&timeEvent.periodic_5ms_100ms);
}

void timeMgr_armOneShotTimer100ms(void)
{
  EDF_timeEvent_arm(&timeEvent.one_shot_100ms,
                    (EDF_timeEvent_cnt_t)(100U / TICK_RATE_1_MS),
                    (EDF_timeEvent_cnt_t)(0U));
}

void timeMgr_disarmOneShotTimer100ms(void)
{
  EDF_timeEvent_disarm(&timeEvent.one_shot_100ms);
}
