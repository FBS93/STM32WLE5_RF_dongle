/*******************************************************************************
 * @brief COBS test.
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
#include <stdbool.h>
#include <stdint.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "etf.h"
#include "emf_cobs.h"

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

/**
 * @brief Verifies that two buffers have equal content.
 *
 * @param[in] lhs Pointer to first buffer.
 * @param[in] rhs Pointer to second buffer.
 * @param[in] len Number of bytes to compare.
 */
static void verifyBuffersEqual(const uint8_t* lhs,
                               const uint8_t* rhs,
                               uint16_t len);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static void verifyBuffersEqual(const uint8_t* lhs,
                               const uint8_t* rhs,
                               uint16_t len)
{
  uint16_t byte_index;

  for (byte_index = 0U; byte_index < len; byte_index++)
  {
    // Caller verifies buffer length equivalence before this helper call.
    // NOLINTNEXTLINE(clang-analyzer-core.UndefinedBinaryOperatorResult)
    ETF_VERIFY(lhs[byte_index] == rhs[byte_index]);
  }
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

ETF_TEST_SUITE(test_emf_cobs)
{
  ETF_TEST(macro_contract)
  {
    ETF_VERIFY(EMF_COBS_PACKET_DELIMITER == 0x00U);
    ETF_VERIFY(EMF_COBS_PACKET_DELIMITER_SIZE == 1U);
    ETF_VERIFY(EMF_COBS_BLOCK_MAX == 0xFEU);

    ETF_VERIFY(EMF_COBS_OVERHEAD(0U) == 1U);
    ETF_VERIFY(EMF_COBS_OVERHEAD(1U) == 1U);
    ETF_VERIFY(EMF_COBS_OVERHEAD(EMF_COBS_BLOCK_MAX) == 2U);
    ETF_VERIFY(EMF_COBS_OVERHEAD(300U) == 2U);

    ETF_VERIFY(EMF_COBS_ENCODED_SIZE(0U) == 2U);
    ETF_VERIFY(EMF_COBS_ENCODED_SIZE(1U) == 3U);
    ETF_VERIFY(EMF_COBS_ENCODED_SIZE(EMF_COBS_BLOCK_MAX) ==
               (EMF_COBS_BLOCK_MAX + 3U));
  }

  ETF_TEST(encode_empty_input_includes_trailing_delimiter)
  {
    uint8_t input[1U] = {0U};
    uint8_t output[EMF_COBS_ENCODED_SIZE(0U)] = {0U};
    uint16_t output_len = 0U;

    EMF_cobs_encode(input, 0U, output, &output_len);

    ETF_VERIFY(output_len == 2U);
    ETF_VERIFY(output[0U] == 0x01U);
    ETF_VERIFY(output[1U] == EMF_COBS_PACKET_DELIMITER);
  }

  ETF_TEST(encode_known_vector_with_internal_zeroes)
  {
    uint8_t input[] = {0x11U, 0x22U, 0x00U, 0x33U};
    uint8_t expected[] = {0x03U, 0x11U, 0x22U, 0x02U, 0x33U, 0x00U};
    uint8_t output[EMF_COBS_ENCODED_SIZE(sizeof(input))] = {0U};
    uint16_t output_len = 0U;

    EMF_cobs_encode(input, (uint16_t)sizeof(input), output, &output_len);

    ETF_VERIFY(output_len == (uint16_t)sizeof(expected));
    verifyBuffersEqual(output, expected, output_len);
    ETF_VERIFY(output[output_len - 1U] == EMF_COBS_PACKET_DELIMITER);
  }

  ETF_TEST(encode_worst_case_size_stays_within_documented_bound)
  {
    uint8_t input[300U] = {0U};
    uint8_t output[EMF_COBS_ENCODED_SIZE(300U)] = {0U};
    uint16_t output_len = 0U;
    uint16_t byte_index;

    for (byte_index = 0U; byte_index < 300U; byte_index++)
    {
      input[byte_index] = (uint8_t)((byte_index % EMF_COBS_BLOCK_MAX) + 1U);
    }

    EMF_cobs_encode(input, 300U, output, &output_len);

    ETF_VERIFY(output_len <= EMF_COBS_ENCODED_SIZE(300U));
    ETF_VERIFY(output[output_len - 1U] == EMF_COBS_PACKET_DELIMITER);
  }

  ETF_TEST(decode_round_trip_preserves_payload)
  {
    // clang-format off
    uint8_t input[] = {
      0x00U, 0x11U, 0x22U, 0x00U, 0x33U, 0x44U, 0x55U, 0x00U, 0x66U
    };
    // clang-format on
    uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(input))] = {0U};
    uint8_t decoded[sizeof(input)] = {0U};
    uint16_t encoded_len = 0U;
    uint16_t decoded_len = 0U;
    bool is_ok = false;

    EMF_cobs_encode(input, (uint16_t)sizeof(input), encoded, &encoded_len);
    is_ok = EMF_cobs_decode(encoded, encoded_len, decoded, &decoded_len);

    ETF_VERIFY(is_ok);
    ETF_VERIFY(decoded_len == (uint16_t)sizeof(input));
    verifyBuffersEqual(decoded, input, decoded_len);
  }

  ETF_TEST(decode_returns_false_when_delimiter_not_found)
  {
    uint8_t encoded_without_delimiter[] = {0x03U, 0x11U, 0x22U, 0x02U, 0x33U};
    uint8_t decoded[sizeof(encoded_without_delimiter)] = {0U};
    uint16_t decoded_len = 0U;
    bool is_ok = false;

    is_ok = EMF_cobs_decode(encoded_without_delimiter,
                            (uint16_t)sizeof(encoded_without_delimiter),
                            decoded,
                            &decoded_len);

    ETF_VERIFY(!is_ok);
  }

  ETF_TEST(decode_stops_at_first_delimiter)
  {
    uint8_t input[] = {0xA1U, 0x00U, 0xB2U};
    uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(input)) + 4U] = {0U};
    uint8_t decoded[sizeof(input) + 4U] = {0U};
    uint16_t encoded_len = 0U;
    uint16_t decoded_len = 0U;
    uint16_t decode_input_len = 0U;
    bool is_ok = false;

    EMF_cobs_encode(input, (uint16_t)sizeof(input), encoded, &encoded_len);

    encoded[encoded_len + 0U] = 0x99U;
    encoded[encoded_len + 1U] = 0x88U;
    encoded[encoded_len + 2U] = 0x77U;
    encoded[encoded_len + 3U] = 0x66U;
    decode_input_len = (uint16_t)(encoded_len + 4U);

    is_ok = EMF_cobs_decode(encoded, decode_input_len, decoded, &decoded_len);

    ETF_VERIFY(is_ok);
    ETF_VERIFY(decoded_len == (uint16_t)sizeof(input));
    verifyBuffersEqual(decoded, input, decoded_len);
  }
}
