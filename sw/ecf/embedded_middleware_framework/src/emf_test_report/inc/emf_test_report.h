/*******************************************************************************
 * @brief Test report utilities.
 *
 * Provides formatted test reporting compatible on both target and host
 * environments.
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

#ifndef TEST_REPORT_H
#define TEST_REPORT_H

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */
#include <stdint.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief Enumeration of possible test results.
 */
typedef enum
{
  TEST_PASSED,
  TEST_SKIPPED,
  TEST_FAILED
} EMF_testReport_testResult_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Prints the test report header with framework and suite names.
 *
 * @param[in] frameworkName Name of the test framework.
 * @param[in] suiteName Name of the test suite.
 */
void EMF_testReport_printHeader(const char* frameworkName,
                                const char* suiteName);

/**
 * @brief Prints the test number and name at the start of test execution.
 *
 * @param[in] test_num Number of the test.
 * @param[in] test_name Name of the test.
 */
void EMF_testReport_printTestIntro(uint16_t test_num, const char* test_name);

/**
 * @brief Prints the result of a test.
 *
 * @param[in] test_result Test result (passed, skipped, failed).
 * @param[in] fail_cond Failed condition (if any).
 * @param[in] fail_file File where the failure occurred.
 * @param[in] fail_line Line number of the failure.
 */
void EMF_testReport_printTestResult(EMF_testReport_testResult_t test_result,
                                    const char* fail_cond,
                                    const char* fail_file,
                                    int fail_line);

/**
 * @brief Prints a summary of all test results.
 *
 * @param[in] totalTests Total number of tests run.
 * @param[in] skippedTests Number of skipped tests.
 * @param[in] failedTests Number of failed tests.
 */
void EMF_testReport_printSummary(uint16_t totalTests,
                                 uint16_t skippedTests,
                                 uint16_t failedTests);

#endif /* TEST_REPORT_H */
