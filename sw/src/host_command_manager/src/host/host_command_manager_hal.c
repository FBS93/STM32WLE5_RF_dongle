/*******************************************************************************
 * @brief Minimal host implementation of the host command manager HAL.
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
#include <stddef.h>
#include <stdint.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */
#include "eaf.h"

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "host_command_manager_hal.h"

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/** @brief Define static file name string for asserts. */
EAF_DEFINE_THIS_FILE(__FILE__);
/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void hostCommandManagerHal_init(void)
{
}

uint32_t hostCommandManagerHal_getAvailableBytes(void)
{
  return 0U;
}

void hostCommandManagerHal_read(uint8_t* data, uint32_t length)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(length > 0U);
  EAF_ASSERT_BLOCK_END();
}

void hostCommandManagerHal_write(const uint8_t* data, uint32_t length)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(length > 0U);
  EAF_ASSERT_BLOCK_END();
}
