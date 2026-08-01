/*******************************************************************************
 * @brief Event driven test framework.
 *
 * Provides a lightweight and self-contained test mechanism tailored for
 * Event Driven Framework (EDF). Tests run inside the actual EDF loop by using
 * a dedicated active object created at the lowest priority, ensuring
 * minimal interference with normal application behavior. This enables the
 * execution of test stimuli and validations under real runtime conditions,
 * while avoiding the intrusive main control flow of traditional test
 * frameworks or log-driven validation approaches that rely on parallel
 * executables.
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

#ifndef EDF_TEST_H
#define EDF_TEST_H

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
#include "edf.h"

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/**
 * @brief Defines the name of a test suite.
 *
 * Defines a constant string holding the name of the test suite.
 *
 * @param[in] name Name of the test suite.
 */
#define EDF_TEST_SUITE_NAME(name) const char EDFTest_testSuiteName[] = #name;

/**
 * @brief Defines the event buffer for the EDFTest active object.
 *
 * The buffer size should be set according to system needs.
 *
 * @param[in] size Buffer size.
 */
#define EDF_TEST_EVENT_BUFF(size)          \
  EDF_event_ptr EDFTest_eventBuffer[size]; \
  const EDF_eventQueue_cnt_t EDFTest_eventBufferSize = size;

/**
 * @brief Declares and defines a test case.
 *
 * @param[in] name Name of the test case.
 * @param[in] trigger_signal Signal used to trigger the test case verify phase.
 */
#define EDF_TEST_CASE(name, trigger_signal)        \
  static void name##_init(void);                   \
  static void name##_verify(const EDF_event_t* e); \
  static const EDFTest_test_t name##_test = {      \
    .test_name = #name,                            \
    .test_init = name##_init,                      \
    .test_verify = name##_verify,                  \
    .test_trigger = (trigger_signal)};

/**
 * @brief Defines the init function for a test case.
 *
 * @param[in] name Name of the test case.
 */
#define EDF_TEST_CASE_INIT(name) static void name##_init(void)

/**
 * @brief Defines the verify function for a test case.
 *
 * @param[in] name Name of the test case.
 * @param[in] input_evt_name Variable name for the input event.
 */
#define EDF_TEST_CASE_VERIFY(name, input_evt_name) \
  static void name##_verify(const EDF_event_t*(input_evt_name))

/**
 * @brief Verifies a test condition.
 *
 * If the condition fails, it triggers a test failure report.
 *
 * @param[in] cond_ Boolean expression to evaluate.
 */
#define EDF_TEST_VERIFY(cond_) \
  ((cond_) ? (void)0 : EDFTest_fail(#cond_, __FILE__, __LINE__));

/**
 * @brief Forces a test failure.
 *
 * Fails the test unconditionally with a custom message.
 *
 * @param[in] note Explanation or description of the failure (string literal).
 */
#define EDF_TEST_FAIL(note) (EDFTest_fail(note, __FILE__, __LINE__));

/**
 * @brief Register a test to be executed.
 *
 * @param[in] name Name of the test case.
 */
#define EDF_TEST_RUN(name)             \
  {                                    \
    .test = name##_test, .skip = false \
  }

/**
 * @brief Register a test to be skipped.
 *
 * @param[in] name Name of the test case.
 */
#define EDF_TEST_SKIP(name)           \
  {                                   \
    .test = name##_test, .skip = true \
  }

/**
 * @brief Define an array of test entries.
 *
 * @param[in] ... List of registered tests.
 */
#define EDF_TEST_SUITE(...)                                      \
  const EDFTest_testEntry_t EDFTest_testSuite[] = {__VA_ARGS__}; \
  const uint16_t EDFTest_numTests =                              \
    sizeof(EDFTest_testSuite) / sizeof(EDFTest_testEntry_t);

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief Function pointer type for test initialization.
 */
typedef void (*testInit_t)(void);

/**
 * @brief Function pointer type for test verification.
 */
typedef void (*testVerify_t)(const EDF_event_t* e);

/**
 * @brief Test case definition.
 */
typedef struct
{
  const char* test_name;           /**< Name of the test case */
  testInit_t test_init;            /**< Init function pointer */
  testVerify_t test_verify;        /**< Verify function pointer */
  EDF_event_signal_t test_trigger; /**< Test verify signal trigger*/
} EDFTest_test_t;

/**
 * @brief Represents an entry in the test suite.
 */
typedef struct
{
  EDFTest_test_t test; /**< Test case definition */
  bool skip;           /**< Whether the test should be skipped */
} EDFTest_testEntry_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/**
 * @brief Name of the current test suite.
 *
 * Defined in the test source file.
 */
extern const char EDFTest_testSuiteName[];

/**
 * @brief Event buffer for the EDFTest active object.
 *
 * Defined in the test source file.
 */
extern EDF_event_ptr EDFTest_eventBuffer[];

/**
 * @brief Event buffer size for the EDFTest active object.
 *
 * Defined in the test source file.
 */
extern const EDF_eventQueue_cnt_t EDFTest_eventBufferSize;

/**
 * @brief Array of test entries.
 *
 * Defined in the test source file.
 */
extern const EDFTest_testEntry_t EDFTest_testSuite[];

/**
 * @brief Number of tests in the suite.
 *
 * Defined in the test source file.
 */
extern const uint16_t EDFTest_numTests;

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Reports a test failure.
 *
 * Used internally by macros.
 *
 * @param[in] cond Description of the failed condition or reason.
 * @param[in] file Source file where the failure occurred.
 * @param[in] line Line number where the failure occurred.
 */
void EDFTest_fail(const char* cond, const char* file, int line);

/**
 * @brief Init callback.
 *
 * This function is defined as weak within the EDF Test framework,
 * providing a default implementation in case the user does not define
 * a specific behavior.
 *
 * @note All user software shall be initialized in this function,
 * but @ref EDF_run() must NOT be called, as control must return to the
 * EDF test framework.
 */
void EDFTest_onInit(void);

/**
 * @brief Exit callback.
 *
 * This function is defined as weak within the EDF Test framework,
 * providing a default implementation in case the user does not define
 * a specific behavior.
 *
 * @param[in] err Exit code to return.
 */
void EDFTest_onExit(int err);

/**
 * @brief Pre-test callback.
 *
 * This function is defined as weak within the EDF Test framework,
 * providing a default implementation in case the user does not define
 * a specific behavior.
 */
void EDFTest_setUp(void);

/**
 * @brief Post-test callback.
 *
 * This function is defined as weak within the EDF Test framework,
 * providing a default implementation in case the user does not define
 * a specific behavior.
 */
void EDFTest_tearDown(void);

#endif /* EDF_TEST_H */
