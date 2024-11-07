#ifndef RADIO_HAL_H_
#define RADIO_HAL_H_

#include <stdint.h>
#include <stdbool.h>

/* Radio configuration */
typedef struct
{
  /*------------------ TX config ------------------*/
  uint8_t tx_modem;         /* Radio modem to be used in TX [0: FSK, 1: LoRa] */
  uint32_t tx_channel;      /* TX channel RF frequency (Frequency range: 150 MHz to 960 MHz) */
  uint32_t tx_bandwidth;    /* Sets the TX bandwidth (LoRa only)
                             * FSK: N/A (set to 0)
                             * LoRa: [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] */
  uint32_t tx_data_rate;    /* Sets the TX data rate
                             * FSK: 600..300000 bits/s
                             * LoRa: [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips] */
  uint8_t tx_code_rate;     /* Sets the TX coding rate (LoRa only)
                             * FSK : N/A (set to 0)
                             * LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8] */
  uint16_t tx_preamble_len; /* Sets the TX preamble length
                             * FSK : Number of bytes
                             * LoRa: Length in symbols (the hardware adds 4 more symbols) */
  bool tx_fixed_len;        /* TX fixed length packets [false: variable, true: fixed] */
  bool tx_crc_on;           /* Enables/Disables the CRC on TX [false: OFF, true: ON] */
  bool tx_iq_inverted;      /* Inverts IQ signals on TX (LoRa only)
                             * FSK : N/A (set to 0)
                             * LoRa: [false: not inverted, true: inverted] */
  int8_t tx_power;          /* Sets the TX output power [from -9dBm to +22dBm] */
  uint32_t tx_fdev;         /* Sets the TX frequency deviation (FSK only)
                             * FSK : [Hz]
                             * LoRa: N/A (set to 0) */
  uint32_t tx_timeout;      /* TX timeout [ms] */

  /*------------------ RX config ------------------*/
  uint8_t rx_modem;         /* Radio modem to be used in TX [0: FSK, 1: LoRa] */
  uint32_t rx_channel;      /* RX channel RF frequency (Frequency range: 150 MHz to 960 MHz) */
  uint32_t rx_bandwidth;    /* Sets the RX bandwidth
                             * FSK: >= 2600 and <= 250000 Hz
                             * LoRa: [0: 125 kHz, 1: 250 kHz, 2: 500 kHz, 3: Reserved] */
  uint32_t rx_data_rate;    /* Sets the RX data rate
                             * FSK: 600..300000 bits/s
                             * LoRa: [6: 64, 7: 128, 8: 256, 9: 512, 10: 1024, 11: 2048, 12: 4096  chips] */
  uint8_t rx_code_rate;     /* Sets the RX coding rate (LoRa only)
                             * FSK : N/A (set to 0)
                             * LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8] */
  uint16_t rx_preamble_len; /* Sets the RX preamble length
                             * FSK : Number of bytes
                             * LoRa: Length in symbols (the hardware adds 4 more symbols) */
  bool rx_fixed_len;        /* RX fixed length packets [false: variable, true: fixed] */
  bool rx_crc_on;           /* Enables/Disables the CRC on RX [false: OFF, true: ON] */
  bool rx_iq_inverted;      /* Inverts IQ signals on RX (LoRa only)
                             * FSK : N/A (set to 0)
                             * LoRa: [false: not inverted, true: inverted] */
  uint8_t rx_payload_len;   /* Sets RX payload length when fixed length is used */
} s_radioHAL_config_t;

void radioHAL_init(void);
void radioHAL_setConfig(s_radioHAL_config_t *s_config);
void radioHAL_read(uint16_t *pu16_len, uint8_t *u8_data);
void radioHAL_write(uint8_t u8_len, uint8_t *u8_data);

#endif /* RADIO_HAL_H_ */
