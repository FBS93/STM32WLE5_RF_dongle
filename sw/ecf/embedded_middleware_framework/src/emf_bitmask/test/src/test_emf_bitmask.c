/*******************************************************************************
 * @brief Bitmask test.
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
#include "emf_bitmask.h"

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

ETF_TEST_SUITE(test_emf_bitmask)
{
  ETF_TEST(clear_all_and_any_set)
  {
    uint8_t bitmask[2U] = {0xFFU, 0xA5U};

    EMF_bitmask_clearAll(bitmask, 2U);

    ETF_VERIFY(bitmask[0U] == 0U);
    ETF_VERIFY(bitmask[1U] == 0U);
    ETF_VERIFY(!EMF_bitmask_isAnySet(bitmask, 2U));
  }

  ETF_TEST(set_clear_and_is_bit_set)
  {
    uint8_t bitmask[2U] = {0U, 0U};

    EMF_bitmask_setBit(bitmask, 1U);
    EMF_bitmask_setBit(bitmask, 8U);
    EMF_bitmask_setBit(bitmask, 9U);

    ETF_VERIFY(EMF_bitmask_isBitSet(bitmask, 1U));
    ETF_VERIFY(EMF_bitmask_isBitSet(bitmask, 8U));
    ETF_VERIFY(EMF_bitmask_isBitSet(bitmask, 9U));
    ETF_VERIFY(!EMF_bitmask_isBitSet(bitmask, 10U));

    EMF_bitmask_clearBit(bitmask, 8U);
    ETF_VERIFY(!EMF_bitmask_isBitSet(bitmask, 8U));
    ETF_VERIFY(EMF_bitmask_isBitSet(bitmask, 9U));
  }

  ETF_TEST(find_max_returns_highest_priority_set_bit)
  {
    uint8_t bitmask[2U] = {0U, 0U};

    ETF_VERIFY(EMF_bitmask_findMax(bitmask, 2U) == 0U);

    EMF_bitmask_setBit(bitmask, 16U);
    ETF_VERIFY(EMF_bitmask_findMax(bitmask, 2U) == 16U);

    EMF_bitmask_setBit(bitmask, 1U);
    ETF_VERIFY(EMF_bitmask_findMax(bitmask, 2U) == 1U);

    EMF_bitmask_clearBit(bitmask, 16U);
    ETF_VERIFY(EMF_bitmask_findMax(bitmask, 2U) == 1U);
  }
}
