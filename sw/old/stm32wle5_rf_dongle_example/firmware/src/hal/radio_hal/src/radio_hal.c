#include <stdint.h>
#include <stdbool.h>

#include "stm32wlxx_hal.h"

#include "radio.h"
#include "radio_hal.h"
#include "fifo.h"

/* Max payload len */
#define RADIO_HAL_MAX_PAYLOAD_LEN ((uint8_t)254)

/* SubGHz module critical section*/
#define SUBGHZ_CRITICAL_SECTION_BEGIN() HAL_NVIC_DisableIRQ(SUBGHZ_Radio_IRQn);
#define SUBGHZ_CRITICAL_SECTION_END() HAL_NVIC_EnableIRQ(SUBGHZ_Radio_IRQn);

/* Radio events function pointers */
static RadioEvents_t s_radioHAL_eventCallbacks;

/* Radio configuration */
static s_radioHAL_config_t s_radioHAL_config;

/* Rx FIFO */
static s_fifo_t s_radioHAL_rxFIFO;

static void radioHAL_OnTxDone(void)
{
  // TODO: This may not be necessary because the radio automatically enters standby mode when an interrupt is triggered. Verify this behavior.
  Radio.Standby(); // Switch to standby mode to avoid new interruptions

  /* Set Rx config */
  Radio.SetRxConfig((RadioModems_t)s_radioHAL_config.rx_modem,
                    s_radioHAL_config.rx_bandwidth,
                    s_radioHAL_config.rx_data_rate,
                    s_radioHAL_config.rx_code_rate,
                    0,
                    s_radioHAL_config.rx_preamble_len,
                    0,
                    s_radioHAL_config.rx_fixed_len,
                    s_radioHAL_config.rx_payload_len,
                    s_radioHAL_config.rx_crc_on,
                    false,
                    0,
                    s_radioHAL_config.rx_iq_inverted,
                    true);

  /* Frequency channel calibration takes 1.6ms, so check if it is really necessary set Rx frequency channel */
  if (s_radioHAL_config.rx_channel != s_radioHAL_config.tx_channel)
  {
    Radio.SetChannel(s_radioHAL_config.rx_channel);
  }

  /* set Rx in continuous mode */
  Radio.Rx(0);
}

static void radioHAL_OnRxDone(uint8_t *pu8_payload, uint16_t u16_size, int16_t i16_rssi, int8_t i8_LoraSnr_FskCfo)
{
  // TODO: This may not be necessary because the radio automatically enters standby mode when an interrupt is triggered. Verify this behavior.
  Radio.Standby(); // Switch to standby mode to avoid new interruptions

  /* Push Rx data into FIFO */
  if (fifo_getAvailableSpace(&s_radioHAL_rxFIFO) >= u16_size)
  {
    fifo_push(&s_radioHAL_rxFIFO, u16_size, pu8_payload);
  }
  else
  {
    // FIFO overflow, error not managed
  }

  Radio.Rx(0); // set RX in continuous mode again
}

static void radioHAL_OnTxTimeout(void)
{
  // Tx timeout, error not managed
}

static void radioHAL_OnRxTimeout(void)
{
  // Rx timeout, error not managed
}

static void radioHAL_OnRxError(void)
{
  // Rx error, error not managed
}

void radioHAL_init(void)
{
  /* Rx FIFO initialization */
  fifo_init(&s_radioHAL_rxFIFO);

  /* Radio initialization */
  s_radioHAL_eventCallbacks.TxDone = radioHAL_OnTxDone;
  s_radioHAL_eventCallbacks.RxDone = radioHAL_OnRxDone;
  s_radioHAL_eventCallbacks.TxTimeout = radioHAL_OnTxTimeout;
  s_radioHAL_eventCallbacks.RxTimeout = radioHAL_OnRxTimeout;
  s_radioHAL_eventCallbacks.RxError = radioHAL_OnRxError;
  Radio.Init(&s_radioHAL_eventCallbacks);
}

void radioHAL_setConfig(s_radioHAL_config_t *s_config)
{
  /* Switch to standby mode to avoid interruptions during the configuration */
  Radio.Standby();

  /* Copy configuration */
  for (uint8_t u8_i = 0; u8_i < sizeof(s_radioHAL_config_t); u8_i++)
  {
    ((uint8_t *)(&s_radioHAL_config))[u8_i] = ((uint8_t *)s_config)[u8_i];
  }

  /* Set Rx config as default */
  Radio.SetRxConfig((RadioModems_t)s_radioHAL_config.rx_modem,
                    s_radioHAL_config.rx_bandwidth,
                    s_radioHAL_config.rx_data_rate,
                    s_radioHAL_config.rx_code_rate,
                    0,
                    s_radioHAL_config.rx_preamble_len,
                    0,
                    s_radioHAL_config.rx_fixed_len,
                    s_radioHAL_config.rx_payload_len,
                    s_radioHAL_config.rx_crc_on,
                    false,
                    0,
                    s_radioHAL_config.rx_iq_inverted,
                    true);
  Radio.SetChannel(s_radioHAL_config.rx_channel);

  /* set Rx in continuous mode */
  Radio.Rx(0);
}

void radioHAL_read(uint16_t *pu16_len, uint8_t *pu8_data)
{
  /* Enter critical section to protect the FIFO that is also used from SubGHz module IRQs */
  SUBGHZ_CRITICAL_SECTION_BEGIN();

  /* Get all bytes from FIFO */
  *pu16_len = fifo_getOccupiedSpace(&s_radioHAL_rxFIFO);
  fifo_pop(&s_radioHAL_rxFIFO, *pu16_len, pu8_data);

  /* Exit critical section */
  SUBGHZ_CRITICAL_SECTION_END();
}

void radioHAL_write(uint8_t u8_len, uint8_t *pu8_data)
{

  /* Blocks while Radio is in Tx mode to avoid losing information if this function is used in a loop */
  while (Radio.GetStatus() == RF_TX_RUNNING)
  {
  };

  if (u8_len <= RADIO_HAL_MAX_PAYLOAD_LEN)
  {
    /* Switch to standby mode to avoid interruptions during the configuration */
    Radio.Standby();

    /* Set Tx config */
    Radio.SetTxConfig((RadioModems_t)s_radioHAL_config.tx_modem,
                      s_radioHAL_config.tx_power,
                      s_radioHAL_config.tx_fdev,
                      s_radioHAL_config.tx_bandwidth,
                      s_radioHAL_config.tx_data_rate,
                      s_radioHAL_config.tx_code_rate,
                      s_radioHAL_config.tx_preamble_len,
                      s_radioHAL_config.tx_fixed_len,
                      s_radioHAL_config.tx_crc_on,
                      false,
                      0,
                      s_radioHAL_config.tx_iq_inverted,
                      s_radioHAL_config.tx_timeout);

    /* Frequency channel calibration takes 1.6ms, so check if it is really necessary set Tx frequency channel */
    if (s_radioHAL_config.tx_channel != s_radioHAL_config.rx_channel)
    {
      Radio.SetChannel(s_radioHAL_config.tx_channel);
    }

    /* Start Tx */
    Radio.Send(pu8_data, u8_len);
  }
}
