/*******************************************************************************
 * @brief Continuous RX integration test (SW_INTEGRATION_TEST_3).
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
/** @brief Input size. */
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
/** @brief Known RX and replacement TX payload, unlike configured RX length. */
static const uint8_t packet[] = {0x00U, 0xA5U, 0xFFU, 0x31U, 0x00U};
/** @brief Raw RSSI and signed quarter-dB SNR. */
static const rfTypes_packetStatus_t packetStatus = {.rssi = 182U,
                                                    .packet_info = 0xF4U};
/** @brief Snapshot of transient TX bytes. */
static uint8_t transmittedPacket[sizeof(packet)];
/** @brief Complete LoRa configuration payload for continuous RX. */
static const uint8_t continuousRxConfigurationPayload[40U] = {
  0x01U, 0x00U, 0x00U, 0x40U, 0x36U, 32U, 14U,   7U,    4U,    1U,
  0U,    8U,    0U,    0U,    1U,    0U,  0x24U, 0x14U, 0x34U, 0x12U,
  0U,    8U,    4U,    2U,    22U,   10U, 0U,    0x56U, 0x34U, 0U,
  1U,    0x45U, 0x23U, 0x01U, 0U,    0U,  0U,    0U,    0U,    0U};
/** @brief Known error flags, including a high byte, supplied at the HAL
 * boundary. */
static const uint16_t radioErrors = 0x0140U;
/** @brief Only fabricated stimulus event: host bytes arrived. */
static const events_hostDataAvailable_t hostDataAvailable = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/** @brief EFF globals required by the HAL doubles. */
DEFINE_EFF_GLOBALS;
/** @brief Continuous RX persistence followed by replacement with packet TX. */
EDF_TEST_SUITE_NAME(rx_continuous)
/** @brief Three-event host burst with one spare slot as in RX. */
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
 * @brief Supply a CRC/COBS command through the real parser.
 * @param[in] command Public command ID.
 * @param[in] payload Payload or NULL.
 * @param[in] length Payload size.
 */
static void sendCommand(hostCommandManager_commandId_t command,
                        const uint8_t* payload,
                        uint16_t length);
/**
 * @brief Verify framing and return report payload.
 * @param[in] command Expected command ID.
 * @param[in] length Expected payload size.
 * @return Read-only payload.
 */
static const uint8_t* verifyReport(hostCommandManager_commandId_t command,
                                   uint16_t length);
/**
 * @brief Return unread input size.
 * @return Available bytes.
 */
static uint32_t getAvailableBytes(void);
/**
 * @brief Consume input bytes requested by the parser.
 * @param[out] data Destination.
 * @param[in] length Requested size.
 */
static void readBytes(uint8_t* data, uint32_t length);
/**
 * @brief Decode actual transient output.
 * @param[in] data Report bytes.
 * @param[in] length Encoded size.
 */
static void captureReport(const uint8_t* data, uint32_t length);
/**
 * @brief Supply packet and metadata at the radio boundary.
 * @param[out] data Destination.
 * @param[in] capacity Destination capacity.
 * @param[in] configuration Configuration retained by EFF.
 * @param[out] packet_status Metadata destination.
 * @return Received size.
 */
static uint8_t readReceivedPacket(uint8_t* data,
                                  uint8_t capacity,
                                  const rfTypes_configuration_t* configuration,
                                  rfTypes_packetStatus_t* packet_status);
/**
 * @brief Snapshot TX bytes before their event expires.
 * @param[in] configuration Persistent configuration retained by EFF.
 * @param[in] data Transient TX bytes.
 * @param[in] length TX size.
 */
static void captureTransmission(const rfTypes_configuration_t* configuration,
                                const uint8_t* data,
                                uint8_t length);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

/** @brief Observe host initialization. */
FAKE_VOID_FUNC(hostCommandManagerHal_init);
/** @brief Supply input availability. */
FAKE_VALUE_FUNC(uint32_t, hostCommandManagerHal_getAvailableBytes);
/** @brief Supply parser input. */
FAKE_VOID_FUNC(hostCommandManagerHal_read, uint8_t*, uint32_t);
/** @brief Capture actual reports. */
FAKE_VOID_FUNC(hostCommandManagerHal_write, const uint8_t*, uint32_t);
/** @brief Retain callback/context values; EFF sizeof(pointer) is intentional.
 */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_init, const rfManagerHal_callbacks_t*, void*);
/** @brief Observe configuration; EFF sizeof(pointer) is intentional. */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_setConfiguration, const rfTypes_configuration_t*);
/** @brief Observe RX; EFF sizeof(pointer) is intentional. */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_startReceive, const rfTypes_configuration_t*);
/** @brief Observe stops. */
FAKE_VOID_FUNC(rfManagerHal_stop);
/** @brief Supply packet/status; EFF sizeof(pointer) is intentional. */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VALUE_FUNC(uint8_t,
                rfManagerHal_readReceivedPacket,
                uint8_t*,
                uint8_t,
                const rfTypes_configuration_t*,
                rfTypes_packetStatus_t*);
/** @brief Supply known error flags. */
FAKE_VALUE_FUNC(uint16_t, rfManagerHal_getErrors);
/** @brief Stub calibration. */
FAKE_VOID_FUNC(rfManagerHal_calibrate, uint8_t, uint8_t);
/** @brief Observe TX pointer values; EFF sizeof(pointer) is intentional. */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_transmit,
               const rfTypes_configuration_t*,
               const uint8_t*,
               uint8_t);
/** @brief Stub CAD; EFF sizeof(pointer) is intentional. */
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
  EDF_TEST_VERIFY(lora->cad.exit_mode == 0U);
  EDF_TEST_VERIFY(lora->cad.timeout == 0x3456UL);
  EDF_TEST_VERIFY(lora->rx_mode == 1U);
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
  EDF_TEST_VERIFY(length <= sizeof(continuousRxConfigurationPayload));
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

static void captureTransmission(const rfTypes_configuration_t* configuration,
                                const uint8_t* data,
                                uint8_t length)
{
  uint8_t index;

  EMF_UTILS_UNUSED_PARAM(configuration);
  EDF_TEST_VERIFY(data != NULL);
  EDF_TEST_VERIFY(length == sizeof(transmittedPacket));
  for (index = 0U; index < length; ++index)
  {
    transmittedPacket[index] = data[index];
  }
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
  rfManagerHal_transmit_fake.custom_fake = captureTransmission;
  rfManagerHal_getErrors_fake.return_val = radioErrors;

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
 * Scenario: continuous RX persists through queries/packet, then TX replaces it.
 * -------------------------------------------------------------------------- */

/** @brief Establish idle before configuring continuous RX. */
EDF_TEST_CASE(continuousRxPrepareIdle, EVENTS_STOP)
EDF_TEST_CASE_INIT(continuousRxPrepareIdle)
{
  rfManager_startup();
  hostCommandManager_startup();
  sendCommand(HOST_COMMAND_MANAGER_STOP, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousRxPrepareIdle, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_STOP);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Confirm idle and the supplied radio-error flags before RX. */
EDF_TEST_CASE(continuousRxConfirmIdle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(continuousRxConfirmIdle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousRxConfirmIdle, e)
{
  const events_getDiagnosticsResponse_t* response;
  const uint8_t* payload;
  uint16_t errors;

  response = (const events_getDiagnosticsResponse_t*)e;
  EDF_TEST_VERIFY(response->active_rf_operation == RF_TYPES_OPERATION_IDLE);
  EDF_TEST_VERIFY(response->radio_errors == radioErrors);
  payload = verifyReport(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_RESPONSE, 3U);
  EDF_TEST_VERIFY(payload[0] == RF_TYPES_OPERATION_IDLE);
  EMF_endian_u16ReadBuffLE(&payload[1], &errors);
  EDF_TEST_VERIFY(errors == radioErrors);
}

/** @brief Apply the scenario's complete continuous-RX configuration. */
EDF_TEST_CASE(continuousRxConfigure, EVENTS_SET_CONFIG)
EDF_TEST_CASE_INIT(continuousRxConfigure)
{
  sendCommand(HOST_COMMAND_MANAGER_SET_CONFIG,
              continuousRxConfigurationPayload,
              sizeof(continuousRxConfigurationPayload));
}
EDF_TEST_CASE_VERIFY(continuousRxConfigure, e)
{
  verifyConfiguration(&((const events_setConfig_t*)e)->configuration);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Start continuous receive with the stored configuration. */
EDF_TEST_CASE(continuousRxStart, EVENTS_RX_START)
EDF_TEST_CASE_INIT(continuousRxStart)
{
  sendCommand(HOST_COMMAND_MANAGER_RX_START, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousRxStart, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_RX_START);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Configuration query preserves all stored fields and active RF. */
EDF_TEST_CASE(continuousRxConfiguration, EVENTS_GET_CONFIG_RESPONSE)
EDF_TEST_CASE_INIT(continuousRxConfiguration)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_CONFIG_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousRxConfiguration, e)
{
  const uint8_t* payload;

  verifyConfiguration(&((const events_getConfigResponse_t*)e)->configuration);
  payload = verifyReport(HOST_COMMAND_MANAGER_GET_CONFIG_RESPONSE,
                         sizeof(continuousRxConfigurationPayload));
  EDF_TEST_VERIFY(memcmp(payload,
                         continuousRxConfigurationPayload,
                         sizeof(continuousRxConfigurationPayload)) == 0);
}

/** @brief Diagnostics preserve RX and carry supplied radio errors. */
EDF_TEST_CASE(continuousRxActive, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(continuousRxActive)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousRxActive, e)
{
  const events_getDiagnosticsResponse_t* response;
  const uint8_t* payload;
  uint16_t errors;

  response = (const events_getDiagnosticsResponse_t*)e;
  EDF_TEST_VERIFY(response->active_rf_operation == RF_TYPES_OPERATION_RX);
  EDF_TEST_VERIFY(response->radio_errors == radioErrors);
  payload = verifyReport(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_RESPONSE, 3U);
  EDF_TEST_VERIFY(payload[0] == RF_TYPES_OPERATION_RX);
  EMF_endian_u16ReadBuffLE(&payload[1], &errors);
  EDF_TEST_VERIFY(errors == radioErrors);
}

/** @brief Verify packet event and host report data during continuous RX. */
EDF_TEST_CASE(continuousRxPacket, EVENTS_RX_PACKET)
EDF_TEST_CASE_INIT(continuousRxPacket)
{
  const rfManagerHal_callbacks_t* callbacks = rfManagerHal_init_fake.arg0_val;
  void* context = rfManagerHal_init_fake.arg1_val;

  EDF_TEST_VERIFY(callbacks != NULL);
  // EDFTest_fail never returns, but its declaration lacks noreturn.
  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  EDF_TEST_VERIFY(callbacks->rx_done != NULL);
  EDF_TEST_VERIFY(context != NULL);
  callbacks->rx_done(context);
}
EDF_TEST_CASE_VERIFY(continuousRxPacket, e)
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

/** @brief A subsequent query proves RX persisted after the packet. */
EDF_TEST_CASE(continuousRxPreserved, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(continuousRxPreserved)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousRxPreserved, e)
{
  const events_getDiagnosticsResponse_t* response;
  const uint8_t* payload;
  uint16_t errors;

  response = (const events_getDiagnosticsResponse_t*)e;
  EDF_TEST_VERIFY(response->active_rf_operation == RF_TYPES_OPERATION_RX);
  EDF_TEST_VERIFY(response->radio_errors == radioErrors);
  payload = verifyReport(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_RESPONSE, 3U);
  EDF_TEST_VERIFY(payload[0] == RF_TYPES_OPERATION_RX);
  EMF_endian_u16ReadBuffLE(&payload[1], &errors);
  EDF_TEST_VERIFY(errors == radioErrors);
}

/** @brief Replace RX with packet TX through the host boundary. */
EDF_TEST_CASE(continuousRxReplace, EVENTS_TX_PACKET)
EDF_TEST_CASE_INIT(continuousRxReplace)
{
  sendCommand(HOST_COMMAND_MANAGER_TX_PACKET, packet, sizeof(packet));
}
EDF_TEST_CASE_VERIFY(continuousRxReplace, e)
{
  const events_txPacket_t* request;

  request = (const events_txPacket_t*)e;
  EDF_TEST_VERIFY(request->payload_length == sizeof(packet));
  EDF_TEST_VERIFY(memcmp(request->packet_data, packet, sizeof(packet)) == 0);
  EDF_TEST_VERIFY(memcmp(transmittedPacket, packet, sizeof(packet)) == 0);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Diagnose packet TX before explicit STOP, with no tx_done injected. */
EDF_TEST_CASE(continuousRxReplaced, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(continuousRxReplaced)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousRxReplaced, e)
{
  const events_getDiagnosticsResponse_t* response;
  const uint8_t* payload;
  uint16_t errors;

  response = (const events_getDiagnosticsResponse_t*)e;
  EDF_TEST_VERIFY(response->active_rf_operation ==
                  RF_TYPES_OPERATION_PACKET_TX);
  EDF_TEST_VERIFY(response->radio_errors == radioErrors);
  payload = verifyReport(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_RESPONSE, 3U);
  EDF_TEST_VERIFY(payload[0] == RF_TYPES_OPERATION_PACKET_TX);
  EMF_endian_u16ReadBuffLE(&payload[1], &errors);
  EDF_TEST_VERIFY(errors == radioErrors);
}

/** @brief Explicit STOP terminates the replacement TX without a host report. */
EDF_TEST_CASE(continuousRxStop, EVENTS_STOP)
EDF_TEST_CASE_INIT(continuousRxStop)
{
  sendCommand(HOST_COMMAND_MANAGER_STOP, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousRxStop, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_STOP);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Final diagnosis reports idle after STOP. */
EDF_TEST_CASE(continuousRxLifecycle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(continuousRxLifecycle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousRxLifecycle, e)
{
  const events_getDiagnosticsResponse_t* response;
  const uint8_t* payload;
  uint16_t errors;

  response = (const events_getDiagnosticsResponse_t*)e;
  EDF_TEST_VERIFY(response->active_rf_operation == RF_TYPES_OPERATION_IDLE);
  EDF_TEST_VERIFY(response->radio_errors == radioErrors);
  payload = verifyReport(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_RESPONSE, 3U);
  EDF_TEST_VERIFY(payload[0] == RF_TYPES_OPERATION_IDLE);
  EMF_endian_u16ReadBuffLE(&payload[1], &errors);
  EDF_TEST_VERIFY(errors == radioErrors);
  EDF_TEST_VERIFY(inputOffset == inputLength);
}

EDF_TEST_SUITE(EDF_TEST_RUN(continuousRxPrepareIdle),
               EDF_TEST_RUN(continuousRxConfirmIdle),
               EDF_TEST_RUN(continuousRxConfigure),
               EDF_TEST_RUN(continuousRxStart),
               EDF_TEST_RUN(continuousRxConfiguration),
               EDF_TEST_RUN(continuousRxActive),
               EDF_TEST_RUN(continuousRxPacket),
               EDF_TEST_RUN(continuousRxPreserved),
               EDF_TEST_RUN(continuousRxReplace),
               EDF_TEST_RUN(continuousRxReplaced),
               EDF_TEST_RUN(continuousRxStop),
               EDF_TEST_RUN(continuousRxLifecycle))
