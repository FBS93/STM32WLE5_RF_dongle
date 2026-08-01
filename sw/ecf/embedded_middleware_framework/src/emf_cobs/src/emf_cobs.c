/*******************************************************************************
 * @brief Consistent Overhead Byte Stuffing utilities implementation.
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
#include "emf_cobs.h"
#include "eaf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief Maximum possible code value (block size + 1)
 */
#define COBS_CODE_MAX (EMF_COBS_BLOCK_MAX + 1U)

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

void EMF_cobs_encode(const uint8_t* buff_in,
                     uint16_t len_in,
                     uint8_t* buff_out,
                     uint16_t* len_out)
{
  const uint8_t* end_ptr;
  uint8_t* out_ptr;
  uint8_t* code_ptr;
  uint8_t code;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff_in != NULL);
  EAF_ASSERT_IN_BLOCK(buff_out != NULL);
  EAF_ASSERT_IN_BLOCK(len_out != NULL);
  EAF_ASSERT_BLOCK_END();

  end_ptr = buff_in + len_in;
  out_ptr = buff_out;
  code_ptr = out_ptr++;
  code = 1;

  while (buff_in < end_ptr)
  {
    if (*buff_in != EMF_COBS_PACKET_DELIMITER)
    {
      // Precondition enforced by contract assertions.
      // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
      *out_ptr++ = *buff_in;
      code++;
      if (code == COBS_CODE_MAX)
      {
        // Precondition enforced by contract assertions.
        // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
        *code_ptr = code;
        code_ptr = out_ptr++;
        code = 1;
      }
    }
    else
    {
      // Precondition enforced by contract assertions.
      // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
      *code_ptr = code;
      code_ptr = out_ptr++;
      code = 1;
    }

    buff_in++;
  }

  // Precondition enforced by contract assertions.
  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  *code_ptr = code;
  *out_ptr++ = EMF_COBS_PACKET_DELIMITER;
  *len_out = (uint16_t)(out_ptr - buff_out);
}

bool EMF_cobs_decode(const uint8_t* buff_in,
                     uint16_t len_in,
                     uint8_t* buff_out,
                     uint16_t* len_out)
{
  const uint8_t* in_ptr;
  const uint8_t* end_ptr;
  uint8_t* out_ptr;
  uint8_t code;
  uint8_t block_bytes;
  bool delimiter_found;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff_in != NULL);
  EAF_ASSERT_IN_BLOCK(buff_out != NULL);
  EAF_ASSERT_IN_BLOCK(len_out != NULL);
  EAF_ASSERT_BLOCK_END();

  in_ptr = buff_in;
  end_ptr = buff_in + len_in;
  out_ptr = buff_out;
  delimiter_found = false;

  while (in_ptr < end_ptr)
  {
    code = *in_ptr++;
    if (code == EMF_COBS_PACKET_DELIMITER)
    {
      delimiter_found = true;
      break;
    }
    // Safe: code == 0 (EMF_COBS_PACKET_DELIMITER) is handled above
    block_bytes = code - 1;
    if ((uint16_t)block_bytes > (uint16_t)(end_ptr - in_ptr))
    {
      break;  // Invalid input
    }
    for (uint8_t i = 0; i < block_bytes; i++)
    {
      // Precondition enforced by contract assertions.
      // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
      *out_ptr++ = *in_ptr++;
    }
    if ((code < COBS_CODE_MAX) && (in_ptr < end_ptr) &&
        (*in_ptr != EMF_COBS_PACKET_DELIMITER))
    {
      // Precondition enforced by contract assertions.
      // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
      *out_ptr++ = EMF_COBS_PACKET_DELIMITER;
    }
  }

  if (delimiter_found)
  {
    *len_out = (uint16_t)(out_ptr - buff_out);
  }
  else
  {
    *len_out = 0;
  }

  return delimiter_found;
}
