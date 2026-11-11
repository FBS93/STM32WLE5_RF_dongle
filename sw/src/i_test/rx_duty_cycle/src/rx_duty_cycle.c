/*******************************************************************************
 * @brief Duty-cycle RX integration test (SW_INTEGRATION_TEST_7).
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
/** @brief Production subscription table. */
static EDF_activeObject_bitmask_t subscribers[EVENTS_LAST_SIGNAL];
/** @brief Two largest-event slots allow overlap with previous verification. */
static EMF_UTILS_MEM_ALIGNED_SLOT(sizeof(events_rxPacket_t)) eventPool[2U];
/** @brief Configuration-sized encoded input. */
static uint8_t input[EMF_COBS_ENCODED_SIZE(46U)];
/** @brief Input size. */
static uint16_t inputLength;
/** @brief Next unread offset. */
static uint16_t inputOffset;
/** @brief Latest decoded host report in the current phase. */
static uint8_t report[32U];
/** @brief Decoded report length; zero when no output was captured. */
static uint16_t reportLength;
/** @brief Host byte-stream capture, independent of HAL write boundaries. */
static uint8_t encodedReport[sizeof(report)];
/** @brief Bytes accumulated in the current host frame. */
static uint16_t reportOffset;
/** @brief Known packet with zero and high-bit bytes. */
static const uint8_t packet[] = {0x00U, 0xA5U, 0xFFU};
/** @brief Raw RSSI and signed quarter-dB SNR. */
static const rfTypes_packetStatus_t packetStatus = {.rssi = 182U,
                                                    .packet_info = 0xF4U};
/** @brief Complete LoRa duty-cycle payload: 64000 RX and 32000 sleep ticks. */
static const uint8_t dutyRxConfigurationPayload[40U] = {
  0x01U, 0x00U, 0x00U, 0x40U, 0x36U, 32U,   14U,   7U,    4U,    1U,
  0U,    8U,    0U,    0U,    1U,    0U,    0x24U, 0x14U, 0x34U, 0x12U,
  0U,    8U,    4U,    2U,    22U,   10U,   0U,    0x56U, 0x34U, 0U,
  2U,    0U,    0U,    0U,    0U,    0xFAU, 0U,    0U,    0x7DU, 0U};
/** @brief Only fabricated event: host bytes arrived. */
static const events_hostDataAvailable_t hostDataAvailable = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/** @brief EFF globals required by the HAL doubles. */
DEFINE_EFF_GLOBALS;
/** @brief Duty-cycle RX persistence across packets, timeout and rejection. */
EDF_TEST_SUITE_NAME(rx_duty_cycle)
/** @brief Three-event host burst plus one spare slot. */
EDF_TEST_EVENT_BUFF(4U)

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/**
 * @brief Verify mode and periods in the published configuration event.
 * @param[in] configuration Actual event configuration.
 */
static void verifyConfiguration(const rfTypes_configuration_t* configuration);
/**
 * @brief Supply a CRC/COBS command through host HAL input.
 * @param[in] command Public command ID.
 * @param[in] payload Payload or NULL.
 * @param[in] length Payload size.
 */
static void sendCommand(hostCommandManager_commandId_t command,
                        const uint8_t* payload,
                        uint16_t length);
/**
 * @brief Verify report framing and return its payload.
 * @param[in] command Expected command ID.
 * @param[in] length Expected payload size.
 * @return Read-only decoded payload.
 */
static const uint8_t* verifyReport(hostCommandManager_commandId_t command,
                                   uint16_t length);
/**
 * @brief Verify an actual diagnostic event and host report.
 * @param[in] e Response event.
 * @param[in] operation Expected operation.
 */
static void verifyDiagnostics(const EDF_event_t* e,
                              rfTypes_operation_t operation);
/**
 * @brief Verify actual packet event and host report data.
 * @param[in] e Packet event.
 */
static void verifyPacket(const EDF_event_t* e);
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
 * @brief Decode and copy transient host output.
 * @param[in] data Encoded report.
 * @param[in] length Encoded size.
 */
static void captureReport(const uint8_t* data, uint32_t length);
/**
 * @brief Supply packet data and metadata at the HAL boundary.
 * @param[out] data Packet destination.
 * @param[in] capacity Destination capacity.
 * @param[in] configuration Configuration retained by EFF.
 * @param[out] packet_status Metadata destination.
 * @return Packet size.
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
/** @brief Supply parser input. */
FAKE_VOID_FUNC(hostCommandManagerHal_read, uint8_t*, uint32_t);
/** @brief Capture reports. */
FAKE_VOID_FUNC(hostCommandManagerHal_write, const uint8_t*, uint32_t);
/** @brief Retain callback/context pointers; EFF sizeof(pointer) is intentional.
 */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_init, const rfManagerHal_callbacks_t*, void*);
/** @brief Observe configuration; EFF sizeof(pointer) is intentional. */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_setConfiguration, const rfTypes_configuration_t*);
/** @brief Retain RX configuration; EFF sizeof(pointer) is intentional. */
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
  lora = &configuration->packet_type_configuration.lora;
  EDF_TEST_VERIFY(lora->rx_mode == 2U);
  EDF_TEST_VERIFY(lora->rx_duty_cycle.rx_period == 64000UL);
  EDF_TEST_VERIFY(lora->rx_duty_cycle.sleep_period == 32000UL);
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
  EDF_TEST_VERIFY(length <= sizeof(dutyRxConfigurationPayload));
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
  uint16_t crc;

  EDF_TEST_VERIFY(reportLength == length + 6U);
  EMF_endian_u16ReadBuffLE(&report[0], &actual_command);
  EMF_endian_u16ReadBuffLE(&report[2], &actual_length);
  EMF_endian_u16ReadBuffLE(&report[length + 4U], &crc);
  EDF_TEST_VERIFY(actual_command == (uint16_t)command);
  EDF_TEST_VERIFY(actual_length == length);
  EDF_TEST_VERIFY(crc == EMF_crc_16CCITT(report, (uint16_t)(length + 4U)));
  return &report[4];
}

static void verifyDiagnostics(const EDF_event_t* e,
                              rfTypes_operation_t operation)
{
  const events_getDiagnosticsResponse_t* response;
  const uint8_t* payload;

  response = (const events_getDiagnosticsResponse_t*)e;
  EDF_TEST_VERIFY(response->active_rf_operation == operation);
  EDF_TEST_VERIFY(response->radio_errors == 0U);
  payload = verifyReport(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_RESPONSE, 3U);
  EDF_TEST_VERIFY(payload[0] == (uint8_t)operation);
  EDF_TEST_VERIFY((payload[1] == 0U) && (payload[2] == 0U));
}

static void verifyPacket(const EDF_event_t* e)
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
 * Scenario: duty-cycle RX persists across packets, timeout and rejection.
 * -------------------------------------------------------------------------- */

/** @brief Establish idle before configuring duty-cycle RX. */
EDF_TEST_CASE(dutyRxPrepareIdle, EVENTS_STOP)
EDF_TEST_CASE_INIT(dutyRxPrepareIdle)
{
  rfManager_startup();
  hostCommandManager_startup();
  sendCommand(HOST_COMMAND_MANAGER_STOP, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(dutyRxPrepareIdle, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_STOP);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Confirm the scenario's idle precondition through diagnostics. */
EDF_TEST_CASE(dutyRxConfirmIdle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(dutyRxConfirmIdle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(dutyRxConfirmIdle, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_IDLE);
}

/** @brief Apply the scenario's complete duty-cycle RX configuration. */
EDF_TEST_CASE(dutyRxConfigure, EVENTS_SET_CONFIG)
EDF_TEST_CASE_INIT(dutyRxConfigure)
{
  sendCommand(HOST_COMMAND_MANAGER_SET_CONFIG,
              dutyRxConfigurationPayload,
              sizeof(dutyRxConfigurationPayload));
}
EDF_TEST_CASE_VERIFY(dutyRxConfigure, e)
{
  verifyConfiguration(&((const events_setConfig_t*)e)->configuration);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Publish duty-cycle RX through the host boundary. */
EDF_TEST_CASE(dutyRxStart, EVENTS_RX_START)
EDF_TEST_CASE_INIT(dutyRxStart)
{
  sendCommand(HOST_COMMAND_MANAGER_RX_START, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(dutyRxStart, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_RX_START);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief First packet is delivered as an event and host report. */
EDF_TEST_CASE(dutyRxPacket, EVENTS_RX_PACKET)
EDF_TEST_CASE_INIT(dutyRxPacket)
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
EDF_TEST_CASE_VERIFY(dutyRxPacket, e)
{
  verifyPacket(e);
}

/** @brief Independent diagnosis proves the first packet did not complete RX. */
EDF_TEST_CASE(dutyRxPreserved, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(dutyRxPreserved)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(dutyRxPreserved, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_RX);
}

/** @brief Diagnostics still report RX after a timeout indication. */
EDF_TEST_CASE(dutyRxTimeout, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(dutyRxTimeout)
{
  const rfManagerHal_callbacks_t* callbacks = rfManagerHal_init_fake.arg0_val;

  EDF_TEST_VERIFY(callbacks->rx_timeout != NULL);
  callbacks->rx_timeout(rfManagerHal_init_fake.arg1_val);
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(dutyRxTimeout, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_RX);
}

/** @brief Diagnostics still report RX after packet rejection.
 */
EDF_TEST_CASE(dutyRxRejected, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(dutyRxRejected)
{
  const rfManagerHal_callbacks_t* callbacks = rfManagerHal_init_fake.arg0_val;

  EDF_TEST_VERIFY(callbacks->rx_error != NULL);
  callbacks->rx_error(rfManagerHal_init_fake.arg1_val);
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(dutyRxRejected, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_RX);
}

/** @brief A second indication produces a packet report after both outcomes. */
EDF_TEST_CASE(dutyRxPacketAgain, EVENTS_RX_PACKET)
EDF_TEST_CASE_INIT(dutyRxPacketAgain)
{
  const rfManagerHal_callbacks_t* callbacks = rfManagerHal_init_fake.arg0_val;

  callbacks->rx_done(rfManagerHal_init_fake.arg1_val);
}
EDF_TEST_CASE_VERIFY(dutyRxPacketAgain, e)
{
  verifyPacket(e);
}

/** @brief RX still persists after the second successful packet. */
EDF_TEST_CASE(dutyRxStillActive, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(dutyRxStillActive)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(dutyRxStillActive, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_RX);
}

/** @brief Explicit STOP ends persistent duty-cycle RX. */
EDF_TEST_CASE(dutyRxStop, EVENTS_STOP)
EDF_TEST_CASE_INIT(dutyRxStop)
{
  sendCommand(HOST_COMMAND_MANAGER_STOP, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(dutyRxStop, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_STOP);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Final diagnosis reports idle after STOP. */
EDF_TEST_CASE(dutyRxLifecycle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(dutyRxLifecycle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(dutyRxLifecycle, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_IDLE);
  EDF_TEST_VERIFY(inputOffset == inputLength);
}

EDF_TEST_SUITE(EDF_TEST_RUN(dutyRxPrepareIdle),
               EDF_TEST_RUN(dutyRxConfirmIdle),
               EDF_TEST_RUN(dutyRxConfigure),
               EDF_TEST_RUN(dutyRxStart),
               EDF_TEST_RUN(dutyRxPacket),
               EDF_TEST_RUN(dutyRxPreserved),
               EDF_TEST_RUN(dutyRxTimeout),
               EDF_TEST_RUN(dutyRxRejected),
               EDF_TEST_RUN(dutyRxPacketAgain),
               EDF_TEST_RUN(dutyRxStillActive),
               EDF_TEST_RUN(dutyRxStop),
               EDF_TEST_RUN(dutyRxLifecycle))
