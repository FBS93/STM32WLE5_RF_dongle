/*******************************************************************************
 * @brief Bitmask utility implementation.
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
#include <stdbool.h>
#include <stddef.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "emf_bitmask.h"
#include "ebf_utils.h"
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

void EMF_bitmask_clearAll(uint8_t* bitmask, uint8_t len)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(bitmask != NULL);
  EAF_ASSERT_IN_BLOCK(len > 0);
  EAF_ASSERT_BLOCK_END();

  for (uint8_t i = 0U; i < len; i++)
  {
    bitmask[i] = 0U;
  }
}

bool EMF_bitmask_isAnySet(const uint8_t* bitmask, uint8_t len)
{
  bool is_any_set;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(bitmask != NULL);
  EAF_ASSERT_IN_BLOCK(len > 0);
  EAF_ASSERT_BLOCK_END();

  is_any_set = false;
  for (uint8_t i = 0U; i < len; i++)
  {
    if (bitmask[i] != 0U)
    {
      is_any_set = true;
    }
  }

  return is_any_set;
}

bool EMF_bitmask_isBitSet(const uint8_t* bitmask, uint8_t bit_pos)
{
  uint8_t idx;
  uint8_t bit;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(bitmask != NULL);
  EAF_ASSERT_IN_BLOCK(bit_pos > 0);
  EAF_ASSERT_BLOCK_END();

  idx = (uint8_t)((bit_pos - 1U) / 8U);
  bit = (uint8_t)((bit_pos - 1U) % 8U);

  return ((bitmask[idx] & ((uint8_t)1U << bit)) != 0U);
}

void EMF_bitmask_setBit(uint8_t* bitmask, uint8_t bit_pos)
{
  uint8_t idx;
  uint8_t bit;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(bitmask != NULL);
  EAF_ASSERT_IN_BLOCK(bit_pos > 0);
  EAF_ASSERT_BLOCK_END();

  idx = (uint8_t)((bit_pos - 1U) / 8U);
  bit = (uint8_t)((bit_pos - 1U) % 8U);

  bitmask[idx] |= ((uint8_t)1U << bit);
}

void EMF_bitmask_clearBit(uint8_t* bitmask, uint8_t bit_pos)
{
  uint8_t idx;
  uint8_t bit;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(bitmask != NULL);
  EAF_ASSERT_IN_BLOCK(bit_pos > 0);
  EAF_ASSERT_BLOCK_END();

  idx = (uint8_t)((bit_pos - 1U) / 8U);
  bit = (uint8_t)((bit_pos - 1U) % 8U);

  bitmask[idx] &= (uint8_t)(~((uint8_t)1U << bit));
}

uint8_t EMF_bitmask_findMax(const uint8_t* bitmask, uint8_t len)
{
  uint8_t max;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(bitmask != NULL);
  EAF_ASSERT_IN_BLOCK(len > 0);
  EAF_ASSERT_BLOCK_END();

  max = 0U;
  while (len > 0U)
  {
    len--;

    if (bitmask[len] != 0U)
    {
      max = (uint8_t)(EBF_utils_log2(bitmask[len]) + (len * 8U));
    }
  }

  return max;
}
