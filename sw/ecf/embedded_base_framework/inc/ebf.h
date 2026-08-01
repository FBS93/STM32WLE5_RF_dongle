/*******************************************************************************
 * @brief Core-independent base.
 *
 * Provides a reusable software foundation shared across all cores/ports.
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

#ifndef EBF_H
#define EBF_H

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
#include "ebf_utils.h"
#include "ebf_config.h"

/**
 * Conditional library headers include depending on EBF_CORE
 * (defined in ebf_config.h)
 */
#if (EBF_CORE == EBF_CORE_OS)
#include <pthread.h>
#endif

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/**
 * @brief Compiler-agnostic definition of weak function attribute.
 */
#if (defined(__GNUC__) || defined(__clang__))
#define EBF_WEAK __attribute__((weak))
#else
#define EBF_WEAK
#warning "weak functions not supported on this compiler"
#endif

/**
 * @brief Enters a critical section.
 *
 * @note Nested critical sections not supported.
 */
#define EBF_CRITICAL_SECTION_ENTRY() EBF_entryCriticalSection()

/**
 * @brief Exits a critical section.
 *
 * @note Nested critical sections not supported.
 */
#define EBF_CRITICAL_SECTION_EXIT() EBF_exitCriticalSection()

/**
 * @brief Returns true if currently inside a critical section, false otherwise.
 */
#define EBF_IS_IN_CRITICAL_SECTION() (EBF_isInCriticalSection())

/**
 * @brief No operation.
 */
#define EBF_NOP() EBF_noOperation();

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief Function pointer type for stdin listener.
 *
 * @param[in] data Pointer to received data.
 * @param[in] len Length of the data in bytes.
 */
typedef void (*EBF_stdin_t)(const uint8_t* data, uint16_t len);

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

#if (EBF_CORE == EBF_CORE_OS)
/**
 * @brief Mutex protecting critical sections to ensure exclusive thread access.
 *
 * The mutex guarantees mutual exclusion but does not prevent context switches
 * within the critical section.
 */
extern pthread_mutex_t EBF_critSecMutex;

/**
 * @brief Tracks critical section nesting depth.
 *
 * Used to detect unbalanced entry/exit. Nesting is not supported (must be 0 or
 * 1).
 */
extern int8_t EBF_critSecNestCnt;
#endif  // (EBF_CORE == EBF_CORE_OS)

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Registers a callback to receive standard input data.
 *
 * Defined as weak, so it can be overridden by a custom implementation
 * if needed.
 *
 * @param[in] listener Function to be called with incoming data.
 */
void EBF_setStdinListener(EBF_stdin_t listener);

/**
 * @brief Sends data to the standard output.
 *
 * Defined as weak, so it can be overridden by a custom implementation
 * if needed.
 *
 * @param[in] data Pointer to data to be sent.
 * @param[in] len Length of the data in bytes.
 */
void EBF_stdoutWrite(const uint8_t* data, uint16_t len);

/**
 * @brief Checks if the output is ready to send the specified number of bytes.
 *
 * Defined as weak, so it can be overridden by a custom implementation
 * if needed.
 *
 * @param[in] len Number of bytes to check for availability.
 * @return true if ready, false otherwise.
 */
bool EBF_stdoutIsReadyToWrite(uint16_t len);

/**
 * @brief Entry critical section.
 *
 * Do not call directly. Use EBF_CRITICAL_SECTION_ENTRY macro instead.
 *
 * This declaration is required for correct macro expansion in user code.
 */
void EBF_entryCriticalSection(void);

/**
 * @brief Exit critical section.
 *
 * Do not call directly. Use EBF_CRITICAL_SECTION_EXIT macro instead.
 *
 * This declaration is required for correct macro expansion in user code.
 */
void EBF_exitCriticalSection(void);

/**
 * @brief Returns true if currently inside a critical section.
 *
 * Do not call directly. Use EBF_IS_IN_CRITICAL_SECTION macro instead.
 *
 * This declaration is required for correct macro expansion in user code.
 */
bool EBF_isInCriticalSection(void);

/**
 * @brief No operation.
 *
 * Do not call directly. Use EBF_NOP macro instead.
 *
 * This declaration is required for correct macro expansion in user code.
 */
void EBF_noOperation(void);

#endif  // EBF_H