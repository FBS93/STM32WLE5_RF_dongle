/*******************************************************************************
 * @brief Host fatal-error handling.
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
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 *******************************************************************************/

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */
#include "eaf.h"
#include "emf.h"

/*******************************************************************************
 * PUBLIC FUNCTIONS
 *******************************************************************************/

/** @brief Handles host fatal errors by halting without accessing hardware. */
void EAF_onError(const char* file, int line, int id)
{
  EMF_UTILS_UNUSED_PARAM(file);
  EMF_UTILS_UNUSED_PARAM(line);
  EMF_UTILS_UNUSED_PARAM(id);

  while (1)
  {
    // Forever. This function shall not return.
  }
}
