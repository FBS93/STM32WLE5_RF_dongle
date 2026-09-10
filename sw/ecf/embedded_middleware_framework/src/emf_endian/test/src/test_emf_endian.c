/*******************************************************************************
 * @brief Endian test.
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
#include "emf_endian.h"

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
 * @brief Verifies that two buffers have equal byte content.
 *
 * @param[in] expected Pointer to expected buffer.
 * @param[in] actual Pointer to actual buffer.
 * @param[in] size Number of bytes to compare.
 */
static void verifyBuff(const uint8_t* expected,
                       const uint8_t* actual,
                       uint8_t size);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static void verifyBuff(const uint8_t* expected,
                       const uint8_t* actual,
                       uint8_t size)
{
  uint8_t byte_index;

  for (byte_index = 0U; byte_index < size; byte_index++)
  {
    ETF_VERIFY(expected[byte_index] == actual[byte_index]);
  }
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

ETF_TEST_SUITE(test_emf_endian)
{
  ETF_TEST(big_endian_u16_roundtrip)
  {
    uint16_t input_16 = 0x1234U;
    uint16_t output_16 = 0U;
    uint8_t buffer_16[2U] = {0U};

    EMF_endian_u16WriteBuffBE(buffer_16, &input_16);
    ETF_VERIFY(buffer_16[0U] == 0x12U && buffer_16[1U] == 0x34U);

    EMF_endian_u16ReadBuffBE(buffer_16, &output_16);
    ETF_VERIFY(output_16 == input_16);
  }

  ETF_TEST(big_endian_u24_roundtrip)
  {
    uint32_t input_24 = 0xFF123456U;
    uint32_t output_24 = UINT32_MAX;
    uint8_t expected_24[3U] = {0x12U, 0x34U, 0x56U};
    uint8_t buffer_24[4U] = {0U, 0U, 0U, 0xA5U};
    uint8_t maximum_24[3U] = {0xFFU, 0xFFU, 0xFFU};

    EMF_endian_u24WriteBuffBE(buffer_24, &input_24);
    verifyBuff(expected_24, buffer_24, 3U);
    ETF_VERIFY(buffer_24[3U] == 0xA5U);

    EMF_endian_u24ReadBuffBE(buffer_24, &output_24);
    ETF_VERIFY(output_24 == 0x123456U);
    EMF_endian_u24ReadBuffBE(maximum_24, &output_24);
    ETF_VERIFY(output_24 == 0xFFFFFFU);
  }

  ETF_TEST(big_endian_u32_roundtrip)
  {
    uint32_t input_32 = 0x89ABCDEFU;
    uint32_t output_32 = 0U;
    uint8_t buffer_32[4U] = {0U};

    EMF_endian_u32WriteBuffBE(buffer_32, &input_32);
    ETF_VERIFY(buffer_32[0U] == 0x89U && buffer_32[1U] == 0xABU &&
               buffer_32[2U] == 0xCDU && buffer_32[3U] == 0xEFU);

    EMF_endian_u32ReadBuffBE(buffer_32, &output_32);
    ETF_VERIFY(output_32 == input_32);
  }

  ETF_TEST(big_endian_u40_roundtrip)
  {
    uint64_t input_40 = 0xFFFFFF123456789AULL;
    uint64_t output_40 = UINT64_MAX;
    uint8_t expected_40[5U] = {0x12U, 0x34U, 0x56U, 0x78U, 0x9AU};
    uint8_t buffer_40[6U] = {0U, 0U, 0U, 0U, 0U, 0xA5U};
    uint8_t maximum_40[5U] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};

    EMF_endian_u40WriteBuffBE(buffer_40, &input_40);
    verifyBuff(expected_40, buffer_40, 5U);
    ETF_VERIFY(buffer_40[5U] == 0xA5U);

    EMF_endian_u40ReadBuffBE(buffer_40, &output_40);
    ETF_VERIFY(output_40 == 0x123456789AULL);
    EMF_endian_u40ReadBuffBE(maximum_40, &output_40);
    ETF_VERIFY(output_40 == 0xFFFFFFFFFFULL);
  }

  ETF_TEST(big_endian_u48_roundtrip)
  {
    uint64_t input_48 = 0xFFFF123456789ABCULL;
    uint64_t output_48 = UINT64_MAX;
    uint8_t expected_48[6U] = {0x12U, 0x34U, 0x56U, 0x78U, 0x9AU, 0xBCU};
    uint8_t buffer_48[7U] = {0U, 0U, 0U, 0U, 0U, 0U, 0xA5U};
    uint8_t maximum_48[6U] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};

    EMF_endian_u48WriteBuffBE(buffer_48, &input_48);
    verifyBuff(expected_48, buffer_48, 6U);
    ETF_VERIFY(buffer_48[6U] == 0xA5U);

    EMF_endian_u48ReadBuffBE(buffer_48, &output_48);
    ETF_VERIFY(output_48 == 0x123456789ABCULL);
    EMF_endian_u48ReadBuffBE(maximum_48, &output_48);
    ETF_VERIFY(output_48 == 0xFFFFFFFFFFFFULL);
  }

  ETF_TEST(big_endian_u56_roundtrip)
  {
    uint64_t input_56 = 0xFF123456789ABCDEULL;
    uint64_t output_56 = UINT64_MAX;
    uint8_t expected_56[7U] = {0x12U, 0x34U, 0x56U, 0x78U, 0x9AU, 0xBCU, 0xDEU};
    uint8_t buffer_56[8U] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0xA5U};
    uint8_t maximum_56[7U] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};

    EMF_endian_u56WriteBuffBE(buffer_56, &input_56);
    verifyBuff(expected_56, buffer_56, 7U);
    ETF_VERIFY(buffer_56[7U] == 0xA5U);

    EMF_endian_u56ReadBuffBE(buffer_56, &output_56);
    ETF_VERIFY(output_56 == 0x123456789ABCDEULL);
    EMF_endian_u56ReadBuffBE(maximum_56, &output_56);
    ETF_VERIFY(output_56 == 0xFFFFFFFFFFFFFFULL);
  }

  ETF_TEST(big_endian_u64_roundtrip)
  {
    uint64_t input_64 = 0x0123456789ABCDEFULL;
    uint64_t output_64 = 0U;
    uint8_t buffer_64[8U] = {0U};

    EMF_endian_u64WriteBuffBE(buffer_64, &input_64);
    EMF_endian_u64ReadBuffBE(buffer_64, &output_64);

    ETF_VERIFY(output_64 == input_64);
  }

  ETF_TEST(little_endian_u16_roundtrip)
  {
    uint16_t input_16 = 0x1234U;
    uint16_t output_16 = 0U;
    uint8_t buffer_16[2U] = {0U};

    EMF_endian_u16WriteBuffLE(buffer_16, &input_16);
    ETF_VERIFY(buffer_16[0U] == 0x34U && buffer_16[1U] == 0x12U);

    EMF_endian_u16ReadBuffLE(buffer_16, &output_16);
    ETF_VERIFY(output_16 == input_16);
  }

  ETF_TEST(little_endian_u24_roundtrip)
  {
    uint32_t input_24 = 0xFF123456U;
    uint32_t output_24 = UINT32_MAX;
    uint8_t expected_24[3U] = {0x56U, 0x34U, 0x12U};
    uint8_t buffer_24[4U] = {0U, 0U, 0U, 0xA5U};
    uint8_t maximum_24[3U] = {0xFFU, 0xFFU, 0xFFU};

    EMF_endian_u24WriteBuffLE(buffer_24, &input_24);
    verifyBuff(expected_24, buffer_24, 3U);
    ETF_VERIFY(buffer_24[3U] == 0xA5U);

    EMF_endian_u24ReadBuffLE(buffer_24, &output_24);
    ETF_VERIFY(output_24 == 0x123456U);
    EMF_endian_u24ReadBuffLE(maximum_24, &output_24);
    ETF_VERIFY(output_24 == 0xFFFFFFU);
  }

  ETF_TEST(little_endian_u32_roundtrip)
  {
    uint32_t input_32 = 0x89ABCDEFU;
    uint32_t output_32 = 0U;
    uint8_t buffer_32[4U] = {0U};

    EMF_endian_u32WriteBuffLE(buffer_32, &input_32);
    ETF_VERIFY(buffer_32[0U] == 0xEFU && buffer_32[1U] == 0xCDU &&
               buffer_32[2U] == 0xABU && buffer_32[3U] == 0x89U);

    EMF_endian_u32ReadBuffLE(buffer_32, &output_32);
    ETF_VERIFY(output_32 == input_32);
  }

  ETF_TEST(little_endian_u40_roundtrip)
  {
    uint64_t input_40 = 0xFFFFFF123456789AULL;
    uint64_t output_40 = UINT64_MAX;
    uint8_t expected_40[5U] = {0x9AU, 0x78U, 0x56U, 0x34U, 0x12U};
    uint8_t buffer_40[6U] = {0U, 0U, 0U, 0U, 0U, 0xA5U};
    uint8_t maximum_40[5U] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};

    EMF_endian_u40WriteBuffLE(buffer_40, &input_40);
    verifyBuff(expected_40, buffer_40, 5U);
    ETF_VERIFY(buffer_40[5U] == 0xA5U);

    EMF_endian_u40ReadBuffLE(buffer_40, &output_40);
    ETF_VERIFY(output_40 == 0x123456789AULL);
    EMF_endian_u40ReadBuffLE(maximum_40, &output_40);
    ETF_VERIFY(output_40 == 0xFFFFFFFFFFULL);
  }

  ETF_TEST(little_endian_u48_roundtrip)
  {
    uint64_t input_48 = 0xFFFF123456789ABCULL;
    uint64_t output_48 = UINT64_MAX;
    uint8_t expected_48[6U] = {0xBCU, 0x9AU, 0x78U, 0x56U, 0x34U, 0x12U};
    uint8_t buffer_48[7U] = {0U, 0U, 0U, 0U, 0U, 0U, 0xA5U};
    uint8_t maximum_48[6U] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};

    EMF_endian_u48WriteBuffLE(buffer_48, &input_48);
    verifyBuff(expected_48, buffer_48, 6U);
    ETF_VERIFY(buffer_48[6U] == 0xA5U);

    EMF_endian_u48ReadBuffLE(buffer_48, &output_48);
    ETF_VERIFY(output_48 == 0x123456789ABCULL);
    EMF_endian_u48ReadBuffLE(maximum_48, &output_48);
    ETF_VERIFY(output_48 == 0xFFFFFFFFFFFFULL);
  }

  ETF_TEST(little_endian_u56_roundtrip)
  {
    uint64_t input_56 = 0xFF123456789ABCDEULL;
    uint64_t output_56 = UINT64_MAX;
    uint8_t expected_56[7U] = {0xDEU, 0xBCU, 0x9AU, 0x78U, 0x56U, 0x34U, 0x12U};
    uint8_t buffer_56[8U] = {0U, 0U, 0U, 0U, 0U, 0U, 0U, 0xA5U};
    uint8_t maximum_56[7U] = {0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU, 0xFFU};

    EMF_endian_u56WriteBuffLE(buffer_56, &input_56);
    verifyBuff(expected_56, buffer_56, 7U);
    ETF_VERIFY(buffer_56[7U] == 0xA5U);

    EMF_endian_u56ReadBuffLE(buffer_56, &output_56);
    ETF_VERIFY(output_56 == 0x123456789ABCDEULL);
    EMF_endian_u56ReadBuffLE(maximum_56, &output_56);
    ETF_VERIFY(output_56 == 0xFFFFFFFFFFFFFFULL);
  }

  ETF_TEST(little_endian_u64_roundtrip)
  {
    uint64_t input_64 = 0x0123456789ABCDEFULL;
    uint64_t output_64 = 0U;
    uint8_t buffer_64[8U] = {0U};

    EMF_endian_u64WriteBuffLE(buffer_64, &input_64);
    EMF_endian_u64ReadBuffLE(buffer_64, &output_64);

    ETF_VERIFY(output_64 == input_64);
  }

  ETF_TEST(generic_big_endian_read_write_roundtrip)
  {
    uint32_t input_value = 0x10203040U;
    uint32_t output_value = 0U;
    uint8_t expected_be[4U] = {0x10U, 0x20U, 0x30U, 0x40U};
    uint8_t buffer[4U] = {0U};

    EMF_endian_writeBuffBE(buffer, &input_value, (uint8_t)sizeof(input_value));
    verifyBuff(expected_be, buffer, 4U);
    EMF_endian_readBuffBE(buffer, &output_value, (uint8_t)sizeof(output_value));
    ETF_VERIFY(output_value == input_value);
  }

  ETF_TEST(generic_little_endian_read_write_roundtrip)
  {
    uint32_t input_value = 0x10203040U;
    uint32_t output_value = 0U;
    uint8_t expected_le[4U] = {0x40U, 0x30U, 0x20U, 0x10U};
    uint8_t buffer[4U] = {0U};

    EMF_endian_writeBuffLE(buffer, &input_value, (uint8_t)sizeof(input_value));
    verifyBuff(expected_le, buffer, 4U);
    EMF_endian_readBuffLE(buffer, &output_value, (uint8_t)sizeof(output_value));
    ETF_VERIFY(output_value == input_value);
  }
}
