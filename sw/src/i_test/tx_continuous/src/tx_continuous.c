/*******************************************************************************
 * @brief Continuous TX integration test (SW_INTEGRATION_TEST_9).
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
/** @brief Complete LoRa configuration payload for wave/preamble replacement. */
static const uint8_t continuousTxConfigurationPayload[40U] = {
  0x01U, 0x00U, 0x00U, 0x40U, 0x36U, 32U, 14U,   7U,    4U,    1U,
  0U,    8U,    0U,    0U,    1U,    0U,  0x24U, 0x14U, 0x34U, 0x12U,
  0U,    8U,    4U,    2U,    22U,   10U, 0U,    0x56U, 0x34U, 0U,
  0U,    0x45U, 0x23U, 0x01U, 0U,    0U,  0U,    0U,    0U,    0U};
/** @brief Only fabricated event: host bytes arrived. */
static const events_hostDataAvailable_t hostDataAvailable = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/** @brief EFF globals required by the HAL doubles. */
DEFINE_EFF_GLOBALS;
/** @brief Continuous TX replacement followed by explicit STOP. */
EDF_TEST_SUITE_NAME(tx_continuous)
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
/** @brief Stub RX; EFF sizeof(pointer) is intentional. */
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
/** @brief Stub packet TX; EFF sizeof(pointer) is intentional. */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_transmit,
               const rfTypes_configuration_t*,
               const uint8_t*,
               uint8_t);
/** @brief Stub CAD; EFF sizeof(pointer) is intentional. */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_startCad, const rfTypes_configuration_t*);
/** @brief Observe continuous-wave TX. */
FAKE_VOID_FUNC(rfManagerHal_startContinuousWave);
/** @brief Observe preamble TX. */
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
  EDF_TEST_VERIFY(length <= sizeof(continuousTxConfigurationPayload));
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
 * Scenario: continuous wave persists until replaced by preamble, then STOP.
 * -------------------------------------------------------------------------- */

/** @brief Establish idle before configuring continuous TX. */
EDF_TEST_CASE(continuousTxPrepareIdle, EVENTS_STOP)
EDF_TEST_CASE_INIT(continuousTxPrepareIdle)
{
  rfManager_startup();
  hostCommandManager_startup();
  sendCommand(HOST_COMMAND_MANAGER_STOP, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousTxPrepareIdle, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_STOP);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Confirm the scenario's idle precondition through diagnostics. */
EDF_TEST_CASE(continuousTxConfirmIdle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(continuousTxConfirmIdle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousTxConfirmIdle, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_IDLE);
}

/** @brief Apply the complete LoRa configuration for wave/preamble replacement.
 */
EDF_TEST_CASE(continuousTxConfigure, EVENTS_SET_CONFIG)
EDF_TEST_CASE_INIT(continuousTxConfigure)
{
  sendCommand(HOST_COMMAND_MANAGER_SET_CONFIG,
              continuousTxConfigurationPayload,
              sizeof(continuousTxConfigurationPayload));
}
EDF_TEST_CASE_VERIFY(continuousTxConfigure, e)
{
  EDF_TEST_VERIFY(((const events_setConfig_t*)e)->configuration.packet_type ==
                  RF_TYPES_PACKET_TYPE_LORA);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Publish a continuous-wave request through the host boundary. */
EDF_TEST_CASE(continuousWaveStart, EVENTS_TX_CONTINUOUS_WAVE)
EDF_TEST_CASE_INIT(continuousWaveStart)
{
  sendCommand(HOST_COMMAND_MANAGER_TX_CONTINUOUS_WAVE, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousWaveStart, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_TX_CONTINUOUS_WAVE);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Diagnose continuous-wave identity without a completion indication. */
EDF_TEST_CASE(continuousWaveActive, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(continuousWaveActive)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousWaveActive, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_CONTINUOUS_WAVE_TX);
}

/** @brief A second query reports continuous-wave identity. */
EDF_TEST_CASE(continuousWavePreserved, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(continuousWavePreserved)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousWavePreserved, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_CONTINUOUS_WAVE_TX);
}

/** @brief Replace wave with an infinite-preamble request.
 */
EDF_TEST_CASE(continuousPreambleStart, EVENTS_TX_CONTINUOUS_PREAMBLE)
EDF_TEST_CASE_INIT(continuousPreambleStart)
{
  sendCommand(HOST_COMMAND_MANAGER_TX_CONTINUOUS_PREAMBLE, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousPreambleStart, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_TX_CONTINUOUS_PREAMBLE);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Diagnose the replacement's distinct infinite-preamble identity. */
EDF_TEST_CASE(continuousPreambleActive, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(continuousPreambleActive)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousPreambleActive, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_CONTINUOUS_PREAMBLE_TX);
}

/** @brief A second query reports infinite-preamble identity. */
EDF_TEST_CASE(continuousPreamblePreserved, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(continuousPreamblePreserved)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousPreamblePreserved, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_CONTINUOUS_PREAMBLE_TX);
}

/** @brief Explicit STOP terminates infinite preamble without a host report. */
EDF_TEST_CASE(continuousTxStop, EVENTS_STOP)
EDF_TEST_CASE_INIT(continuousTxStop)
{
  sendCommand(HOST_COMMAND_MANAGER_STOP, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousTxStop, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_STOP);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Final diagnosis reports idle after STOP. */
EDF_TEST_CASE(continuousTxLifecycle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(continuousTxLifecycle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(continuousTxLifecycle, e)
{
  verifyDiagnostics(e, RF_TYPES_OPERATION_IDLE);
  EDF_TEST_VERIFY(inputOffset == inputLength);
}

EDF_TEST_SUITE(EDF_TEST_RUN(continuousTxPrepareIdle),
               EDF_TEST_RUN(continuousTxConfirmIdle),
               EDF_TEST_RUN(continuousTxConfigure),
               EDF_TEST_RUN(continuousWaveStart),
               EDF_TEST_RUN(continuousWaveActive),
               EDF_TEST_RUN(continuousWavePreserved),
               EDF_TEST_RUN(continuousPreambleStart),
               EDF_TEST_RUN(continuousPreambleActive),
               EDF_TEST_RUN(continuousPreamblePreserved),
               EDF_TEST_RUN(continuousTxStop),
               EDF_TEST_RUN(continuousTxLifecycle))
