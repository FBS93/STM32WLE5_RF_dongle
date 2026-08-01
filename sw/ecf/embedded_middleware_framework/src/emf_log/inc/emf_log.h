/*******************************************************************************
 * @brief Logging utilities.
 *
 * Supports logging on both target and host environments.
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

#ifndef EMF_LOG_H
#define EMF_LOG_H

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
#include "emf_print.h"

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/**
 * @brief Casts an unsigned integer for logging.
 *
 * @param[in] val Unsigned integer value.
 * @return Casted value.
 */
#define EMF_LOG_ARG_U(val) EMF_PRINT_ARG_U(val)

/**
 * @brief Casts a signed integer for logging.
 *
 * @param[in] val Signed integer value.
 * @return Casted value.
 */
#define EMF_LOG_ARG_I(val) EMF_PRINT_ARG_I(val)

/**
 * @brief Casts a float for logging.
 *
 * @param[in] val Floating-point value.
 * @return Casted value.
 */
#define EMF_LOG_ARG_F(val) EMF_PRINT_ARG_F(val)

/**
 * @brief Casts a string pointer for logging.
 *
 * @param[in] ptr Pointer to a null-terminated string.
 * @return Casted pointer.
 */
#define EMF_LOG_ARG_S(ptr) EMF_PRINT_ARG_S(ptr)

/**
 * @brief Casts a character for logging.
 *
 * @param[in] val Character value.
 * @return Casted value.
 */
#define EMF_LOG_ARG_C(val) EMF_PRINT_ARG_C(val)

/**
 * @brief Dispatch helper macro for @ref EMF_log().
 *
 * Builds a local array of arguments and passes them with the format string.
 *
 * @todo Review if 8 is sufficient for maximum number of arguments.
 *
 * @param[in] log_level Logging level.
 * @param[in] fmt Format string.
 * @param[in] ... Variable arguments.
 */
#define EMF_LOG_DISPATCH(log_level, fmt, ...) \
  do                                          \
  {                                           \
    if (EMF_PRINT_NARG(__VA_ARGS__) == 0)     \
    {                                         \
      EMF_log(log_level, fmt, NULL);          \
    }                                         \
    else                                      \
    {                                         \
      uintptr_t args[] = {__VA_ARGS__};       \
      EMF_log(log_level, fmt, args);          \
    }                                         \
  } while (0)

/**
 * @brief Logs an error-level message using @ref EMF_log().
 *
 * Builds a local array of arguments and passes them with the format string.
 *
 * @param[in] fmt Format string.
 * @param[in] ... Variable arguments.
 */
#define EMF_LOG_ERROR(fmt, ...) \
  EMF_LOG_DISPATCH(LOG_LEVEL_ERROR, fmt, __VA_ARGS__)

/**
 * @brief Logs a warning-level message using @ref EMF_log().
 *
 * Builds a local array of arguments and passes them with the format string.
 *
 * @param[in] fmt Format string.
 * @param[in] ... Variable arguments.
 */
#define EMF_LOG_WARNING(fmt, ...) \
  EMF_LOG_DISPATCH(LOG_LEVEL_WARNING, fmt, __VA_ARGS__)

/**
 * @brief Logs an info-level message using @ref EMF_log().
 *
 * Builds a local array of arguments and passes them with the format string.
 *
 * @param[in] fmt Format string.
 * @param[in] ... Variable arguments.
 */
#define EMF_LOG_INFO(fmt, ...) \
  EMF_LOG_DISPATCH(LOG_LEVEL_INFO, fmt, __VA_ARGS__)

/**
 * @brief Logs a debug-level message using @ref EMF_log().
 *
 * Builds a local array of arguments and passes them with the format string.
 *
 * @param[in] fmt Format string.
 * @param[in] ... Variable arguments.
 */
#define EMF_LOG_DEBUG(fmt, ...) \
  EMF_LOG_DISPATCH(LOG_LEVEL_DEBUG, fmt, __VA_ARGS__)

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief Logging levels.
 */
typedef enum
{
  LOG_LEVEL_ERROR = 0,
  LOG_LEVEL_WARNING = 1,
  LOG_LEVEL_INFO = 2,
  LOG_LEVEL_DEBUG = 3
} EMF_log_level_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the EMF log module.
 *
 * After initialization, the default logging level is @ref LOG_LEVEL_INFO.
 */
void EMF_log_init(void);

/**
 * @brief Sets the active logging level.
 *
 * @param[in] log_level Logging level to apply.
 */
void EMF_log_setLevel(EMF_log_level_t log_level);

/**
 * @brief Logs a formatted message with arguments.
 *
 * Function used by the @c EMF_LOG_... macros.
 *
 * @param[in] log_level Logging level of the message.
 * @param[in] fmt Null-terminated format string.
 * @param[in] args Pointer to array of arguments, or NULL.
 */
void EMF_log(EMF_log_level_t log_level, const char* fmt, const uintptr_t* args);

#endif /* EMF_LOG_H */
