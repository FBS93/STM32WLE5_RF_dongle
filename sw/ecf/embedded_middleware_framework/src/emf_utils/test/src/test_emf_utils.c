/*******************************************************************************
 * @brief Utils test.
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

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "etf.h"
#include "emf_utils.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

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

ETF_TEST_SUITE(test_emf_utils)
{
  ETF_TEST(clear_zeroes_requested_region)
  {
    // clang-format off
    uint8_t buffer[10U] = {
      0xAAU, 0xAAU, 0xAAU, 0xAAU, 0xAAU,
      0xAAU, 0xAAU, 0xAAU, 0xAAU, 0xAAU
    };
    // clang-format on
    uint8_t byte_index;

    EMF_utils_clear(&buffer[1U], 7U);

    ETF_VERIFY(buffer[0U] == 0xAAU);
    for (byte_index = 1U; byte_index < 8U; byte_index++)
    {
      ETF_VERIFY(buffer[byte_index] == 0U);
    }
    ETF_VERIFY(buffer[8U] == 0xAAU);
    ETF_VERIFY(buffer[9U] == 0xAAU);
  }

  ETF_TEST(macro_contracts)
  {
    uint8_t array[5U] = {0U};
    uint8_t dummy = 0x11U;
    EMF_UTILS_MEM_ALIGNED_SLOT(9U) aligned_slot;

    EMF_UTILS_UNUSED_PARAM(dummy);
    ETF_VERIFY(EMF_UTILS_SIZEOF_ARRAY(array) == 5U);
    ETF_VERIFY(sizeof(aligned_slot) >= 9U);
    ETF_VERIFY((sizeof(aligned_slot) % sizeof(void*)) == 0U);
  }
}
