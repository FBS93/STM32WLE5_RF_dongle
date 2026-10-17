/*******************************************************************************
 * @brief Isolated validation of host command frame reception and integrity.
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
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */
#include "eaf.h"
#include "edf.h"
#include "emf.h"
#include "eff.h"
#include "etf.h"

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "events.h"
#include "host_command_manager.h"
#include "host_command_manager_activities.h"
#include "host_command_manager_hal.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief Fixture event capacity for the largest scenario.
 */
#define TEST_EVENT_CAPACITY 3U

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/**
 * @brief Real FIFO supplying the consecutive frame stream through EFF.
 */
static EMF_byteFifo_handler_t rxFifo;

/**
 * @brief Input storage sufficient for the maximum frame or consecutive stream.
 */
static uint8_t fifoStorage[HOST_COMMAND_MANAGER_RX_FRAME_CAPACITY];

/**
 * @brief Distinct typed storage for the scenario's three mutable events.
 */
static events_txPacket_t allocatedPackets[TEST_EVENT_CAPACITY];

/**
 * @brief Next unused fixture slot, independent of publication snapshots.
 */
static uint32_t allocationIndex;

/**
 * @brief Copies of actual publications, independent of allocated storage.
 */
static events_txPacket_t publishedPackets[TEST_EVENT_CAPACITY];

/**
 * @brief Number of captured publication snapshots.
 */
static uint32_t publicationCount;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/**
 * @brief EFF call history shared by this test's fakes.
 */
DEFINE_EFF_GLOBALS;

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/**
 * @brief Returns bytes remaining in the real input FIFO.
 * @return Available bytes.
 */
static uint32_t getAvailableBytes(void);

/**
 * @brief Adapts the EFF HAL read callback to the real EMF FIFO.
 *
 * @param[out] data Destination for received bytes.
 * @param[in] length Number of bytes to consume.
 */
static void readBytes(uint8_t* data, uint32_t length);

/**
 * @brief Supplies a distinct typed slot initialized from allocator arguments.
 *
 * @param[in] event_size Requested storage size in bytes.
 * @param[in] signal Requested event signal.
 * @return Initialized event, or NULL if fixture storage cannot serve it.
 */
static EDF_event_t* allocateEvent(uint_fast16_t event_size, int signal);

/**
 * @brief Snapshots the event while its publication argument is valid.
 *
 * @param[in] event Event actually passed to EDF publication.
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

  // Keep all three slots valid even if several events are allocated up front.
  if ((event_size <= sizeof(allocatedPackets[0])) &&
      (allocationIndex < TEST_EVENT_CAPACITY))
  {
    allocatedPackets[allocationIndex] = (events_txPacket_t){
      .super = {
        .sig = (EDF_event_signal_t)signal, .pool_num = 1U, .ref_cnt = 0U}};
    event = &allocatedPackets[allocationIndex].super;
    ++allocationIndex;
  }

  return event;
}

static void capturePublication(const EDF_event_t* event)
{
  ETF_VERIFY(event != NULL);
  ETF_VERIFY(publicationCount < TEST_EVENT_CAPACITY);

  // Observe the actual signal before interpreting the TX_PACKET parameters.
  publishedPackets[publicationCount].super = *event;
  if (event->sig == EVENTS_TX_PACKET)
  {
    publishedPackets[publicationCount] = *(const events_txPacket_t*)event;
  }
  ++publicationCount;
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief EFF replacement for the HAL available-byte query.
 */
FAKE_VALUE_FUNC(uint32_t, hostCommandManagerHal_getAvailableBytes);
/**
 * @brief EFF replacement for consuming received HAL bytes.
 */
FAKE_VOID_FUNC(hostCommandManagerHal_read, uint8_t*, uint32_t);
/**
 * @brief EFF observer for unexpected host responses.
 */
FAKE_VOID_FUNC(hostCommandManagerHal_write, const uint8_t*, uint32_t);
/**
 * @brief EFF allocator backed by test-owned typed storage.
 *
 * EFF copies returned pointer values into return history, not event data;
 * sizeof(pointer) is intentional in that macro-generated memcpy.
 */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VALUE_FUNC(EDF_event_t*, EDF_event_initMutable, uint_fast16_t, int);
/**
 * @brief EFF observer with separate snapshots of published event data.
 *
 * EFF copies pointer arguments into argument history, not event data;
 * sizeof(pointer) is intentional in that macro-generated memcpy.
 */
// NOLINTNEXTLINE(bugprone-sizeof-expression)
FAKE_VOID_FUNC(EDF_activeObject_publish, const EDF_event_t*);

/**
 * @brief Reports unexpected production assertions as ETF failures.
 */
void EAF_onError(const char* file, int line, int id)
{
  EMF_UTILS_UNUSED_PARAM(id);

  ETF_fail("Unexpected production assertion", file, line);
}

/**
 * @brief Reset shared state and wire isolated collaborators before every case.
 */
void ETF_setUp(void)
{
  size_t slot_index;
  size_t byte_index;

  RESET_FAKE(hostCommandManagerHal_getAvailableBytes);
  RESET_FAKE(hostCommandManagerHal_read);
  RESET_FAKE(hostCommandManagerHal_write);
  RESET_FAKE(EDF_event_initMutable);
  RESET_FAKE(EDF_activeObject_publish);
  // Clear retained pointers too, not only the EFF history write index.
  eff = (eff_globals_t){0};

  for (byte_index = 0U; byte_index < sizeof(fifoStorage); ++byte_index)
  {
    fifoStorage[byte_index] = 0U;
  }
  EMF_byteFifo_init(&rxFifo, sizeof(fifoStorage), fifoStorage);
  for (slot_index = 0U;
       slot_index < sizeof(allocatedPackets) / sizeof(allocatedPackets[0]);
       ++slot_index)
  {
    allocatedPackets[slot_index] = (events_txPacket_t){0};
    publishedPackets[slot_index] = (events_txPacket_t){0};
  }
  allocationIndex = 0U;
  publicationCount = 0U;

  hostCommandManagerHal_getAvailableBytes_fake.custom_fake = getAvailableBytes;
  hostCommandManagerHal_read_fake.custom_fake = readBytes;
  EDF_event_initMutable_fake.custom_fake = allocateEvent;
  EDF_activeObject_publish_fake.custom_fake = capturePublication;
}

/**
 * @brief Receive and validate command frames through the isolated activity.
 */
ETF_TEST_SUITE(host_command_reception)
{
  ETF_TEST(fragmentedFramePublishesOnce)
  {
    // Start with empty assembly state and an immutable data-available trigger.
    hostCommandManager_t manager = {0};
    events_hostDataAvailable_t data_available = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};

    // Include a zero payload byte to exercise COBS byte stuffing.
    uint8_t payload[] = {0x11U, 0x00U, 0x33U};
    // LE header: TX_PACKET (0x0003), length 3; payload; CRC placeholders.
    uint8_t decoded[] = {
      0x03U, 0x00U, 0x03U, 0x00U, 0x11U, 0x00U, 0x33U, 0x00U, 0x00U};
    uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(decoded))];
    uint16_t crc;
    uint16_t encoded_length;
    uint16_t first_length;

    // Real EMF computes CRC over the header and payload, then stores it as LE.
    crc = EMF_crc_16CCITT(decoded, 7U);
    EMF_endian_u16WriteBuffLE(&decoded[7], &crc);

    // Real EMF COBS encoding appends the zero delimiter in encoded_length.
    EMF_cobs_encode(decoded, sizeof(decoded), encoded, &encoded_length);
    first_length = encoded_length / 2U;

    // Invoke only the activity with the first half, before the frame delimiter.
    EMF_byteFifo_push(&rxFifo, encoded, first_length);
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    // Drain RX, but do not publish before a complete command is available.
    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(EDF_activeObject_publish_fake.call_count == 0U);

    // Second invocation completes that same frame, including its delimiter.
    EMF_byteFifo_push(&rxFifo,
                      &encoded[first_length],
                      (uint32_t)(encoded_length - first_length));
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    // Drain RX and publish the completed command exactly once.
    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(EDF_activeObject_publish_fake.call_count == 1U);

    // Verify the publication snapshot, not the allocator's fixture object.
    ETF_VERIFY(publishedPackets[0].super.sig == EVENTS_TX_PACKET);
    ETF_VERIFY(publishedPackets[0].payload_length == sizeof(payload));
    ETF_VERIFY(
      memcmp(publishedPackets[0].packet_data, payload, sizeof(payload)) == 0);

    // Inherited host contract: TX_PACKET has no response.
    ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
  }

  ETF_TEST(completeFramesAndTrailingFragment)
  {
    // Start with empty assembly state, without starting the AO or its HSM.
    hostCommandManager_t manager = {0};
    events_hostDataAvailable_t data_available = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};

    // Distinct payloads and lengths make losses, duplicates and mixing visible.
    uint8_t first_payload[] = {0x11U, 0x00U, 0x33U};
    uint8_t second_payload[] = {0x44U, 0x55U};
    uint8_t third_payload[] = {0x66U, 0x77U, 0x00U, 0x99U};

    // LE TX_PACKET headers, payloads and two CRC placeholders per frame.
    uint8_t first_decoded[] = {
      0x03U, 0x00U, 0x03U, 0x00U, 0x11U, 0x00U, 0x33U, 0x00U, 0x00U};
    uint8_t second_decoded[] = {
      0x03U, 0x00U, 0x02U, 0x00U, 0x44U, 0x55U, 0x00U, 0x00U};
    uint8_t third_decoded[] = {
      0x03U, 0x00U, 0x04U, 0x00U, 0x66U, 0x77U, 0x00U, 0x99U, 0x00U, 0x00U};
    uint8_t first_encoded[EMF_COBS_ENCODED_SIZE(sizeof(first_decoded))];
    uint8_t second_encoded[EMF_COBS_ENCODED_SIZE(sizeof(second_decoded))];
    uint8_t third_encoded[EMF_COBS_ENCODED_SIZE(sizeof(third_decoded))];
    uint16_t crc;
    uint16_t first_length;
    uint16_t second_length;
    uint16_t third_length;
    uint16_t fragment_length;
    uint32_t first_matches = 0U;
    uint32_t second_matches = 0U;
    uint32_t publication_index;

    // Real EMF computes CRC over each header and payload and writes it as LE.
    crc = EMF_crc_16CCITT(first_decoded, 7U);
    EMF_endian_u16WriteBuffLE(&first_decoded[7], &crc);
    crc = EMF_crc_16CCITT(second_decoded, 6U);
    EMF_endian_u16WriteBuffLE(&second_decoded[6], &crc);
    crc = EMF_crc_16CCITT(third_decoded, 8U);
    EMF_endian_u16WriteBuffLE(&third_decoded[8], &crc);

    // Real COBS encoding includes the delimiter in each returned length.
    EMF_cobs_encode(
      first_decoded, sizeof(first_decoded), first_encoded, &first_length);
    EMF_cobs_encode(
      second_decoded, sizeof(second_decoded), second_encoded, &second_length);
    EMF_cobs_encode(
      third_decoded, sizeof(third_decoded), third_encoded, &third_length);
    fragment_length = third_length / 2U;

    // One input stream contains two whole frames and a third without delimiter.
    EMF_byteFifo_push(&rxFifo, first_encoded, first_length);
    EMF_byteFifo_push(&rxFifo, second_encoded, second_length);
    EMF_byteFifo_push(&rxFifo, third_encoded, fragment_length);
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    // Drain all bytes, publishing only the two complete commands.
    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(EDF_activeObject_publish_fake.call_count == 2U);
    ETF_VERIFY(publicationCount == 2U);

    // Check the publications as a set: each expected command appears once.
    for (publication_index = 0U; publication_index < 2U; ++publication_index)
    {
      ETF_VERIFY(publishedPackets[publication_index].super.sig ==
                 EVENTS_TX_PACKET);
      if ((publishedPackets[publication_index].payload_length ==
           sizeof(first_payload)) &&
          (memcmp(publishedPackets[publication_index].packet_data,
                  first_payload,
                  sizeof(first_payload)) == 0))
      {
        ++first_matches;
      }
      if ((publishedPackets[publication_index].payload_length ==
           sizeof(second_payload)) &&
          (memcmp(publishedPackets[publication_index].packet_data,
                  second_payload,
                  sizeof(second_payload)) == 0))
      {
        ++second_matches;
      }
    }
    ETF_VERIFY(first_matches == 1U);
    ETF_VERIFY(second_matches == 1U);

    // Complete only the third frame in a second invocation of the activity.
    EMF_byteFifo_push(&rxFifo,
                      &third_encoded[fragment_length],
                      (uint32_t)(third_length - fragment_length));
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    // Exactly one new snapshot must represent the completed third command.
    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(EDF_activeObject_publish_fake.call_count == 3U);
    ETF_VERIFY(publicationCount == 3U);
    ETF_VERIFY(publishedPackets[2].super.sig == EVENTS_TX_PACKET);
    ETF_VERIFY(publishedPackets[2].payload_length == sizeof(third_payload));
    ETF_VERIFY(memcmp(publishedPackets[2].packet_data,
                      third_payload,
                      sizeof(third_payload)) == 0);

    // Inherited host contract: TX_PACKET has no response.
    ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
  }

  ETF_TEST(maximumPayloadPublishesIntact)
  {
    // Start with empty assembly state, without starting the AO or its HSM.
    hostCommandManager_t manager = {0};
    events_hostDataAvailable_t data_available = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};

    // SYS_REQ_PARAMETER_40 permits up to 255 TX packet bytes.
    uint8_t payload[255U];
    uint8_t decoded[sizeof(payload) + 6U];
    uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(decoded))];
    uint16_t command_id = 0x0003U;
    uint16_t payload_length = sizeof(payload);
    uint16_t crc;
    uint16_t encoded_length;
    size_t byte_index;

    // Unique bytes distinguish every position; index 128 contains a COBS zero.
    for (byte_index = 0U; byte_index < sizeof(payload); ++byte_index)
    {
      payload[byte_index] = (uint8_t)(byte_index ^ 0x80U);
    }

    // Serialize the contract's LE header, payload and CRC using real EMF.
    EMF_endian_u16WriteBuffLE(&decoded[0], &command_id);
    EMF_endian_u16WriteBuffLE(&decoded[2], &payload_length);
    for (byte_index = 0U; byte_index < sizeof(payload); ++byte_index)
    {
      decoded[4U + byte_index] = payload[byte_index];
    }
    crc = EMF_crc_16CCITT(decoded, sizeof(payload) + 4U);
    EMF_endian_u16WriteBuffLE(&decoded[sizeof(payload) + 4U], &crc);
    EMF_cobs_encode(decoded, sizeof(decoded), encoded, &encoded_length);

    // Supply the complete encoded frame, including its real COBS delimiter.
    EMF_byteFifo_push(&rxFifo, encoded, encoded_length);
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    // Drain RX and publish exactly the corresponding event with all 255 bytes.
    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(EDF_activeObject_publish_fake.call_count == 1U);
    ETF_VERIFY(publishedPackets[0].super.sig == EVENTS_TX_PACKET);
    ETF_VERIFY(publishedPackets[0].payload_length == 255U);
    ETF_VERIFY(
      memcmp(publishedPackets[0].packet_data, payload, sizeof(payload)) == 0);

    // Inherited TX_PACKET contract: no host response.
    ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
  }

  ETF_TEST(invalidCrcIsDiscarded)
  {
    // Invoke the activity with empty assembly state, not the AO or HSM.
    hostCommandManager_t manager = {0};
    events_hostDataAvailable_t data_available = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};

    // Valid three-byte payload, including a zero handled by real COBS.
    uint8_t decoded[] = {
      0x00U, 0x00U, 0x00U, 0x00U, 0x11U, 0x00U, 0x33U, 0x00U, 0x00U};
    uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(decoded))];
    uint16_t command_id = 0x0003U;
    uint16_t payload_length = 3U;
    uint16_t crc;
    uint16_t encoded_length;

    // Shared typed allocation remains available even before validation.
    // Real EMF serializes the valid header and computes its header/payload CRC.
    EMF_endian_u16WriteBuffLE(&decoded[0], &command_id);
    EMF_endian_u16WriteBuffLE(&decoded[2], &payload_length);
    crc = EMF_crc_16CCITT(decoded, 7U);

    // Flip exactly one CRC bit before COBS so framing and content stay valid.
    crc ^= 0x0001U;
    EMF_endian_u16WriteBuffLE(&decoded[7], &crc);
    EMF_cobs_encode(decoded, sizeof(decoded), encoded, &encoded_length);

    // Supply the complete encoded frame, including the real COBS delimiter.
    EMF_byteFifo_push(&rxFifo, encoded, encoded_length);
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    // DD draining and inherited discard-without-response contract.
    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(EDF_activeObject_publish_fake.call_count == 0U);
    ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
  }

  ETF_TEST(declaredPayloadTooShortIsDiscarded)
  {
    hostCommandManager_t manager = {0};
    events_hostDataAvailable_t data_available = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};
    // Declare two bytes but supply three; CRC covers all present bytes.
    uint8_t decoded[] = {3U, 0U, 2U, 0U, 0x11U, 0U, 0x33U, 0U, 0U};
    uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(decoded))];
    uint16_t crc;
    uint16_t encoded_length;

    crc = EMF_crc_16CCITT(decoded, sizeof(decoded) - 2U);
    EMF_endian_u16WriteBuffLE(&decoded[sizeof(decoded) - 2U], &crc);
    EMF_cobs_encode(decoded, sizeof(decoded), encoded, &encoded_length);
    EMF_byteFifo_push(&rxFifo, encoded, encoded_length);
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    // A complete, CRC-correct frame still fails the inherited length contract.
    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(publicationCount == 0U);
    ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
  }

  ETF_TEST(declaredPayloadTooLongIsDiscarded)
  {
    hostCommandManager_t manager = {0};
    events_hostDataAvailable_t data_available = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};
    // Declare four bytes but supply three; no missing-byte CRC fiction.
    uint8_t decoded[] = {3U, 0U, 4U, 0U, 0x11U, 0U, 0x33U, 0U, 0U};
    uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(decoded))];
    uint16_t crc;
    uint16_t encoded_length;

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

  ETF_TEST(malformedCobsIsDiscarded)
  {
    hostCommandManager_t manager = {0};
    events_hostDataAvailable_t data_available = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};
    // Code 8 promises seven nonzero bytes but only six precede the delimiter.
    uint8_t encoded[] = {8U, 3U, 1U, 1U, 1U, 1U, 1U, 0U};

    EMF_byteFifo_push(&rxFifo, encoded, sizeof(encoded));
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    // Validate the SWU reaction, not the COBS library's implementation.
    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(publicationCount == 0U);
    ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
  }

  ETF_TEST(decodedFrameBelowMinimumIsDiscarded)
  {
    hostCommandManager_t manager = {0};
    events_hostDataAvailable_t data_available = {
      .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};
    // Five decoded bytes cannot contain the four-byte header and two-byte CRC.
    uint8_t decoded[] = {3U, 0U, 0U, 0U, 0x11U};
    uint8_t encoded[EMF_COBS_ENCODED_SIZE(sizeof(decoded))];
    uint16_t encoded_length;

    EMF_cobs_encode(decoded, sizeof(decoded), encoded, &encoded_length);
    EMF_byteFifo_push(&rxFifo, encoded, encoded_length);
    hostCommandManagerActivities_handleHostDataAvailable(&manager,
                                                         &data_available);

    ETF_VERIFY(EMF_byteFifo_isEmpty(&rxFifo));
    ETF_VERIFY(publicationCount == 0U);
    ETF_VERIFY(hostCommandManagerHal_write_fake.call_count == 0U);
  }
}
