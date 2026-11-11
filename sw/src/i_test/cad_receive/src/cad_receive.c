/*******************************************************************************
 * @brief CAD receive integration test (SW_INTEGRATION_TEST_2).
 *
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
#include "edf_test.h"
#include "eff.h"
#include "emf.h"

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "events.h"
#include "host_command_manager.h"
#include "host_command_manager_hal.h"
#include "rf_manager.h"
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

/** @brief Real host AO with production priority. */
static hostCommandManager_t hostManager;
/** @brief Real RF AO and HSM. */
static rfManager_t rfManager;
/** @brief Production signal subscription table. */
static EDF_activeObject_bitmask_t subscribers[EVENTS_LAST_SIGNAL];
/** @brief Two largest-event slots cover overlap with the previous dispatch. */
static EMF_UTILS_MEM_ALIGNED_SLOT(sizeof(events_rxPacket_t)) eventPool[2U];
/** @brief Encoded configuration-sized host input. */
static uint8_t input[EMF_COBS_ENCODED_SIZE(46U)];
/** @brief Input byte count. */
static uint16_t inputLength;
/** @brief Next unread input offset. */
static uint16_t inputOffset;
/** @brief Latest decoded host report in the current phase. */
static uint8_t report[64U];
/** @brief Decoded report length; zero when no output was captured. */
static uint16_t reportLength;
/** @brief Host byte-stream capture, independent of HAL write boundaries. */
static uint8_t encodedReport[sizeof(report)];
/** @brief Bytes accumulated in the current host frame. */
static uint16_t reportOffset;
/** @brief Known packet distinct from the configured RX length. */
static const uint8_t packet[] = {0x00U, 0xA5U, 0xFFU, 0x31U, 0x00U};
/** @brief Raw RSSI and signed quarter-dB SNR. */
static const rfTypes_packetStatus_t packetStatus = {.rssi = 182U,
                                                    .packet_info = 0xF4U};
/** @brief Complete LoRa configuration payload for receive-on-detection CAD. */
static const uint8_t cadReceiveConfigurationPayload[40U] = {
  0x01U, 0x00U, 0x00U, 0x40U, 0x36U, 32U, 14U,   7U,    4U,    1U,
  0U,    8U,    0U,    0U,    1U,    0U,  0x24U, 0x14U, 0x34U, 0x12U,
  0U,    8U,    4U,    2U,    22U,   10U, 1U,    0x56U, 0x34U, 0U,
  0U,    0x45U, 0x23U, 0x01U, 0U,    0U,  0U,    0U,    0U,    0U};
/** @brief Only fabricated stimulus event: bytes arrived at the host HAL. */
static const events_hostDataAvailable_t hostDataAvailable = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/** @brief EFF globals required by the HAL doubles. */
DEFINE_EFF_GLOBALS;
/** @brief CAD persistence across a clear scan and a received packet. */
EDF_TEST_SUITE_NAME(cad_receive)
/** @brief Three-event host burst, with one spare slot as in RX. */
EDF_TEST_EVENT_BUFF(4U)

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/**
 * @brief Verify semantic configuration without padding comparisons.
 * @param[in] configuration Actual event/HAL configuration.
 */
static void verifyConfiguration(const rfTypes_configuration_t* configuration);
/**
 * @brief Supply a CRC/COBS command through the real host parser.
 * @param[in] command Public protocol command ID.
 * @param[in] payload Payload or NULL for an empty request.
 * @param[in] length Payload size.
 */
static void sendCommand(hostCommandManager_commandId_t command,
                        const uint8_t* payload,
                        uint16_t length);
/**
 * @brief Verify framing and return the decoded payload.
 * @param[in] command Expected command.
 * @param[in] length Expected payload size.
 * @return Read-only payload.
 */
static const uint8_t* verifyReport(hostCommandManager_commandId_t command,
                                   uint16_t length);
/**
 * @brief Return unread input size.
 * @return Available byte count.
 */
static uint32_t getAvailableBytes(void);
/**
 * @brief Consume bytes requested by the real parser.
 * @param[out] data Destination.
 * @param[in] length Requested size.
 */
static void readBytes(uint8_t* data, uint32_t length);
/**
 * @brief Decode actual transient host output.
 * @param[in] data Encoded report.
 * @param[in] length Encoded size.
 */
static void captureReport(const uint8_t* data, uint32_t length);
/**
 * @brief Supply packet and status at the radio boundary.
 * @param[out] data Destination.
 * @param[in] capacity Destination size.
 * @param[in] configuration Configuration retained by EFF.
 * @param[out] packet_status Metadata destination.
 * @return Received size.
 */
static uint8_t readReceivedPacket(uint8_t* data,
                                  uint8_t capacity,
                                  const rfTypes_configuration_t* configuration,
                                  rfTypes_packetStatus_t* packet_status);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

/** @brief Observe host initialization. */
FAKE_VOID_FUNC(hostCommandManagerHal_init);
/** @brief Supply input availability. */
FAKE_VALUE_FUNC(uint32_t, hostCommandManagerHal_getAvailableBytes);
/** @brief Supply input bytes. */
FAKE_VOID_FUNC(hostCommandManagerHal_read, uint8_t*, uint32_t);
/** @brief Capture transient reports. */
FAKE_VOID_FUNC(hostCommandManagerHal_write, const uint8_t*, uint32_t);
/** @brief Retain callback/context pointer values; EFF sizeof(pointer) is
 * intentional. */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_init, const rfManagerHal_callbacks_t*, void*);
/** @brief Observe configuration pointer; EFF sizeof(pointer) is intentional. */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_setConfiguration, const rfTypes_configuration_t*);
/** @brief Stub RX start; EFF sizeof(pointer) is intentional. */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_startReceive, const rfTypes_configuration_t*);
/** @brief Observe stops. */
FAKE_VOID_FUNC(rfManagerHal_stop);
/** @brief Supply packet/status; EFF copies pointers, so sizeof(pointer) is
 * intentional. */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VALUE_FUNC(uint8_t,
                rfManagerHal_readReceivedPacket,
                uint8_t*,
                uint8_t,
                const rfTypes_configuration_t*,
                rfTypes_packetStatus_t*);
/** @brief Healthy diagnostics use the default zero result. */
FAKE_VALUE_FUNC(uint16_t, rfManagerHal_getErrors);
/** @brief Stub calibration. */
FAKE_VOID_FUNC(rfManagerHal_calibrate, uint8_t, uint8_t);
/** @brief Stub TX; EFF sizeof(pointer) is intentional. */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_transmit,
               const rfTypes_configuration_t*,
               const uint8_t*,
               uint8_t);
/** @brief Retain persistent CAD pointer; EFF sizeof(pointer) is intentional. */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_startCad, const rfTypes_configuration_t*);
/** @brief Stub continuous-wave TX. */
FAKE_VOID_FUNC(rfManagerHal_startContinuousWave);
/** @brief Stub preamble TX. */
FAKE_VOID_FUNC(rfManagerHal_startContinuousPreamble);

static void verifyConfiguration(const rfTypes_configuration_t* configuration)
{
  const rfTypes_configurationLora_t* lora;

  EDF_TEST_VERIFY(configuration != NULL);
  // EDFTest_fail never returns, but its declaration lacks noreturn.
  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  EDF_TEST_VERIFY(configuration->packet_type == RF_TYPES_PACKET_TYPE_LORA);
  EDF_TEST_VERIFY(configuration->rf_frequency_hz == 0x36400000UL);
  EDF_TEST_VERIFY(configuration->payload_length_bytes == 32U);
  EDF_TEST_VERIFY(configuration->tx_output_power_dbm == 14);
  lora = &configuration->packet_type_configuration.lora;
  EDF_TEST_VERIFY(lora->spreading_factor == 7U);
  EDF_TEST_VERIFY(lora->bandwidth_khz == 4U);
  EDF_TEST_VERIFY(lora->coding_rate == 1U);
  EDF_TEST_VERIFY(lora->low_data_rate_optimize == 0U);
  EDF_TEST_VERIFY(lora->preamble_length == 8U);
  EDF_TEST_VERIFY(lora->header_mode == 0U);
  EDF_TEST_VERIFY(lora->payload_crc_enabled == 1U);
  EDF_TEST_VERIFY(lora->invert_iq == 0U);
  EDF_TEST_VERIFY(lora->synchronization_word == 0x1424U);
  EDF_TEST_VERIFY(lora->operation_timeout == 0x1234UL);
  EDF_TEST_VERIFY(lora->symbol_timeout == 8U);
  EDF_TEST_VERIFY(lora->tx_ramp_time_us == 4U);
  EDF_TEST_VERIFY(lora->cad.scan_symbol_count == 2U);
  EDF_TEST_VERIFY(lora->cad.detection_peak_threshold == 22U);
  EDF_TEST_VERIFY(lora->cad.detection_minimum_threshold == 10U);
  EDF_TEST_VERIFY(lora->cad.exit_mode == 1U);
  EDF_TEST_VERIFY(lora->cad.timeout == 0x3456UL);
  EDF_TEST_VERIFY(lora->rx_mode == 0U);
  EDF_TEST_VERIFY(lora->rx_timeout_control == 0x012345UL);
  EDF_TEST_VERIFY(lora->rx_duty_cycle.rx_period == 0UL);
  EDF_TEST_VERIFY(lora->rx_duty_cycle.sleep_period == 0UL);
}

static void sendCommand(hostCommandManager_commandId_t command,
                        const uint8_t* payload,
                        uint16_t length)
{
  uint8_t decoded[46U];
  uint16_t u16_command_id;
  uint16_t crc;
  uint16_t index;

  EDF_TEST_VERIFY(inputOffset == inputLength);
  EDF_TEST_VERIFY(length <= sizeof(cadReceiveConfigurationPayload));
  EDF_TEST_VERIFY((payload != NULL) || (length == 0U));
  u16_command_id = (uint16_t)command;
  EMF_endian_u16WriteBuffLE(&decoded[0], &u16_command_id);
  EMF_endian_u16WriteBuffLE(&decoded[2], &length);
  for (index = 0U; index < length; ++index)
  {
    decoded[4U + index] = payload[index];
  }
  crc = EMF_crc_16CCITT(decoded, (uint16_t)(length + 4U));
  EMF_endian_u16WriteBuffLE(&decoded[length + 4U], &crc);
  EMF_cobs_encode(decoded, (uint16_t)(length + 6U), input, &inputLength);
  inputOffset = 0U;
  EDF_activeObject_publish(&hostDataAvailable.super);
}

static const uint8_t* verifyReport(hostCommandManager_commandId_t command,
                                   uint16_t length)
{
  uint16_t actual_command;
  uint16_t actual_length;
  uint16_t actual_crc;

  EDF_TEST_VERIFY(reportLength == (length + 6U));
  EMF_endian_u16ReadBuffLE(&report[0], &actual_command);
  EMF_endian_u16ReadBuffLE(&report[2], &actual_length);
  EMF_endian_u16ReadBuffLE(&report[length + 4U], &actual_crc);
  EDF_TEST_VERIFY(actual_command == (uint16_t)command);
  EDF_TEST_VERIFY(actual_length == length);
  EDF_TEST_VERIFY(actual_crc ==
                  EMF_crc_16CCITT(report, (uint16_t)(length + 4U)));
  return &report[4];
}

static uint32_t getAvailableBytes(void)
{
  return (uint32_t)inputLength - inputOffset;
}

static void readBytes(uint8_t* data, uint32_t length)
{
  uint32_t index;

  EDF_TEST_VERIFY(data != NULL);
  EDF_TEST_VERIFY(length <= getAvailableBytes());
  for (index = 0U; index < length; ++index)
  {
    data[index] = input[inputOffset + index];
  }
  inputOffset = (uint16_t)(inputOffset + length);
}

static void captureReport(const uint8_t* data, uint32_t length)
{
  uint32_t index;

  EDF_TEST_VERIFY(data != NULL);
  for (index = 0U; index < length; ++index)
  {
    EDF_TEST_VERIFY(reportOffset < sizeof(encodedReport));
    encodedReport[reportOffset++] = data[index];
    if (data[index] == EMF_COBS_PACKET_DELIMITER)
    {
      EDF_TEST_VERIFY(
        EMF_cobs_decode(encodedReport, reportOffset, report, &reportLength));
      reportOffset = 0U;
    }
  }
}

static uint8_t readReceivedPacket(uint8_t* data,
                                  uint8_t capacity,
                                  const rfTypes_configuration_t* configuration,
                                  rfTypes_packetStatus_t* packet_status)
{
  size_t index;

  EDF_TEST_VERIFY(data != NULL);
  EDF_TEST_VERIFY(packet_status != NULL);
  EDF_TEST_VERIFY(capacity >= sizeof(packet));
  EMF_UTILS_UNUSED_PARAM(configuration);
  for (index = 0U; index < sizeof(packet); ++index)
  {
    data[index] = packet[index];
  }
  *packet_status = packetStatus;
  return sizeof(packet);
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void EAF_onError(const char* file, int line, int id)
{
  EMF_UTILS_UNUSED_PARAM(id);
  EDFTest_fail("Unexpected production assertion", file, line);
}

void EDFTest_onInit(void)
{
  RESET_FAKE(hostCommandManagerHal_init);
  RESET_FAKE(hostCommandManagerHal_getAvailableBytes);
  RESET_FAKE(hostCommandManagerHal_read);
  RESET_FAKE(hostCommandManagerHal_write);
  RESET_FAKE(rfManagerHal_init);
  RESET_FAKE(rfManagerHal_setConfiguration);
  RESET_FAKE(rfManagerHal_startReceive);
  RESET_FAKE(rfManagerHal_stop);
  RESET_FAKE(rfManagerHal_readReceivedPacket);
  RESET_FAKE(rfManagerHal_getErrors);
  RESET_FAKE(rfManagerHal_calibrate);
  RESET_FAKE(rfManagerHal_transmit);
  RESET_FAKE(rfManagerHal_startCad);
  RESET_FAKE(rfManagerHal_startContinuousWave);
  RESET_FAKE(rfManagerHal_startContinuousPreamble);
  eff = (eff_globals_t){0};
  hostCommandManagerHal_getAvailableBytes_fake.custom_fake = getAvailableBytes;
  hostCommandManagerHal_read_fake.custom_fake = readBytes;
  hostCommandManagerHal_write_fake.custom_fake = captureReport;
  rfManagerHal_readReceivedPacket_fake.custom_fake = readReceivedPacket;

  EDF_init();
  EDF_activeObject_pubSubInit(subscribers, EVENTS_LAST_SIGNAL);
  EDF_pool_init(eventPool, sizeof(eventPool), sizeof(events_rxPacket_t));
  rfManager_init(&rfManager);
  hostCommandManager_init(&hostManager);
}

void EDFTest_setUp(void)
{
  reportLength = 0U;
}

void EDFTest_tearDown(void)
{
  EDF_TEST_VERIFY(reportOffset == 0U);
}

/* -----------------------------------------------------------------------------
 * Scenario: CAD persists across a clear scan, detection and packet delivery.
 * -------------------------------------------------------------------------- */

/** @brief Establish idle before configuring receive-on-detection CAD. */
EDF_TEST_CASE(cadReceivePrepareIdle, EVENTS_STOP)
EDF_TEST_CASE_INIT(cadReceivePrepareIdle)
{
  rfManager_startup();
  hostCommandManager_startup();
  sendCommand(HOST_COMMAND_MANAGER_STOP, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(cadReceivePrepareIdle, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_STOP);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Confirm the scenario's idle precondition through diagnostics. */
EDF_TEST_CASE(cadReceiveConfirmIdle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(cadReceiveConfirmIdle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(cadReceiveConfirmIdle, e)
{
  const events_getDiagnosticsResponse_t* response;
  const uint8_t* payload;

  response = (const events_getDiagnosticsResponse_t*)e;
  EDF_TEST_VERIFY(response->active_rf_operation == RF_TYPES_OPERATION_IDLE);
  EDF_TEST_VERIFY(response->radio_errors == 0U);
  payload = verifyReport(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_RESPONSE, 3U);
  EDF_TEST_VERIFY(payload[0] == RF_TYPES_OPERATION_IDLE);
  EDF_TEST_VERIFY((payload[1] == 0U) && (payload[2] == 0U));
}

/** @brief Apply the scenario's complete receive-on-detection configuration. */
EDF_TEST_CASE(cadReceiveConfigure, EVENTS_SET_CONFIG)
EDF_TEST_CASE_INIT(cadReceiveConfigure)
{
  sendCommand(HOST_COMMAND_MANAGER_SET_CONFIG,
              cadReceiveConfigurationPayload,
              sizeof(cadReceiveConfigurationPayload));
}
EDF_TEST_CASE_VERIFY(cadReceiveConfigure, e)
{
  verifyConfiguration(&((const events_setConfig_t*)e)->configuration);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Publish CAD through the host boundary. */
EDF_TEST_CASE(cadReceiveStart, EVENTS_CAD_START)
EDF_TEST_CASE_INIT(cadReceiveStart)
{
  sendCommand(HOST_COMMAND_MANAGER_CAD_START, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(cadReceiveStart, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_CAD_START);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Diagnose CAD before delivering any indication. */
EDF_TEST_CASE(cadReceiveActive, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(cadReceiveActive)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(cadReceiveActive, e)
{
  const events_getDiagnosticsResponse_t* response;
  const uint8_t* payload;

  response = (const events_getDiagnosticsResponse_t*)e;
  EDF_TEST_VERIFY(response->active_rf_operation == RF_TYPES_OPERATION_CAD);
  EDF_TEST_VERIFY(response->radio_errors == 0U);
  payload = verifyReport(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_RESPONSE, 3U);
  EDF_TEST_VERIFY(payload[0] == RF_TYPES_OPERATION_CAD);
  EDF_TEST_VERIFY((payload[1] == 0U) && (payload[2] == 0U));
}

/** @brief Diagnostics retain CAD identity after a clear scan. */
EDF_TEST_CASE(cadReceiveClear, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(cadReceiveClear)
{
  const rfManagerHal_callbacks_t* callbacks = rfManagerHal_init_fake.arg0_val;
  void* context = rfManagerHal_init_fake.arg1_val;

  EDF_TEST_VERIFY(callbacks != NULL);
  // EDFTest_fail never returns, but its declaration lacks noreturn.
  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  EDF_TEST_VERIFY(callbacks->cad_done != NULL);
  EDF_TEST_VERIFY(context != NULL);
  callbacks->cad_done(context, false);
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(cadReceiveClear, e)
{
  const events_getDiagnosticsResponse_t* response;
  const uint8_t* payload;

  response = (const events_getDiagnosticsResponse_t*)e;
  EDF_TEST_VERIFY(response->active_rf_operation == RF_TYPES_OPERATION_CAD);
  EDF_TEST_VERIFY(response->radio_errors == 0U);
  payload = verifyReport(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_RESPONSE, 3U);
  EDF_TEST_VERIFY(payload[0] == RF_TYPES_OPERATION_CAD);
  EDF_TEST_VERIFY((payload[1] == 0U) && (payload[2] == 0U));
}

/** @brief Detection produces a host report. */
EDF_TEST_CASE(cadReceiveDetection, EVENTS_CAD_DETECTED)
EDF_TEST_CASE_INIT(cadReceiveDetection)
{
  const rfManagerHal_callbacks_t* callbacks = rfManagerHal_init_fake.arg0_val;

  callbacks->cad_done(rfManagerHal_init_fake.arg1_val, true);
}
EDF_TEST_CASE_VERIFY(cadReceiveDetection, e)
{
  EMF_UTILS_UNUSED_PARAM(e);
  (void)verifyReport(HOST_COMMAND_MANAGER_CAD_DETECTED, 0U);
}

/** @brief Automatic radio reception preserves the CAD identity. */
EDF_TEST_CASE(cadReceiveDetectedActive, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(cadReceiveDetectedActive)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(cadReceiveDetectedActive, e)
{
  const events_getDiagnosticsResponse_t* response;
  const uint8_t* payload;

  response = (const events_getDiagnosticsResponse_t*)e;
  EDF_TEST_VERIFY(response->active_rf_operation == RF_TYPES_OPERATION_CAD);
  EDF_TEST_VERIFY(response->radio_errors == 0U);
  payload = verifyReport(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_RESPONSE, 3U);
  EDF_TEST_VERIFY(payload[0] == RF_TYPES_OPERATION_CAD);
  EDF_TEST_VERIFY((payload[1] == 0U) && (payload[2] == 0U));
}

/** @brief Preserve received bytes and metadata in the event and host report. */
EDF_TEST_CASE(cadReceivePacket, EVENTS_RX_PACKET)
EDF_TEST_CASE_INIT(cadReceivePacket)
{
  const rfManagerHal_callbacks_t* callbacks = rfManagerHal_init_fake.arg0_val;

  EDF_TEST_VERIFY(callbacks->rx_done != NULL);
  callbacks->rx_done(rfManagerHal_init_fake.arg1_val);
}
EDF_TEST_CASE_VERIFY(cadReceivePacket, e)
{
  const events_rxPacket_t* response;
  const uint8_t* payload;

  response = (const events_rxPacket_t*)e;
  EDF_TEST_VERIFY(response->received_packet_length_bytes == sizeof(packet));
  EDF_TEST_VERIFY(memcmp(response->packet_data, packet, sizeof(packet)) == 0);
  EDF_TEST_VERIFY(response->packet_status.rssi == packetStatus.rssi);
  EDF_TEST_VERIFY(response->packet_status.packet_info ==
                  packetStatus.packet_info);
  payload = verifyReport(HOST_COMMAND_MANAGER_RX_PACKET, sizeof(packet) + 3U);
  EDF_TEST_VERIFY(payload[0] == sizeof(packet));
  EDF_TEST_VERIFY(memcmp(&payload[1], packet, sizeof(packet)) == 0);
  EDF_TEST_VERIFY(payload[sizeof(packet) + 1U] == packetStatus.rssi);
  EDF_TEST_VERIFY(payload[sizeof(packet) + 2U] == packetStatus.packet_info);
}

/** @brief Final diagnosis reports CAD after packet delivery. */
EDF_TEST_CASE(cadReceiveCycle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(cadReceiveCycle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(cadReceiveCycle, e)
{
  const events_getDiagnosticsResponse_t* response;
  const uint8_t* payload;

  response = (const events_getDiagnosticsResponse_t*)e;
  EDF_TEST_VERIFY(response->active_rf_operation == RF_TYPES_OPERATION_CAD);
  EDF_TEST_VERIFY(response->radio_errors == 0U);
  payload = verifyReport(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_RESPONSE, 3U);
  EDF_TEST_VERIFY(payload[0] == RF_TYPES_OPERATION_CAD);
  EDF_TEST_VERIFY((payload[1] == 0U) && (payload[2] == 0U));
  EDF_TEST_VERIFY(inputOffset == inputLength);
}

EDF_TEST_SUITE(EDF_TEST_RUN(cadReceivePrepareIdle),
               EDF_TEST_RUN(cadReceiveConfirmIdle),
               EDF_TEST_RUN(cadReceiveConfigure),
               EDF_TEST_RUN(cadReceiveStart),
               EDF_TEST_RUN(cadReceiveActive),
               EDF_TEST_RUN(cadReceiveClear),
               EDF_TEST_RUN(cadReceiveDetection),
               EDF_TEST_RUN(cadReceiveDetectedActive),
               EDF_TEST_RUN(cadReceivePacket),
               EDF_TEST_RUN(cadReceiveCycle))
