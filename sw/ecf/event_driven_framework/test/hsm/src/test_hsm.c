/*******************************************************************************
 * @brief Hierarchical state machine (HSM) tests.
 *
 * Verifies correct execution of all kinds of HSM transitions.
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
#include "hsm.h"
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
 * @brief Immutable event used to trigger state transitions in the test HSM.
 */
const EDF_event_t transitionEvent = EDF_EVENT_IMMUTABLE_INIT(TRANSITION);

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

EDF_TEST_SUITE_NAME(test_hsm)

EDF_TEST_EVENT_BUFF(1)

void EDFTest_onInit(void)
{
  // Initial transition
  hsm_init();
  EDF_TEST_VERIFY(EDF_HSM_UPCAST(&hsm)->current_state ==
                  (EDF_hsm_stateHandler_t)S11111);
}

EDF_TEST_CASE(transition_1, TRANSITION)
EDF_TEST_CASE_INIT(transition_1)
{
  EDF_activeObject_publish(&transitionEvent);
}
EDF_TEST_CASE_VERIFY(transition_1, e)
{
  EMF_UTILS_UNUSED_PARAM(e);

  EDF_TEST_VERIFY(EDF_HSM_UPCAST(&hsm)->current_state ==
                  (EDF_hsm_stateHandler_t)S2111);
}

EDF_TEST_CASE(transition_2, TRANSITION)
EDF_TEST_CASE_INIT(transition_2)
{
  EDF_activeObject_publish(&transitionEvent);
}
EDF_TEST_CASE_VERIFY(transition_2, e)
{
  EMF_UTILS_UNUSED_PARAM(e);

  EDF_TEST_VERIFY(EDF_HSM_UPCAST(&hsm)->current_state ==
                  (EDF_hsm_stateHandler_t)S2112);
}

EDF_TEST_CASE(transition_3, TRANSITION)
EDF_TEST_CASE_INIT(transition_3)
{
  EDF_activeObject_publish(&transitionEvent);
}
EDF_TEST_CASE_VERIFY(transition_3, e)
{
  EMF_UTILS_UNUSED_PARAM(e);

  EDF_TEST_VERIFY(EDF_HSM_UPCAST(&hsm)->current_state ==
                  (EDF_hsm_stateHandler_t)S2);
}

EDF_TEST_CASE(transition_4, TRANSITION)
EDF_TEST_CASE_INIT(transition_4)
{
  EDF_activeObject_publish(&transitionEvent);
}
EDF_TEST_CASE_VERIFY(transition_4, e)
{
  EMF_UTILS_UNUSED_PARAM(e);

  EDF_TEST_VERIFY(EDF_HSM_UPCAST(&hsm)->current_state ==
                  (EDF_hsm_stateHandler_t)S21);
}

EDF_TEST_CASE(transition_5, TRANSITION)
EDF_TEST_CASE_INIT(transition_5)
{
  EDF_activeObject_publish(&transitionEvent);
}
EDF_TEST_CASE_VERIFY(transition_5, e)
{
  EMF_UTILS_UNUSED_PARAM(e);

  EDF_TEST_VERIFY(EDF_HSM_UPCAST(&hsm)->current_state ==
                  (EDF_hsm_stateHandler_t)S31);
}

EDF_TEST_CASE(transition_6, TRANSITION)
EDF_TEST_CASE_INIT(transition_6)
{
  EDF_activeObject_publish(&transitionEvent);
}
EDF_TEST_CASE_VERIFY(transition_6, e)
{
  EMF_UTILS_UNUSED_PARAM(e);

  EDF_TEST_VERIFY(EDF_HSM_UPCAST(&hsm)->current_state ==
                  (EDF_hsm_stateHandler_t)S3);
}

EDF_TEST_CASE(transition_7, TRANSITION)
EDF_TEST_CASE_INIT(transition_7)
{
  EDF_activeObject_publish(&transitionEvent);
}
EDF_TEST_CASE_VERIFY(transition_7, e)
{
  EMF_UTILS_UNUSED_PARAM(e);

  EDF_TEST_VERIFY(EDF_HSM_UPCAST(&hsm)->current_state ==
                  (EDF_hsm_stateHandler_t)S311);
}

EDF_TEST_CASE(transition_8, TRANSITION)
EDF_TEST_CASE_INIT(transition_8)
{
  EDF_activeObject_publish(&transitionEvent);
}
EDF_TEST_CASE_VERIFY(transition_8, e)
{
  EMF_UTILS_UNUSED_PARAM(e);

  EDF_TEST_VERIFY(EDF_HSM_UPCAST(&hsm)->current_state ==
                  (EDF_hsm_stateHandler_t)S41);
}

EDF_TEST_CASE(transition_9, TRANSITION)
EDF_TEST_CASE_INIT(transition_9)
{
  hsm.self_transition = true;
  EDF_activeObject_publish(&transitionEvent);
}
EDF_TEST_CASE_VERIFY(transition_9, e)
{
  EMF_UTILS_UNUSED_PARAM(e);

  EDF_TEST_VERIFY(EDF_HSM_UPCAST(&hsm)->current_state ==
                  (EDF_hsm_stateHandler_t)S41);
}

EDF_TEST_CASE(transition_10, TRANSITION)
EDF_TEST_CASE_INIT(transition_10)
{
  hsm.self_transition = false;
  EDF_activeObject_publish(&transitionEvent);
}
EDF_TEST_CASE_VERIFY(transition_10, e)
{
  EMF_UTILS_UNUSED_PARAM(e);

  EDF_TEST_VERIFY(EDF_HSM_UPCAST(&hsm)->current_state ==
                  (EDF_hsm_stateHandler_t)S41);
}

EDF_TEST_SUITE(EDF_TEST_RUN(transition_1),
               EDF_TEST_RUN(transition_2),
               EDF_TEST_RUN(transition_3),
               EDF_TEST_RUN(transition_4),
               EDF_TEST_RUN(transition_5),
               EDF_TEST_RUN(transition_6),
               EDF_TEST_RUN(transition_7),
               EDF_TEST_RUN(transition_8),
               EDF_TEST_RUN(transition_9),
               EDF_TEST_RUN(transition_10))