/*******************************************************************************
 * @brief Time event tests.
 *
 * Verifies the correct operation of time events, including event delivery and
 * timing accuracy.
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
#include "edf_test.h"
#include "edf.h"
#include "time_mgr.h"
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
 * @brief Immutable event used to trigger the test verification.
 */
static const EDF_event_t verifyEvent = EDF_EVENT_IMMUTABLE_INIT(VERIFY);

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

EDF_TEST_SUITE_NAME(test_time_event)

/**
 * The buffer must be large enough to hold the maximum number of consecutive
 * posted events in the worst-case scenario without preemption. This ensures
 * the test can also be used in non-preemptive core/port configurations.
 */
EDF_TEST_EVENT_BUFF(50)

void EDFTest_onInit(void)
{
  timeMgr_init();
}

void EDFTest_setUp(void)
{
  timeMgr_clearExpirationCnt();
}

EDF_TEST_CASE(periodic_timers, VERIFY)
EDF_TEST_CASE_INIT(periodic_timers)
{
  timeMgr_armPeriodicTimer5ms();
  timeMgr_armPeriodicTimer100ms();
  timeMgr_armPeriodicTimer5ms100ms();

  for (uint8_t ms = 0; ms < 205; ms++)
  {
    EDF_timeEvent_tick(TICK_RATE_IDX_1_MS);
  }

  EDF_activeObject_publish(&verifyEvent);
}
EDF_TEST_CASE_VERIFY(periodic_timers, e)
{
  EMF_UTILS_UNUSED_PARAM(e);

  timeMgr_disarmPeriodicTimer5ms();
  timeMgr_disarmPeriodicTimer100ms();
  timeMgr_disarmPeriodicTimer5ms100ms();

  EDF_TEST_VERIFY(timeMgr.periodic_5ms_cnt == 41U);
  EDF_TEST_VERIFY(timeMgr.periodic_100ms_cnt == 2U);
  EDF_TEST_VERIFY(timeMgr.periodic_5ms_100ms_cnt == 3U);
}

EDF_TEST_CASE(one_shot_timer, VERIFY)
EDF_TEST_CASE_INIT(one_shot_timer)
{
  timeMgr_armOneShotTimer100ms();

  for (uint8_t ms = 0; ms < 200; ms++)
  {
    EDF_timeEvent_tick(TICK_RATE_IDX_1_MS);
  }

  EDF_activeObject_publish(&verifyEvent);
}
EDF_TEST_CASE_VERIFY(one_shot_timer, e)
{
  EMF_UTILS_UNUSED_PARAM(e);

  EDF_TEST_VERIFY(timeMgr.one_shot_100ms_cnt == 1U);
}

// clang-format off
EDF_TEST_SUITE(EDF_TEST_RUN(periodic_timers),
               EDF_TEST_RUN(one_shot_timer))
// clang-format on