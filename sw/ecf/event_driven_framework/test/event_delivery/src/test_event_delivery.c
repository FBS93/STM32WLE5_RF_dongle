/*******************************************************************************
 * @brief Event delivery tests.
 *
 * Verifies correct routing of published events to the subscribed active
 * objects.
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
#include "edf_test.h"
#include "edf.h"
#include "active_object.h"
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
 * @brief Immutable events used to validate the event delivery in the tests.
 */
const EDF_event_t sig1 = EDF_EVENT_IMMUTABLE_INIT(S1);
const EDF_event_t sig2 = EDF_EVENT_IMMUTABLE_INIT(S2);
const EDF_event_t sig3 = EDF_EVENT_IMMUTABLE_INIT(S3);
const EDF_event_t lastSig = EDF_EVENT_IMMUTABLE_INIT(LAST_EVENT_SIGNAL);

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

EDF_TEST_SUITE_NAME(test_event_delivery)

EDF_TEST_EVENT_BUFF(1)

void EDFTest_onInit(void)
{
  activeObject_init();
}

void EDFTest_setUp(void)
{
  activeObject_resetEventReceivedFlag();
}

EDF_TEST_CASE(signal_1, S1)
EDF_TEST_CASE_INIT(signal_1)
{
  EDF_activeObject_publish(&sig1);
}
EDF_TEST_CASE_VERIFY(signal_1, e)
{
  bool success;

  EMF_UTILS_UNUSED_PARAM(e);

  // Check for the active object that is only subscribed to that event signal.
  success = false;
  for (uint8_t ao_idx = 0; ao_idx < (ACTIVE_OBJECT_N_TOTAL - 1); ao_idx++)
  {
    if (ACTIVE_OBJECT_PRIO_TO_SIGNAL(
          EDF_AO_UPCAST(activeObject[ao_idx])->prio) == e->sig)
    {
      EDF_TEST_VERIFY(activeObject[ao_idx].event_received);
      success = true;
    }
    else
    {
      EDF_TEST_VERIFY(!activeObject[ao_idx].event_received);
    }
  }
  EDF_TEST_VERIFY(success);

  // The last active object is subscibed to all event signals.
  EDF_TEST_VERIFY(activeObject[ACTIVE_OBJECT_N_TOTAL - 1].event_received);
}

EDF_TEST_CASE(signal_2, S2)
EDF_TEST_CASE_INIT(signal_2)
{
  EDF_activeObject_publish(&sig2);
}
EDF_TEST_CASE_VERIFY(signal_2, e)
{
  bool success;

  EMF_UTILS_UNUSED_PARAM(e);

  // Check for the active object that is only subscribed to that event signal.
  success = false;
  for (uint8_t ao_idx = 0; ao_idx < (ACTIVE_OBJECT_N_TOTAL - 1); ao_idx++)
  {
    if (ACTIVE_OBJECT_PRIO_TO_SIGNAL(
          EDF_AO_UPCAST(activeObject[ao_idx])->prio) == e->sig)
    {
      EDF_TEST_VERIFY(activeObject[ao_idx].event_received);
      success = true;
    }
    else
    {
      EDF_TEST_VERIFY(!activeObject[ao_idx].event_received);
    }
  }
  EDF_TEST_VERIFY(success);

  // The last active object is subscibed to all event signals.
  EDF_TEST_VERIFY(activeObject[ACTIVE_OBJECT_N_TOTAL - 1].event_received);
}

EDF_TEST_CASE(signal_3, S3)
EDF_TEST_CASE_INIT(signal_3)
{
  EDF_activeObject_publish(&sig3);
}
EDF_TEST_CASE_VERIFY(signal_3, e)
{
  bool success;

  EMF_UTILS_UNUSED_PARAM(e);

  // Check for the active object that is only subscribed to that event signal.
  success = false;
  for (uint8_t ao_idx = 0; ao_idx < (ACTIVE_OBJECT_N_TOTAL - 1); ao_idx++)
  {
    if (ACTIVE_OBJECT_PRIO_TO_SIGNAL(
          EDF_AO_UPCAST(activeObject[ao_idx])->prio) == e->sig)
    {
      EDF_TEST_VERIFY(activeObject[ao_idx].event_received);
      success = true;
    }
    else
    {
      EDF_TEST_VERIFY(!activeObject[ao_idx].event_received);
    }
  }
  EDF_TEST_VERIFY(success);

  // The last active object is subscibed to all event signals.
  EDF_TEST_VERIFY(activeObject[ACTIVE_OBJECT_N_TOTAL - 1].event_received);
}

EDF_TEST_CASE(last_signal, LAST_EVENT_SIGNAL)
EDF_TEST_CASE_INIT(last_signal)
{
  EDF_activeObject_publish(&lastSig);
}
EDF_TEST_CASE_VERIFY(last_signal, e)
{
  EMF_UTILS_UNUSED_PARAM(e);

  /**
   * Check that any active object except the last is subscribed to
   * LAST_EVENT_SIGNAL.
   */
  for (uint8_t ao_idx = 0; ao_idx < (ACTIVE_OBJECT_N_TOTAL - 1); ao_idx++)
  {
    EDF_TEST_VERIFY(!activeObject[ao_idx].event_received);
  }
  EDF_TEST_VERIFY(activeObject[ACTIVE_OBJECT_N_TOTAL - 1].event_received);
}

EDF_TEST_SUITE(EDF_TEST_RUN(signal_1),
               EDF_TEST_RUN(signal_2),
               EDF_TEST_RUN(signal_3),
               EDF_TEST_RUN(last_signal))