/*******************************************************************************
 * @brief Lightweight C test framework.
 *
 * Test execution compatible on both target and host environments.
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

#ifndef ETF_H
#define ETF_H

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */
#include <stdbool.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/**
 * @brief Defines a test suite.
 *
 * Defines a test suite name and the main test runner function.
 *
 * @param[in] name Name of the test suite.
 */
#define ETF_TEST_SUITE(name)          \
  const char ETF_testSuite[] = #name; \
  void ETF_run(void)

/**
 * @brief Start test.
 *
 * @param[in] title Name of the test.
 */
#define ETF_TEST(title) if (ETF_test(#title, true))

/**
 * @brief Skip test.
 *
 * The test is registered but not executed.
 *
 * @param[in] title Name of the test.
 */
#define ETF_SKIP_TEST(title) if (ETF_test(#title, false))

/**
 * @brief Verifies a test condition.
 *
 * If the condition fails, it triggers a test failure.
 *
 * @param[in] cond Boolean expression to evaluate.
 */
#define ETF_VERIFY(cond) \
  ((cond) ? (void)0 : ETF_fail(#cond, __FILE__, __LINE__))

/**
 * @brief Forces a test failure.
 *
 * Fails the test unconditionally with a custom message.
 *
 * @param[in] note Explanation or description of the failure (string literal).
 */
#define ETF_FAIL(note) (ETF_fail(note, __FILE__, __LINE__))

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/**
 * @brief Name of the current test suite.
 *
 * Defined in the test source file.
 */
extern const char ETF_testSuite[];

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Internal runner for the test suite.
 *
 * Automatically invoked by TEST_SUITE macro.
 */
void ETF_run(void);

/**
 * @brief Registers a test.
 *
 * Called by macros to control test execution.
 *
 * @param[in] title Name of the test case.
 * @param[in] execute Whether the test should be executed or just registered.
 * @return true if the test should execute, false otherwise.
 */
bool ETF_test(const char* title, bool execute);

/**
 * @brief Reports a test failure.
 *
 * Used internally by macros.
 *
 * @param[in] cond Description of the failed condition or reason.
 * @param[in] file Source file where the failure occurred.
 * @param[in] line Line number where the failure occurred.
 */
void ETF_fail(const char* cond, const char* file, int line);

/**
 * @brief Init callback.
 *
 * This function is defined as weak within the ETF framework,
 * providing a default implementation in case the user does not define
 * a specific behavior.
 */
void ETF_onInit(void);

/**
 * @brief Exit callback.
 *
 * This function is defined as weak within the ETF framework,
 * providing a default implementation in case the user does not define
 * a specific behavior.
 *
 * @param[in] err Exit code to return.
 */
void ETF_onExit(int err);

/**
 * @brief Pre-test callback.
 *
 * This function is defined as weak within the ETF framework,
 * providing a default implementation in case the user does not define
 * a specific behavior.
 */
void ETF_setUp(void);

/**
 * @brief Post-test callback.
 *
 * This function is defined as weak within the ETF framework,
 * providing a default implementation in case the user does not define
 * a specific behavior.
 */
void ETF_tearDown(void);

#endif /* ETF_H */
