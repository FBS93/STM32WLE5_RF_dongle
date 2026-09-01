/*******************************************************************************
 * @brief Print utilities.
 *
 * Provides lightweight formatted output functions compatible on both target
 * and host environments.
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

#ifndef EMF_PRINT_H
#define EMF_PRINT_H

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */
#include <stdint.h>
#include <stddef.h>

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
 * @brief Casts an unsigned integer for printing.
 *
 * @param[in] val Unsigned integer value.
 * @return Casted value.
 */
#define EMF_PRINT_ARG_U(val) ((uintptr_t)(val))

/**
 * @brief Casts a signed integer for printing.
 *
 * @param[in] val Signed integer value.
 * @return Casted value.
 */
#define EMF_PRINT_ARG_I(val) ((uintptr_t)(int32_t)(val))

/**
 * @brief Casts a float for printing.
 *
 * @param[in] val Floating-point value.
 * @return Casted value.
 */
// clang-format off
#define EMF_PRINT_ARG_F(val) \
  (((union { float f; uintptr_t u; }){ .f = (val) }).u)
// clang-format on

/**
 * @brief Casts a string pointer for printing.
 *
 * @param[in] ptr Pointer to a null-terminated string.
 * @return Casted pointer.
 */
#define EMF_PRINT_ARG_S(ptr) ((uintptr_t)(ptr))

/**
 * @brief Casts a character for printing.
 *
 * @param[in] val Character value.
 * @return Casted value.
 */
#define EMF_PRINT_ARG_C(val) ((uintptr_t)(uint8_t)(val))

/**
 * @brief Counts number of variadic arguments passed (up to 8).
 *
 * @todo Review if 8 is sufficient for maximum number of arguments.
 * @todo Consider moving this macro to a generic middleware module and using
 * it wherever macros of this type are used (e.g., emf_log, eff).
 *
 * @return Number of arguments (0–8).
 */
#define EMF_PRINT_NARG(...) \
  EMF_PRINT_NARG_(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)

/**
 * @brief Helper macro for counting variadic arguments.
 *
 * @todo Review if 8 is sufficient for maximum number of arguments.
 * @todo Consider moving this macro to a generic middleware module and using
 * it wherever macros of this type are used (e.g., emf_log, eff).
 *
 * @return Number of arguments (0–8).
 */
#define EMF_PRINT_NARG_(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N

/**
 * @brief Safe variadic print macro using EMF_print.
 *
 * If no arguments are passed, it calls EMF_print with NULL.
 * Otherwise, it builds a local array of arguments and passes them.
 *
 * @todo Limit of 8 arguments is imposed by EMF_PRINT_NARG.
 * @todo If the limit is changed update the documentation.
 *
 * @note This macro shall not be called from an active EBF critical section.
 * See @ref emf_stdout_critical_section_precondition.
 *
 * @param[in] fmt Format string.
 * @param[in] ... Variable arguments (max 8).
 */
#define EMF_PRINT(fmt, ...)               \
  do                                      \
  {                                       \
    if (EMF_PRINT_NARG(__VA_ARGS__) == 0) \
    {                                     \
      EMF_print(fmt, NULL);               \
    }                                     \
    else                                  \
    {                                     \
      uintptr_t args[] = {__VA_ARGS__};   \
      EMF_print(fmt, args);               \
    }                                     \
  } while (0)

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
 * @brief Print a formatted message with arguments.
 *
 * Function used by EMF_PRINT macro.
 *
 * The complete message is written atomically through EBF stdout. EAF is
 * triggered if the formatted message exceeds @c EMF_PRINT_MAX_LEN or EBF
 * stdout rejects the write.
 *
 * @note This function shall not be called from an active EBF critical section.
 * See @ref emf_stdout_critical_section_precondition.
 *
 * @param[in] fmt Null-terminated format string.
 * @param[in] args Pointer to array of arguments, or NULL.
 */
void EMF_print(const char* fmt, const uintptr_t* args);

/**
 * @anchor uintptr_t_for_variadic_arguments
 * @par @c uintptr_t for variadic arguments
 *
 * This module uses @c uintptr_t to represent variadic arguments in a
 * platform-agnostic and type-safe way. @c uintptr_t is an unsigned integer
 * type guaranteed to be capable of storing any pointer value without loss,
 * making it suitable for encoding both scalar values (e.g., int, float, char)
 * and pointers (e.g., strings).
 *
 * It is critical to use @c uintptr_t for pointers because their size can
 * vary between platforms (e.g., 32-bit vs 64-bit) and storing a pointers like
 * string in a smaller type like @c uint32_t would lead to data loss and
 * invalid pointers. For scalar values that are known to fit in 32 bits,
 * @c uint32_t could be used, but using @c uintptr_t unifies the
 * representation safely.
 *
 * Using @c uintptr_t ensures the implementation is portable across 32-bit
 * and 64-bit systems, which is especially important for dual-targeted code.
 * Using @c void* instead would not be portable, since it is only valid for
 * storing object pointers, not arbitrary scalar values.
 *
 * @todo Consider supporting 64-bit values (e.g., int64_t, double) if required.
 * This would require extending the representation (e.g., using uint64_t or a
 * wider union) to handle types larger than uintptr_t, while ensuring
 * portability on both 32-bit and 64-bit systems.
 */

/**
 * @anchor emf_stdout_critical_section_precondition
 * @par Standard-output critical-section precondition
 *
 * Formatted output is written through EBF stdout. The @c EBF_stdoutWrite()
 * implementation shall protect stdout internally with an EBF critical section.
 * Nested EBF critical sections are not supported. Therefore, functions relying
 * on EBF stdout shall not be called from an active EBF critical section.
 */

#endif /* EMF_PRINT_H */
