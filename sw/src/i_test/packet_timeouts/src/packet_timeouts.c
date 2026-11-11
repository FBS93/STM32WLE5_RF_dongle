/*******************************************************************************
 * @brief Packet timeout integration test (SW_INTEGRATION_TEST_6).
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
static uint8_t report[16U];
/** @brief Decoded report length; zero when no output was captured. */
static uint16_t reportLength;
/** @brief Host byte-stream capture, independent of HAL write boundaries. */
static uint8_t encodedReport[sizeof(report)];
/** @brief Bytes accumulated in the current host frame. */
static uint16_t reportOffset;
/** @brief Complete LoRa configuration payload for the single-RX timeout. */
static const uint8_t rxTimeoutConfigurationPayload[40U] = {
  0x01U, 0x00U, 0x00U, 0x40U, 0x36U, 32U, 14U,   7U,    4U,    1U,
  0U,    8U,    0U,    0U,    1U,    0U,  0x24U, 0x14U, 0x34U, 0x12U,
  0U,    8U,    4U,    2U,    22U,   10U, 0U,    0x56U, 0x34U, 0U,
  0U,    0x45U, 0x23U, 0x01U, 0U,    0U,  0U,    0U,    0U,    0U};
/** @brief Complete LoRa configuration payload for the packet-TX timeout. */
static const uint8_t txTimeoutConfigurationPayload[40U] = {
  0x01U, 0x00U, 0x00U, 0x40U, 0x36U, 32U, 14U,   7U,    4U,    1U,
  0U,    8U,    0U,    0U,    1U,    0U,  0x24U, 0x14U, 0x34U, 0x12U,
  0U,    8U,    4U,    2U,    22U,   10U, 0U,    0x56U, 0x34U, 0U,
  0U,    0x45U, 0x23U, 0x01U, 0U,    0U,  0U,    0U,    0U,    0U};
/** @brief Packet supplied to the real host parser. */
static const uint8_t packet[] = {0x00U, 0xA5U, 0xFFU};
/** @brief Only fabricated event: host bytes arrived. */
static const events_hostDataAvailable_t hostDataAvailable = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/** @brief EFF globals required by the HAL doubles. */
DEFINE_EFF_GLOBALS;
/** @brief Single-RX and packet-TX timeout scenarios with their own preparation.
 */
EDF_TEST_SUITE_NAME(packet_timeouts)
/** @brief Three-event host burst plus one spare slot. */
EDF_TEST_EVENT_BUFF(4U)

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

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
 * @brief Verify diagnostic event and independently framed host report.
 * @param[in] e Actual response event.
 * @param[in] operation Expected functional operation.
 */
static void verifyDiagnostics(const EDF_event_t* e,
                              rfTypes_operation_t operation);
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
/** @brief Stub packet reads; EFF sizeof(pointer) is intentional.
 */
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
/** @brief Retain TX arguments; EFF sizeof(pointer) is intentional. */
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

static void sendCommand(hostCommandManager_commandId_t command,
                        const uint8_t* payload,
                        uint16_t length)
{
  uint8_t decoded[46U];
  uint16_t u16_command_id;
  uint16_t crc;
  uint16_t index;

  EDF_TEST_VERIFY(inputOffset == inputLength);
  EDF_TEST_VERIFY(length <= sizeof(rxTimeoutConfigurationPayload));
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

static void verifyDiagnostics(const EDF_event_t* e,
                              rfTypes_operation_t operation)
{
  const events_getDiagnosticsResponse_t* response;
  uint16_t command;
  uint16_t length;
  uint16_t crc;

  response = (const events_getDiagnosticsResponse_t*)e;
  EDF_TEST_VERIFY(response->active_rf_operation == operation);
  EDF_TEST_VERIFY(response->radio_errors == 0U);
  EDF_TEST_VERIFY(reportLength == 9U);
  EMF_endian_u16ReadBuffLE(&report[0], &command);
  EMF_endian_u16ReadBuffLE(&report[2], &length);
  EMF_endian_u16ReadBuffLE(&report[7], &crc);
  EDF_TEST_VERIFY(command ==
                  (uint16_t)HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_RESPONSE);
  EDF_TEST_VERIFY(length == 3U);
  EDF_TEST_VERIFY(report[4] == (uint8_t)operation);
  EDF_TEST_VERIFY((report[5] == 0U) && (report[6] == 0U));
  EDF_TEST_VERIFY(crc == EMF_crc_16CCITT(report, 7U));
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
 * Scenario A: a single-RX timeout ends the operation without a packet report.
 * -------------------------------------------------------------------------- */

/** @brief Establish idle before configuring the single-RX timeout. */
EDF_TEST_CASE(timeoutRxPrepareIdle, EVENTS_STOP)
EDF_TEST_CASE_INIT(timeoutRxPrepareIdle)
{
  rfManager_startup();
  hostCommandManager_startup();
  sendCommand(HOST_COMMAND_MANAGER_STOP, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(timeoutRxPrepareIdle, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_STOP);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Confirm the RX scenario's idle precondition through diagnostics. */
EDF_TEST_CASE(timeoutRxConfirmIdle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(timeoutRxConfirmIdle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(timeoutRxConfirmIdle, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_IDLE);
}

/** @brief Apply the complete single-RX timeout configuration. */
EDF_TEST_CASE(timeoutRxConfigure, EVENTS_SET_CONFIG)
EDF_TEST_CASE_INIT(timeoutRxConfigure)
{
  sendCommand(HOST_COMMAND_MANAGER_SET_CONFIG,
              rxTimeoutConfigurationPayload,
              sizeof(rxTimeoutConfigurationPayload));
}
EDF_TEST_CASE_VERIFY(timeoutRxConfigure, e)
{
  const events_setConfig_t* request = (const events_setConfig_t*)e;

  EDF_TEST_VERIFY(
    request->configuration.packet_type_configuration.lora.rx_mode == 0U);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Publish a single RX request through the host boundary. */
EDF_TEST_CASE(timeoutRxStart, EVENTS_RX_START)
EDF_TEST_CASE_INIT(timeoutRxStart)
{
  sendCommand(HOST_COMMAND_MANAGER_RX_START, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(timeoutRxStart, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_RX_START);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Single RX remains active before the timeout indication. */
EDF_TEST_CASE(timeoutRxActive, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(timeoutRxActive)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(timeoutRxActive, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_RX);
}

/** @brief Registered RX timeout generates actual completion without a packet.
 */
EDF_TEST_CASE(timeoutRxComplete, EVENTS_OPERATION_COMPLETE)
EDF_TEST_CASE_INIT(timeoutRxComplete)
{
  const rfManagerHal_callbacks_t* callbacks = rfManagerHal_init_fake.arg0_val;
  void* context = rfManagerHal_init_fake.arg1_val;

  EDF_TEST_VERIFY(callbacks != NULL);
  // EDFTest_fail never returns, but its declaration lacks noreturn.
  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  EDF_TEST_VERIFY(callbacks->rx_timeout != NULL);
  EDF_TEST_VERIFY(context != NULL);
  callbacks->rx_timeout(context);
}
EDF_TEST_CASE_VERIFY(timeoutRxComplete, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_OPERATION_COMPLETE);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief An independent host diagnosis confirms RX timeout ended in idle. */
EDF_TEST_CASE(timeoutRxIdle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(timeoutRxIdle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(timeoutRxIdle, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_IDLE);
}

/* -----------------------------------------------------------------------------
 * Scenario B: a packet-TX timeout ends the operation without tx_done.
 * -------------------------------------------------------------------------- */

/** @brief Establish idle without relying on the RX timeout scenario. */
EDF_TEST_CASE(timeoutTxPrepareIdle, EVENTS_STOP)
EDF_TEST_CASE_INIT(timeoutTxPrepareIdle)
{
  sendCommand(HOST_COMMAND_MANAGER_STOP, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(timeoutTxPrepareIdle, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_STOP);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Confirm the TX scenario's idle precondition through diagnostics. */
EDF_TEST_CASE(timeoutTxConfirmIdle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(timeoutTxConfirmIdle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(timeoutTxConfirmIdle, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_IDLE);
}

/** @brief Apply the complete packet-TX timeout configuration. */
EDF_TEST_CASE(timeoutTxConfigure, EVENTS_SET_CONFIG)
EDF_TEST_CASE_INIT(timeoutTxConfigure)
{
  sendCommand(HOST_COMMAND_MANAGER_SET_CONFIG,
              txTimeoutConfigurationPayload,
              sizeof(txTimeoutConfigurationPayload));
}
EDF_TEST_CASE_VERIFY(timeoutTxConfigure, e)
{
  EDF_TEST_VERIFY(((const events_setConfig_t*)e)->configuration.packet_type ==
                  RF_TYPES_PACKET_TYPE_LORA);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Publish packet TX data through the host boundary. */
EDF_TEST_CASE(timeoutTxStart, EVENTS_TX_PACKET)
EDF_TEST_CASE_INIT(timeoutTxStart)
{
  sendCommand(HOST_COMMAND_MANAGER_TX_PACKET, packet, sizeof(packet));
}
EDF_TEST_CASE_VERIFY(timeoutTxStart, e)
{
  EDF_TEST_VERIFY(((const events_txPacket_t*)e)->payload_length ==
                  sizeof(packet));
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Packet TX stays active until its indication. */
EDF_TEST_CASE(timeoutTxActive, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(timeoutTxActive)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(timeoutTxActive, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_PACKET_TX);
}

/** @brief Registered TX timeout completes without fabricating tx_done. */
EDF_TEST_CASE(timeoutTxComplete, EVENTS_OPERATION_COMPLETE)
EDF_TEST_CASE_INIT(timeoutTxComplete)
{
  const rfManagerHal_callbacks_t* callbacks = rfManagerHal_init_fake.arg0_val;

  EDF_TEST_VERIFY(callbacks->tx_timeout != NULL);
  callbacks->tx_timeout(rfManagerHal_init_fake.arg1_val);
}
EDF_TEST_CASE_VERIFY(timeoutTxComplete, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_OPERATION_COMPLETE);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Final diagnosis reports idle after TX timeout. */
EDF_TEST_CASE(timeoutTxIdle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(timeoutTxIdle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(timeoutTxIdle, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_IDLE);
  EDF_TEST_VERIFY(inputOffset == inputLength);
}

EDF_TEST_SUITE(EDF_TEST_RUN(timeoutRxPrepareIdle),
               EDF_TEST_RUN(timeoutRxConfirmIdle),
               EDF_TEST_RUN(timeoutRxConfigure),
               EDF_TEST_RUN(timeoutRxStart),
               EDF_TEST_RUN(timeoutRxActive),
               EDF_TEST_RUN(timeoutRxComplete),
               EDF_TEST_RUN(timeoutRxIdle),
               EDF_TEST_RUN(timeoutTxPrepareIdle),
               EDF_TEST_RUN(timeoutTxConfirmIdle),
               EDF_TEST_RUN(timeoutTxConfigure),
               EDF_TEST_RUN(timeoutTxStart),
               EDF_TEST_RUN(timeoutTxActive),
               EDF_TEST_RUN(timeoutTxComplete),
               EDF_TEST_RUN(timeoutTxIdle))
