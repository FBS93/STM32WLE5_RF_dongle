/*******************************************************************************
 * @brief Logging utilities implementation.
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
#include <stddef.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "emf_log.h"
#include "emf_config.h"
#include "emf_print.h"
#include "eaf.h"

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
 * @brief Define static file name string for asserts.
 */
EAF_DEFINE_THIS_FILE(__FILE__);

/**
 * @brief Current logging level state.
 *
 * Stores the active logging level used by ESF module.
 */
static EMF_log_level_t logLevel;

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
 * @brief Gets the length of a null-terminated string up to a maximum length.
 *
 * @param[in] str Pointer to a null-terminated string.
 * @param[in] max_len Maximum permitted string length.
 * @return String length, or max_len plus one if it is exceeded.
 */
static size_t getStrLen(const char* str, size_t max_len);

/**
 * @brief Concatenates two strings with their specified lengths.
 *
 * @param[out] dst Destination buffer.
 * @param[in] first First string.
 * @param[in] first_len Length of the first string.
 * @param[in] second Second string.
 * @param[in] second_len Length of the second string.
 */
static void concatStr(char* dst,
                      const char* first,
                      size_t first_len,
                      const char* second,
                      size_t second_len);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static size_t getStrLen(const char* str, size_t max_len)
{
  size_t len;

  len = 0U;
  while ((len <= max_len) && (str[len] != '\0'))
  {
    len++;
  }

  return len;
}

static void concatStr(char* dst,
                      const char* first,
                      size_t first_len,
                      const char* second,
                      size_t second_len)
{
  size_t index;

  for (index = 0U; index < first_len; index++)
  {
    dst[index] = first[index];
  }
  for (index = 0U; index < second_len; index++)
  {
    dst[first_len + index] = second[index];
  }
  dst[first_len + second_len] = '\0';
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void EMF_log_init(void)
{
  logLevel = LOG_LEVEL_INFO;
}

void EMF_log_setLevel(EMF_log_level_t log_level)
{
  logLevel = log_level;
}

void EMF_log(EMF_log_level_t log_level, const char* fmt, const uintptr_t* args)
{
  const char* prefix;
  size_t prefix_len;
  size_t format_len;
  char log_format[EMF_PRINT_MAX_LEN + 1U];

  EAF_ASSERT(fmt != NULL);

  if (log_level <= logLevel)
  {
    switch (log_level)
    {
      case LOG_LEVEL_ERROR:
      {
        prefix = "[ERROR] ";
        break;
      }
      case LOG_LEVEL_WARNING:
      {
        prefix = "[WARNING] ";
        break;
      }
      case LOG_LEVEL_INFO:
      {
        prefix = "[INFO] ";
        break;
      }
      case LOG_LEVEL_DEBUG:
      {
        prefix = "[DEBUG] ";
        break;
      }
      default:
      {
        EAF_ERROR();
        return;
      }
    }

    // Get and validate prefix and format lengths.
    prefix_len = getStrLen(prefix, EMF_PRINT_MAX_LEN);
    EAF_ASSERT(prefix_len <= EMF_PRINT_MAX_LEN);
    format_len = getStrLen(fmt, EMF_PRINT_MAX_LEN - prefix_len);
    EAF_ASSERT(format_len <= (EMF_PRINT_MAX_LEN - prefix_len));

    // Concatenate prefix and format string.
    concatStr(log_format, prefix, prefix_len, fmt, format_len);

    // Print full log message.
    EMF_print(log_format, args);
  }
}
