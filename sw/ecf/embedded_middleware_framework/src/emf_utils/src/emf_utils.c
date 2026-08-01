/*******************************************************************************
 * @brief Generic utility macros and functions implementation.
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
#include "emf_utils.h"
#include "ebf.h"
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

void EMF_utils_clear(void* start, uint32_t len)
{
  uint8_t* ptr8;
  uint32_t* ptr32;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(start != NULL);
  EAF_ASSERT_IN_BLOCK(len > 0U);
  EAF_ASSERT_BLOCK_END();

  ptr8 = (uint8_t*)start;

  // Align pointer to 4 bytes by clearing single bytes if needed.
  while ((((uintptr_t)ptr8 & 0x3) != 0) && (len > 0))
  {
    // Precondition enforced by contract assertions.
    // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
    *ptr8++ = 0;
    len--;
  }

  // Clear memory using 32-bit words.
  ptr32 = (uint32_t*)ptr8;
  while (len >= 4)
  {
    // Precondition enforced by contract assertions.
    // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
    *ptr32++ = 0;
    len -= 4;
  }

  // Clear remaining tail bytes if needed.
  ptr8 = (uint8_t*)ptr32;
  while (len > 0)
  {
    // Precondition enforced by contract assertions.
    // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
    *ptr8++ = 0;
    len--;
  }
}
