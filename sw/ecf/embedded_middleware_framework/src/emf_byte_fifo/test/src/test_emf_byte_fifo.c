/*******************************************************************************
 * @brief Byte FIFO test.
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
#include "emf_byte_fifo.h"

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
 * @brief Verifies byte array equality.
 *
 * @param[in] expected Expected data.
 * @param[in] actual Actual data.
 * @param[in] size Number of bytes to compare.
 */
static void verifyBytesEq(const uint8_t* expected,
                          const uint8_t* actual,
                          uint32_t size);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static void verifyBytesEq(const uint8_t* expected,
                          const uint8_t* actual,
                          uint32_t size)
{
  for (uint32_t i = 0U; i < size; ++i)
  {
    ETF_VERIFY(expected[i] == actual[i]);
  }
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

ETF_TEST_SUITE(test_emf_byte_fifo)
{
  ETF_TEST(init_sets_empty_state)
  {
    EMF_byteFifo_handler_t fifo;
    uint8_t storage[8U] = {0U};

    EMF_byteFifo_init(&fifo, 8U, storage);

    ETF_VERIFY(EMF_byteFifo_isEmpty(&fifo));
    ETF_VERIFY(!EMF_byteFifo_isFull(&fifo));
    ETF_VERIFY(EMF_byteFifo_getUsed(&fifo) == 0U);
    ETF_VERIFY(EMF_byteFifo_getFree(&fifo) == 8U);
  }

  ETF_TEST(push_pop_preserves_order_and_counters)
  {
    EMF_byteFifo_handler_t fifo;
    uint8_t storage[10U] = {0U};
    uint8_t in_1[3U] = {1U, 2U, 3U};
    uint8_t in_2[2U] = {4U, 5U};
    uint8_t out[5U] = {0U};
    uint8_t expected[5U] = {1U, 2U, 3U, 4U, 5U};

    EMF_byteFifo_init(&fifo, 10U, storage);
    EMF_byteFifo_push(&fifo, in_1, 3U);
    EMF_byteFifo_push(&fifo, in_2, 2U);

    ETF_VERIFY(EMF_byteFifo_getUsed(&fifo) == 5U);
    ETF_VERIFY(EMF_byteFifo_getFree(&fifo) == 5U);

    EMF_byteFifo_peek(&fifo, out, 3U);
    verifyBytesEq(in_1, out, 3U);

    EMF_byteFifo_pop(&fifo, out, 5U);
    verifyBytesEq(expected, out, 5U);

    ETF_VERIFY(EMF_byteFifo_isEmpty(&fifo));
    ETF_VERIFY(EMF_byteFifo_getUsed(&fifo) == 0U);
  }

  ETF_TEST(peek_at_reads_without_removing_and_handles_wrap)
  {
    EMF_byteFifo_handler_t fifo;
    uint8_t storage[5U] = {0U};
    uint8_t first[4U] = {10U, 11U, 12U, 13U};
    uint8_t second[2U] = {20U, 21U};
    uint8_t tmp_pop[2U] = {0U};
    uint8_t peeked[3U] = {0U};
    uint8_t expected_peek[3U] = {13U, 20U, 21U};

    EMF_byteFifo_init(&fifo, 5U, storage);
    EMF_byteFifo_push(&fifo, first, 4U);
    EMF_byteFifo_pop(&fifo, tmp_pop, 2U);  // Remaining: 12,13
    EMF_byteFifo_push(&fifo, second, 2U);  // Remaining: 12,13,20,21

    EMF_byteFifo_peekAt(&fifo, 1U, peeked, 3U);
    verifyBytesEq(expected_peek, peeked, 3U);

    ETF_VERIFY(EMF_byteFifo_getUsed(&fifo) == 4U);
  }

  ETF_TEST(drop_and_flush_update_state)
  {
    EMF_byteFifo_handler_t fifo;
    uint8_t storage[6U] = {0U};
    uint8_t in[4U] = {0xA1U, 0xA2U, 0xA3U, 0xA4U};
    uint8_t out[2U] = {0U};
    uint8_t expected[2U] = {0xA3U, 0xA4U};

    EMF_byteFifo_init(&fifo, 6U, storage);
    EMF_byteFifo_push(&fifo, in, 4U);

    EMF_byteFifo_drop(&fifo, 2U);
    EMF_byteFifo_pop(&fifo, out, 2U);
    verifyBytesEq(expected, out, 2U);

    EMF_byteFifo_push(&fifo, in, 2U);
    EMF_byteFifo_flush(&fifo);

    ETF_VERIFY(EMF_byteFifo_isEmpty(&fifo));
    ETF_VERIFY(EMF_byteFifo_getUsed(&fifo) == 0U);
    ETF_VERIFY(EMF_byteFifo_getFree(&fifo) == 6U);
  }
}
