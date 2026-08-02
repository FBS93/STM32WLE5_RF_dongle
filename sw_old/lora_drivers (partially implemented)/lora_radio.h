#ifndef HAL_LORA_RADIO_H_
#define HAL_LORA_RADIO_H_

#include <stdint.h>
#include <stdbool.h>

// TODO: Review IRQ callback formats and names to ensure they match the intended format. Delete any unused callbacks.

/*!
 * \brief Radio driver callback functions
 */
typedef struct
{
    /*!
     * \brief  Tx Done callback prototype.
     */
    void (*tx_done)(void);
    /*!
     * \brief  Tx Timeout callback prototype.
     */
    void (*tx_timeout)(void);
    /*!
     * \brief Rx Done callback prototype.
     *
     * \param [in] payload Received buffer pointer
     * \param [in] size    Received buffer size
     * \param [in] rssi    Received Signal Strength Indicator (RSSI) value computed while receiving the frame [dBm]
     * \param [in] snr	   SNR value in dB
     */
    void (*rx_done)(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
    /*!
     * \brief  Rx Timeout callback prototype.
     */
    void (*rx_timeout)(void);
    /*!
     * \brief Rx Error callback prototype.
     */
    void (*rx_error)(void);
    /*!
     * \brief CAD Done callback prototype.
     *
     * \param [in] channelDetected    Channel Activity detected during the CAD
     */
    void (*cad_done)(bool channelActivityDetected);
} loraRadio_events_t;

/*!
 * \brief Represents the possible spreading factor values in LoRa packet types
 */
typedef enum
{
    LORA_SF5 = 0x05,
    LORA_SF6 = 0x06,
    LORA_SF7 = 0x07,
    LORA_SF8 = 0x08,
    LORA_SF9 = 0x09,
    LORA_SF10 = 0x0A,
    LORA_SF11 = 0x0B,
    LORA_SF12 = 0x0C,
} loraRadio_spreadingFactors_t;

/*!
 * \brief Represents the bandwidth values for LoRa packet type
 */
typedef enum
{
    LORA_BW_500 = 6,
    LORA_BW_250 = 5,
    LORA_BW_125 = 4,
    LORA_BW_062 = 3,
    LORA_BW_041 = 10,
    LORA_BW_031 = 2,
    LORA_BW_020 = 9,
    LORA_BW_015 = 1,
    LORA_BW_010 = 8,
    LORA_BW_007 = 0,
} loraRadio_bandwidths_t;

/*!
 * \brief Represents the coding rate values for LoRa packet type
 */
typedef enum
{
    LORA_CR_4_5 = 0x01,
    LORA_CR_4_6 = 0x02,
    LORA_CR_4_7 = 0x03,
    LORA_CR_4_8 = 0x04,
} loraRadio_codingRates_t;

/*!
 * \brief TODO
 */
typedef enum
{
    IMPLICIT_HEADER = 0x00,
    EXPLICIT_HEADER = 0x01,
} loraRadio_headerType_t;

/*!
 * \brief TODO
 */
typedef enum
{
    CRC_DISABLED = 0x00,
    CRC_ENABLED = 0x01,
} loraRadio_crcModes_t;

/*!
 * \brief TODO
 */
typedef enum
{
    STANDARD_IQ = 0x00,
    INVERTED_IQ = 0x01,
} loraRadio_iqModes_t;

/*!
 * \brief TODO
 */
typedef enum
{
    POWER_SAVING_MODE = 0x94,
    BOOST_MODE_LEVEL_1 = 0x95,
    BOOST_MODE_LEVEL_2 = 0x96,
    BOOST_MODE_MAX = 0x97,
} loraRadio_rxPowerMode_t;

/*!
 * \brief TODO
 */
typedef enum
{
    TX_SINGLE_MODE = 0x00,
    TX_SINGLE_WITH_TIMEOUT_MODE = 0x01,
} loraRadio_txModes_t;

/*!
 * \brief TODO
 */
typedef enum
{
    RX_CONTINUOUS_MODE = 0x00,
    RX_SINGLE_MODE = 0x01,
    RX_SINGLE_WITH_TIMEOUT_MODE = 0x02,
    RX_LISTENING_MODE = 0x03,
} loraRadio_rxModes_t;

void loraRadio_init(void);
void loraRadio_setListenersOnRadioEvents(loraRadio_events_t *radio_events);
void loraRadio_setTxParameters(loraRadio_spreadingFactors_t spreading_factor,
                               loraRadio_bandwidths_t bandwidth,
                               loraRadio_codingRates_t coding_rate,
                               uint32_t channel_freq,
                               uint16_t preamble_len,
                               loraRadio_headerType_t header_type,
                               loraRadio_crcModes_t crc_mode,
                               loraRadio_iqModes_t iq_mode,
                               int8_t tx_power_dBm,
                               loraRadio_txModes_t tx_mode,
                               uint32_t tx_timeout);
void loraRadio_setRxParameters(loraRadio_spreadingFactors_t spreading_factor,
                               loraRadio_bandwidths_t bandwidth,
                               loraRadio_codingRates_t coding_rate,
                               uint32_t channel_freq,
                               uint16_t preamble_len,
                               loraRadio_headerType_t header_type,
                               loraRadio_crcModes_t crc_mode,
                               loraRadio_iqModes_t iq_mode,
                               uint8_t symbol_timeout,
                               loraRadio_rxModes_t rx_mode,
                               loraRadio_rxPowerMode_t rx_power_mode,
                               uint32_t rx_timeout,
                               uint32_t listening_mode_rx_time,
                               uint32_t listening_mode_sleep_time);
void loraRadio_send(uint8_t *payload, uint8_t len);
void loraRadio_receive(uint8_t len);
void loraRadio_startTxContinuousWave(uint32_t channel_freq, int8_t power_dBm);
void loraRadio_stopTxContinuousWave(void);

#endif /* HAL_LORA_RADIO_H_ */
