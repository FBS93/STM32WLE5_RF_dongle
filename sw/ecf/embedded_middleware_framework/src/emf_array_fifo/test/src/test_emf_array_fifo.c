/*******************************************************************************
 * @brief Array FIFO test.
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
#include "emf_array_fifo.h"

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
 * @brief Verifies two slots have the same byte content.
 *
 * @param[in] expected Pointer to expected slot content.
 * @param[in] actual Pointer to actual slot content.
 * @param[in] size Number of bytes in each slot.
 */
static void verifySlotEq(const uint8_t* expected,
                         const uint8_t* actual,
                         uint16_t size);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static void verifySlotEq(const uint8_t* expected,
                         const uint8_t* actual,
                         uint16_t size)
{
  uint16_t byte_index;

  for (byte_index = 0U; byte_index < size; byte_index++)
  {
    ETF_VERIFY(expected[byte_index] == actual[byte_index]);
  }
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

ETF_TEST_SUITE(test_emf_array_fifo)
{
  ETF_TEST(init_sets_empty_state)
  {
    EMF_arrayFifo_handler_t fifo;
    uint8_t storage[2U * 3U] = {0U};

    EMF_arrayFifo_init(&fifo, 2U, 3U, storage);

    ETF_VERIFY(EMF_arrayFifo_isEmpty(&fifo));
    ETF_VERIFY(!EMF_arrayFifo_isFull(&fifo));
    ETF_VERIFY(EMF_arrayFifo_getUsed(&fifo) == 0U);
    ETF_VERIFY(EMF_arrayFifo_getFree(&fifo) == 2U);
  }

  ETF_TEST(push_pop_preserves_fifo_order)
  {
    EMF_arrayFifo_handler_t fifo;
    uint8_t storage[3U * 2U] = {0U};
    uint8_t input_1[2U] = {1U, 2U};
    uint8_t input_2[2U] = {3U, 4U};
    uint8_t output[2U] = {0U, 0U};

    EMF_arrayFifo_init(&fifo, 3U, 2U, storage);
    EMF_arrayFifo_push(&fifo, input_1);
    EMF_arrayFifo_push(&fifo, input_2);
    ETF_VERIFY(EMF_arrayFifo_getUsed(&fifo) == 2U);
    ETF_VERIFY(EMF_arrayFifo_getFree(&fifo) == 1U);

    EMF_arrayFifo_pop(&fifo, output);
    verifySlotEq(input_1, output, 2U);
    ETF_VERIFY(EMF_arrayFifo_getUsed(&fifo) == 1U);
    ETF_VERIFY(EMF_arrayFifo_getFree(&fifo) == 2U);

    EMF_arrayFifo_pop(&fifo, output);
    verifySlotEq(input_2, output, 2U);

    ETF_VERIFY(EMF_arrayFifo_isEmpty(&fifo));
    ETF_VERIFY(EMF_arrayFifo_getUsed(&fifo) == 0U);
    ETF_VERIFY(EMF_arrayFifo_getFree(&fifo) == 3U);
  }

  ETF_TEST(peek_does_not_remove_and_drop_removes)
  {
    EMF_arrayFifo_handler_t fifo;
    uint8_t storage[2U * 2U] = {0U};
    uint8_t input_1[2U] = {10U, 11U};
    uint8_t input_2[2U] = {20U, 21U};
    uint8_t output[2U] = {0U, 0U};

    EMF_arrayFifo_init(&fifo, 2U, 2U, storage);
    EMF_arrayFifo_push(&fifo, input_1);
    EMF_arrayFifo_push(&fifo, input_2);
    ETF_VERIFY(EMF_arrayFifo_getUsed(&fifo) == 2U);

    EMF_arrayFifo_peek(&fifo, output);
    verifySlotEq(input_1, output, 2U);
    ETF_VERIFY(EMF_arrayFifo_getUsed(&fifo) == 2U);

    EMF_arrayFifo_pop(&fifo, output);
    verifySlotEq(input_1, output, 2U);

    EMF_arrayFifo_drop(&fifo);
    ETF_VERIFY(EMF_arrayFifo_isEmpty(&fifo));
    ETF_VERIFY(EMF_arrayFifo_getUsed(&fifo) == 0U);
    ETF_VERIFY(EMF_arrayFifo_getFree(&fifo) == 2U);
  }

  ETF_TEST(fill_to_full_updates_state_and_counters)
  {
    EMF_arrayFifo_handler_t fifo;
    uint8_t storage[2U] = {0U};
    uint8_t input_1[1U] = {0xA1U};
    uint8_t input_2[1U] = {0xB2U};
    uint8_t output[1U] = {0U};

    EMF_arrayFifo_init(&fifo, 2U, 1U, storage);
    EMF_arrayFifo_push(&fifo, input_1);
    EMF_arrayFifo_push(&fifo, input_2);

    ETF_VERIFY(EMF_arrayFifo_isFull(&fifo));
    ETF_VERIFY(EMF_arrayFifo_getUsed(&fifo) == 2U);
    ETF_VERIFY(EMF_arrayFifo_getFree(&fifo) == 0U);

    EMF_arrayFifo_pop(&fifo, output);
    ETF_VERIFY(output[0] == input_1[0]);
    ETF_VERIFY(EMF_arrayFifo_getUsed(&fifo) == 1U);
    ETF_VERIFY(EMF_arrayFifo_getFree(&fifo) == 1U);

    EMF_arrayFifo_pop(&fifo, output);
    ETF_VERIFY(output[0] == input_2[0]);
    ETF_VERIFY(EMF_arrayFifo_isEmpty(&fifo));
    ETF_VERIFY(EMF_arrayFifo_getUsed(&fifo) == 0U);
    ETF_VERIFY(EMF_arrayFifo_getFree(&fifo) == 2U);
  }

  ETF_TEST(flush_resets_state)
  {
    EMF_arrayFifo_handler_t fifo;
    uint8_t storage[2U] = {0U};
    uint8_t input[1U] = {0x55U};

    EMF_arrayFifo_init(&fifo, 2U, 1U, storage);
    EMF_arrayFifo_push(&fifo, input);
    EMF_arrayFifo_flush(&fifo);

    ETF_VERIFY(EMF_arrayFifo_isEmpty(&fifo));
    ETF_VERIFY(!EMF_arrayFifo_isFull(&fifo));
    ETF_VERIFY(EMF_arrayFifo_getUsed(&fifo) == 0U);
    ETF_VERIFY(EMF_arrayFifo_getFree(&fifo) == 2U);
  }

  ETF_TEST(pop_then_push_wraps_around_and_keeps_order)
  {
    EMF_arrayFifo_handler_t fifo;
    uint8_t storage[2U] = {0U};
    uint8_t input_1[1U] = {0x11U};
    uint8_t input_2[1U] = {0x22U};
    uint8_t input_3[1U] = {0x33U};
    uint8_t output[1U] = {0U};

    EMF_arrayFifo_init(&fifo, 2U, 1U, storage);
    EMF_arrayFifo_push(&fifo, input_1);
    EMF_arrayFifo_push(&fifo, input_2);
    EMF_arrayFifo_pop(&fifo, output);
    ETF_VERIFY(output[0] == input_1[0]);

    EMF_arrayFifo_push(&fifo, input_3);
    ETF_VERIFY(EMF_arrayFifo_isFull(&fifo));
    ETF_VERIFY(EMF_arrayFifo_getUsed(&fifo) == 2U);
    ETF_VERIFY(EMF_arrayFifo_getFree(&fifo) == 0U);

    EMF_arrayFifo_pop(&fifo, output);
    ETF_VERIFY(output[0] == input_2[0]);
    EMF_arrayFifo_pop(&fifo, output);
    ETF_VERIFY(output[0] == input_3[0]);
    ETF_VERIFY(EMF_arrayFifo_isEmpty(&fifo));
    ETF_VERIFY(EMF_arrayFifo_getUsed(&fifo) == 0U);
    ETF_VERIFY(EMF_arrayFifo_getFree(&fifo) == 2U);
  }
}
