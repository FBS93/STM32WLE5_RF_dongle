/*******************************************************************************
 * @brief Host command manager HSM activity implementation.
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
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */
#include "eaf.h"
#include "edf.h"
#include "emf.h"

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "events.h"
#include "host_command_manager_activities.h"
#include "host_command_manager_hal.h"
#include "rf_types.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/** @brief Decoded frame bytes excluding payload. */
#define FRAME_OVERHEAD 6U

/** @brief Maximum decoded request or response frame size. */
#define DECODED_FRAME_CAPACITY 264U

/** @brief Maximum encoded response size including the COBS delimiter. */
#define ENCODED_FRAME_CAPACITY EMF_COBS_ENCODED_SIZE(DECODED_FRAME_CAPACITY)

/** @brief HSE32 frequency used by the encoded FSK deviation, in hertz. */
#define HSE32_FREQUENCY_HZ 32000000ULL
/**
 * @brief 160 Hz is 5 ppm at 32 MHz, conservatively rounded from 4.4 ppm
 * without aging for Y1 ECS-TXO-25CSMV-320-DY-TR; datasheet:
 * https://ecsxtal.com/store/pdf/ECS-TXO-25CSMV.pdf.
 */
#define HSE32_MAX_FREQUENCY_ERROR_HZ 160ULL
/** @brief Denominator of one encoded FSK frequency-deviation step. */
#define FSK_DEVIATION_STEP_DENOMINATOR (1ULL << 25U)
/** @brief Number of 15.625 microsecond radio timer ticks per second. */
#define RADIO_TIMER_TICKS_PER_SECOND 64000ULL

/** @brief GET_CONFIG_REQUEST command identifier. */
#define COMMAND_GET_CONFIG_REQUEST 0x0000U
/** @brief SET_CONFIG command identifier. */
#define COMMAND_SET_CONFIG 0x0001U
/** @brief CALIBRATE command identifier. */
#define COMMAND_CALIBRATE 0x0002U
/** @brief TX_PACKET command identifier. */
#define COMMAND_TX_PACKET 0x0003U
/** @brief RX_START command identifier. */
#define COMMAND_RX_START 0x0004U
/** @brief CAD_START command identifier. */
#define COMMAND_CAD_START 0x0005U
/** @brief TX_CONTINUOUS_WAVE command identifier. */
#define COMMAND_TX_CONTINUOUS_WAVE 0x0006U
/** @brief TX_CONTINUOUS_PREAMBLE command identifier. */
#define COMMAND_TX_CONTINUOUS_PREAMBLE 0x0007U
/** @brief STOP command identifier. */
#define COMMAND_STOP 0x0008U
/** @brief GET_DIAGNOSTICS_REQUEST command identifier. */
#define COMMAND_GET_DIAGNOSTICS_REQUEST 0x0009U
/** @brief GET_CONFIG_RESPONSE command identifier. */
#define COMMAND_GET_CONFIG_RESPONSE 0x8001U
/** @brief GET_DIAGNOSTICS_RESPONSE command identifier. */
#define COMMAND_GET_DIAGNOSTICS_RESPONSE 0x8002U
/** @brief RX_PACKET report command identifier. */
#define COMMAND_RX_PACKET 0x8003U
/** @brief CAD_DETECTED report command identifier. */
#define COMMAND_CAD_DETECTED 0x8004U

/** @brief Common configuration payload prefix size. */
#define CONFIGURATION_COMMON_SIZE 7U
/** @brief LoRa configuration payload size. */
#define CONFIGURATION_LORA_SIZE 40U
/** @brief FSK configuration payload size. */
#define CONFIGURATION_FSK_SIZE 53U
/** @brief MSK configuration payload size. */
#define CONFIGURATION_MSK_SIZE 38U
/** @brief BPSK configuration payload size. */
#define CONFIGURATION_BPSK_SIZE 13U

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/** @brief Define static file name string for asserts. */
EAF_DEFINE_THIS_FILE(__FILE__);

/** @brief Immutable payload-free request events. */
static const events_getConfigRequest_t getConfigRequestEvent = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_GET_CONFIG_REQUEST)};
/** @brief Immutable RX_START request event. */
static const events_rxStart_t rxStartEvent = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_RX_START)};
/** @brief Immutable CAD_START request event. */
static const events_cadStart_t cadStartEvent = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_CAD_START)};
/** @brief Immutable continuous-wave request event. */
static const events_txContinuousWave_t txContinuousWaveEvent = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_TX_CONTINUOUS_WAVE)};
/** @brief Immutable continuous-preamble request event. */
static const events_txContinuousPreamble_t txContinuousPreambleEvent = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_TX_CONTINUOUS_PREAMBLE)};
/** @brief Immutable STOP request event. */
static const events_stop_t stopEvent = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_STOP)};
/** @brief Immutable diagnostics request event. */
static const events_getDiagnosticsRequest_t getDiagnosticsRequestEvent = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_GET_DIAGNOSTICS_REQUEST)};

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/** @brief Returns the exact serialized size selected by a packet type. */
static uint16_t getConfigurationSize(uint8_t packet_type);
/** @brief Returns whether a complete configuration is semantically valid. */
static bool isConfigurationValid(const rfTypes_configuration_t* configuration);
/** @brief Returns whether a generic pulse-shape encoding is valid. */
static bool isPulseShapeValid(uint8_t pulse_shape);
/** @brief Returns whether a generic CRC-type encoding is valid. */
static bool isCrcTypeValid(uint8_t crc_type);
/** @brief Returns whether a LoRa bandwidth encoding is valid. */
static bool isLoraBandwidthValid(uint8_t bandwidth);
/** @brief Converts an FSK receive-bandwidth encoding to hertz, or zero. */
static uint32_t getFskBandwidthHz(uint8_t bandwidth);
/** @brief Returns whether a single-RX timeout is valid for the RX mode. */
static bool isRxTimeoutValid(uint8_t rx_mode, uint32_t timeout);
/** @brief Returns whether the FSK modulation constraints are valid. */
static bool isFskModulationValid(const rfTypes_configurationFsk_t* fsk);
/** @brief Returns whether the FSK RX duty-cycle timing is valid. */
static bool isFskRxDutyCycleValid(const rfTypes_configurationFsk_t* fsk);
/** @brief Gets the LoRa-specific configuration from its wire representation. */
static void getLoraConfiguration(const uint8_t* data,
                                 rfTypes_configurationLora_t* lora);
/** @brief Sets the LoRa-specific wire representation. */
static void setLoraConfiguration(uint8_t* data,
                                 const rfTypes_configurationLora_t* lora);
/** @brief Gets the FSK-specific configuration from its wire representation. */
static void getFskConfiguration(const uint8_t* data,
                                rfTypes_configurationFsk_t* fsk);
/** @brief Sets the FSK-specific wire representation. */
static void setFskConfiguration(uint8_t* data,
                                const rfTypes_configurationFsk_t* fsk);
/** @brief Gets the MSK-specific configuration from its wire representation. */
static void getMskConfiguration(const uint8_t* data,
                                rfTypes_configurationMsk_t* msk);
/** @brief Sets the MSK-specific wire representation. */
static void setMskConfiguration(uint8_t* data,
                                const rfTypes_configurationMsk_t* msk);
/** @brief Gets the BPSK-specific configuration from its wire representation. */
static void getBpskConfiguration(const uint8_t* data,
                                 rfTypes_configurationBpsk_t* bpsk);
/** @brief Sets the BPSK-specific wire representation. */
static void setBpskConfiguration(uint8_t* data,
                                 const rfTypes_configurationBpsk_t* bpsk);
/** @brief Gets one complete configuration from its wire representation. */
static bool getConfiguration(const uint8_t* data,
                             uint16_t length,
                             rfTypes_configuration_t* configuration);
/** @brief Sets one complete configuration wire representation. */
static void setConfiguration(uint8_t* data,
                             const rfTypes_configuration_t* configuration);
/** @brief Validates and publishes a SET_CONFIG request. */
static bool publishSetConfigRequest(const uint8_t* payload,
                                    uint16_t payload_length);
/** @brief Validates and publishes a CALIBRATE request. */
static bool publishCalibrateRequest(const uint8_t* payload,
                                    uint16_t payload_length);
/** @brief Validates and publishes a TX_PACKET request. */
static bool publishTxPacketRequest(const uint8_t* payload,
                                   uint16_t payload_length);
/** @brief Publishes a validated request frame. */
static void publishRequest(uint16_t command_id,
                           const uint8_t* payload,
                           uint16_t payload_length);
/** @brief Decodes, validates, and dispatches one assembled frame. */
static void processFrame(const uint8_t* encoded, uint16_t encoded_length);
/** @brief Encodes and writes one response frame. */
static void sendResponse(uint16_t command_id,
                         const uint8_t* payload,
                         uint16_t payload_length);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static uint16_t getConfigurationSize(uint8_t packet_type)
{
  uint16_t size;

  switch (packet_type)
  {
    case RF_TYPES_PACKET_TYPE_LORA:
      size = CONFIGURATION_LORA_SIZE;
      break;
    case RF_TYPES_PACKET_TYPE_FSK:
      size = CONFIGURATION_FSK_SIZE;
      break;
    case RF_TYPES_PACKET_TYPE_MSK:
      size = CONFIGURATION_MSK_SIZE;
      break;
    case RF_TYPES_PACKET_TYPE_BPSK:
      size = CONFIGURATION_BPSK_SIZE;
      break;
    default:
      size = 0U;
      break;
  }

  return size;
}

static bool isPulseShapeValid(uint8_t pulse_shape)
{
  return (pulse_shape == 0x00U) ||
         ((pulse_shape >= 0x08U) && (pulse_shape <= 0x0BU));
}

static bool isCrcTypeValid(uint8_t crc_type)
{
  return (crc_type == 0x00U) || (crc_type == 0x01U) || (crc_type == 0x02U) ||
         (crc_type == 0x04U) || (crc_type == 0x06U);
}

static bool isLoraBandwidthValid(uint8_t bandwidth)
{
  return (bandwidth <= 0x06U) || (bandwidth == 0x08U) || (bandwidth == 0x09U) ||
         (bandwidth == 0x0AU);
}

static uint32_t getFskBandwidthHz(uint8_t bandwidth)
{
  static const struct
  {
    uint8_t encoding;
    uint32_t hertz;
  } bandwidths[] = {{0x1FU, 4800UL},   {0x17U, 5800UL},   {0x0FU, 7300UL},
                    {0x1EU, 9700UL},   {0x16U, 11700UL},  {0x0EU, 14600UL},
                    {0x1DU, 19500UL},  {0x15U, 23400UL},  {0x0DU, 29300UL},
                    {0x1CU, 39000UL},  {0x14U, 46900UL},  {0x0CU, 58600UL},
                    {0x1BU, 78200UL},  {0x13U, 93800UL},  {0x0BU, 117300UL},
                    {0x1AU, 156200UL}, {0x12U, 187200UL}, {0x0AU, 234300UL},
                    {0x19U, 312000UL}, {0x11U, 373600UL}, {0x09U, 467000UL}};
  uint32_t bandwidth_hz = 0UL;
  size_t index;

  for (index = 0U; index < (sizeof(bandwidths) / sizeof(bandwidths[0]));
       ++index)
  {
    if (bandwidths[index].encoding == bandwidth)
    {
      bandwidth_hz = bandwidths[index].hertz;
      break;
    }
  }

  return bandwidth_hz;
}

static bool isRxTimeoutValid(uint8_t rx_mode, uint32_t timeout)
{
  return (rx_mode != 0U) || (timeout <= 0xFFFFFEUL);
}

static bool isFskModulationValid(const rfTypes_configurationFsk_t* fsk)
{
  uint64_t occupied_bandwidth_scaled;
  uint64_t bandwidth_hz;

  EAF_ASSERT(fsk != NULL);

  bandwidth_hz = getFskBandwidthHz(fsk->receive_bandwidth_khz);
  // Evaluate BW > BR + 2D + 2E_HSE32 in D's encoded scale to avoid rounding.
  occupied_bandwidth_scaled =
    (2ULL * fsk->frequency_deviation_hz * HSE32_FREQUENCY_HZ) +
    (fsk->bit_rate_bps * FSK_DEVIATION_STEP_DENOMINATOR);

  return (bandwidth_hz != 0ULL) &&
         (occupied_bandwidth_scaled <=
          (250000ULL * 2ULL * FSK_DEVIATION_STEP_DENOMINATOR)) &&
         ((occupied_bandwidth_scaled + (2ULL * HSE32_MAX_FREQUENCY_ERROR_HZ *
                                        FSK_DEVIATION_STEP_DENOMINATOR)) <
          (bandwidth_hz * FSK_DEVIATION_STEP_DENOMINATOR));
}

static bool isFskRxDutyCycleValid(const rfTypes_configurationFsk_t* fsk)
{
  uint64_t detection_bits;
  uint64_t detection_window_ticks;

  EAF_ASSERT(fsk != NULL);

  detection_bits = (uint64_t)fsk->preamble_length + fsk->sync_word_length_bits;
  detection_window_ticks =
    (2ULL * fsk->rx_duty_cycle.rx_period) + fsk->rx_duty_cycle.sleep_period;

  return (fsk->rx_mode != 2U) || (fsk->rx_duty_cycle.rx_period == 0UL) ||
         ((detection_bits * RADIO_TIMER_TICKS_PER_SECOND) <
          (detection_window_ticks * fsk->bit_rate_bps));
}

static bool isConfigurationValid(const rfTypes_configuration_t* configuration)
{
  rfTypes_configurationLora_t lora;
  rfTypes_configurationFsk_t fsk;
  rfTypes_configurationMsk_t msk;
  rfTypes_configurationBpsk_t bpsk;
  bool valid;

  EAF_ASSERT(configuration != NULL);

  valid = (configuration->rf_frequency_hz >= 0x09600000UL) &&
          (configuration->rf_frequency_hz <= 0x3C000000UL) &&
          (configuration->tx_output_power_dbm >= -9) &&
          (configuration->tx_output_power_dbm <= 22);

  if (valid && (configuration->packet_type == RF_TYPES_PACKET_TYPE_LORA))
  {
    lora = configuration->packet_type_configuration.lora;
    valid = (lora.spreading_factor >= 5U) && (lora.spreading_factor <= 12U) &&
            isLoraBandwidthValid(lora.bandwidth_khz) &&
            !((configuration->rf_frequency_hz < 0x19000000UL) &&
              (lora.bandwidth_khz >= 0x05U) && (lora.bandwidth_khz <= 0x06U)) &&
            (lora.coding_rate <= 4U) && (lora.low_data_rate_optimize <= 1U) &&
            (lora.preamble_length >= 1U) &&
            !((lora.spreading_factor <= 6U) && (lora.preamble_length < 12U)) &&
            (lora.header_mode <= 1U) && (lora.payload_crc_enabled <= 1U) &&
            (lora.invert_iq <= 1U) &&
            ((lora.synchronization_word == 0x1424U) ||
             (lora.synchronization_word == 0x3444U)) &&
            (lora.tx_ramp_time_us <= 7U) &&
            (lora.cad.scan_symbol_count <= 4U) && (lora.cad.exit_mode <= 1U) &&
            (lora.rx_mode <= 2U) &&
            isRxTimeoutValid(lora.rx_mode, lora.rx_timeout_control);
  }
  else if (valid && (configuration->packet_type == RF_TYPES_PACKET_TYPE_FSK))
  {
    fsk = configuration->packet_type_configuration.fsk;
    valid = (fsk.bit_rate_bps >= 600UL) && (fsk.bit_rate_bps <= 300000UL) &&
            (fsk.frequency_deviation_hz >= 0x000276UL) &&
            (fsk.frequency_deviation_hz <= 0x033333UL) &&
            isPulseShapeValid(fsk.pulse_shape) && isFskModulationValid(&fsk) &&
            (fsk.preamble_length >= 8U) && (fsk.payload_length_mode <= 1U) &&
            ((fsk.preamble_detection_length_bits == 0x00U) ||
             ((fsk.preamble_detection_length_bits >= 0x04U) &&
              (fsk.preamble_detection_length_bits <= 0x07U))) &&
            (fsk.sync_word_length_bits <= 64U) &&
            (fsk.address_filtering <= 2U) &&
            !((fsk.address_filtering != 0U) &&
              (configuration->payload_length_bytes > 254U)) &&
            isCrcTypeValid(fsk.crc_type) && (fsk.whitening_enabled <= 1U) &&
            (fsk.whitening_initial_value <= 0x01FFU) &&
            (fsk.tx_ramp_time_us <= 7U) && (fsk.rx_mode <= 2U) &&
            isRxTimeoutValid(fsk.rx_mode, fsk.rx_timeout_control) &&
            isFskRxDutyCycleValid(&fsk);
  }
  else if (valid && (configuration->packet_type == RF_TYPES_PACKET_TYPE_MSK))
  {
    msk = configuration->packet_type_configuration.msk;
    valid =
      (msk.bit_rate_bps >= 100UL) && (msk.bit_rate_bps <= 10000UL) &&
      isPulseShapeValid(msk.pulse_shape) && (msk.preamble_length >= 8U) &&
      (msk.payload_length_mode <= 1U) && (msk.sync_word_length_bits <= 64U) &&
      (msk.address_filtering <= 2U) &&
      !((msk.address_filtering != 0U) &&
        (configuration->payload_length_bytes > 254U)) &&
      isCrcTypeValid(msk.crc_type) && (msk.whitening_enabled <= 1U) &&
      (msk.whitening_initial_value <= 0x01FFU) && (msk.tx_ramp_time_us <= 7U);
  }
  else if (valid && (configuration->packet_type == RF_TYPES_PACKET_TYPE_BPSK))
  {
    bpsk = configuration->packet_type_configuration.bpsk;
    valid =
      (bpsk.bit_rate_bps == 0x9C4000UL) || (bpsk.bit_rate_bps == 0x1A0AAAUL);
  }
  else
  {
    valid = false;
  }

  return valid;
}

static void getLoraConfiguration(const uint8_t* data,
                                 rfTypes_configurationLora_t* lora)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(lora != NULL);
  EAF_ASSERT_BLOCK_END();

  lora->spreading_factor = data[0];
  lora->bandwidth_khz = data[1];
  lora->coding_rate = data[2];
  lora->low_data_rate_optimize = data[3];
  EMF_endian_u16ReadBuffLE(&data[4], &lora->preamble_length);
  lora->header_mode = data[6];
  lora->payload_crc_enabled = data[7];
  lora->invert_iq = data[8];
  EMF_endian_u16ReadBuffLE(&data[9], &lora->synchronization_word);
  EMF_endian_u24ReadBuffLE(&data[11], &lora->operation_timeout);
  lora->symbol_timeout = data[14];
  lora->tx_ramp_time_us = data[15];

  lora->cad.scan_symbol_count = data[16];
  lora->cad.detection_peak_threshold = data[17];
  lora->cad.detection_minimum_threshold = data[18];
  lora->cad.exit_mode = data[19];
  EMF_endian_u24ReadBuffLE(&data[20], &lora->cad.timeout);

  lora->rx_mode = data[23];
  EMF_endian_u24ReadBuffLE(&data[24], &lora->rx_timeout_control);
  EMF_endian_u24ReadBuffLE(&data[27], &lora->rx_duty_cycle.rx_period);
  EMF_endian_u24ReadBuffLE(&data[30], &lora->rx_duty_cycle.sleep_period);
}

static void setLoraConfiguration(uint8_t* data,
                                 const rfTypes_configurationLora_t* lora)
{
  EAF_ASSERT((data != NULL) && (lora != NULL));

  data[0] = lora->spreading_factor;
  data[1] = lora->bandwidth_khz;
  data[2] = lora->coding_rate;
  data[3] = lora->low_data_rate_optimize;
  EMF_endian_u16WriteBuffLE(&data[4], &lora->preamble_length);
  data[6] = lora->header_mode;
  data[7] = lora->payload_crc_enabled;
  data[8] = lora->invert_iq;
  EMF_endian_u16WriteBuffLE(&data[9], &lora->synchronization_word);
  EMF_endian_u24WriteBuffLE(&data[11], &lora->operation_timeout);
  data[14] = lora->symbol_timeout;
  data[15] = lora->tx_ramp_time_us;

  data[16] = lora->cad.scan_symbol_count;
  data[17] = lora->cad.detection_peak_threshold;
  data[18] = lora->cad.detection_minimum_threshold;
  data[19] = lora->cad.exit_mode;
  EMF_endian_u24WriteBuffLE(&data[20], &lora->cad.timeout);

  data[23] = lora->rx_mode;
  EMF_endian_u24WriteBuffLE(&data[24], &lora->rx_timeout_control);
  EMF_endian_u24WriteBuffLE(&data[27], &lora->rx_duty_cycle.rx_period);
  EMF_endian_u24WriteBuffLE(&data[30], &lora->rx_duty_cycle.sleep_period);
}

static void getFskConfiguration(const uint8_t* data,
                                rfTypes_configurationFsk_t* fsk)
{
  uint8_t index;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(fsk != NULL);
  EAF_ASSERT_BLOCK_END();

  EMF_endian_u24ReadBuffLE(&data[0], &fsk->bit_rate_bps);
  EMF_endian_u24ReadBuffLE(&data[3], &fsk->frequency_deviation_hz);
  fsk->pulse_shape = data[6];
  fsk->receive_bandwidth_khz = data[7];
  EMF_endian_u16ReadBuffLE(&data[8], &fsk->preamble_length);
  EMF_endian_u24ReadBuffLE(&data[10], &fsk->operation_timeout);
  fsk->payload_length_mode = data[13];
  fsk->preamble_detection_length_bits = data[14];
  fsk->sync_word_length_bits = data[15];
  for (index = 0U; index < RF_TYPES_GENERIC_SYNC_WORD_SIZE; ++index)
  {
    fsk->sync_word[index] = data[16U + index];
  }
  fsk->address_filtering = data[24];
  fsk->node_address = data[25];
  fsk->broadcast_address = data[26];
  fsk->crc_type = data[27];
  EMF_endian_u16ReadBuffLE(&data[28], &fsk->crc_initial_value);
  EMF_endian_u16ReadBuffLE(&data[30], &fsk->crc_polynomial);
  fsk->whitening_enabled = data[32];
  EMF_endian_u16ReadBuffLE(&data[33], &fsk->whitening_initial_value);
  fsk->tx_ramp_time_us = data[35];

  fsk->rx_mode = data[36];
  EMF_endian_u24ReadBuffLE(&data[37], &fsk->rx_timeout_control);
  EMF_endian_u24ReadBuffLE(&data[40], &fsk->rx_duty_cycle.rx_period);
  EMF_endian_u24ReadBuffLE(&data[43], &fsk->rx_duty_cycle.sleep_period);
}

static void setFskConfiguration(uint8_t* data,
                                const rfTypes_configurationFsk_t* fsk)
{
  uint8_t index;

  EAF_ASSERT((data != NULL) && (fsk != NULL));

  EMF_endian_u24WriteBuffLE(&data[0], &fsk->bit_rate_bps);
  EMF_endian_u24WriteBuffLE(&data[3], &fsk->frequency_deviation_hz);
  data[6] = fsk->pulse_shape;
  data[7] = fsk->receive_bandwidth_khz;
  EMF_endian_u16WriteBuffLE(&data[8], &fsk->preamble_length);
  EMF_endian_u24WriteBuffLE(&data[10], &fsk->operation_timeout);
  data[13] = fsk->payload_length_mode;
  data[14] = fsk->preamble_detection_length_bits;
  data[15] = fsk->sync_word_length_bits;
  for (index = 0U; index < RF_TYPES_GENERIC_SYNC_WORD_SIZE; ++index)
  {
    data[16U + index] = fsk->sync_word[index];
  }
  data[24] = fsk->address_filtering;
  data[25] = fsk->node_address;
  data[26] = fsk->broadcast_address;
  data[27] = fsk->crc_type;
  EMF_endian_u16WriteBuffLE(&data[28], &fsk->crc_initial_value);
  EMF_endian_u16WriteBuffLE(&data[30], &fsk->crc_polynomial);
  data[32] = fsk->whitening_enabled;
  EMF_endian_u16WriteBuffLE(&data[33], &fsk->whitening_initial_value);
  data[35] = fsk->tx_ramp_time_us;

  data[36] = fsk->rx_mode;
  EMF_endian_u24WriteBuffLE(&data[37], &fsk->rx_timeout_control);
  EMF_endian_u24WriteBuffLE(&data[40], &fsk->rx_duty_cycle.rx_period);
  EMF_endian_u24WriteBuffLE(&data[43], &fsk->rx_duty_cycle.sleep_period);
}

static void getMskConfiguration(const uint8_t* data,
                                rfTypes_configurationMsk_t* msk)
{
  uint8_t index;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(msk != NULL);
  EAF_ASSERT_BLOCK_END();

  EMF_endian_u24ReadBuffLE(&data[0], &msk->bit_rate_bps);
  msk->pulse_shape = data[3];
  EMF_endian_u16ReadBuffLE(&data[4], &msk->preamble_length);
  EMF_endian_u24ReadBuffLE(&data[6], &msk->operation_timeout);
  msk->payload_length_mode = data[9];
  msk->sync_word_length_bits = data[10];
  for (index = 0U; index < RF_TYPES_GENERIC_SYNC_WORD_SIZE; ++index)
  {
    msk->sync_word[index] = data[11U + index];
  }
  msk->address_filtering = data[19];
  msk->node_address = data[20];
  msk->broadcast_address = data[21];
  msk->crc_type = data[22];
  EMF_endian_u16ReadBuffLE(&data[23], &msk->crc_initial_value);
  EMF_endian_u16ReadBuffLE(&data[25], &msk->crc_polynomial);
  msk->whitening_enabled = data[27];
  EMF_endian_u16ReadBuffLE(&data[28], &msk->whitening_initial_value);
  msk->tx_ramp_time_us = data[30];
}

static void setMskConfiguration(uint8_t* data,
                                const rfTypes_configurationMsk_t* msk)
{
  uint8_t index;

  EAF_ASSERT((data != NULL) && (msk != NULL));

  EMF_endian_u24WriteBuffLE(&data[0], &msk->bit_rate_bps);
  data[3] = msk->pulse_shape;
  EMF_endian_u16WriteBuffLE(&data[4], &msk->preamble_length);
  EMF_endian_u24WriteBuffLE(&data[6], &msk->operation_timeout);
  data[9] = msk->payload_length_mode;
  data[10] = msk->sync_word_length_bits;
  for (index = 0U; index < RF_TYPES_GENERIC_SYNC_WORD_SIZE; ++index)
  {
    data[11U + index] = msk->sync_word[index];
  }
  data[19] = msk->address_filtering;
  data[20] = msk->node_address;
  data[21] = msk->broadcast_address;
  data[22] = msk->crc_type;
  EMF_endian_u16WriteBuffLE(&data[23], &msk->crc_initial_value);
  EMF_endian_u16WriteBuffLE(&data[25], &msk->crc_polynomial);
  data[27] = msk->whitening_enabled;
  EMF_endian_u16WriteBuffLE(&data[28], &msk->whitening_initial_value);
  data[30] = msk->tx_ramp_time_us;
}

static void getBpskConfiguration(const uint8_t* data,
                                 rfTypes_configurationBpsk_t* bpsk)
{
  EAF_ASSERT((data != NULL) && (bpsk != NULL));

  EMF_endian_u24ReadBuffLE(&data[0], &bpsk->bit_rate_bps);
  EMF_endian_u24ReadBuffLE(&data[3], &bpsk->operation_timeout);
}

static void setBpskConfiguration(uint8_t* data,
                                 const rfTypes_configurationBpsk_t* bpsk)
{
  EAF_ASSERT((data != NULL) && (bpsk != NULL));

  EMF_endian_u24WriteBuffLE(&data[0], &bpsk->bit_rate_bps);
  EMF_endian_u24WriteBuffLE(&data[3], &bpsk->operation_timeout);
}

static bool getConfiguration(const uint8_t* data,
                             uint16_t length,
                             rfTypes_configuration_t* configuration)
{
  bool configuration_valid;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(configuration != NULL);
  EAF_ASSERT_BLOCK_END();

  configuration_valid = (length >= CONFIGURATION_COMMON_SIZE) &&
                        (length == getConfigurationSize(data[0]));
  if (configuration_valid)
  {
    configuration->packet_type = data[0];
    EMF_endian_u32ReadBuffLE(&data[1], &configuration->rf_frequency_hz);
    configuration->payload_length_bytes = data[5];
    configuration->tx_output_power_dbm = (int8_t)data[6];

    switch (configuration->packet_type)
    {
      case RF_TYPES_PACKET_TYPE_LORA:
        getLoraConfiguration(&data[CONFIGURATION_COMMON_SIZE],
                             &configuration->packet_type_configuration.lora);
        break;
      case RF_TYPES_PACKET_TYPE_FSK:
        getFskConfiguration(&data[CONFIGURATION_COMMON_SIZE],
                            &configuration->packet_type_configuration.fsk);
        break;
      case RF_TYPES_PACKET_TYPE_MSK:
        getMskConfiguration(&data[CONFIGURATION_COMMON_SIZE],
                            &configuration->packet_type_configuration.msk);
        break;
      case RF_TYPES_PACKET_TYPE_BPSK:
        getBpskConfiguration(&data[CONFIGURATION_COMMON_SIZE],
                             &configuration->packet_type_configuration.bpsk);
        break;
      default:
        EAF_ERROR();
        break;
    }
  }

  return configuration_valid;
}

static void setConfiguration(uint8_t* data,
                             const rfTypes_configuration_t* configuration)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(configuration != NULL);
  EAF_ASSERT_IN_BLOCK(getConfigurationSize(configuration->packet_type) != 0U);
  EAF_ASSERT_BLOCK_END();

  data[0] = configuration->packet_type;
  EMF_endian_u32WriteBuffLE(&data[1], &configuration->rf_frequency_hz);
  data[5] = configuration->payload_length_bytes;
  data[6] = (uint8_t)configuration->tx_output_power_dbm;

  switch (configuration->packet_type)
  {
    case RF_TYPES_PACKET_TYPE_LORA:
      setLoraConfiguration(&data[CONFIGURATION_COMMON_SIZE],
                           &configuration->packet_type_configuration.lora);
      break;
    case RF_TYPES_PACKET_TYPE_FSK:
      setFskConfiguration(&data[CONFIGURATION_COMMON_SIZE],
                          &configuration->packet_type_configuration.fsk);
      break;
    case RF_TYPES_PACKET_TYPE_MSK:
      setMskConfiguration(&data[CONFIGURATION_COMMON_SIZE],
                          &configuration->packet_type_configuration.msk);
      break;
    case RF_TYPES_PACKET_TYPE_BPSK:
      setBpskConfiguration(&data[CONFIGURATION_COMMON_SIZE],
                           &configuration->packet_type_configuration.bpsk);
      break;
    default:
      EAF_ERROR();
      break;
  }
}

static bool publishSetConfigRequest(const uint8_t* payload,
                                    uint16_t payload_length)
{
  rfTypes_configuration_t configuration;
  events_setConfig_t* event;
  bool payload_valid;

  EAF_ASSERT(payload != NULL);

  payload_valid = getConfiguration(payload, payload_length, &configuration) &&
                  isConfigurationValid(&configuration);
  if (payload_valid)
  {
    event = (events_setConfig_t*)EDF_event_initMutable(sizeof(*event),
                                                       EVENTS_SET_CONFIG);
    EAF_ASSERT(event != NULL);

    event->configuration = configuration;
    EDF_activeObject_publish(&event->super);
  }

  return payload_valid;
}

static bool publishCalibrateRequest(const uint8_t* payload,
                                    uint16_t payload_length)
{
  events_calibrate_t* event;
  bool payload_valid;

  EAF_ASSERT(payload != NULL);

  payload_valid = (payload_length == 2U) && (payload[0] >= 0x26U) &&
                  (payload[1] <= 0xF0U) && (payload[0] <= payload[1]);
  if (payload_valid)
  {
    event = (events_calibrate_t*)EDF_event_initMutable(sizeof(*event),
                                                       EVENTS_CALIBRATE);
    EAF_ASSERT(event != NULL);

    event->image_calibration_lower_frequency_hz = payload[0];
    event->image_calibration_upper_frequency_hz = payload[1];
    EDF_activeObject_publish(&event->super);
  }

  return payload_valid;
}

static bool publishTxPacketRequest(const uint8_t* payload,
                                   uint16_t payload_length)
{
  events_txPacket_t* event;
  uint16_t index;
  bool payload_valid;

  EAF_ASSERT(payload != NULL);

  payload_valid = (payload_length <= RF_TYPES_PACKET_DATA_CAPACITY);
  if (payload_valid)
  {
    event = (events_txPacket_t*)EDF_event_initMutable(sizeof(*event),
                                                      EVENTS_TX_PACKET);
    EAF_ASSERT(event != NULL);

    EMF_utils_clear(event->packet_data, sizeof(event->packet_data));
    for (index = 0U; index < payload_length; ++index)
    {
      event->packet_data[index] = payload[index];
    }
    event->payload_length = (uint8_t)payload_length;
    EDF_activeObject_publish(&event->super);
  }

  return payload_valid;
}

static void publishRequest(uint16_t command_id,
                           const uint8_t* payload,
                           uint16_t payload_length)
{
  bool payload_valid;

  EAF_ASSERT(payload != NULL);

  switch (command_id)
  {
    case COMMAND_GET_CONFIG_REQUEST:
      payload_valid = (payload_length == 0U);
      if (payload_valid)
      {
        EDF_activeObject_publish(&getConfigRequestEvent.super);
      }
      break;
    case COMMAND_SET_CONFIG:
      (void)publishSetConfigRequest(payload, payload_length);
      break;
    case COMMAND_CALIBRATE:
      (void)publishCalibrateRequest(payload, payload_length);
      break;
    case COMMAND_TX_PACKET:
      (void)publishTxPacketRequest(payload, payload_length);
      break;
    case COMMAND_RX_START:
      payload_valid = (payload_length == 0U);
      if (payload_valid)
      {
        EDF_activeObject_publish(&rxStartEvent.super);
      }
      break;
    case COMMAND_CAD_START:
      payload_valid = (payload_length == 0U);
      if (payload_valid)
      {
        EDF_activeObject_publish(&cadStartEvent.super);
      }
      break;
    case COMMAND_TX_CONTINUOUS_WAVE:
      payload_valid = (payload_length == 0U);
      if (payload_valid)
      {
        EDF_activeObject_publish(&txContinuousWaveEvent.super);
      }
      break;
    case COMMAND_TX_CONTINUOUS_PREAMBLE:
      payload_valid = (payload_length == 0U);
      if (payload_valid)
      {
        EDF_activeObject_publish(&txContinuousPreambleEvent.super);
      }
      break;
    case COMMAND_STOP:
      payload_valid = (payload_length == 0U);
      if (payload_valid)
      {
        EDF_activeObject_publish(&stopEvent.super);
      }
      break;
    case COMMAND_GET_DIAGNOSTICS_REQUEST:
      payload_valid = (payload_length == 0U);
      if (payload_valid)
      {
        EDF_activeObject_publish(&getDiagnosticsRequestEvent.super);
      }
      break;
    default:
      break;
  }
}

static void processFrame(const uint8_t* encoded, uint16_t encoded_length)
{
  uint8_t decoded[DECODED_FRAME_CAPACITY];
  uint16_t decoded_length;
  uint16_t command_id;
  uint16_t payload_length;
  uint16_t expected_crc;
  uint16_t received_crc;
  bool decoded_ok;

  EAF_ASSERT(encoded != NULL);

  // The assembled input includes the trailing COBS packet delimiter.
  decoded_ok =
    EMF_cobs_decode(encoded, encoded_length, decoded, &decoded_length);
  if (decoded_ok && (decoded_length >= FRAME_OVERHEAD))
  {
    EMF_endian_u16ReadBuffLE(&decoded[0], &command_id);
    EMF_endian_u16ReadBuffLE(&decoded[2], &payload_length);
    if (decoded_length == (uint16_t)(payload_length + FRAME_OVERHEAD))
    {
      EMF_endian_u16ReadBuffLE(&decoded[decoded_length - 2U], &received_crc);
      // The CRC covers the command, payload length, and payload only.
      expected_crc = EMF_crc_16CCITT(decoded, (uint16_t)(decoded_length - 2U));
      if (received_crc == expected_crc)
      {
        publishRequest(command_id, &decoded[4], payload_length);
      }
    }
  }
}

static void sendResponse(uint16_t command_id,
                         const uint8_t* payload,
                         uint16_t payload_length)
{
  uint8_t decoded[DECODED_FRAME_CAPACITY];
  uint8_t encoded[ENCODED_FRAME_CAPACITY];
  uint16_t crc;
  uint16_t decoded_length;
  uint16_t encoded_length;
  uint16_t index;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK((payload != NULL) || (payload_length == 0U));
  EAF_ASSERT_IN_BLOCK(payload_length <=
                      (DECODED_FRAME_CAPACITY - FRAME_OVERHEAD));
  EAF_ASSERT_BLOCK_END();

  EMF_endian_u16WriteBuffLE(&decoded[0], &command_id);
  EMF_endian_u16WriteBuffLE(&decoded[2], &payload_length);
  for (index = 0U; index < payload_length; ++index)
  {
    decoded[4U + index] = payload[index];
  }
  decoded_length = (uint16_t)(payload_length + 4U);

  // The CRC covers the command, payload length, and payload only.
  crc = EMF_crc_16CCITT(decoded, decoded_length);
  EMF_endian_u16WriteBuffLE(&decoded[decoded_length], &crc);
  decoded_length = (uint16_t)(decoded_length + 2U);

  // COBS encoding appends the packet delimiter.
  EMF_cobs_encode(decoded, decoded_length, encoded, &encoded_length);
  EAF_ASSERT(encoded_length <= ENCODED_FRAME_CAPACITY);

  hostCommandManagerHal_write(encoded, encoded_length);
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void hostCommandManagerActivities_handleHostDataAvailable(
  hostCommandManager_t* me,
  const events_hostDataAvailable_t* e)
{
  uint8_t byte;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  while (hostCommandManagerHal_getAvailableBytes() > 0U)
  {
    hostCommandManagerHal_read(&byte, 1U);
    if (me->rx_frame_length < HOST_COMMAND_MANAGER_RX_FRAME_CAPACITY)
    {
      me->rx_frame[me->rx_frame_length++] = byte;
      if (byte == EMF_COBS_PACKET_DELIMITER)
      {
        processFrame(me->rx_frame, me->rx_frame_length);
        me->rx_frame_length = 0U;
      }
    }
    else if (byte == EMF_COBS_PACKET_DELIMITER)
    {
      // Resynchronize after an overlength malformed frame.
      me->rx_frame_length = 0U;
    }
  }
}

void hostCommandManagerActivities_handleGetConfigResponse(
  hostCommandManager_t* me,
  const events_getConfigResponse_t* e)
{
  uint8_t payload[CONFIGURATION_FSK_SIZE];
  uint16_t payload_length;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  payload_length = getConfigurationSize(e->configuration.packet_type);
  setConfiguration(payload, &e->configuration);
  sendResponse(COMMAND_GET_CONFIG_RESPONSE, payload, payload_length);
}

void hostCommandManagerActivities_handleGetDiagnosticsResponse(
  hostCommandManager_t* me,
  const events_getDiagnosticsResponse_t* e)
{
  uint8_t payload[3];

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  payload[0] = (uint8_t)e->active_rf_operation;
  EMF_endian_u16WriteBuffLE(&payload[1], &e->radio_errors);
  sendResponse(COMMAND_GET_DIAGNOSTICS_RESPONSE, payload, sizeof(payload));
}

void hostCommandManagerActivities_handleRxPacket(hostCommandManager_t* me,
                                                 const events_rxPacket_t* e)
{
  uint8_t payload[RF_TYPES_PACKET_DATA_CAPACITY + 3U];
  uint16_t payload_length;
  uint16_t index;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  payload[0] = e->received_packet_length_bytes;
  for (index = 0U; index < e->received_packet_length_bytes; ++index)
  {
    payload[1U + index] = e->packet_data[index];
  }
  payload_length = (uint16_t)e->received_packet_length_bytes + 1U;
  payload[payload_length++] = e->packet_status.rssi;
  payload[payload_length++] = e->packet_status.packet_info;
  sendResponse(COMMAND_RX_PACKET, payload, payload_length);
}

void hostCommandManagerActivities_handleCadDetected(
  hostCommandManager_t* me,
  const events_cadDetected_t* e)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  sendResponse(COMMAND_CAD_DETECTED, NULL, 0U);
}
