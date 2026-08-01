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

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

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
  EAF_ASSERT(fmt != NULL);

  if (log_level <= logLevel)
  {
    switch (log_level)
    {
      case LOG_LEVEL_ERROR:
      {
        EMF_print("[ERROR] ", NULL);
        break;
      }
      case LOG_LEVEL_WARNING:
      {
        EMF_print("[WARNING] ", NULL);
        break;
      }
      case LOG_LEVEL_INFO:
      {
        EMF_print("[INFO] ", NULL);
        break;
      }
      case LOG_LEVEL_DEBUG:
      {
        EMF_print("[DEBUG] ", NULL);
        break;
      }
      default:
      {
        EAF_ERROR();
        break;
      }
    }

    EMF_print(fmt, args);
  }
}
