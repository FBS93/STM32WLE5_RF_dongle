/*******************************************************************************
 * @brief Embedded Fake Framework functional test.
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
#include "eff_dummy_mock.h"

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

/**
 * @brief EFF global state used by the generated fakes.
 */
DEFINE_EFF_GLOBALS;

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */
/**
 * @brief Dummy callback used to exercise callback mock arguments.
 *
 * @param[in] status Callback status value.
 * @param[in] payload Pointer to callback payload data.
 */
static void dummy_callback(uint8_t status, const uint8_t* payload);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */
static void dummy_callback(uint8_t status, const uint8_t* payload)
{
  (void)status;
  (void)payload;
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

ETF_TEST_SUITE(test_eff)
{
  ETF_TEST(generated_mock_tracks_calls_arguments_and_global_order)
  {
    uint8_t input_buffer[4U] = {0x10U, 0x20U, 0x30U, 0x40U};
    uint16_t output_value = 0U;
    uint8_t sample_count = 2U;
    uint8_t sample_window[4U] = {1U, 2U, 3U, 4U};
    uint16_t candidate_buffers[2U] = {11U, 22U};
    uint16_t reference_value = 33U;
    uint16_t* selected_buffer;
    uint8_t return_sequence[] = {5U, 7U};
    uint8_t measured_value;

    EFF_effDummy_ResetAll();
    EFF_RESET_HISTORY();

    effDummy_selectBuffer_fake.return_val = &candidate_buffers[1U];
    SET_RETURN_SEQ(effDummy_measure, return_sequence, 2U);

    effDummy_storeSample(input_buffer, &output_value);
    effDummy_registerCallback(dummy_callback);
    selected_buffer =
      effDummy_selectBuffer(candidate_buffers, &reference_value);
    (void)effDummy_measure(&sample_count, sample_window);
    (void)effDummy_measure(&sample_count, sample_window);
    measured_value = effDummy_measure(&sample_count, sample_window);

    ETF_VERIFY(effDummy_storeSample_fake.call_count == 1U);
    ETF_VERIFY(effDummy_storeSample_fake.arg0_val == input_buffer);
    ETF_VERIFY(effDummy_storeSample_fake.arg1_val == &output_value);

    ETF_VERIFY(effDummy_registerCallback_fake.call_count == 1U);
    ETF_VERIFY(effDummy_registerCallback_fake.arg0_val == dummy_callback);

    ETF_VERIFY(effDummy_selectBuffer_fake.call_count == 1U);
    ETF_VERIFY(effDummy_selectBuffer_fake.arg0_val == candidate_buffers);
    ETF_VERIFY(effDummy_selectBuffer_fake.arg1_val == &reference_value);
    ETF_VERIFY(selected_buffer == &candidate_buffers[1U]);

    ETF_VERIFY(effDummy_measure_fake.call_count == 3U);
    ETF_VERIFY(effDummy_measure_fake.arg0_val == &sample_count);
    ETF_VERIFY(effDummy_measure_fake.arg1_val == sample_window);
    ETF_VERIFY(effDummy_measure_fake.arg1_history[0U] == sample_window);
    ETF_VERIFY(effDummy_measure_fake.arg1_history[1U] == sample_window);
    ETF_VERIFY(effDummy_measure_fake.arg1_history[2U] == sample_window);
    ETF_VERIFY(effDummy_measure_fake.return_val_seq_len == 2);
    ETF_VERIFY(effDummy_measure_fake.return_val_history[0U] == 5U);
    ETF_VERIFY(effDummy_measure_fake.return_val_history[1U] == 7U);
    ETF_VERIFY(effDummy_measure_fake.return_val_history[2U] == 7U);
    ETF_VERIFY(measured_value == 7U);

    ETF_VERIFY(eff.call_history_idx == 6U);
    ETF_VERIFY(eff.call_history[0U] == (eff_function_t)effDummy_storeSample);
    ETF_VERIFY(eff.call_history[1U] ==
               (eff_function_t)effDummy_registerCallback);
    ETF_VERIFY(eff.call_history[2U] == (eff_function_t)effDummy_selectBuffer);
    ETF_VERIFY(eff.call_history[3U] == (eff_function_t)effDummy_measure);
    ETF_VERIFY(eff.call_history[4U] == (eff_function_t)effDummy_measure);
    ETF_VERIFY(eff.call_history[5U] == (eff_function_t)effDummy_measure);
  }

  ETF_TEST(reset_helpers_clear_fake_state_and_global_history)
  {
    uint8_t input_buffer[4U] = {0xAAU, 0xBBU, 0xCCU, 0xDDU};
    uint16_t output_value = 0U;
    uint8_t sample_count = 0U;
    uint8_t sample_window[4U] = {9U, 8U, 7U, 6U};
    uint8_t return_sequence[] = {1U, 2U};

    EFF_effDummy_ResetAll();
    EFF_RESET_HISTORY();

    SET_RETURN_SEQ(effDummy_measure, return_sequence, 2U);
    effDummy_storeSample(input_buffer, &output_value);
    (void)effDummy_measure(&sample_count, sample_window);
    effDummy_registerCallback(dummy_callback);

    ETF_VERIFY(effDummy_storeSample_fake.call_count == 1U);
    ETF_VERIFY(effDummy_measure_fake.call_count == 1U);
    ETF_VERIFY(effDummy_registerCallback_fake.call_count == 1U);
    ETF_VERIFY(eff.call_history_idx == 3U);

    EFF_effDummy_ResetAll();

    ETF_VERIFY(effDummy_storeSample_fake.call_count == 0U);
    ETF_VERIFY(effDummy_storeSample_fake.arg_history_len ==
               EFF_ARG_HISTORY_LEN);
    ETF_VERIFY(effDummy_measure_fake.call_count == 0U);
    ETF_VERIFY(effDummy_measure_fake.return_val_seq_len == 0);
    ETF_VERIFY(effDummy_measure_fake.return_val_seq_idx == 0);
    ETF_VERIFY(effDummy_registerCallback_fake.call_count == 0U);
    ETF_VERIFY(effDummy_registerCallback_fake.arg0_val == 0);

    EFF_RESET_HISTORY();

    ETF_VERIFY(eff.call_history_idx == 0U);
    ETF_VERIFY(eff.call_history[0U] == (eff_function_t)0);
    ETF_VERIFY(eff.call_history[1U] == (eff_function_t)0);
    ETF_VERIFY(eff.call_history[2U] == (eff_function_t)0);
  }
}
