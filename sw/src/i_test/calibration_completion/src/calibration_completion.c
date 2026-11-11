/*******************************************************************************
 * @brief Calibration integration test (SW_INTEGRATION_TEST_5).
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
/** @brief Production signal subscription table. */
static EDF_activeObject_bitmask_t subscribers[EVENTS_LAST_SIGNAL];
/** @brief Largest-event slots cover overlap with previous verification. */
static EMF_UTILS_MEM_ALIGNED_SLOT(sizeof(events_rxPacket_t)) eventPool[2U];
/** @brief Encoded two-byte calibration input plus framing. */
static uint8_t input[EMF_COBS_ENCODED_SIZE(8U)];
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
/** @brief Valid ordered image interval, 864..880 MHz in 4 MHz units. */
static const uint8_t calibrationPayload[] = {216U, 220U};
/** @brief Only fabricated stimulus event: host bytes arrived. */
static const events_hostDataAvailable_t hostDataAvailable = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/** @brief EFF globals required by the HAL doubles. */
DEFINE_EFF_GLOBALS;
/** @brief Calibration completes without prior RF configuration. */
EDF_TEST_SUITE_NAME(calibration_completion)
/** @brief Three-event host burst with one spare slot as in RX. */
EDF_TEST_EVENT_BUFF(4U)

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/**
 * @brief Supply a CRC/COBS host command.
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
 * @brief Consume input bytes requested by the real parser.
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
/** @brief Stub configuration; EFF sizeof(pointer) is intentional.
 */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_setConfiguration, const rfTypes_configuration_t*);
/** @brief Stub RX; EFF sizeof(pointer) is intentional. */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(rfManagerHal_startReceive, const rfTypes_configuration_t*);
/** @brief Observe stops. */
FAKE_VOID_FUNC(rfManagerHal_stop);
/** @brief Stub packet read; EFF sizeof(pointer) is intentional. */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VALUE_FUNC(uint8_t,
                rfManagerHal_readReceivedPacket,
                uint8_t*,
                uint8_t,
                const rfTypes_configuration_t*,
                rfTypes_packetStatus_t*);
/** @brief Healthy diagnostics use the default zero result. */
FAKE_VALUE_FUNC(uint16_t, rfManagerHal_getErrors);
/** @brief Synchronous HAL calibration returns normally; no callback is
 * invented. */
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

static void sendCommand(hostCommandManager_commandId_t command,
                        const uint8_t* payload,
                        uint16_t length)
{
  uint8_t decoded[8U];
  uint16_t u16_command_id;
  uint16_t crc;
  uint16_t index;

  EDF_TEST_VERIFY(inputOffset == inputLength);
  EDF_TEST_VERIFY(length <= sizeof(calibrationPayload));
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
 * Scenario: calibration without prior configuration completes in idle.
 * -------------------------------------------------------------------------- */

/** @brief Establish idle without applying an RF configuration. */
EDF_TEST_CASE(calibrationPrepareIdle, EVENTS_STOP)
EDF_TEST_CASE_INIT(calibrationPrepareIdle)
{
  rfManager_startup();
  hostCommandManager_startup();
  sendCommand(HOST_COMMAND_MANAGER_STOP, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(calibrationPrepareIdle, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_STOP);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Confirm the scenario's idle precondition through diagnostics. */
EDF_TEST_CASE(calibrationConfirmIdle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(calibrationConfirmIdle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(calibrationConfirmIdle, e)
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

/** @brief Supply the calibration interval through the real host path. */
EDF_TEST_CASE(calibrationStart, EVENTS_CALIBRATE)
EDF_TEST_CASE_INIT(calibrationStart)
{
  sendCommand(HOST_COMMAND_MANAGER_CALIBRATE,
              calibrationPayload,
              sizeof(calibrationPayload));
}
EDF_TEST_CASE_VERIFY(calibrationStart, e)
{
  const events_calibrate_t* request;

  request = (const events_calibrate_t*)e;
  EDF_TEST_VERIFY(request->image_calibration_lower_frequency_hz ==
                  calibrationPayload[0]);
  EDF_TEST_VERIFY(request->image_calibration_upper_frequency_hz ==
                  calibrationPayload[1]);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Observe actual HAL-return-generated completion after CALIBRATE in the
 * FIFO. */
EDF_TEST_CASE(calibrationCompletion, EVENTS_OPERATION_COMPLETE)
EDF_TEST_CASE_INIT(calibrationCompletion)
{
}
EDF_TEST_CASE_VERIFY(calibrationCompletion, e)
{
  EDF_TEST_VERIFY(e->sig == EVENTS_OPERATION_COMPLETE);
  EDF_TEST_VERIFY(reportLength == 0U);
}

/** @brief Final diagnosis reports idle without an invented completion message.
 */
EDF_TEST_CASE(calibrationLifecycle, EVENTS_GET_DIAGNOSTICS_RESPONSE)
EDF_TEST_CASE_INIT(calibrationLifecycle)
{
  sendCommand(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_REQUEST, NULL, 0U);
}
EDF_TEST_CASE_VERIFY(calibrationLifecycle, e)
{
  const events_getDiagnosticsResponse_t* response;
  const uint8_t* payload;

  response = (const events_getDiagnosticsResponse_t*)e;
  EDF_TEST_VERIFY(response->active_rf_operation == RF_TYPES_OPERATION_IDLE);
  EDF_TEST_VERIFY(response->radio_errors == 0U);
  payload = verifyReport(HOST_COMMAND_MANAGER_GET_DIAGNOSTICS_RESPONSE, 3U);
  EDF_TEST_VERIFY(payload[0] == RF_TYPES_OPERATION_IDLE);
  EDF_TEST_VERIFY((payload[1] == 0U) && (payload[2] == 0U));
  EDF_TEST_VERIFY(inputOffset == inputLength);
}

EDF_TEST_SUITE(EDF_TEST_RUN(calibrationPrepareIdle),
               EDF_TEST_RUN(calibrationConfirmIdle),
               EDF_TEST_RUN(calibrationStart),
               EDF_TEST_RUN(calibrationCompletion),
               EDF_TEST_RUN(calibrationLifecycle))
