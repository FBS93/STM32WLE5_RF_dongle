/*******************************************************************************
 * @brief Isolated validation of RF operation admission and replacement.
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
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */
#include "eaf.h"
#include "edf.h"
#include "eff.h"
#include "emf.h"
#include "etf.h"

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "events.h"
#include "rf_manager.h"
#include "rf_manager_activities.h"
#include "rf_manager_hal.h"
#include "rf_types.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/** @brief Representative LoRa configuration using the DD's semantic fields. */
static const rfTypes_configuration_t loraConfiguration = {
  .packet_type = RF_TYPES_PACKET_TYPE_LORA,
  .rf_frequency_hz = 0x1B100000UL,
  .payload_length_bytes = 37U,
  .tx_output_power_dbm = -3,
  .packet_type_configuration.lora = {
    .spreading_factor = 7U,
    .bandwidth_khz = 4U,
    .coding_rate = 2U,
    .low_data_rate_optimize = 1U,
    .preamble_length = 12U,
    .header_mode = 1U,
    .payload_crc_enabled = 1U,
    .invert_iq = 1U,
    .synchronization_word = 0x3444U,
    .operation_timeout = 0x123456UL,
    .symbol_timeout = 19U,
    .tx_ramp_time_us = 3U,
    .cad = {.scan_symbol_count = 2U,
            .detection_peak_threshold = 22U,
            .detection_minimum_threshold = 10U,
            .exit_mode = 1U,
            .timeout = 0x234567UL},
    .rx_mode = 0U,
    .rx_timeout_control = 0x345678UL,
    .rx_duty_cycle = {.rx_period = 0x456789UL, .sleep_period = 0x56789AUL}}};

/** @brief Representative FSK configuration, not a hardware-default oracle. */
static const rfTypes_configuration_t fskConfiguration = {
  .packet_type = RF_TYPES_PACKET_TYPE_FSK,
  .rf_frequency_hz = 0x36400000UL,
  .payload_length_bytes = 73U,
  .tx_output_power_dbm = 14,
  .packet_type_configuration.fsk = {
    .bit_rate_bps = 50000UL,
    .frequency_deviation_hz = 0x6666UL,
    .pulse_shape = 0U,
    .receive_bandwidth_khz = 0x1AU,
    .preamble_length = 16U,
    .operation_timeout = 0x234567UL,
    .payload_length_mode = 1U,
    .preamble_detection_length_bits = 5U,
    .sync_word_length_bits = 64U,
    .sync_word = {0x12U, 0x23U, 0x34U, 0x45U, 0x56U, 0x67U, 0x78U, 0x89U},
    .address_filtering = 2U,
    .node_address = 0x35U,
    .broadcast_address = 0xCAU,
    .crc_type = 6U,
    .crc_initial_value = 0x1D0FU,
    .crc_polynomial = 0x1021U,
    .whitening_enabled = 1U,
    .whitening_initial_value = 0x123U,
    .tx_ramp_time_us = 4U,
    .rx_mode = 0U,
    .rx_timeout_control = 0x345678UL,
    .rx_duty_cycle = {.rx_period = 0x456789UL, .sleep_period = 0x56789AUL}}};

/** @brief MSK configuration exercises the preamble-only admission distinction.
 */
static const rfTypes_configuration_t mskConfiguration = {
  .packet_type = RF_TYPES_PACKET_TYPE_MSK,
  .rf_frequency_hz = 0x36400000UL,
  .payload_length_bytes = 29U,
  .tx_output_power_dbm = 10,
  .packet_type_configuration.msk = {
    .bit_rate_bps = 5000UL,
    .pulse_shape = 0U,
    .preamble_length = 16U,
    .operation_timeout = 0x123456UL,
    .payload_length_mode = 1U,
    .sync_word_length_bits = 64U,
    .sync_word = {0x12U, 0x23U, 0x34U, 0x45U, 0x56U, 0x67U, 0x78U, 0x89U},
    .address_filtering = 2U,
    .node_address = 0x35U,
    .broadcast_address = 0xCAU,
    .crc_type = 6U,
    .crc_initial_value = 0x1D0FU,
    .crc_polynomial = 0x1021U,
    .whitening_enabled = 1U,
    .whitening_initial_value = 0x123U,
    .tx_ramp_time_us = 4U}};

/** @brief BPSK configuration for unsupported-operation rejection. */
static const rfTypes_configuration_t bpskConfiguration = {
  .packet_type = RF_TYPES_PACKET_TYPE_BPSK,
  .rf_frequency_hz = 0x36400000UL,
  .payload_length_bytes = 11U,
  .tx_output_power_dbm = 8,
  .packet_type_configuration.bpsk = {.bit_rate_bps = 0x9C4000UL,
                                     .operation_timeout = 0x123456UL}};

/** @brief Activity state, never initialized as an AO. */
static rfManager_t manager;
/** @brief Synchronous snapshot of the actual HAL configuration argument. */
static rfTypes_configuration_t startedConfiguration;
/** @brief Synchronous snapshot of bytes actually passed to packet TX. */
static uint8_t transmittedData[RF_TYPES_PACKET_DATA_CAPACITY];

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/** @brief Shared EFF call history. */
DEFINE_EFF_GLOBALS;

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/**
 * @brief Compare every common and selected configuration member, not padding.
 * @param[in] actual Observed configuration.
 * @param[in] expected Normative fixture input to preserve or forward.
 */
static void verifyConfiguration(const rfTypes_configuration_t* actual,
                                const rfTypes_configuration_t* expected);
/**
 * @brief Snapshot a configuration while the HAL input argument is valid.
 * @param[in] configuration Actual HAL configuration argument.
 */
static void captureConfiguration(const rfTypes_configuration_t* configuration);
/**
 * @brief Snapshot actual packet TX arguments without requiring pointer
 * identity.
 * @param[in] configuration Actual HAL configuration argument.
 * @param[in] data Actual HAL packet bytes.
 * @param[in] length Actual HAL packet length.
 */
static void captureTransmission(const rfTypes_configuration_t* configuration,
                                const uint8_t* data,
                                uint8_t length);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static void verifyConfiguration(const rfTypes_configuration_t* actual,
                                const rfTypes_configuration_t* expected)
{
  ETF_VERIFY(actual->packet_type == expected->packet_type);
  ETF_VERIFY(actual->rf_frequency_hz == expected->rf_frequency_hz);
  ETF_VERIFY(actual->payload_length_bytes == expected->payload_length_bytes);
  ETF_VERIFY(actual->tx_output_power_dbm == expected->tx_output_power_dbm);

  // Only the selected union arm has semantic meaning in the DD.
  if (expected->packet_type == RF_TYPES_PACKET_TYPE_LORA)
  {
    const rfTypes_configurationLora_t* observed =
      &actual->packet_type_configuration.lora;
    const rfTypes_configurationLora_t* input =
      &expected->packet_type_configuration.lora;
    ETF_VERIFY(observed->spreading_factor == input->spreading_factor);
    ETF_VERIFY(observed->bandwidth_khz == input->bandwidth_khz);
    ETF_VERIFY(observed->coding_rate == input->coding_rate);
    ETF_VERIFY(observed->low_data_rate_optimize ==
               input->low_data_rate_optimize);
    ETF_VERIFY(observed->preamble_length == input->preamble_length);
    ETF_VERIFY(observed->header_mode == input->header_mode);
    ETF_VERIFY(observed->payload_crc_enabled == input->payload_crc_enabled);
    ETF_VERIFY(observed->invert_iq == input->invert_iq);
    ETF_VERIFY(observed->synchronization_word == input->synchronization_word);
    ETF_VERIFY(observed->operation_timeout == input->operation_timeout);
    ETF_VERIFY(observed->symbol_timeout == input->symbol_timeout);
    ETF_VERIFY(observed->tx_ramp_time_us == input->tx_ramp_time_us);
    ETF_VERIFY(observed->cad.scan_symbol_count == input->cad.scan_symbol_count);
    ETF_VERIFY(observed->cad.detection_peak_threshold ==
               input->cad.detection_peak_threshold);
    ETF_VERIFY(observed->cad.detection_minimum_threshold ==
               input->cad.detection_minimum_threshold);
    ETF_VERIFY(observed->cad.exit_mode == input->cad.exit_mode);
    ETF_VERIFY(observed->cad.timeout == input->cad.timeout);
    ETF_VERIFY(observed->rx_mode == input->rx_mode);
    ETF_VERIFY(observed->rx_timeout_control == input->rx_timeout_control);
    ETF_VERIFY(observed->rx_duty_cycle.rx_period ==
               input->rx_duty_cycle.rx_period);
    ETF_VERIFY(observed->rx_duty_cycle.sleep_period ==
               input->rx_duty_cycle.sleep_period);
  }
  else if (expected->packet_type == RF_TYPES_PACKET_TYPE_FSK)
  {
    const rfTypes_configurationFsk_t* observed =
      &actual->packet_type_configuration.fsk;
    const rfTypes_configurationFsk_t* input =
      &expected->packet_type_configuration.fsk;
    ETF_VERIFY(observed->bit_rate_bps == input->bit_rate_bps);
    ETF_VERIFY(observed->frequency_deviation_hz ==
               input->frequency_deviation_hz);
    ETF_VERIFY(observed->pulse_shape == input->pulse_shape);
    ETF_VERIFY(observed->receive_bandwidth_khz == input->receive_bandwidth_khz);
    ETF_VERIFY(observed->preamble_length == input->preamble_length);
    ETF_VERIFY(observed->operation_timeout == input->operation_timeout);
    ETF_VERIFY(observed->payload_length_mode == input->payload_length_mode);
    ETF_VERIFY(observed->preamble_detection_length_bits ==
               input->preamble_detection_length_bits);
    ETF_VERIFY(observed->sync_word_length_bits == input->sync_word_length_bits);
    ETF_VERIFY(memcmp(observed->sync_word,
                      input->sync_word,
                      sizeof(input->sync_word)) == 0);
    ETF_VERIFY(observed->address_filtering == input->address_filtering);
    ETF_VERIFY(observed->node_address == input->node_address);
    ETF_VERIFY(observed->broadcast_address == input->broadcast_address);
    ETF_VERIFY(observed->crc_type == input->crc_type);
    ETF_VERIFY(observed->crc_initial_value == input->crc_initial_value);
    ETF_VERIFY(observed->crc_polynomial == input->crc_polynomial);
    ETF_VERIFY(observed->whitening_enabled == input->whitening_enabled);
    ETF_VERIFY(observed->whitening_initial_value ==
               input->whitening_initial_value);
    ETF_VERIFY(observed->tx_ramp_time_us == input->tx_ramp_time_us);
    ETF_VERIFY(observed->rx_mode == input->rx_mode);
    ETF_VERIFY(observed->rx_timeout_control == input->rx_timeout_control);
    ETF_VERIFY(observed->rx_duty_cycle.rx_period ==
               input->rx_duty_cycle.rx_period);
    ETF_VERIFY(observed->rx_duty_cycle.sleep_period ==
               input->rx_duty_cycle.sleep_period);
  }
  else if (expected->packet_type == RF_TYPES_PACKET_TYPE_MSK)
  {
    const rfTypes_configurationMsk_t* observed =
      &actual->packet_type_configuration.msk;
    const rfTypes_configurationMsk_t* input =
      &expected->packet_type_configuration.msk;
    ETF_VERIFY(observed->bit_rate_bps == input->bit_rate_bps);
    ETF_VERIFY(observed->pulse_shape == input->pulse_shape);
    ETF_VERIFY(observed->preamble_length == input->preamble_length);
    ETF_VERIFY(observed->operation_timeout == input->operation_timeout);
    ETF_VERIFY(observed->payload_length_mode == input->payload_length_mode);
    ETF_VERIFY(observed->sync_word_length_bits == input->sync_word_length_bits);
    ETF_VERIFY(memcmp(observed->sync_word,
                      input->sync_word,
                      sizeof(input->sync_word)) == 0);
    ETF_VERIFY(observed->address_filtering == input->address_filtering);
    ETF_VERIFY(observed->node_address == input->node_address);
    ETF_VERIFY(observed->broadcast_address == input->broadcast_address);
    ETF_VERIFY(observed->crc_type == input->crc_type);
    ETF_VERIFY(observed->crc_initial_value == input->crc_initial_value);
    ETF_VERIFY(observed->crc_polynomial == input->crc_polynomial);
    ETF_VERIFY(observed->whitening_enabled == input->whitening_enabled);
    ETF_VERIFY(observed->whitening_initial_value ==
               input->whitening_initial_value);
    ETF_VERIFY(observed->tx_ramp_time_us == input->tx_ramp_time_us);
  }
  else
  {
    ETF_VERIFY(expected->packet_type == RF_TYPES_PACKET_TYPE_BPSK);
    ETF_VERIFY(actual->packet_type_configuration.bpsk.bit_rate_bps ==
               expected->packet_type_configuration.bpsk.bit_rate_bps);
    ETF_VERIFY(actual->packet_type_configuration.bpsk.operation_timeout ==
               expected->packet_type_configuration.bpsk.operation_timeout);
  }
}

static void captureConfiguration(const rfTypes_configuration_t* configuration)
{
  ETF_VERIFY(configuration != NULL);
  startedConfiguration = *configuration;
}

static void captureTransmission(const rfTypes_configuration_t* configuration,
                                const uint8_t* data,
                                uint8_t length)
{
  size_t byte_index;

  ETF_VERIFY(configuration != NULL);
  ETF_VERIFY(data != NULL);
  startedConfiguration = *configuration;
  for (byte_index = 0U; byte_index < length; ++byte_index)
  {
    transmittedData[byte_index] = data[byte_index];
  }
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/** @brief Observe RF stopping, including forbidden stop-on-rejection. */
FAKE_VOID_FUNC(rfManagerHal_stop);
/**
 * @brief Observe forbidden configuration changes.
 * EFF copies pointer values, not configurations; sizeof(pointer) is
 * intentional.
 */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_setConfiguration, const rfTypes_configuration_t*);
/** @brief Observe forbidden calibration. */
FAKE_VOID_FUNC(rfManagerHal_calibrate, uint8_t, uint8_t);
/**
 * @brief Capture packet TX arguments synchronously.
 * EFF copies pointer values into history, not their pointed-to configurations.
 */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_transmit,
               const rfTypes_configuration_t*,
               const uint8_t*,
               uint8_t);
/**
 * @brief Capture the RX configuration argument synchronously.
 * EFF copies pointer values, not configurations; sizeof(pointer) is
 * intentional.
 */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_startReceive, const rfTypes_configuration_t*);
/**
 * @brief Capture the CAD configuration argument synchronously.
 * EFF copies pointer values, not configurations; sizeof(pointer) is
 * intentional.
 */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_startCad, const rfTypes_configuration_t*);
/** @brief Observe continuous-wave TX. */
FAKE_VOID_FUNC(rfManagerHal_startContinuousWave);
/** @brief Observe continuous-preamble TX. */
FAKE_VOID_FUNC(rfManagerHal_startContinuousPreamble);
/**
 * @brief Link-only observer for unused receive callbacks in the activity
 * object. EFF copies pointer values, not configurations/status, into argument
 * history.
 */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VALUE_FUNC(uint8_t,
                rfManagerHal_readReceivedPacket,
                uint8_t*,
                uint8_t,
                const rfTypes_configuration_t*,
                rfTypes_packetStatus_t*);
/** @brief Link-only observer for the unused diagnostics activity. */
FAKE_VALUE_FUNC(uint16_t, rfManagerHal_getErrors);
/**
 * @brief EFF allocator observer; admission must not allocate events.
 * EFF copies returned pointer values, not events; sizeof(pointer) is
 * intentional.
 */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VALUE_FUNC(EDF_event_t*, EDF_event_initMutable, uint_fast16_t, int);
/**
 * @brief EFF publisher observer; admission must not publish events.
 * EFF copies pointer values, not events; sizeof(pointer) is intentional.
 */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(EDF_activeObject_publish, const EDF_event_t*);

/** @brief Turn unexpected production assertions into ETF failures. */
void EAF_onError(const char* file, int line, int id)
{
  EMF_UTILS_UNUSED_PARAM(id);
  ETF_fail("Unexpected production assertion", file, line);
}

/** @brief Reset the isolated manager, all collaborators and snapshots directly.
 */
void ETF_setUp(void)
{
  size_t byte_index;

  RESET_FAKE(rfManagerHal_stop);
  RESET_FAKE(rfManagerHal_setConfiguration);
  RESET_FAKE(rfManagerHal_calibrate);
  RESET_FAKE(rfManagerHal_transmit);
  RESET_FAKE(rfManagerHal_startReceive);
  RESET_FAKE(rfManagerHal_startCad);
  RESET_FAKE(rfManagerHal_startContinuousWave);
  RESET_FAKE(rfManagerHal_startContinuousPreamble);
  RESET_FAKE(rfManagerHal_readReceivedPacket);
  RESET_FAKE(rfManagerHal_getErrors);
  RESET_FAKE(EDF_event_initMutable);
  RESET_FAKE(EDF_activeObject_publish);
  eff = (eff_globals_t){0};
  manager =
    (rfManager_t){.configuration = loraConfiguration,
                  .configuration_valid = false,
                  .active_operation = RF_TYPES_OPERATION_CONTINUOUS_WAVE_TX};
  startedConfiguration = (rfTypes_configuration_t){0};
  for (byte_index = 0U; byte_index < sizeof(transmittedData); ++byte_index)
  {
    transmittedData[byte_index] = 0U;
  }
  rfManagerHal_startReceive_fake.custom_fake = captureConfiguration;
  rfManagerHal_startCad_fake.custom_fake = captureConfiguration;
  rfManagerHal_transmit_fake.custom_fake = captureTransmission;
}

/** @brief Validate the DD's admission decisions, not complete HSM transitions.
 */
ETF_TEST_SUITE(rf_operation_admission)
{
  ETF_TEST(rxRequiresConfiguration)
  {
    events_rxStart_t request = {.super =
                                  EDF_EVENT_IMMUTABLE_INIT(EVENTS_RX_START)};
    bool accepted;

    accepted = rfManagerActivities_handleRxStart(&manager, &request);

    ETF_VERIFY(!accepted);
    ETF_VERIFY(!manager.configuration_valid);
    ETF_VERIFY(manager.active_operation ==
               RF_TYPES_OPERATION_CONTINUOUS_WAVE_TX);
    verifyConfiguration(&manager.configuration, &loraConfiguration);
    ETF_VERIFY(eff.call_history_idx == 0U);
  }

  ETF_TEST(rxAcceptsLoraAndFsk)
  {
    const rfTypes_configuration_t* configurations[] = {&loraConfiguration,
                                                       &fskConfiguration};
    events_rxStart_t request = {.super =
                                  EDF_EVENT_IMMUTABLE_INIT(EVENTS_RX_START)};
    size_t configuration_index;
    bool accepted;

    // Independent stored configurations, not a sequence of HSM states.
    for (configuration_index = 0U;
         configuration_index <
         sizeof(configurations) / sizeof(configurations[0]);
         ++configuration_index)
    {
      manager.configuration = *configurations[configuration_index];
      manager.configuration_valid = true;
      manager.active_operation = RF_TYPES_OPERATION_CONTINUOUS_WAVE_TX;

      accepted = rfManagerActivities_handleRxStart(&manager, &request);

      ETF_VERIFY(accepted);
      ETF_VERIFY(manager.configuration_valid);
      ETF_VERIFY(manager.active_operation == RF_TYPES_OPERATION_RX);
      verifyConfiguration(&manager.configuration,
                          configurations[configuration_index]);
      verifyConfiguration(&startedConfiguration,
                          configurations[configuration_index]);
      ETF_VERIFY(rfManagerHal_stop_fake.call_count == configuration_index + 1U);
      ETF_VERIFY(rfManagerHal_startReceive_fake.call_count ==
                 configuration_index + 1U);
      ETF_VERIFY(eff.call_history_idx == 2U * (configuration_index + 1U));
      ETF_VERIFY(eff.call_history[2U * configuration_index] ==
                 (eff_function_t)rfManagerHal_stop);
      ETF_VERIFY(eff.call_history[2U * configuration_index + 1U] ==
                 (eff_function_t)rfManagerHal_startReceive);
    }
  }

  ETF_TEST(rxRejectsMskAndBpsk)
  {
    const rfTypes_configuration_t* configurations[] = {&mskConfiguration,
                                                       &bpskConfiguration};
    events_rxStart_t request = {.super =
                                  EDF_EVENT_IMMUTABLE_INIT(EVENTS_RX_START)};
    size_t configuration_index;
    bool accepted;

    for (configuration_index = 0U;
         configuration_index <
         sizeof(configurations) / sizeof(configurations[0]);
         ++configuration_index)
    {
      manager.configuration = *configurations[configuration_index];
      manager.configuration_valid = true;
      manager.active_operation = RF_TYPES_OPERATION_CONTINUOUS_WAVE_TX;

      accepted = rfManagerActivities_handleRxStart(&manager, &request);

      ETF_VERIFY(!accepted);
      ETF_VERIFY(manager.configuration_valid);
      ETF_VERIFY(manager.active_operation ==
                 RF_TYPES_OPERATION_CONTINUOUS_WAVE_TX);
      verifyConfiguration(&manager.configuration,
                          configurations[configuration_index]);
      ETF_VERIFY(eff.call_history_idx == 0U);
    }
  }

  ETF_TEST(cadRequiresConfiguration)
  {
    events_cadStart_t request = {.super =
                                   EDF_EVENT_IMMUTABLE_INIT(EVENTS_CAD_START)};
    bool accepted;

    accepted = rfManagerActivities_handleCadStart(&manager, &request);

    ETF_VERIFY(!accepted);
    ETF_VERIFY(!manager.configuration_valid);
    ETF_VERIFY(manager.active_operation ==
               RF_TYPES_OPERATION_CONTINUOUS_WAVE_TX);
    verifyConfiguration(&manager.configuration, &loraConfiguration);
    ETF_VERIFY(eff.call_history_idx == 0U);
  }

  ETF_TEST(cadAcceptsLora)
  {
    events_cadStart_t request = {.super =
                                   EDF_EVENT_IMMUTABLE_INIT(EVENTS_CAD_START)};
    bool accepted;

    manager.configuration_valid = true;
    accepted = rfManagerActivities_handleCadStart(&manager, &request);

    ETF_VERIFY(accepted);
    ETF_VERIFY(manager.configuration_valid);
    ETF_VERIFY(manager.active_operation == RF_TYPES_OPERATION_CAD);
    verifyConfiguration(&manager.configuration, &loraConfiguration);
    verifyConfiguration(&startedConfiguration, &loraConfiguration);
    ETF_VERIFY(rfManagerHal_stop_fake.call_count == 1U);
    ETF_VERIFY(rfManagerHal_startCad_fake.call_count == 1U);
    ETF_VERIFY(eff.call_history_idx == 2U);
    ETF_VERIFY(eff.call_history[0] == (eff_function_t)rfManagerHal_stop);
    ETF_VERIFY(eff.call_history[1] == (eff_function_t)rfManagerHal_startCad);
  }

  ETF_TEST(cadRejectsFsk)
  {
    events_cadStart_t request = {.super =
                                   EDF_EVENT_IMMUTABLE_INIT(EVENTS_CAD_START)};
    bool accepted;

    manager.configuration = fskConfiguration;
    manager.configuration_valid = true;
    accepted = rfManagerActivities_handleCadStart(&manager, &request);

    ETF_VERIFY(!accepted);
    ETF_VERIFY(manager.configuration_valid);
    ETF_VERIFY(manager.active_operation ==
               RF_TYPES_OPERATION_CONTINUOUS_WAVE_TX);
    verifyConfiguration(&manager.configuration, &fskConfiguration);
    ETF_VERIFY(eff.call_history_idx == 0U);
  }

  ETF_TEST(preambleRequiresConfiguration)
  {
    events_txContinuousPreamble_t request = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_TX_CONTINUOUS_PREAMBLE)};
    bool accepted;

    manager.active_operation = RF_TYPES_OPERATION_RX;
    accepted =
      rfManagerActivities_handleTxContinuousPreamble(&manager, &request);

    ETF_VERIFY(!accepted);
    ETF_VERIFY(!manager.configuration_valid);
    ETF_VERIFY(manager.active_operation == RF_TYPES_OPERATION_RX);
    verifyConfiguration(&manager.configuration, &loraConfiguration);
    ETF_VERIFY(eff.call_history_idx == 0U);
  }

  ETF_TEST(preambleAcceptsLoraFskAndMsk)
  {
    const rfTypes_configuration_t* configurations[] = {
      &loraConfiguration, &fskConfiguration, &mskConfiguration};
    events_txContinuousPreamble_t request = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_TX_CONTINUOUS_PREAMBLE)};
    size_t configuration_index;
    bool accepted;

    for (configuration_index = 0U;
         configuration_index <
         sizeof(configurations) / sizeof(configurations[0]);
         ++configuration_index)
    {
      manager.configuration = *configurations[configuration_index];
      manager.configuration_valid = true;
      manager.active_operation = RF_TYPES_OPERATION_RX;

      accepted =
        rfManagerActivities_handleTxContinuousPreamble(&manager, &request);

      ETF_VERIFY(accepted);
      ETF_VERIFY(manager.configuration_valid);
      ETF_VERIFY(manager.active_operation ==
                 RF_TYPES_OPERATION_CONTINUOUS_PREAMBLE_TX);
      verifyConfiguration(&manager.configuration,
                          configurations[configuration_index]);
      ETF_VERIFY(rfManagerHal_stop_fake.call_count == configuration_index + 1U);
      ETF_VERIFY(rfManagerHal_startContinuousPreamble_fake.call_count ==
                 configuration_index + 1U);
      ETF_VERIFY(eff.call_history_idx == 2U * (configuration_index + 1U));
      ETF_VERIFY(eff.call_history[2U * configuration_index] ==
                 (eff_function_t)rfManagerHal_stop);
      ETF_VERIFY(eff.call_history[2U * configuration_index + 1U] ==
                 (eff_function_t)rfManagerHal_startContinuousPreamble);
    }
  }

  ETF_TEST(preambleRejectsBpsk)
  {
    events_txContinuousPreamble_t request = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_TX_CONTINUOUS_PREAMBLE)};
    bool accepted;

    manager.configuration = bpskConfiguration;
    manager.configuration_valid = true;
    manager.active_operation = RF_TYPES_OPERATION_RX;
    accepted =
      rfManagerActivities_handleTxContinuousPreamble(&manager, &request);

    ETF_VERIFY(!accepted);
    ETF_VERIFY(manager.configuration_valid);
    ETF_VERIFY(manager.active_operation == RF_TYPES_OPERATION_RX);
    verifyConfiguration(&manager.configuration, &bpskConfiguration);
    ETF_VERIFY(eff.call_history_idx == 0U);
  }

  ETF_TEST(packetRequiresConfiguration)
  {
    events_txPacket_t request = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_TX_PACKET),
      .packet_data = {0x11U, 0x00U, 0xA5U},
      .payload_length = 3U};

    manager.active_operation = RF_TYPES_OPERATION_RX;
    rfManagerActivities_handleTxPacket(&manager, &request);

    ETF_VERIFY(!manager.configuration_valid);
    ETF_VERIFY(manager.active_operation == RF_TYPES_OPERATION_RX);
    verifyConfiguration(&manager.configuration, &loraConfiguration);
    ETF_VERIFY(eff.call_history_idx == 0U);
  }

  ETF_TEST(packetReplacesPreviousOperation)
  {
    events_txPacket_t request = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_TX_PACKET),
      .packet_data = {0x11U, 0x00U, 0xA5U},
      .payload_length = 3U};

    // Request length differs from configured RX length (37 bytes).
    manager.configuration_valid = true;
    manager.active_operation = RF_TYPES_OPERATION_RX;
    rfManagerActivities_handleTxPacket(&manager, &request);

    ETF_VERIFY(manager.configuration_valid);
    ETF_VERIFY(manager.active_operation == RF_TYPES_OPERATION_PACKET_TX);
    verifyConfiguration(&manager.configuration, &loraConfiguration);
    verifyConfiguration(&startedConfiguration, &loraConfiguration);
    ETF_VERIFY(rfManagerHal_transmit_fake.arg2_val == request.payload_length);
    ETF_VERIFY(memcmp(transmittedData,
                      request.packet_data,
                      request.payload_length) == 0);
    ETF_VERIFY(rfManagerHal_stop_fake.call_count == 1U);
    ETF_VERIFY(rfManagerHal_transmit_fake.call_count == 1U);
    ETF_VERIFY(eff.call_history_idx == 2U);
    ETF_VERIFY(eff.call_history[0] == (eff_function_t)rfManagerHal_stop);
    ETF_VERIFY(eff.call_history[1] == (eff_function_t)rfManagerHal_transmit);
  }

  ETF_TEST(waveRequiresConfiguration)
  {
    events_txContinuousWave_t request = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_TX_CONTINUOUS_WAVE)};

    manager.active_operation = RF_TYPES_OPERATION_RX;
    rfManagerActivities_handleTxContinuousWave(&manager, &request);

    ETF_VERIFY(!manager.configuration_valid);
    ETF_VERIFY(manager.active_operation == RF_TYPES_OPERATION_RX);
    verifyConfiguration(&manager.configuration, &loraConfiguration);
    ETF_VERIFY(eff.call_history_idx == 0U);
  }

  ETF_TEST(waveReplacesPreviousOperation)
  {
    events_txContinuousWave_t request = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_TX_CONTINUOUS_WAVE)};

    manager.configuration_valid = true;
    manager.active_operation = RF_TYPES_OPERATION_RX;
    rfManagerActivities_handleTxContinuousWave(&manager, &request);

    ETF_VERIFY(manager.configuration_valid);
    ETF_VERIFY(manager.active_operation ==
               RF_TYPES_OPERATION_CONTINUOUS_WAVE_TX);
    verifyConfiguration(&manager.configuration, &loraConfiguration);
    ETF_VERIFY(rfManagerHal_stop_fake.call_count == 1U);
    ETF_VERIFY(rfManagerHal_startContinuousWave_fake.call_count == 1U);
    ETF_VERIFY(eff.call_history_idx == 2U);
    ETF_VERIFY(eff.call_history[0] == (eff_function_t)rfManagerHal_stop);
    ETF_VERIFY(eff.call_history[1] ==
               (eff_function_t)rfManagerHal_startContinuousWave);
  }
}
