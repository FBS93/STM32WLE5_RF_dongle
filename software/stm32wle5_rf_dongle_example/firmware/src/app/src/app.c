#include <stdint.h>

#include "app.h"
#include "command_manager.h"
#include "uart_hal.h"
#include "radio_hal.h"

static void app_commandReceivedCallback(commandManager_hostToTargetCommands_t e_command, uint8_t u8_len, uint8_t *pu8_data)
{
  s_radioHAL_config_t radio_config;
  uint8_t index;

  switch (e_command)
  {
  case SET_CONFIG:
    index = 0;

    radio_config.tx_modem = pu8_data[index++];

    radio_config.tx_channel = 0;
    radio_config.tx_channel |= (uint32_t)pu8_data[index++] << 24;
    radio_config.tx_channel |= (uint32_t)pu8_data[index++] << 16;
    radio_config.tx_channel |= (uint32_t)pu8_data[index++] << 8;
    radio_config.tx_channel |= (uint32_t)pu8_data[index++];

    radio_config.tx_bandwidth = 0;
    radio_config.tx_bandwidth |= (uint32_t)pu8_data[index++] << 24;
    radio_config.tx_bandwidth |= (uint32_t)pu8_data[index++] << 16;
    radio_config.tx_bandwidth |= (uint32_t)pu8_data[index++] << 8;
    radio_config.tx_bandwidth |= (uint32_t)pu8_data[index++];

    radio_config.tx_data_rate = 0;
    radio_config.tx_data_rate |= (uint32_t)pu8_data[index++] << 24;
    radio_config.tx_data_rate |= (uint32_t)pu8_data[index++] << 16;
    radio_config.tx_data_rate |= (uint32_t)pu8_data[index++] << 8;
    radio_config.tx_data_rate |= (uint32_t)pu8_data[index++];

    radio_config.tx_code_rate = pu8_data[index++];

    radio_config.tx_preamble_len = 0;
    radio_config.tx_preamble_len |= (uint16_t)pu8_data[index++] << 8;
    radio_config.tx_preamble_len |= (uint16_t)pu8_data[index++];

    radio_config.tx_fixed_len = pu8_data[index++] != 0;
    radio_config.tx_crc_on = pu8_data[index++] != 0;
    radio_config.tx_iq_inverted = pu8_data[index++] != 0;

    radio_config.tx_power = (int8_t)pu8_data[index++];

    radio_config.tx_fdev = 0;
    radio_config.tx_fdev |= (uint32_t)pu8_data[index++] << 24;
    radio_config.tx_fdev |= (uint32_t)pu8_data[index++] << 16;
    radio_config.tx_fdev |= (uint32_t)pu8_data[index++] << 8;
    radio_config.tx_fdev |= (uint32_t)pu8_data[index++];

    radio_config.tx_timeout = 0;
    radio_config.tx_timeout |= (uint32_t)pu8_data[index++] << 24;
    radio_config.tx_timeout |= (uint32_t)pu8_data[index++] << 16;
    radio_config.tx_timeout |= (uint32_t)pu8_data[index++] << 8;
    radio_config.tx_timeout |= (uint32_t)pu8_data[index++];

    radio_config.rx_modem = pu8_data[index++];

    radio_config.rx_channel = 0;
    radio_config.rx_channel |= (uint32_t)pu8_data[index++] << 24;
    radio_config.rx_channel |= (uint32_t)pu8_data[index++] << 16;
    radio_config.rx_channel |= (uint32_t)pu8_data[index++] << 8;
    radio_config.rx_channel |= (uint32_t)pu8_data[index++];

    radio_config.rx_bandwidth = 0;
    radio_config.rx_bandwidth |= (uint32_t)pu8_data[index++] << 24;
    radio_config.rx_bandwidth |= (uint32_t)pu8_data[index++] << 16;
    radio_config.rx_bandwidth |= (uint32_t)pu8_data[index++] << 8;
    radio_config.rx_bandwidth |= (uint32_t)pu8_data[index++];

    radio_config.rx_data_rate = 0;
    radio_config.rx_data_rate |= (uint32_t)pu8_data[index++] << 24;
    radio_config.rx_data_rate |= (uint32_t)pu8_data[index++] << 16;
    radio_config.rx_data_rate |= (uint32_t)pu8_data[index++] << 8;
    radio_config.rx_data_rate |= (uint32_t)pu8_data[index++];

    radio_config.rx_code_rate = pu8_data[index++];

    radio_config.rx_preamble_len = 0;
    radio_config.rx_preamble_len |= (uint16_t)pu8_data[index++] << 8;
    radio_config.rx_preamble_len |= (uint16_t)pu8_data[index++];

    radio_config.rx_fixed_len = pu8_data[index++] != 0;
    radio_config.rx_crc_on = pu8_data[index++] != 0;
    radio_config.rx_iq_inverted = pu8_data[index++] != 0;

    radio_config.rx_payload_len = pu8_data[index++];

    radioHAL_setConfig(&radio_config);
    break;
  case SEND_DATA:
    radioHAL_write(u8_len, pu8_data);
    break;
  default:
    break;
  }
}

void app_init(void)
{
  radioHAL_init();
  uartHAL_init();
  commandManager_init(app_commandReceivedCallback,
                      uartHAL_read,
                      uartHAL_write);
}

void app_executePeriodicTask(void)
{
  uint8_t pu8_data[COMMAND_MANAGER_MAX_PAYLOAD_SIZE];
  uint16_t u16_len;

  radioHAL_read(&u16_len, pu8_data);

  /* If there is new data received from radio interface send it to host */
  if (u16_len > 0)
  {
    commandManager_targetToHostInput(DATA_RECEIVED, (uint8_t)u16_len, pu8_data);
  }

  /* Execute periodic task of commandManager */
  commandManager_executePeriodicTask();
}
