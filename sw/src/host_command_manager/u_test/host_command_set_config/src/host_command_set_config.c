/*******************************************************************************
 * @brief Isolated validation of SET_CONFIG structure and semantic translation.
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
#include "host_command_manager.h"
#include "host_command_manager_activities.h"
#include "host_command_manager_hal.h"
#include "rf_types.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/** @brief Bounded allocation/snapshot capacity for SF controls. */
#define TEST_EVENT_CAPACITY 4U

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/** @brief Aligned storage for requests, without imposing allocator identity. */
typedef union
{
  EDF_event_t event;                 //!< Event base view.
  events_setConfig_t configuration;  //!< Complete semantic SET_CONFIG event.
  events_txPacket_t packet;          //!< Largest payload-bearing request.
} eventStorage_t;

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/** @brief LoRa payload from SYS_REQ_INTERFACE_1 and its parameters. */
// clang-format off
static const uint8_t loraPayload[] = {
  // Common: LoRa, 433 MHz encoded, RX length 37, requested power -3 dBm.
  1U, 0U, 0U, 0x10U, 0x1BU, 37U, 0xFDU,
  // SF7, 125 kHz, 4/6, LDRO enabled, preamble 0x1234, implicit, CRC, IQ.
  7U, 4U, 2U, 1U, 0x34U, 0x12U, 1U, 1U, 1U,
  // Public sync, TX timeout 0x123456, symbol timeout 19, 80 us ramp.
  0x44U, 0x34U, 0x56U, 0x34U, 0x12U, 19U, 3U,
  // CAD: 4 symbols, peak 22, minimum 10, receive, timeout 0x234567.
  2U, 22U, 10U, 1U, 0x67U, 0x45U, 0x23U,
  // Single RX, timeout 0x345678, duty-cycle fields retained but inactive.
  0U, 0x78U, 0x56U, 0x34U, 0x89U, 0x67U, 0x45U, 0x9AU, 0x78U, 0x56U};

/** @brief Contract-ordered FSK payload, with unambiguous RX bandwidth margin. */
static const uint8_t fskPayload[] = {
  // Common: FSK, 868 MHz encoded, RX length 73, requested power 14 dBm.
  0U, 0U, 0U, 0x40U, 0x36U, 73U, 14U,
  // 50000 bit/s, deviation 0x006666 (~24999.62 Hz), no shaping, 156.2 kHz.
  0x50U, 0xC3U, 0U, 0x66U, 0x66U, 0U, 0U, 0x1AU,
  // Preamble 0x2345, TX timeout 0x234567, variable, detect16, sync64.
  0x45U, 0x23U, 0x67U, 0x45U, 0x23U, 1U, 5U, 64U,
  // Eight distinct sync bytes, node+broadcast filtering, addresses.
  0x12U, 0x23U, 0x34U, 0x45U, 0x56U, 0x67U, 0x78U, 0x89U,
  2U, 0x35U, 0xCAU,
  // Inverted CRC16, init 0x1D0F, polynomial 0x1021, whitening, seed0x0123.
  6U, 0x0FU, 0x1DU, 0x21U, 0x10U, 1U, 0x23U, 1U,
  // Ramp200us, single RX, timeout0x345678, inactive duty-cycle periods.
  4U, 0U, 0x78U, 0x56U, 0x34U, 0x89U, 0x67U, 0x45U, 0x9AU, 0x78U, 0x56U};
// clang-format on

/** @brief Real RX FIFO exposed through EFF HAL callbacks. */
static EMF_byteFifo_handler_t rxFifo;
/** @brief Input storage bounded by the DD's RX capacity. */
static uint8_t fifoStorage[HOST_COMMAND_MANAGER_RX_FRAME_CAPACITY];
/** @brief Allocation slots independent of publication snapshots. */
static eventStorage_t allocatedEvents[TEST_EVENT_CAPACITY];
/** @brief Next unused allocation slot. */
static uint32_t allocationIndex;
/** @brief Copies of events actually published by the activity. */
static events_setConfig_t publishedConfigurations[TEST_EVENT_CAPACITY];
/** @brief Number of actual publication snapshots. */
static uint32_t publicationCount;

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

/** @brief Query real RX FIFO occupancy. @return Available bytes. */
static uint32_t getAvailableBytes(void);
/**
 * @brief Consume real RX FIFO bytes.
 * @param[out] data Destination buffer.
 * @param[in] length Number of bytes to consume.
 */
static void readBytes(uint8_t* data, uint32_t length);
/**
 * @brief Allocate bounded typed storage initialized from the requested signal.
 * @param[in] event_size Requested bytes.
 * @param[in] signal Requested signal.
 * @return Initialized event, or NULL if fixture capacity is exceeded.
 */
static EDF_event_t* allocateEvent(uint_fast16_t event_size, int signal);
/**
 * @brief Snapshot actual publication independently of allocator storage.
 * @param[in] event Published event.
 */
static void capturePublication(const EDF_event_t* event);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static uint32_t getAvailableBytes(void)
{
  return EMF_byteFifo_getUsed(&rxFifo);
}

static void readBytes(uint8_t* data, uint32_t length)
{
  EMF_byteFifo_pop(&rxFifo, data, length);
}

static EDF_event_t* allocateEvent(uint_fast16_t event_size, int signal)
{
  EDF_event_t* event = NULL;

  if ((event_size <= sizeof(allocatedEvents[0])) &&
      (allocationIndex < TEST_EVENT_CAPACITY))
  {
    allocatedEvents[allocationIndex] = (eventStorage_t){0};
    allocatedEvents[allocationIndex].event = (EDF_event_t){
      .sig = (EDF_event_signal_t)signal, .pool_num = 1U, .ref_cnt = 0U};
    event = &allocatedEvents[allocationIndex].event;
    ++allocationIndex;
  }

  return event;
}

static void capturePublication(const EDF_event_t* event)
{
  ETF_VERIFY(event != NULL);
  ETF_VERIFY(publicationCount < TEST_EVENT_CAPACITY);

  publishedConfigurations[publicationCount].super = *event;
  if (event->sig == EVENTS_SET_CONFIG)
  {
    publishedConfigurations[publicationCount] =
      *(const events_setConfig_t*)event;
  }
  ++publicationCount;
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/** @brief EFF replacement for available-byte query. */
FAKE_VALUE_FUNC(uint32_t, hostCommandManagerHal_getAvailableBytes);
/** @brief EFF replacement for consuming RX bytes. */
FAKE_VOID_FUNC(hostCommandManagerHal_read, uint8_t*, uint32_t);
/** @brief Observer for forbidden host output. */
FAKE_VOID_FUNC(hostCommandManagerHal_write, const uint8_t*, uint32_t);
/**
 * @brief EFF allocator using test-owned storage.
 * EFF copies pointer values into return history, not the pointed-to event;
 * sizeof(pointer) in the generated memcpy is intentional.
 */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VALUE_FUNC(EDF_event_t*, EDF_event_initMutable, uint_fast16_t, int);
/**
 * @brief EFF publisher with independent semantic snapshots.
 * EFF copies pointer values into argument history, not the pointed-to event;
 * sizeof(pointer) in the generated memcpy is intentional.
 */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(EDF_activeObject_publish, const EDF_event_t*);

/** @brief Turn unexpected production assertions into test failures. */
void EAF_onError(const char* file, int line, int id)
{
  EMF_UTILS_UNUSED_PARAM(id);
  ETF_fail("Unexpected production assertion", file, line);
}

/** @brief Reset all fixture state directly before every ETF case. */
void ETF_setUp(void)
{
  size_t slot_index;
  size_t byte_index;

  RESET_FAKE(hostCommandManagerHal_getAvailableBytes);
  RESET_FAKE(hostCommandManagerHal_read);
  RESET_FAKE(hostCommandManagerHal_write);
  RESET_FAKE(EDF_event_initMutable);
  RESET_FAKE(EDF_activeObject_publish);
  eff = (eff_globals_t){0};
  for (byte_index = 0U; byte_index < sizeof(fifoStorage); ++byte_index)
  {
    fifoStorage[byte_index] = 0U;
  }
  EMF_byteFifo_init(&rxFifo, sizeof(fifoStorage), fifoStorage);
  for (slot_index = 0U; slot_index < TEST_EVENT_CAPACITY; ++slot_index)
  {
    allocatedEvents[slot_index] = (eventStorage_t){0};
    publishedConfigurations[slot_index] = (events_setConfig_t){0};
  }
  allocationIndex = 0U;
  publicationCount = 0U;
  hostCommandManagerHal_getAvailableBytes_fake.custom_fake = getAvailableBytes;
  hostCommandManagerHal_read_fake.custom_fake = readBytes;
  EDF_event_initMutable_fake.custom_fake = allocateEvent;
  EDF_activeObject_publish_fake.custom_fake = capturePublication;
}

/** @brief Validate SET_CONFIG through the isolated reception activity. */
ETF_TEST_SUITE(host_command_set_config)
{
  ETF_TEST(validLoraPublishesCompleteConfiguration)
  {
    hostCommandManager_t manager = {0};
    events_hostDataAvailable_t data_available = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};
    uint8_t decoded[sizeof(loraPayload) + 6U] = {1U, 0U};
    uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(decoded))];
    uint16_t payload_length = sizeof(loraPayload);
    uint16_t crc;
    uint16_t encoded_length;
    rfTypes_configuration_t* configuration;
    rfTypes_configurationLora_t* lora;
    size_t byte_index;

    // Serialize only transport overhead; payload order/values are normative.
    EMF_endian_u16WriteBuffLE(&decoded[2], &payload_length);
    for (byte_index = 0U; byte_index < sizeof(loraPayload); ++byte_index)
    {
      decoded[4U + byte_index] = loraPayload[byte_index];
    }
    crc = EMF_crc_16CCITT(decoded, sizeof(decoded) - 2U);
    EMF_endian_u16WriteBuffLE(&decoded[sizeof(decoded) - 2U], &crc);
    EMF_cobs_encode(decoded, sizeof(decoded), encoded, &encoded_length);
    EMF_byteFifo_push(&rxFifo, encoded, encoded_length);
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(publicationCount == 1U);
    ETF_VERIFY(publishedConfigurations[0].super.sig == EVENTS_SET_CONFIG);
    configuration = &publishedConfigurations[0].configuration;
    ETF_VERIFY(configuration->packet_type == RF_TYPES_PACKET_TYPE_LORA);
    ETF_VERIFY(configuration->rf_frequency_hz == 0x1B100000UL);
    ETF_VERIFY(configuration->payload_length_bytes == 37U);
    ETF_VERIFY(configuration->tx_output_power_dbm == -3);

    // Check every semantic member, not padding or inactive union arms.
    lora = &configuration->packet_type_configuration.lora;
    ETF_VERIFY(lora->spreading_factor == 7U);
    ETF_VERIFY(lora->bandwidth_khz == 4U);
    ETF_VERIFY(lora->coding_rate == 2U);
    ETF_VERIFY(lora->low_data_rate_optimize == 1U);
    ETF_VERIFY(lora->preamble_length == 0x1234U);
    ETF_VERIFY(lora->header_mode == 1U);
    ETF_VERIFY(lora->payload_crc_enabled == 1U);
    ETF_VERIFY(lora->invert_iq == 1U);
    ETF_VERIFY(lora->synchronization_word == 0x3444U);
    ETF_VERIFY(lora->operation_timeout == 0x123456UL);
    ETF_VERIFY(lora->symbol_timeout == 19U);
    ETF_VERIFY(lora->tx_ramp_time_us == 3U);
    ETF_VERIFY(lora->cad.scan_symbol_count == 2U);
    ETF_VERIFY(lora->cad.detection_peak_threshold == 22U);
    ETF_VERIFY(lora->cad.detection_minimum_threshold == 10U);
    ETF_VERIFY(lora->cad.exit_mode == 1U);
    ETF_VERIFY(lora->cad.timeout == 0x234567UL);
    ETF_VERIFY(lora->rx_mode == 0U);
    ETF_VERIFY(lora->rx_timeout_control == 0x345678UL);
    ETF_VERIFY(lora->rx_duty_cycle.rx_period == 0x456789UL);
    ETF_VERIFY(lora->rx_duty_cycle.sleep_period == 0x56789AUL);
    ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
  }

  ETF_TEST(validFskPublishesCompleteConfiguration)
  {
    hostCommandManager_t manager = {0};
    events_hostDataAvailable_t data_available = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};
    uint8_t decoded[sizeof(fskPayload) + 6U] = {1U, 0U};
    uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(decoded))];
    uint8_t expected_sync[] = {
      0x12U, 0x23U, 0x34U, 0x45U, 0x56U, 0x67U, 0x78U, 0x89U};
    uint16_t payload_length = sizeof(fskPayload);
    uint16_t crc;
    uint16_t encoded_length;
    rfTypes_configuration_t* configuration;
    rfTypes_configurationFsk_t* fsk;
    size_t byte_index;

    EMF_endian_u16WriteBuffLE(&decoded[2], &payload_length);
    for (byte_index = 0U; byte_index < sizeof(fskPayload); ++byte_index)
    {
      decoded[4U + byte_index] = fskPayload[byte_index];
    }
    crc = EMF_crc_16CCITT(decoded, sizeof(decoded) - 2U);
    EMF_endian_u16WriteBuffLE(&decoded[sizeof(decoded) - 2U], &crc);
    EMF_cobs_encode(decoded, sizeof(decoded), encoded, &encoded_length);
    EMF_byteFifo_push(&rxFifo, encoded, encoded_length);
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(publicationCount == 1U);
    ETF_VERIFY(publishedConfigurations[0].super.sig == EVENTS_SET_CONFIG);
    configuration = &publishedConfigurations[0].configuration;
    ETF_VERIFY(configuration->packet_type == RF_TYPES_PACKET_TYPE_FSK);
    ETF_VERIFY(configuration->rf_frequency_hz == 0x36400000UL);
    ETF_VERIFY(configuration->payload_length_bytes == 73U);
    ETF_VERIFY(configuration->tx_output_power_dbm == 14);

    fsk = &configuration->packet_type_configuration.fsk;
    ETF_VERIFY(fsk->bit_rate_bps == 50000UL);
    ETF_VERIFY(fsk->frequency_deviation_hz == 0x006666UL);
    ETF_VERIFY(fsk->pulse_shape == 0U);
    ETF_VERIFY(fsk->receive_bandwidth_khz == 0x1AU);
    ETF_VERIFY(fsk->preamble_length == 0x2345U);
    ETF_VERIFY(fsk->operation_timeout == 0x234567UL);
    ETF_VERIFY(fsk->payload_length_mode == 1U);
    ETF_VERIFY(fsk->preamble_detection_length_bits == 5U);
    ETF_VERIFY(fsk->sync_word_length_bits == 64U);
    ETF_VERIFY(memcmp(fsk->sync_word, expected_sync, sizeof(expected_sync)) ==
               0);
    ETF_VERIFY(fsk->address_filtering == 2U);
    ETF_VERIFY(fsk->node_address == 0x35U);
    ETF_VERIFY(fsk->broadcast_address == 0xCAU);
    ETF_VERIFY(fsk->crc_type == 6U);
    ETF_VERIFY(fsk->crc_initial_value == 0x1D0FU);
    ETF_VERIFY(fsk->crc_polynomial == 0x1021U);
    ETF_VERIFY(fsk->whitening_enabled == 1U);
    ETF_VERIFY(fsk->whitening_initial_value == 0x0123U);
    ETF_VERIFY(fsk->tx_ramp_time_us == 4U);
    ETF_VERIFY(fsk->rx_mode == 0U);
    ETF_VERIFY(fsk->rx_timeout_control == 0x345678UL);
    ETF_VERIFY(fsk->rx_duty_cycle.rx_period == 0x456789UL);
    ETF_VERIFY(fsk->rx_duty_cycle.sleep_period == 0x56789AUL);
    ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
  }

  ETF_TEST(reservedPacketTypeIsDiscarded)
  {
    hostCommandManager_t manager = {0};
    events_hostDataAvailable_t data_available = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};
    uint8_t decoded[sizeof(loraPayload) + 6U] = {1U, 0U};
    uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(decoded))];
    uint16_t payload_length = sizeof(loraPayload);
    uint16_t crc;
    uint16_t encoded_length;
    size_t byte_index;

    EMF_endian_u16WriteBuffLE(&decoded[2], &payload_length);
    for (byte_index = 0U; byte_index < sizeof(loraPayload); ++byte_index)
    {
      decoded[4U + byte_index] = loraPayload[byte_index];
    }
    // Change only packet type; 0x04 has no defined packet-type block.
    decoded[4] = 4U;
    crc = EMF_crc_16CCITT(decoded, sizeof(decoded) - 2U);
    EMF_endian_u16WriteBuffLE(&decoded[sizeof(decoded) - 2U], &crc);
    EMF_cobs_encode(decoded, sizeof(decoded), encoded, &encoded_length);
    EMF_byteFifo_push(&rxFifo, encoded, encoded_length);
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(publicationCount == 0U);
    ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
  }

  ETF_TEST(truncatedLoraBlockIsDiscarded)
  {
    hostCommandManager_t manager = {0};
    events_hostDataAvailable_t data_available = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};
    uint8_t decoded[sizeof(loraPayload) - 1U + 6U] = {1U, 0U};
    uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(decoded))];
    uint16_t payload_length = sizeof(loraPayload) - 1U;
    uint16_t crc;
    uint16_t encoded_length;
    size_t byte_index;

    // Remove one selected-block byte, keeping external length and CRC correct.
    EMF_endian_u16WriteBuffLE(&decoded[2], &payload_length);
    for (byte_index = 0U; byte_index < payload_length; ++byte_index)
    {
      decoded[4U + byte_index] = loraPayload[byte_index];
    }
    crc = EMF_crc_16CCITT(decoded, sizeof(decoded) - 2U);
    EMF_endian_u16WriteBuffLE(&decoded[sizeof(decoded) - 2U], &crc);
    EMF_cobs_encode(decoded, sizeof(decoded), encoded, &encoded_length);
    EMF_byteFifo_push(&rxFifo, encoded, encoded_length);
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(publicationCount == 0U);
    ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
  }

  ETF_TEST(extraFskBlockByteIsDiscarded)
  {
    hostCommandManager_t manager = {0};
    events_hostDataAvailable_t data_available = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};
    uint8_t decoded[sizeof(fskPayload) + 1U + 6U] = {1U, 0U};
    uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(decoded))];
    uint16_t payload_length = sizeof(fskPayload) + 1U;
    uint16_t crc;
    uint16_t encoded_length;
    size_t byte_index;

    // Append one selected-block byte, keeping external length and CRC correct.
    EMF_endian_u16WriteBuffLE(&decoded[2], &payload_length);
    for (byte_index = 0U; byte_index < sizeof(fskPayload); ++byte_index)
    {
      decoded[4U + byte_index] = fskPayload[byte_index];
    }
    decoded[4U + sizeof(fskPayload)] = 0xA5U;
    crc = EMF_crc_16CCITT(decoded, sizeof(decoded) - 2U);
    EMF_endian_u16WriteBuffLE(&decoded[sizeof(decoded) - 2U], &crc);
    EMF_cobs_encode(decoded, sizeof(decoded), encoded, &encoded_length);
    EMF_byteFifo_push(&rxFifo, encoded, encoded_length);
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(publicationCount == 0U);
    ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
  }

  ETF_TEST(sf5AndSf6AcceptTwelveSymbolPreamble)
  {
    uint8_t spreading_factor;

    // Two independent managers, not recovery or an ordering requirement.
    for (spreading_factor = 5U; spreading_factor <= 6U; ++spreading_factor)
    {
      hostCommandManager_t manager = {0};
      events_hostDataAvailable_t data_available = {
        .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};
      uint8_t decoded[sizeof(loraPayload) + 6U] = {1U, 0U};
      uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(decoded))];
      uint16_t payload_length = sizeof(loraPayload);
      uint16_t preamble_length = 12U;
      uint16_t crc;
      uint16_t encoded_length;
      uint32_t previous_count = publicationCount;
      size_t byte_index;

      EMF_endian_u16WriteBuffLE(&decoded[2], &payload_length);
      for (byte_index = 0U; byte_index < sizeof(loraPayload); ++byte_index)
      {
        decoded[4U + byte_index] = loraPayload[byte_index];
      }
      decoded[11] = spreading_factor;
      EMF_endian_u16WriteBuffLE(&decoded[15], &preamble_length);
      crc = EMF_crc_16CCITT(decoded, sizeof(decoded) - 2U);
      EMF_endian_u16WriteBuffLE(&decoded[sizeof(decoded) - 2U], &crc);
      EMF_cobs_encode(decoded, sizeof(decoded), encoded, &encoded_length);
      EMF_byteFifo_push(&rxFifo, encoded, encoded_length);
      hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                           &data_available);

      ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
      ETF_VERIFY(publicationCount == previous_count + 1U);
      ETF_VERIFY(publishedConfigurations[previous_count].super.sig ==
                 EVENTS_SET_CONFIG);
      ETF_VERIFY(
        publishedConfigurations[previous_count].configuration.packet_type ==
        RF_TYPES_PACKET_TYPE_LORA);
      ETF_VERIFY(
        publishedConfigurations[previous_count]
          .configuration.packet_type_configuration.lora.spreading_factor ==
        spreading_factor);
      ETF_VERIFY(
        publishedConfigurations[previous_count]
          .configuration.packet_type_configuration.lora.preamble_length == 12U);
      ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
    }
  }

  ETF_TEST(sf5AndSf6RejectElevenSymbolPreamble)
  {
    uint8_t spreading_factor;

    for (spreading_factor = 5U; spreading_factor <= 6U; ++spreading_factor)
    {
      hostCommandManager_t manager = {0};
      events_hostDataAvailable_t data_available = {
        .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};
      uint8_t decoded[sizeof(loraPayload) + 6U] = {1U, 0U};
      uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(decoded))];
      uint16_t payload_length = sizeof(loraPayload);
      uint16_t preamble_length = 11U;
      uint16_t crc;
      uint16_t encoded_length;
      size_t byte_index;

      // Relative to each SF control, only preamble changes from 12 to 11.
      EMF_endian_u16WriteBuffLE(&decoded[2], &payload_length);
      for (byte_index = 0U; byte_index < sizeof(loraPayload); ++byte_index)
      {
        decoded[4U + byte_index] = loraPayload[byte_index];
      }
      decoded[11] = spreading_factor;
      EMF_endian_u16WriteBuffLE(&decoded[15], &preamble_length);
      crc = EMF_crc_16CCITT(decoded, sizeof(decoded) - 2U);
      EMF_endian_u16WriteBuffLE(&decoded[sizeof(decoded) - 2U], &crc);
      EMF_cobs_encode(decoded, sizeof(decoded), encoded, &encoded_length);
      EMF_byteFifo_push(&rxFifo, encoded, encoded_length);
      hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                           &data_available);

      ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
      ETF_VERIFY(publicationCount == 0U);
      ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
    }
  }

  ETF_TEST(fskRejectsInsufficientReceiveBandwidth)
  {
    hostCommandManager_t manager = {0};
    events_hostDataAvailable_t data_available = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};
    uint8_t decoded[sizeof(fskPayload) + 6U] = {1U, 0U};
    uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(decoded))];
    uint16_t payload_length = sizeof(fskPayload);
    uint16_t crc;
    uint16_t encoded_length;
    size_t byte_index;

    EMF_endian_u16WriteBuffLE(&decoded[2], &payload_length);
    for (byte_index = 0U; byte_index < sizeof(fskPayload); ++byte_index)
    {
      decoded[4U + byte_index] = fskPayload[byte_index];
    }
    // Only BW changes: 93.8 kHz < BR + 2D (~100 kHz), even with zero error.
    decoded[18] = 0x13U;
    crc = EMF_crc_16CCITT(decoded, sizeof(decoded) - 2U);
    EMF_endian_u16WriteBuffLE(&decoded[sizeof(decoded) - 2U], &crc);
    EMF_cobs_encode(decoded, sizeof(decoded), encoded, &encoded_length);
    EMF_byteFifo_push(&rxFifo, encoded, encoded_length);
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(publicationCount == 0U);
    ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
  }

  ETF_TEST(fskRejectsIncompatibleBitRate)
  {
    hostCommandManager_t manager = {0};
    events_hostDataAvailable_t data_available = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};
    uint8_t decoded[sizeof(fskPayload) + 6U] = {1U, 0U};
    uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(decoded))];
    uint16_t payload_length = sizeof(fskPayload);
    uint32_t bit_rate = 150000UL;
    uint16_t crc;
    uint16_t encoded_length;
    size_t byte_index;

    EMF_endian_u16WriteBuffLE(&decoded[2], &payload_length);
    for (byte_index = 0U; byte_index < sizeof(fskPayload); ++byte_index)
    {
      decoded[4U + byte_index] = fskPayload[byte_index];
    }
    // Only BR changes: occupied bandwidth ~200 kHz exceeds 156.2 kHz.
    EMF_endian_u24WriteBuffLE(&decoded[11], &bit_rate);
    crc = EMF_crc_16CCITT(decoded, sizeof(decoded) - 2U);
    EMF_endian_u16WriteBuffLE(&decoded[sizeof(decoded) - 2U], &crc);
    EMF_cobs_encode(decoded, sizeof(decoded), encoded, &encoded_length);
    EMF_byteFifo_push(&rxFifo, encoded, encoded_length);
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(publicationCount == 0U);
    ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
  }

  ETF_TEST(fskRejectsIncompatibleDeviation)
  {
    hostCommandManager_t manager = {0};
    events_hostDataAvailable_t data_available = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};
    uint8_t decoded[sizeof(fskPayload) + 6U] = {1U, 0U};
    uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(decoded))];
    uint16_t payload_length = sizeof(fskPayload);
    uint32_t deviation = 0x01999AUL;
    uint16_t crc;
    uint16_t encoded_length;
    size_t byte_index;

    EMF_endian_u16WriteBuffLE(&decoded[2], &payload_length);
    for (byte_index = 0U; byte_index < sizeof(fskPayload); ++byte_index)
    {
      decoded[4U + byte_index] = fskPayload[byte_index];
    }
    // Only D changes to ~100 kHz: BR + 2D ~250 kHz exceeds 156.2 kHz.
    EMF_endian_u24WriteBuffLE(&decoded[14], &deviation);
    crc = EMF_crc_16CCITT(decoded, sizeof(decoded) - 2U);
    EMF_endian_u16WriteBuffLE(&decoded[sizeof(decoded) - 2U], &crc);
    EMF_cobs_encode(decoded, sizeof(decoded), encoded, &encoded_length);
    EMF_byteFifo_push(&rxFifo, encoded, encoded_length);
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(publicationCount == 0U);
    ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
  }
}
