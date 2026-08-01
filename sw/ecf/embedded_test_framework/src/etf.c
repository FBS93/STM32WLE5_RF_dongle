/*******************************************************************************
 * @brief Lightweight C test framework implementation.
 *
 * @copyright
 * Copyright (c) 2026 FBS93.
 * See the LICENSE file of this project for license details.
 * This notice shall be retained in all copies or substantial portions
 * of the software.
 *
 * @note
 * This file is a derivative work based on:
 * Embedded-Test (c) Quantum Leaps, LLC.
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
#include "etf.h"
#include "ebf.h"
#include "eaf.h"
#include "emf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/**
 * @brief Internal state of the ETF.
 *
 * Holds counters and flags related to test execution.
 */
typedef struct
{
  uint16_t test_cnt; /**< Counter of executed tests */
  uint16_t skip_cnt; /**< Counter of skipped tests */
  bool last_skipped; /**< Flag indicating if the last test was skipped */
} ETF_t;

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/**
 * @brief Define static file name string for asserts.
 */
EAF_DEFINE_THIS_FILE(__FILE__);

/**
 * @brief Internal ETF state.
 */
static ETF_t ETF;

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

bool ETF_test(const char* title, bool execute)
{
  EAF_ASSERT(title != NULL);

  if (ETF.last_skipped)
  {
    EMF_testReport_printTestResult(TEST_SKIPPED, NULL, NULL, 0);
  }
  else if (ETF.test_cnt > 0)
  {
    ETF_tearDown();
    EMF_testReport_printTestResult(TEST_PASSED, NULL, NULL, 0);
  }

  ETF.test_cnt++;

  EMF_testReport_printTestIntro(ETF.test_cnt, title);
  if (execute)
  {
    ETF_setUp();
  }
  else
  {
    ETF.skip_cnt++;
  }
  ETF.last_skipped = !execute;

  return execute;
}

void ETF_fail(const char* cond, const char* file, int line)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(cond != NULL);
  EAF_ASSERT_IN_BLOCK(file != NULL);
  EAF_ASSERT_BLOCK_END();

  EMF_testReport_printTestResult(TEST_FAILED, cond, file, line);
  EMF_testReport_printSummary(ETF.test_cnt, ETF.skip_cnt, 1);

  ETF_onExit(-1);  // Failure

  // Halt in case ETF_onExit() returns unexpectedly
  while (true)
  {
  }
}

EBF_WEAK void ETF_onInit(void)
{
}

EBF_WEAK void ETF_onExit(int err)
{
  exit(err);
}

EBF_WEAK void ETF_setUp(void)
{
}

EBF_WEAK void ETF_tearDown(void)
{
}

int main(void)
{
  ETF.test_cnt = 0;
  ETF.skip_cnt = 0;
  ETF.last_skipped = false;

  ETF_onInit();

  EMF_testReport_printHeader("Embedded test framework", ETF_testSuite);

  ETF_run();

  if (ETF.last_skipped)
  {
    EMF_testReport_printTestResult(TEST_SKIPPED, NULL, NULL, 0);
  }
  else
  {
    ETF_tearDown();
    EMF_testReport_printTestResult(TEST_PASSED, NULL, NULL, 0);
  }

  EMF_testReport_printSummary(ETF.test_cnt, ETF.skip_cnt, 0);

  ETF_onExit(0);  // Success
}
