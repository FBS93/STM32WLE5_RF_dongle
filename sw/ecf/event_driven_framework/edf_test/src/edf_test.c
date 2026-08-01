/*******************************************************************************
 * @brief Event driven test framework implementation.
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
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "edf_test.h"
#include "ebf.h"
#include "edf.h"
#include "eaf.h"
#include "emf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief Defines the priority for the EDFTest active object.
 *
 * Set to the lowest available priority. This ensures that tests run with
 * minimal intrusion to the system's behavior.
 *
 * @note In case the user wants to use the EDF Test library, the user shall
 * leave the lowest priority (1) free in their application code so that the
 * EDFTest active object can use it.
 *
 * @todo Move this note to the module documentation to make it highly visible
 * for the user.
 */
#define EDF_TEST_AO_PRIO (EDF_activeObject_prio_t)1

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/**
 * @brief EDFTest active object structure
 */
typedef struct
{
  EDF_activeObject_t super;             /**< Active object base class*/
  EDF_event_ptr* event_buff;            /**< Event buffer */
  EDF_eventQueue_cnt_t event_buff_size; /**< Event buffer size */
  const EDFTest_testEntry_t* tests;     /**< Tests */
  uint16_t test_idx;                    /**< Current test index */
  uint16_t skip_cnt;                    /**< Number of tests skipped */
  uint16_t num_tests;                   /**< Number of test entries */
} EDFTest_t;

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/**
 * @brief Define static file name string for asserts.
 */
EAF_DEFINE_THIS_FILE(__FILE__);

/**
 * @brief Instance of the EDFTest active object.
 */
static EDFTest_t EDFTest;

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
 * @brief Initial transition of the EDFTest HSM.
 *
 * @param[in] me  Pointer to the EDFTest instance.
 * @param[in] e   Pointer to the event.
 */
static EDF_hsm_stateReturn_t initialTransition(EDFTest_t* me,
                                               const EDF_event_t* e);

/**
 * @brief waitForRxCmd state of the EDFTest HSM.
 *
 * @param[in] me  Pointer to the EDFTest instance.
 * @param[in] e   Pointer to the event.
 */
static EDF_hsm_stateReturn_t waitForEvent(EDFTest_t* me, const EDF_event_t* e);

/**
 * @brief Initializes the EDFTest active object.
 */
static void EDFTestInit(void);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static EDF_hsm_stateReturn_t initialTransition(EDFTest_t* me,
                                               const EDF_event_t* e)
{
  EMF_UTILS_UNUSED_PARAM(me);
  EMF_UTILS_UNUSED_PARAM(e);

  return EDF_HSM_RET_TRAN(waitForEvent);
}

static EDF_hsm_stateReturn_t waitForEvent(EDFTest_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  if (me->tests[me->test_idx].test.test_trigger == e->sig)
  {
    me->tests[me->test_idx].test.test_verify(e);  // Verify test
    EDFTest_tearDown();
    EMF_testReport_printTestResult(TEST_PASSED, NULL, NULL, 0);
    me->test_idx++;

    while ((EDFTest.test_idx < EDFTest.num_tests) &&
           EDFTest.tests[EDFTest.test_idx].skip)
    {
      EMF_testReport_printTestIntro(
        EDFTest.test_idx + 1, EDFTest.tests[EDFTest.test_idx].test.test_name);
      EMF_testReport_printTestResult(TEST_SKIPPED, NULL, NULL, 0);
      EDFTest.test_idx++;
      EDFTest.skip_cnt++;
    }

    if (EDFTest.test_idx < EDFTest.num_tests)
    {
      EMF_testReport_printTestIntro(
        EDFTest.test_idx + 1, EDFTest.tests[EDFTest.test_idx].test.test_name);
      EDFTest_setUp();
      EDFTest.tests[EDFTest.test_idx].test.test_init();  // Init test
    }
    else
    {
      EMF_testReport_printSummary(EDFTest.num_tests, EDFTest.skip_cnt, 0);

      // Disable EDFTest AO by unsubscribing from all signals
      EDF_activeObject_unsubscribeAll(EDF_AO_UPCAST(EDFTest));

      EDFTest_onExit(0);  // Success
    }

    state = EDF_HSM_RET_HANDLED();
  }
  else
  {
    state = EDF_HSM_RET_SUPER(&EDF_hsm_top);
  }

  return state;
}

static void EDFTestInit(void)
{
  // Init active object
  EDF_activeObject_init(EDF_AO_UPCAST(EDFTest),
                        (EDF_hsm_stateHandler_t)initialTransition);

  // Init active object private attributes
  EDFTest.event_buff = EDFTest_eventBuffer;
  EDFTest.event_buff_size = EDFTest_eventBufferSize;
  EDFTest.tests = EDFTest_testSuite;
  EDFTest.test_idx = 0;
  EDFTest.num_tests = EDFTest_numTests;

  // Start active object
  /**
   * @todo Review the last three parameters (currently NULL, 0, NULL).
   * They may need to be conditionally defined depending on the CORE and PORT
   * configuration used.
   */
  EDF_activeObject_start(EDF_AO_UPCAST(EDFTest),
                         EDF_TEST_AO_PRIO,
                         EDFTest.event_buff,
                         EDFTest.event_buff_size,
                         NULL,
                         0,
                         NULL);

  // Subscribe test AO to all signals
  EDF_activeObject_subscribeAll(EDF_AO_UPCAST(EDFTest));
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void EDFTest_fail(const char* cond, const char* file, int line)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(cond != NULL);
  EAF_ASSERT_IN_BLOCK(file != NULL);
  EAF_ASSERT_BLOCK_END();

  EMF_testReport_printTestResult(TEST_FAILED, cond, file, line);
  EMF_testReport_printSummary((EDFTest.test_idx + 1), EDFTest.skip_cnt, 1);

  // Disable EDFTest AO by unsubscribing from all signals
  EDF_activeObject_unsubscribeAll(EDF_AO_UPCAST(EDFTest));

  EDFTest_onExit(-1);  // Failure

  // Halt in case EDFTest_onExit() returns unexpectedly
  while (true)
  {
  }
}

EBF_WEAK void EDFTest_onInit(void)
{
}

EBF_WEAK void EDFTest_onExit(int err)
{
  exit(err);
}

EBF_WEAK void EDFTest_setUp(void)
{
}

EBF_WEAK void EDFTest_tearDown(void)
{
}

int main(void)
{
  int ret;

  EMF_testReport_printHeader("Event driven test framework",
                             EDFTest_testSuiteName);

  EDFTest_onInit();

  EDFTestInit();

  // Initialize the first test
  while ((EDFTest.test_idx < EDFTest.num_tests) &&
         EDFTest.tests[EDFTest.test_idx].skip)
  {
    EMF_testReport_printTestIntro(
      EDFTest.test_idx + 1, EDFTest.tests[EDFTest.test_idx].test.test_name);
    EMF_testReport_printTestResult(TEST_SKIPPED, NULL, NULL, 0);
    EDFTest.test_idx++;
    EDFTest.skip_cnt++;
  }

  if (EDFTest.test_idx < EDFTest.num_tests)
  {
    EMF_testReport_printTestIntro(
      EDFTest.test_idx + 1, EDFTest.tests[EDFTest.test_idx].test.test_name);
    EDFTest_setUp();
    EDFTest.tests[EDFTest.test_idx].test.test_init();  // Init test
    ret = EDF_run();
  }
  else
  {
    EMF_testReport_printSummary(EDFTest.num_tests, EDFTest.skip_cnt, 0);

    // Disable EDFTest AO by unsubscribing from all signals
    EDF_activeObject_unsubscribeAll(EDF_AO_UPCAST(EDFTest));

    EDFTest_onExit(0);  // Success
    ret = 0;
  }

  return ret;
}

/**
 * @todo Document that the test framework invokes user-defined functions such as
 * setup, teardown, test initialization and test verification *without*
 * entering a critical section. It is therefore the user's responsibility to
 * ensure proper protection when concurrent contexts or interrupts may occur and
 * could affect the test correctness.
 */
