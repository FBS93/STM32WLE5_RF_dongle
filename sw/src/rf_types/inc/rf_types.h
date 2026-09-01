/*******************************************************************************
 * @brief Shared RF types.
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

#ifndef RF_TYPES_H
#define RF_TYPES_H

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */
#include <stdint.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/** @brief Capacity of the generic sync-word array in bytes. */
#define RF_TYPES_GENERIC_SYNC_WORD_SIZE 8U

/** @brief Capacity of RF TX and RX packet-data arrays in bytes. */
#define RF_TYPES_PACKET_DATA_CAPACITY 255U

/** @brief Generic FSK/GFSK packet-type encoding. */
#define RF_TYPES_PACKET_TYPE_FSK 0x00U

/** @brief LoRa packet-type encoding. */
#define RF_TYPES_PACKET_TYPE_LORA 0x01U

/** @brief BPSK packet-type encoding. */
#define RF_TYPES_PACKET_TYPE_BPSK 0x02U

/** @brief Generic MSK/GMSK packet-type encoding. */
#define RF_TYPES_PACKET_TYPE_MSK 0x03U

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/** @brief Host-visible RF operation encoding. */
typedef enum
{
  RF_TYPES_OPERATION_IDLE = 0x00U,                    //!< No RF operation.
  RF_TYPES_OPERATION_PACKET_TX = 0x01U,               //!< Packet TX.
  RF_TYPES_OPERATION_RX = 0x02U,                      //!< Packet RX.
  RF_TYPES_OPERATION_CAD = 0x03U,                     //!< LoRa CAD.
  RF_TYPES_OPERATION_CONTINUOUS_WAVE_TX = 0x04U,      //!< Continuous-wave TX.
  RF_TYPES_OPERATION_CONTINUOUS_PREAMBLE_TX = 0x05U,  //!< Preamble TX.
  RF_TYPES_OPERATION_CALIBRATION = 0x06U              //!< Radio calibration.
} rfTypes_operation_t;

/**
 * @brief @c sysp_rxDutyCycle parameter from SYS_REQ_PARAMETER_32.
 *
 * Values use the upstream 15.625 microsecond encoding. Only the least
 * significant 24 bits of each member are used; no packed layout is implied.
 */
typedef struct
{
  uint32_t rx_period;     //!< RX period: 0 means until_packet; otherwise 1..0xFFFFFF.
  uint32_t sleep_period;  //!< Sleep period, encoded range 0..0xFFFFFF.
} rfTypes_rxDutyCycle_t;

/**
 * @brief @c sysp_cad parameter from SYS_REQ_PARAMETER_34.
 *
 * The timeout uses the upstream 15.625 microsecond encoding. Only its least
 * significant 24 bits are used; no packed layout is implied.
 */
typedef struct
{
  uint8_t scan_symbol_count;          //!< Encodings 0..4 select 1, 2, 4, 8, 16 symbols.
  uint8_t detection_peak_threshold;   //!< Encoded range 0..255.
  uint8_t detection_minimum_threshold;  //!< Encoded range 0..255.
  uint8_t exit_mode;                  //!< 0: stop_on_detection; 1: receive_on_detection.
  uint32_t timeout;                   //!< CAD timeout, encoded range 0..0xFFFFFF.
} rfTypes_cad_t;

/** @brief LoRa packet-type block of the semantic RF configuration. */
typedef struct
{
  uint8_t spreading_factor;       //!< sysp_loraSpreadingFactor; 5..12.
  uint8_t bandwidth_khz;           //!< sysp_loraBandwidthKHz encoded enumeration.
  uint8_t coding_rate;             //!< sysp_loraCodingRate encoding 0..4.
  uint8_t low_data_rate_optimize;  //!< sysp_loraLowDataRateOptimize; 0/1.
  uint16_t preamble_length;        //!< sysp_preambleLength; 1..65535 symbols.
  uint8_t header_mode;             //!< sysp_loraHeaderMode; 0 explicit, 1 implicit.
  uint8_t payload_crc_enabled;     //!< sysp_loraPayloadCrcEnabled; 0/1.
  uint8_t invert_iq;               //!< sysp_loraInvertIq; 0 standard, 1 inverted.
  uint16_t synchronization_word;   //!< sysp_loraSynchronizationWord.
  uint32_t operation_timeout;      //!< sysp_operationTimeout; 24-bit, 15.625 us.
  uint8_t symbol_timeout;          //!< sysp_loraSymbolTimeout; 0..255 symbols.
  uint8_t tx_ramp_time_us;         //!< sysp_txRampTimeUs encoded enumeration.
  rfTypes_cad_t cad;               //!< sysp_cad settings.
  uint8_t rx_mode;                 //!< sysp_rxMode; 0 single, 1 continuous, 2 duty_cycle.
  uint32_t rx_timeout_control;      //!< sysp_rxTimeoutControl; 24-bit, 15.625 us.
  rfTypes_rxDutyCycle_t rx_duty_cycle;  //!< sysp_rxDutyCycle settings.
} rfTypes_configurationLora_t;

/** @brief Generic FSK/GFSK packet-type block of the semantic RF configuration. */
typedef struct
{
  uint32_t bit_rate_bps;             //!< sysp_genericBitRateBps; 24-bit bit/s.
  uint32_t frequency_deviation_hz;   //!< sysp_genericFrequencyDeviationHz; 24-bit.
  uint8_t pulse_shape;               //!< sysp_genericPulseShape encoding.
  uint8_t receive_bandwidth_khz;     //!< sysp_fskReceiveBandwidthKHz encoding.
  uint16_t preamble_length;          //!< sysp_preambleLength; 8..65535 bits.
  uint32_t operation_timeout;        //!< sysp_operationTimeout; 24-bit, 15.625 us.
  uint8_t payload_length_mode;       //!< sysp_genericPayloadLengthMode; 0/1.
  uint8_t preamble_detection_length_bits;  //!< sysp_genericPreambleDetectionLengthBits.
  uint8_t sync_word_length_bits;     //!< sysp_genericSyncWordLengthBits; 0..64.
  uint8_t sync_word[RF_TYPES_GENERIC_SYNC_WORD_SIZE];  //!< sysp_genericSyncWord.
  uint8_t address_filtering;         //!< sysp_genericAddressFiltering; 0..2.
  uint8_t node_address;              //!< sysp_genericNodeAddress; 0..255.
  uint8_t broadcast_address;         //!< sysp_genericBroadcastAddress; 0..255.
  uint8_t crc_type;                  //!< sysp_genericCrcType encoded enumeration.
  uint16_t crc_initial_value;        //!< sysp_genericCrcInitialValue.
  uint16_t crc_polynomial;           //!< sysp_genericCrcPolynomial.
  uint8_t whitening_enabled;         //!< sysp_genericWhiteningEnabled; 0/1.
  uint16_t whitening_initial_value;  //!< sysp_genericWhiteningInitialValue; 0..511.
  uint8_t tx_ramp_time_us;           //!< sysp_txRampTimeUs encoded enumeration.
  uint8_t rx_mode;                   //!< sysp_rxMode; 0 single, 1 continuous, 2 duty_cycle.
  uint32_t rx_timeout_control;       //!< sysp_rxTimeoutControl; 24-bit, 15.625 us.
  rfTypes_rxDutyCycle_t rx_duty_cycle;  //!< sysp_rxDutyCycle settings.
} rfTypes_configurationFsk_t;

/** @brief Generic MSK/GMSK packet-type block of the semantic RF configuration. */
typedef struct
{
  uint32_t bit_rate_bps;             //!< sysp_genericBitRateBps; 24-bit bit/s.
  uint8_t pulse_shape;               //!< sysp_genericPulseShape encoding.
  uint16_t preamble_length;          //!< sysp_preambleLength; 8..65535 bits.
  uint32_t operation_timeout;        //!< sysp_operationTimeout; 24-bit, 15.625 us.
  uint8_t payload_length_mode;       //!< sysp_genericPayloadLengthMode; 0/1.
  uint8_t sync_word_length_bits;     //!< sysp_genericSyncWordLengthBits; 0..64.
  uint8_t sync_word[RF_TYPES_GENERIC_SYNC_WORD_SIZE];  //!< sysp_genericSyncWord.
  uint8_t address_filtering;         //!< sysp_genericAddressFiltering; 0..2.
  uint8_t node_address;              //!< sysp_genericNodeAddress; 0..255.
  uint8_t broadcast_address;         //!< sysp_genericBroadcastAddress; 0..255.
  uint8_t crc_type;                  //!< sysp_genericCrcType encoded enumeration.
  uint16_t crc_initial_value;        //!< sysp_genericCrcInitialValue.
  uint16_t crc_polynomial;           //!< sysp_genericCrcPolynomial.
  uint8_t whitening_enabled;         //!< sysp_genericWhiteningEnabled; 0/1.
  uint16_t whitening_initial_value;  //!< sysp_genericWhiteningInitialValue; 0..511.
  uint8_t tx_ramp_time_us;            //!< sysp_txRampTimeUs encoded enumeration.
} rfTypes_configurationMsk_t;

/** @brief BPSK packet-type block of the semantic RF configuration. */
typedef struct
{
  uint32_t bit_rate_bps;       //!< sysp_bpskBitRateBps; 24-bit enumeration.
  uint32_t operation_timeout;  //!< sysp_operationTimeout; 24-bit, 15.625 us.
} rfTypes_configurationBpsk_t;

/** @brief Packet-type-specific semantic RF configuration block. */
typedef union
{
  rfTypes_configurationLora_t lora;  //!< Selected by RF_TYPES_PACKET_TYPE_LORA.
  rfTypes_configurationFsk_t fsk;    //!< Selected by RF_TYPES_PACKET_TYPE_FSK.
  rfTypes_configurationMsk_t msk;    //!< Selected by RF_TYPES_PACKET_TYPE_MSK.
  rfTypes_configurationBpsk_t bpsk;  //!< Selected by RF_TYPES_PACKET_TYPE_BPSK.
} rfTypes_configurationPacketType_t;

/** @brief Complete semantic RF configuration. */
typedef struct
{
  uint8_t packet_type;           //!< One RF_TYPES_PACKET_TYPE_* value.
  uint32_t rf_frequency_hz;      //!< Encoded RF frequency.
  uint8_t payload_length_bytes;  //!< Configured payload length.
  int8_t tx_output_power_dbm;    //!< Requested TX output power.
  rfTypes_configurationPacketType_t packet_type_configuration;
  //!< Configuration block selected by packet_type.
} rfTypes_configuration_t;

/** @brief Packet-type-specific information for a received packet. */
typedef union
{
  int8_t lora_snr_db;  //!< Raw signed quarter-dB LoRa SNR encoding.
  uint8_t fsk_status;  //!< Generic FSK/GFSK packet-status flags.
} rfTypes_receivedPacketInfo_t;

/** @brief Raw RSSI and packet-type-specific information for the last received packet. */
typedef struct
{
  uint8_t rssi;  //!< Raw RSSI encoding, with -0.5 dBm per count.
  rfTypes_receivedPacketInfo_t packet_info;  //!< Selected by configured packet type.
} rfTypes_packetStatus_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

#endif /* RF_TYPES_H */
