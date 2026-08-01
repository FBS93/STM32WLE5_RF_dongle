/*******************************************************************************
 * @brief CRC calculation utilities implementation.
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
#include "emf_crc.h"
#include "eaf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief Polynomial used for CRC-16-CCITT calculation (0x1021).
 */
#define CRC16_CCITT_POLY 0x1021U

/**
 * @brief Initial CRC value for CRC-16-CCITT (0x0000).
 */
#define CRC16_CCITT_INIT_VALUE 0x0000U

/**
 * @brief Width of the CRC in bits.
 */
#define CRC16_WIDTH 16U

/**
 * @brief Top (most significant) bit mask for CRC width.
 */
#define CRC16_TOPBIT (1U << (CRC16_WIDTH - 1U))

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

uint16_t EMF_crc_16CCITT(const uint8_t* data, uint16_t length)
{
  uint16_t crc;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(length > 0);
  EAF_ASSERT_BLOCK_END();

  crc = CRC16_CCITT_INIT_VALUE;

  while (length > 0)
  {
    crc ^= (((uint16_t)(*data++)) << 8);
    for (uint8_t i = 0; i < 8; i++)
    {
      if (crc & CRC16_TOPBIT)
      {
        crc = (uint16_t)((crc << 1) ^ CRC16_CCITT_POLY);
      }
      else
      {
        crc <<= 1;
      }
    }

    length--;
  }

  return crc;
}
