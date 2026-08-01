/*******************************************************************************
 * @brief Runtime asserts and error handling.
 *
 * Provides runtime asserts and error handling with critical section protection,
 * ensuring atomic evaluation.
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

#ifndef EAF_H
#define EAF_H

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
 * @brief Define static file name string for error reporting.
 *
 * Shall be declared once per source file before using the other macros of
 * this file.
 */
#define EAF_DEFINE_THIS_FILE(file) static const char EAF_file[] = file

/**
 * @brief Assert expression with critical section protection.
 *
 * @param[in] expr Expression to assert.
 * @param[in] id User-defined identifier for the assertion. Value 0 is
 * reserved for assertions without explicit ID, see @ref EAF_ASSERT.
 */
#define EAF_ASSERT_ID(expr, id)               \
  do                                          \
  {                                           \
    bool in_crit = EAF_isInCriticalSection(); \
    if (!in_crit)                             \
    {                                         \
      EAF_criticalSectionEntry();             \
    }                                         \
    if (!(expr))                              \
    {                                         \
      EAF_onError(EAF_file, (__LINE__), id);  \
    }                                         \
    if (!in_crit)                             \
    {                                         \
      EAF_criticalSectionExit();              \
    }                                         \
  } while (0)

/**
 * @brief Assert expression with critical section protection.
 *
 * @param[in] expr Expression to assert.
 */
#define EAF_ASSERT(expr) EAF_ASSERT_ID((expr), 0)

/**
 * @brief Report an error with critical section protection.
 *
 * @param[in] id User-defined identifier for the error. Value 0 is
 * reserved for errors without explicit ID, see @ref EAF_ERROR.
 */
#define EAF_ERROR_ID(id)                      \
  do                                          \
  {                                           \
    bool in_crit = EAF_isInCriticalSection(); \
    if (!in_crit)                             \
    {                                         \
      EAF_criticalSectionEntry();             \
    }                                         \
    EAF_onError(EAF_file, (__LINE__), id);    \
    if (!in_crit)                             \
    {                                         \
      EAF_criticalSectionExit();              \
    }                                         \
  } while (0)

/**
 * @brief Report an error with critical section protection.
 */
#define EAF_ERROR() EAF_ERROR_ID(0)

/**
 * @brief Assert expression inside a critical section context.
 *
 * @param[in] expr Expression to assert.
 * @param[in] id User-defined identifier for the assertion.
 * Value 0 is reserved for assertions without explicit ID,
 * see @ref EAF_ASSERT_IN_CRITICAL_SECTION.
 */
#define EAF_ASSERT_IN_CRITICAL_SECTION_ID(expr, id) \
  do                                                \
  {                                                 \
    if (!(expr))                                    \
      EAF_onError(EAF_file, (__LINE__), id);        \
  } while (false)

/**
 * @brief Assert expression inside a critical section context.
 *
 * @param[in] expr Expression to assert.
 */
#define EAF_ASSERT_IN_CRITICAL_SECTION(expr) \
  EAF_ASSERT_IN_CRITICAL_SECTION_ID(expr, 0)

/**
 * @brief Report an error inside a critical section context.
 *
 * @param[in] id User-defined identifier for the error.
 * Value 0 is reserved for errors without explicit ID,
 * see @ref EAF_ERROR_IN_CRITICAL_SECTION_ID.
 */
#define EAF_ERROR_IN_CRITICAL_SECTION_ID(id) \
  do                                         \
  {                                          \
    EAF_onError(EAF_file, (__LINE__), id);   \
  } while (false)

/**
 * @brief Report an error inside a critical section context.
 */
#define EAF_ERROR_IN_CRITICAL_SECTION() EAF_ERROR_IN_CRITICAL_SECTION_ID(0)

/**
 * @brief Begin a block of assertions executed atomically.
 *
 * Starts a single critical section so that multiple assertions can be
 * evaluated efficiently without repeated entry and exit operations.
 *
 * @note Shall be closed with @ref EAF_ASSERT_BLOCK_END().
 */
#define EAF_ASSERT_BLOCK_BEGIN()              \
  {                                           \
    bool in_crit = EAF_isInCriticalSection(); \
    if (!in_crit)                             \
    {                                         \
      EAF_criticalSectionEntry();             \
    }

/**
 * @brief Assert expression within an active assertion block.
 *
 * Executes an assertion inside a block started by
 * @ref EAF_ASSERT_BLOCK_BEGIN(), without altering the critical section state.
 *
 * @param[in] expr Expression to assert.
 * @param[in] id User-defined identifier for the assertion. Value 0 is
 * reserved for assertions without explicit ID, see @ref EAF_ASSERT_IN_BLOCK.
 */
#define EAF_ASSERT_IN_BLOCK_ID(expr, id) \
  EAF_ASSERT_IN_CRITICAL_SECTION_ID(expr, id)

/**
 * @brief Assert expression within an active assertion block.
 *
 * Executes an assertion inside a block started by
 * @ref EAF_ASSERT_BLOCK_BEGIN(), without altering the critical section state.
 *
 * @param[in] expr Expression to assert.
 */
#define EAF_ASSERT_IN_BLOCK(expr) EAF_ASSERT_IN_CRITICAL_SECTION_ID(expr, 0)

/**
 * @brief End an assertion block started with @ref EAF_ASSERT_BLOCK_BEGIN().
 *
 * Exits the critical section only if it was entered by the block itself.
 */
#define EAF_ASSERT_BLOCK_END() \
  if (!in_crit)                \
  {                            \
    EAF_criticalSectionExit(); \
  }                            \
  }

/**
 * @note
 * @ref EAF_ASSERT_IN_CRITICAL_SECTION_ID() and
 * @ref EAF_ASSERT_IN_CRITICAL_SECTION() shall always be used inside an already
 * active critical section. These macros do not modify the critical-section
 * state.
 *
 * In contrast, @ref EAF_ASSERT(), @ref EAF_ASSERT_ID() and the in-block
 * assertions (@ref EAF_ASSERT_IN_BLOCK() and @ref EAF_ASSERT_IN_BLOCK_ID())
 * used between @ref EAF_ASSERT_BLOCK_BEGIN() and @ref EAF_ASSERT_BLOCK_END()
 * first check whether execution is already inside a critical section:
 * - If not, they enter a critical section.
 * - Upon completion, they exit the critical section only if it was not active
 * prior to the assertion call.
 *
 * This behavior allows assertions to be used safely in code that may run either
 * inside or outside a critical section, while avoiding nested critical sections
 * which the Embedded C Framework (ECF) framework does not support.
 */

/**
 * @note
 * When using @ref EAF_ASSERT() or @ref EAF_ASSERT_ID() to validate expressions
 * involving shared or global variables, ensure that the entire condition is
 * evaluated atomically within a single assertion. Splitting related checks
 * across multiple calls to these macros can cause repeated entry and exit of
 * critical sections, opening small windows where an interrupt or concurrent
 * context may preempt execution and modify shared or global variables.
 *
 * **Incorrect:**
 * @code
 * EAF_ASSERT(global_var < x);
 * // Context may be interrupted between critical sections
 * EAF_ASSERT(global_var > y);
 * @endcode
 * @code
 * EAF_ASSERT_ID((global_var < x), 100);
 * // Context may be interrupted between critical sections
 * EAF_ASSERT_ID((global_var > y), 100);
 * @endcode
 *
 * **Correct:**
 * @code
 * EAF_ASSERT((global_var < x) && (global_var > y));
 * @endcode
 * @code
 * EAF_ASSERT_ID(((global_var < x) && (global_var > y)), 100);
 * @endcode
 *
 * @todo Integrate this note into the coding guidelines so AI tools can verify
 * it.
 */

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Error handler called on assertion failures.
 *
 * This function is defined as weak within the EAF framework,
 * providing a default implementation in case the user does not define
 * a specific behavior.
 *
 * Do not call directly. This declaration is required so the macros can
 * expand correctly in user code.
 *
 * @note This function is invoked while inside a critical section to ensure
 * atomic error handling. Users providing their own implementation must
 * consider that concurrent or asynchronous execution is blocked during
 * this call.
 *
 * @note The default weak implementation logs the error information and
 * enters an infinite loop to halt execution.
 *
 * @param[in] file Pointer to the file name string.
 * @param[in] line Line number where the error occurred.
 * @param[in] id Identifier associated with the error.
 */
void EAF_onError(const char* file, int line, int id);

/**
 * @brief Enter critical section.
 *
 * Encapsulates the platform-specific mechanism to enter a critical section.
 * This function abstracts dependencies on the underlying EBF library,
 * so users including this header do not require direct linkage or knowledge
 * of EBF API.
 *
 * Do not call directly. This declaration is required so the macros can
 * expand correctly in user code.
 */
void EAF_criticalSectionEntry(void);

/**
 * @brief Exit critical section.
 *
 * Encapsulates the platform-specific mechanism to exit a critical section.
 * This function abstracts dependencies on the underlying EBF library,
 * so users including this header do not require direct linkage or knowledge
 * of EBF API.
 *
 * Do not call directly. This declaration is required so the macros can
 * expand correctly in user code.
 */
void EAF_criticalSectionExit(void);

/**
 * @brief Check if currently inside a critical section.
 *
 * Encapsulates the platform-specific mechanism to determine if execution
 * is within a critical section. This function abstracts dependencies on
 * the underlying EBF library, so users including this header do not require
 * direct linkage or knowledge of the EBF API.
 *
 * Do not call directly. This declaration is required so the macros can
 * expand correctly in user code.
 */
bool EAF_isInCriticalSection(void);

#endif  // EAF_H
