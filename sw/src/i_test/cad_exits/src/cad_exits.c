/*******************************************************************************
 * @brief CAD exits integration test (SW_INTEGRATION_TEST_8).
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
/** @brief Complete LoRa configuration payload for stop-on-detection CAD. */
static const uint8_t cadStopConfigurationPayload[40U] = {
  0x01U, 0x00U, 0x00U, 0x40U, 0x36U, 32U, 14U,   7U,    4U,    1U,
  0U,    8U,    0U,    0U,    1U,    0U,  0x24U, 0x14U, 0x34U, 0x12U,
  0U,    8U,    4U,    2U,    22U,   10U, 0U,    0x56U, 0x34U, 0U,
  0U,    0x45U, 0x23U, 0x01U, 0U,    0U,  0U,    0U,    0U,    0U};
/** @brief Complete LoRa configuration payload for receive-on-detection CAD. */
static const uint8_t cadReceiveConfigurationPayload[40U] = {
  0x01U, 0x00U, 0x00U, 0x40U, 0x36U, 32U, 14U,   7U,    4U,    1U,
  0U,    8U,    0U,    0U,    1U,    0U,  0x24U, 0x14U, 0x34U, 0x12U,
  0U,    8U,    4U,    2U,    22U,   10U, 1U,    0x56U, 0x34U, 0U,
  0U,    0x45U, 0x23U, 0x01U, 0U,    0U,  0U,    0U,    0U,    0U};
/** @brief Only fabricated event: host bytes arrived. */
static const events_hostDataAvailable_t hostDataAvailable = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/** @brief EFF globals required by the HAL doubles. */
DEFINE_EFF_GLOBALS;
/** @brief Terminal and persistent CAD outcomes. */
EDF_TEST_SUITE_NAME(cad_exits)
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
/** @brief Stub RX start; EFF sizeof(pointer) is intentional. */
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
/** @brief Stub TX; EFF sizeof(pointer) is intentional. */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_transmit,
               const rfTypes_configuration_t*,
               const uint8_t*,
               uint8_t);
/** @brief Retain CAD configuration; EFF sizeof(pointer) is intentional. */
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
  EDF_TEST_VERIFY(length <= sizeof(cadStopConfigurationPayload));
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
 * Scenario A: detecting activity ends stop-on-detection CAD.
 * -------------------------------------------------------------------------- */

/** @brief Establish idle independently of the preceding RF operation. */
EDF_TEST_CASE(cadStopPrepareIdle, EVENTS_STOP)
EDF_TEST_CASE_INIT(cadStopPrepareIdle)
{
  // Start the HALs once, from the first phase inside EDF startup.
  rfManager_startup();
  hostCommandManager_startup();
  sendCommand(HOST_COMMAND_MANAGER_STOP, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(cadStopPrepareIdle, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_STOP);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Confirm the scenario's idle precondition through diagnostics. */
EDF_TEST_CASE(cadStopConfirmIdle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(cadStopConfirmIdle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(cadStopConfirmIdle, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_IDLE);
}

/** @brief Apply the scenario's complete stop-on-detection configuration. */
EDF_TEST_CASE(cadStopConfigure, EVENTS_SET_CONFIG)
EDF_TEST_CASE_INIT(cadStopConfigure)
{
  sendCommand(HOST_COMMAND_MANAGER_SET_CONFIG,
              cadStopConfigurationPayload,
              sizeof(cadStopConfigurationPayload));
}
EDF_TEST_CASE_VERIFY(cadStopConfigure, e)
{
  EDF_TEST_VERIFY(
    ((const events_setConfig_t*)e)
      ->configuration.packet_type_configuration.lora.cad.exit_mode == 0U);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Start CAD with the stop-on-detection configuration. */
EDF_TEST_CASE(cadStopStart, EVENTS_CAD_START)
EDF_TEST_CASE_INIT(cadStopStart)
{
  sendCommand(HOST_COMMAND_MANAGER_CAD_START, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(cadStopStart, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_CAD_START);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Registered detection reports channel activity. */
EDF_TEST_CASE(cadStopDetectActivity, EVENTS_CAD_DETECTED)
EDF_TEST_CASE_INIT(cadStopDetectActivity)
{
  const rfManagerHal_callbacks_t* callbacks = rfManagerHal_init_fake.arg0_val;
  void* context = rfManagerHal_init_fake.arg1_val;

  EDF_TEST_VERIFY(callbacks != NULL);
  // EDFTest_fail never returns, but its declaration lacks noreturn.
  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  EDF_TEST_VERIFY(callbacks->cad_done != NULL);
  EDF_TEST_VERIFY(context != NULL);
  callbacks->cad_done(context, true);
}
EDF_TEST_CASE_VERIFY(cadStopDetectActivity, e)
{
  EMF_UTILS_UNUSED_PARAM(e);
  (void)verifyReport(HOST_COMMAND_MANAGER_CAD_DETECTED, 0U);
}

/** @brief Diagnostics confirm that stop-on-detection CAD has ended. */
EDF_TEST_CASE(cadStopEndsIdle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(cadStopEndsIdle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(cadStopEndsIdle, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_IDLE);
}

/* -----------------------------------------------------------------------------
 * Scenario B: receive-on-detection CAD survives timeout until explicit STOP.
 * -------------------------------------------------------------------------- */

/** @brief Establish idle without relying on scenario A's terminal state. */
EDF_TEST_CASE(cadReceivePrepareIdle, EVENTS_STOP)
EDF_TEST_CASE_INIT(cadReceivePrepareIdle)
{
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
  verifyDiagnostics(e, RF_TYPES_OPERATION_IDLE);
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
  EDF_TEST_VERIFY(
    ((const events_setConfig_t*)e)
      ->configuration.packet_type_configuration.lora.cad.exit_mode == 1U);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Start CAD with receive-on-detection and a finite receive timeout. */
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

/** @brief Receive-on-detection reports channel activity. */
EDF_TEST_CASE(cadReceiveDetectActivity, EVENTS_CAD_DETECTED)
EDF_TEST_CASE_INIT(cadReceiveDetectActivity)
{
  const rfManagerHal_callbacks_t* callbacks = rfManagerHal_init_fake.arg0_val;
  void* context = rfManagerHal_init_fake.arg1_val;

  EDF_TEST_VERIFY(callbacks != NULL);
  // EDFTest_fail never returns, but its declaration lacks noreturn.
  // NOLINTNEXTLINE(clang-analyzer-core.NullDereference)
  EDF_TEST_VERIFY(callbacks->cad_done != NULL);
  EDF_TEST_VERIFY(context != NULL);
  callbacks->cad_done(context, true);
}
EDF_TEST_CASE_VERIFY(cadReceiveDetectActivity, e)
{
  EMF_UTILS_UNUSED_PARAM(e);
  (void)verifyReport(HOST_COMMAND_MANAGER_CAD_DETECTED, 0U);
}

/** @brief Automatic receive retains CAD identity before timeout. */
EDF_TEST_CASE(cadReceiveRemainsActive, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(cadReceiveRemainsActive)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(cadReceiveRemainsActive, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_CAD);
}

/** @brief Diagnostics retain CAD identity after a receive timeout. */
EDF_TEST_CASE(cadReceiveSurvivesTimeout, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(cadReceiveSurvivesTimeout)
{
  const rfManagerHal_callbacks_t* callbacks = rfManagerHal_init_fake.arg0_val;

  EDF_TEST_VERIFY(callbacks->rx_timeout != NULL);
  callbacks->rx_timeout(rfManagerHal_init_fake.arg1_val);
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(cadReceiveSurvivesTimeout, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_CAD);
}

/** @brief A second indication produces another detection report.
 */
EDF_TEST_CASE(cadReceiveDetectsAgain, EVENTS_CAD_DETECTED)
EDF_TEST_CASE_INIT(cadReceiveDetectsAgain)
{
  const rfManagerHal_callbacks_t* callbacks = rfManagerHal_init_fake.arg0_val;

  callbacks->cad_done(rfManagerHal_init_fake.arg1_val, true);
}
EDF_TEST_CASE_VERIFY(cadReceiveDetectsAgain, e)
{
  EMF_UTILS_UNUSED_PARAM(e);
  (void)verifyReport(HOST_COMMAND_MANAGER_CAD_DETECTED, 0U);
}

/** @brief Explicit STOP ends the persistent receive-on-detection operation. */
EDF_TEST_CASE(cadReceiveStop, EVENTS_STOP)
EDF_TEST_CASE_INIT(cadReceiveStop)
{
  sendCommand(HOST_COMMAND_MANAGER_STOP, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(cadReceiveStop, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_STOP);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Final diagnosis reports idle after STOP. */
EDF_TEST_CASE(cadReceiveEndsIdle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(cadReceiveEndsIdle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(cadReceiveEndsIdle, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_IDLE);
  EDF_TEST_VERIFY(inputOffset == inputLength);
}

EDF_TEST_SUITE(EDF_TEST_RUN(cadStopPrepareIdle),
               EDF_TEST_RUN(cadStopConfirmIdle),
               EDF_TEST_RUN(cadStopConfigure),
               EDF_TEST_RUN(cadStopStart),
               EDF_TEST_RUN(cadStopDetectActivity),
               EDF_TEST_RUN(cadStopEndsIdle),
               EDF_TEST_RUN(cadReceivePrepareIdle),
               EDF_TEST_RUN(cadReceiveConfirmIdle),
               EDF_TEST_RUN(cadReceiveConfigure),
               EDF_TEST_RUN(cadReceiveStart),
               EDF_TEST_RUN(cadReceiveDetectActivity),
               EDF_TEST_RUN(cadReceiveRemainsActive),
               EDF_TEST_RUN(cadReceiveSurvivesTimeout),
               EDF_TEST_RUN(cadReceiveDetectsAgain),
               EDF_TEST_RUN(cadReceiveStop),
               EDF_TEST_RUN(cadReceiveEndsIdle))
