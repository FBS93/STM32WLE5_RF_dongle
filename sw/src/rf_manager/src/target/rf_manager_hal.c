/*******************************************************************************
 * @brief STM32WLE5 integrated-radio adaptation for rf_manager.
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
#include <stdbool.h>
#include <stdint.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */
#include "eaf.h"
#include "ebf.h"
#include "edf.h"
#include "sx126x.h"
#include "sx126x_bpsk.h"
#include "sx126x_hal.h"

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "rf_manager_hal.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/** @brief Access a 32-bit memory-mapped register.
 *
 * volatile_use: hardware_interaction
 */
#define REG32(address) (*(volatile uint32_t*)(address))
/** @brief Access an 8-bit memory-mapped register.
 *
 * volatile_use: hardware_interaction
 */
#define REG8(address) (*(volatile uint8_t*)(address))
/** @brief RCC AHB2 peripheral clock-enable register. */
#define RCC_AHB2ENR REG32(0x5800004CUL)
/** @brief RCC APB3 peripheral clock-enable register. */
#define RCC_APB3ENR REG32(0x58000064UL)
/** @brief RCC peripheral clock configuration register. */
#define RCC_CCIPR REG32(0x58000088UL)
/** @brief PWR status register 2. */
#define PWR_SR2 REG32(0x58000414UL)
/** @brief PWR sub-GHz SPI control register. */
#define PWR_SUBGHZSPICR REG32(0x58000490UL)
/** @brief GPIOA mode register. */
#define GPIOA_MODER REG32(0x48000000UL)
/** @brief GPIOA bit set/reset register. */
#define GPIOA_BSRR REG32(0x48000018UL)
/** @brief SUBGHZSPI control register 1. */
#define SUBGHZSPI_CR1 REG32(0x58010000UL)
/** @brief SUBGHZSPI control register 2. */
#define SUBGHZSPI_CR2 REG32(0x58010004UL)
/** @brief SUBGHZSPI status register. */
#define SUBGHZSPI_SR REG32(0x58010008UL)
/** @brief SUBGHZSPI byte data register. */
#define SUBGHZSPI_DR8 REG8(0x5801000CUL)
/** @brief NVIC interrupt set-enable register 2. */
#define NVIC_ISER2 REG32(0xE000E108UL)
/** @brief Base address of the byte-addressed NVIC priority registers. */
#define NVIC_IPR_BASE 0xE000E400UL
/** @brief Integrated-radio interrupt number. */
#define RADIO_IRQ_NUMBER 74U
/** @brief Integrated-radio enable bit in NVIC_ISER2. */
#define RADIO_IRQ_MASK (1UL << (RADIO_IRQ_NUMBER - 64U))
/** @brief Radio indications routed to the integrated-radio interrupt. */
#define RADIO_IRQS                                                        \
  (SX126X_IRQ_TX_DONE | SX126X_IRQ_RX_DONE | SX126X_IRQ_HEADER_ERROR |    \
   SX126X_IRQ_CRC_ERROR | SX126X_IRQ_CAD_DONE | SX126X_IRQ_CAD_DETECTED | \
   SX126X_IRQ_TIMEOUT)
/** @brief SetModulationParams radio command. */
#define SET_MODULATION_PARAMETERS_COMMAND 0x8BU
/** @brief GetPacketStatus radio command. */
#define GET_PACKET_STATUS_COMMAND 0x14U
/** @brief Radio no-operation command byte. */
#define RADIO_NOP 0x00U
/** @brief Integrated-radio crystal frequency in hertz. */
#define RADIO_XTAL_FREQUENCY_HZ 32000000UL
/** @brief GPIOA clock-enable bit. */
#define RCC_AHB2ENR_GPIOAEN (1UL << 0U)

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/** @brief Define static file name string for asserts. */
EAF_DEFINE_THIS_FILE(__FILE__);
/** @brief Opaque context passed to the Semtech driver. */
static const uint8_t radioContext;
/** @brief Radio wake-up command stored in ROM. */
static const uint8_t wakeupCommand[2] = {0xC0U, 0U};
/** @brief Registered asynchronous radio callbacks. */
static const rfManagerHal_callbacks_t* callbacks;
/** @brief Caller context passed to asynchronous radio callbacks. */
static void* callbackContext;
/** @brief Whether the active radio command is a TX command.
 *
 * volatile_use: asynchronous_interaction
 */
static volatile bool transmitting;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/** @brief Selects the external TX or RX/idle RF path. */
static void setRfSwitch(bool tx);

/** @brief Transfers one byte through SUBGHZSPI. */
static uint8_t transfer(uint8_t value);

/** @brief Programs packet parameters for one packet operation. */
static void setPacketParameters(const rfTypes_configuration_t* configuration,
                                uint8_t payload_length);

/** @brief Programs GFSK modulation with an exact encoded deviation. */
static void setFskModulationParameters(
  const rfTypes_configurationFsk_t* configuration);

/** @brief Gets the operation timeout selected by the packet type. */
static uint32_t getOperationTimeout(
  const rfTypes_configuration_t* configuration);

/** @brief Reads the raw three-byte packet status. */
static void getPacketStatus(uint8_t status[3]);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static void setRfSwitch(bool tx)
{
  // GPIO BSRR sets PA4 with bit 4 and resets PA4 with bit 20.
  GPIOA_BSRR = tx ? (1UL << 4U) : (1UL << (4U + 16U));
}

static uint8_t transfer(uint8_t value)
{
  uint8_t received;

  while ((SUBGHZSPI_SR & (1UL << 1U)) == 0UL)
  {
  }
  SUBGHZSPI_DR8 = value;
  while ((SUBGHZSPI_SR & (1UL << 0U)) == 0UL)
  {
  }
  received = SUBGHZSPI_DR8;

  return received;
}

static void setPacketParameters(const rfTypes_configuration_t* configuration,
                                uint8_t payload_length)
{
  rfTypes_configurationLora_t lora_configuration;
  rfTypes_configurationFsk_t fsk_configuration;
  rfTypes_configurationMsk_t msk_configuration;
  sx126x_pkt_params_lora_t lora_packet;
  sx126x_pkt_params_gfsk_t gfsk_packet;
  sx126x_pkt_params_bpsk_t bpsk_packet;
  sx126x_status_t status;

  EAF_ASSERT(configuration != NULL);

  if (configuration->packet_type == RF_TYPES_PACKET_TYPE_LORA)
  {
    lora_configuration = configuration->packet_type_configuration.lora;
    lora_packet = (sx126x_pkt_params_lora_t){
      lora_configuration.preamble_length,
      (sx126x_lora_pkt_len_modes_t)lora_configuration.header_mode,
      payload_length,
      lora_configuration.payload_crc_enabled != 0U,
      lora_configuration.invert_iq != 0U};

    status = sx126x_set_lora_pkt_params(&radioContext, &lora_packet);
    EAF_ASSERT(status == SX126X_STATUS_OK);
  }
  else if (configuration->packet_type == RF_TYPES_PACKET_TYPE_FSK)
  {
    fsk_configuration = configuration->packet_type_configuration.fsk;
    gfsk_packet = (sx126x_pkt_params_gfsk_t){
      fsk_configuration.preamble_length,
      (sx126x_gfsk_preamble_detector_t)
        fsk_configuration.preamble_detection_length_bits,
      fsk_configuration.sync_word_length_bits,
      (sx126x_gfsk_address_filtering_t)fsk_configuration.address_filtering,
      (sx126x_gfsk_pkt_len_modes_t)fsk_configuration.payload_length_mode,
      payload_length,
      (sx126x_gfsk_crc_types_t)fsk_configuration.crc_type,
      fsk_configuration.whitening_enabled ? SX126X_GFSK_DC_FREE_WHITENING :
                                            SX126X_GFSK_DC_FREE_OFF};

    status = sx126x_set_gfsk_pkt_params(&radioContext, &gfsk_packet);
    EAF_ASSERT(status == SX126X_STATUS_OK);
  }
  else if (configuration->packet_type == RF_TYPES_PACKET_TYPE_BPSK)
  {
    bpsk_packet = (sx126x_pkt_params_bpsk_t){payload_length, 0U, 0U, 0U};

    status = sx126x_set_bpsk_pkt_params(&radioContext, &bpsk_packet);
    EAF_ASSERT(status == SX126X_STATUS_OK);
  }
  else if (configuration->packet_type == RF_TYPES_PACKET_TYPE_MSK)
  {
    msk_configuration = configuration->packet_type_configuration.msk;
    gfsk_packet = (sx126x_pkt_params_gfsk_t){
      msk_configuration.preamble_length,
      SX126X_GFSK_PREAMBLE_DETECTOR_OFF,
      msk_configuration.sync_word_length_bits,
      (sx126x_gfsk_address_filtering_t)msk_configuration.address_filtering,
      (sx126x_gfsk_pkt_len_modes_t)msk_configuration.payload_length_mode,
      payload_length,
      (sx126x_gfsk_crc_types_t)msk_configuration.crc_type,
      msk_configuration.whitening_enabled ? SX126X_GFSK_DC_FREE_WHITENING :
                                            SX126X_GFSK_DC_FREE_OFF};

    status = sx126x_set_gfsk_pkt_params(&radioContext, &gfsk_packet);
    EAF_ASSERT(status == SX126X_STATUS_OK);
  }
  else
  {
    EAF_ERROR();
  }
}

static void setFskModulationParameters(
  const rfTypes_configurationFsk_t* configuration)
{
  uint32_t bitrate;
  uint32_t deviation;
  uint8_t command[9];
  sx126x_hal_status_t hal_status;
  sx126x_status_t status;

  EAF_ASSERT(configuration != NULL);
  EAF_ASSERT(configuration->bit_rate_bps > 0U);

  bitrate = (32UL * RADIO_XTAL_FREQUENCY_HZ) / configuration->bit_rate_bps;
  deviation = configuration->frequency_deviation_hz;
  command[0] = SET_MODULATION_PARAMETERS_COMMAND;
  command[1] = (uint8_t)(bitrate >> 16U);
  command[2] = (uint8_t)(bitrate >> 8U);
  command[3] = (uint8_t)bitrate;
  command[4] = configuration->pulse_shape;
  command[5] = configuration->receive_bandwidth_khz;
  command[6] = (uint8_t)(deviation >> 16U);
  command[7] = (uint8_t)(deviation >> 8U);
  command[8] = (uint8_t)deviation;

  hal_status =
    sx126x_hal_write(&radioContext, command, sizeof(command), NULL, 0U);
  EAF_ASSERT(hal_status == SX126X_HAL_STATUS_OK);

  status = sx126x_tx_modulation_workaround(
    &radioContext, SX126X_PKT_TYPE_GFSK, (sx126x_lora_bw_t)0U);
  EAF_ASSERT(status == SX126X_STATUS_OK);
}

static uint32_t getOperationTimeout(
  const rfTypes_configuration_t* configuration)
{
  uint32_t timeout;

  EAF_ASSERT(configuration != NULL);

  switch (configuration->packet_type)
  {
    case RF_TYPES_PACKET_TYPE_LORA:
      timeout = configuration->packet_type_configuration.lora.operation_timeout;
      break;
    case RF_TYPES_PACKET_TYPE_FSK:
      timeout = configuration->packet_type_configuration.fsk.operation_timeout;
      break;
    case RF_TYPES_PACKET_TYPE_BPSK:
      timeout = configuration->packet_type_configuration.bpsk.operation_timeout;
      break;
    case RF_TYPES_PACKET_TYPE_MSK:
      timeout = configuration->packet_type_configuration.msk.operation_timeout;
      break;
    default:
      timeout = 0U;
      EAF_ERROR();
      break;
  }

  return timeout;
}

static void getPacketStatus(uint8_t status[3])
{
  uint8_t command[2];
  sx126x_hal_status_t hal_status;

  EAF_ASSERT(status != NULL);

  command[0] = GET_PACKET_STATUS_COMMAND;
  command[1] = RADIO_NOP;
  hal_status =
    sx126x_hal_read(&radioContext, command, sizeof(command), status, 3U);
  EAF_ASSERT(hal_status == SX126X_HAL_STATUS_OK);
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

sx126x_hal_status_t sx126x_hal_write(const void* context,
                                     const uint8_t* command,
                                     uint16_t command_length,
                                     const uint8_t* data,
                                     uint16_t data_length)
{
  uint16_t index;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(context != NULL);
  EAF_ASSERT_IN_BLOCK((command != NULL) || (command_length == 0U));
  EAF_ASSERT_IN_BLOCK((data != NULL) || (data_length == 0U));
  EAF_ASSERT_BLOCK_END();

  while ((PWR_SR2 & (1UL << 8U)) != 0UL)
  {
  }

  // Clear NSS before transferring command and payload bytes.
  PWR_SUBGHZSPICR &= ~(uint32_t)1UL;
  for (index = 0U; index < command_length; ++index)
  {
    (void)transfer(command[index]);
  }
  for (index = 0U; index < data_length; ++index)
  {
    (void)transfer(data[index]);
  }

  while ((SUBGHZSPI_SR & (1UL << 7U)) != 0UL)
  {
  }
  // Set NSS after the SPI peripheral is no longer busy.
  PWR_SUBGHZSPICR |= 1UL;

  return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_read(const void* context,
                                    const uint8_t* command,
                                    uint16_t command_length,
                                    uint8_t* data,
                                    uint16_t data_length)
{
  uint16_t index;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(context != NULL);
  EAF_ASSERT_IN_BLOCK((command != NULL) || (command_length == 0U));
  EAF_ASSERT_IN_BLOCK((data != NULL) || (data_length == 0U));
  EAF_ASSERT_BLOCK_END();

  while ((PWR_SR2 & (1UL << 8U)) != 0UL)
  {
  }

  // Clear NSS before transferring command and payload bytes.
  PWR_SUBGHZSPICR &= ~(uint32_t)1UL;
  for (index = 0U; index < command_length; ++index)
  {
    (void)transfer(command[index]);
  }
  for (index = 0U; index < data_length; ++index)
  {
    data[index] = transfer(0U);
  }

  while ((SUBGHZSPI_SR & (1UL << 7U)) != 0UL)
  {
  }
  // Set NSS after the SPI peripheral is no longer busy.
  PWR_SUBGHZSPICR |= 1UL;

  return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_reset(const void* context)
{
  EAF_ASSERT(context != NULL);

  return SX126X_HAL_STATUS_OK;
}

sx126x_hal_status_t sx126x_hal_wakeup(const void* context)
{
  sx126x_hal_status_t status;

  EAF_ASSERT(context != NULL);

  status =
    sx126x_hal_write(context, wakeupCommand, sizeof(wakeupCommand), NULL, 0U);

  return status;
}

void rfManagerHal_init(const rfManagerHal_callbacks_t* registered_callbacks,
                       void* context)
{
  sx126x_pa_cfg_params_t pa;
  sx126x_status_t status;

  EAF_ASSERT(registered_callbacks != NULL);

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(registered_callbacks->tx_done != NULL);
  EAF_ASSERT_IN_BLOCK(registered_callbacks->tx_timeout != NULL);
  EAF_ASSERT_IN_BLOCK(registered_callbacks->rx_done != NULL);
  EAF_ASSERT_IN_BLOCK(registered_callbacks->rx_timeout != NULL);
  EAF_ASSERT_IN_BLOCK(registered_callbacks->rx_error != NULL);
  EAF_ASSERT_IN_BLOCK(registered_callbacks->cad_done != NULL);
  EAF_ASSERT_IN_BLOCK(context != NULL);
  EAF_ASSERT_BLOCK_END();

  pa = (sx126x_pa_cfg_params_t){.pa_duty_cycle = 0x04U,
                                .hp_max = 0x07U,
                                .device_sel = 0x00U,
                                .pa_lut = 0x01U};

  callbacks = registered_callbacks;
  callbackContext = context;

  // Enable GPIOA and configure PA4 as the RF-switch output.
  RCC_AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
  // Readback ensures the GPIOA clock-enable write completes before GPIO access.
  (void)RCC_AHB2ENR;
  GPIOA_MODER = (GPIOA_MODER & ~(uint32_t)(3UL << 8U)) | (1UL << 8U);
  setRfSwitch(false);

  // Enable and configure the integrated SUBGHZSPI peripheral.
  RCC_APB3ENR |= 1UL;
  RCC_CCIPR &= ~(uint32_t)(3UL << 30U);
  PWR_SUBGHZSPICR |= 1UL;
  SUBGHZSPI_CR1 = (1UL << 2U) | (1UL << 6U);
  SUBGHZSPI_CR2 = (7UL << 8U) | (1UL << 12U);

  status = sx126x_set_standby(&radioContext, SX126X_STANDBY_CFG_RC);
  EAF_ASSERT(status == SX126X_STATUS_OK);

  status =
    sx126x_set_dio3_as_tcxo_ctrl(&radioContext, SX126X_TCXO_CTRL_1_8V, 3200U);
  EAF_ASSERT(status == SX126X_STATUS_OK);

  status = sx126x_set_reg_mode(&radioContext, SX126X_REG_MODE_LDO);
  EAF_ASSERT(status == SX126X_STATUS_OK);

  status = sx126x_set_pa_cfg(&radioContext, &pa);
  EAF_ASSERT(status == SX126X_STATUS_OK);

  status =
    sx126x_set_rx_tx_fallback_mode(&radioContext, SX126X_FALLBACK_STDBY_RC);
  EAF_ASSERT(status == SX126X_STATUS_OK);

  status =
    sx126x_set_dio_irq_params(&radioContext, RADIO_IRQS, RADIO_IRQS, 0U, 0U);
  EAF_ASSERT(status == SX126X_STATUS_OK);

  // Set the radio IRQ priority before enabling it in the NVIC.
  REG8(NVIC_IPR_BASE + RADIO_IRQ_NUMBER) = 0U;
  NVIC_ISER2 = RADIO_IRQ_MASK;
}

void rfManagerHal_setConfiguration(const rfTypes_configuration_t* configuration)
{
  rfTypes_configurationLora_t lora_configuration;
  rfTypes_configurationFsk_t fsk_configuration;
  rfTypes_configurationBpsk_t bpsk_configuration;
  rfTypes_configurationMsk_t msk_configuration;
  sx126x_mod_params_lora_t lora_modulation;
  sx126x_cad_params_t lora_cad;
  sx126x_mod_params_gfsk_t gfsk_modulation;
  sx126x_mod_params_bpsk_t bpsk_modulation;
  sx126x_status_t status;

  EAF_ASSERT(configuration != NULL);

  EAF_ASSERT(
    (configuration->packet_type != RF_TYPES_PACKET_TYPE_BPSK) ||
    (configuration->packet_type_configuration.bpsk.bit_rate_bps ==
     0x9C4000UL) ||
    (configuration->packet_type_configuration.bpsk.bit_rate_bps == 0x1A0AAAUL));

  // RF frequency is encoded in native PLL steps rather than hertz.
  status = sx126x_set_rf_freq_in_pll_steps(&radioContext,
                                           configuration->rf_frequency_hz);
  EAF_ASSERT(status == SX126X_STATUS_OK);

  if (configuration->packet_type == RF_TYPES_PACKET_TYPE_LORA)
  {
    lora_configuration = configuration->packet_type_configuration.lora;
    EAF_ASSERT((lora_configuration.synchronization_word == 0x1424U) ||
               (lora_configuration.synchronization_word == 0x3444U));
    lora_modulation = (sx126x_mod_params_lora_t){
      (sx126x_lora_sf_t)lora_configuration.spreading_factor,
      (sx126x_lora_bw_t)lora_configuration.bandwidth_khz,
      (sx126x_lora_cr_t)lora_configuration.coding_rate,
      lora_configuration.low_data_rate_optimize};
    lora_cad = (sx126x_cad_params_t){
      (sx126x_cad_symbs_t)lora_configuration.cad.scan_symbol_count,
      lora_configuration.cad.detection_peak_threshold,
      lora_configuration.cad.detection_minimum_threshold,
      (sx126x_cad_exit_modes_t)lora_configuration.cad.exit_mode,
      lora_configuration.cad.timeout};

    status = sx126x_set_pkt_type(&radioContext, SX126X_PKT_TYPE_LORA);
    EAF_ASSERT(status == SX126X_STATUS_OK);
    status = sx126x_set_lora_mod_params(&radioContext, &lora_modulation);
    EAF_ASSERT(status == SX126X_STATUS_OK);
    status = sx126x_set_lora_sync_word(
      &radioContext,
      (uint8_t)(((lora_configuration.synchronization_word >> 8U) & 0xF0U) |
                ((lora_configuration.synchronization_word >> 4U) & 0x0FU)));
    EAF_ASSERT(status == SX126X_STATUS_OK);
    status = sx126x_set_lora_symb_nb_timeout(&radioContext,
                                             lora_configuration.symbol_timeout);
    EAF_ASSERT(status == SX126X_STATUS_OK);
    status = sx126x_set_cad_params(&radioContext, &lora_cad);
    EAF_ASSERT(status == SX126X_STATUS_OK);

    status = sx126x_set_tx_params(
      &radioContext,
      configuration->tx_output_power_dbm,
      (sx126x_ramp_time_t)lora_configuration.tx_ramp_time_us);
    EAF_ASSERT(status == SX126X_STATUS_OK);
  }
  else if (configuration->packet_type == RF_TYPES_PACKET_TYPE_FSK)
  {
    fsk_configuration = configuration->packet_type_configuration.fsk;

    status = sx126x_set_pkt_type(&radioContext, SX126X_PKT_TYPE_GFSK);
    EAF_ASSERT(status == SX126X_STATUS_OK);

    setFskModulationParameters(&fsk_configuration);

    status = sx126x_set_gfsk_sync_word(&radioContext,
                                       fsk_configuration.sync_word,
                                       RF_TYPES_GENERIC_SYNC_WORD_SIZE);
    EAF_ASSERT(status == SX126X_STATUS_OK);

    status = sx126x_set_gfsk_pkt_address(&radioContext,
                                         fsk_configuration.node_address,
                                         fsk_configuration.broadcast_address);
    EAF_ASSERT(status == SX126X_STATUS_OK);

    status = sx126x_set_gfsk_crc_seed(&radioContext,
                                      fsk_configuration.crc_initial_value);
    EAF_ASSERT(status == SX126X_STATUS_OK);

    status = sx126x_set_gfsk_crc_polynomial(&radioContext,
                                            fsk_configuration.crc_polynomial);
    EAF_ASSERT(status == SX126X_STATUS_OK);

    status = sx126x_set_gfsk_whitening_seed(
      &radioContext, fsk_configuration.whitening_initial_value);
    EAF_ASSERT(status == SX126X_STATUS_OK);

    status = sx126x_set_tx_params(
      &radioContext,
      configuration->tx_output_power_dbm,
      (sx126x_ramp_time_t)fsk_configuration.tx_ramp_time_us);
    EAF_ASSERT(status == SX126X_STATUS_OK);
  }
  else if (configuration->packet_type == RF_TYPES_PACKET_TYPE_BPSK)
  {
    bpsk_configuration = configuration->packet_type_configuration.bpsk;
    bpsk_modulation = (sx126x_mod_params_bpsk_t){
      bpsk_configuration.bit_rate_bps == 0x9C4000UL ? 100U : 600U,
      SX126X_DBPSK_PULSE_SHAPE};

    status = sx126x_set_pkt_type(&radioContext, SX126X_PKT_TYPE_BPSK);
    EAF_ASSERT(status == SX126X_STATUS_OK);

    status = sx126x_set_bpsk_mod_params(&radioContext, &bpsk_modulation);
    EAF_ASSERT(status == SX126X_STATUS_OK);

    status = sx126x_set_tx_params(
      &radioContext, configuration->tx_output_power_dbm, SX126X_RAMP_10_US);
    EAF_ASSERT(status == SX126X_STATUS_OK);
  }
  else if (configuration->packet_type == RF_TYPES_PACKET_TYPE_MSK)
  {
    msk_configuration = configuration->packet_type_configuration.msk;
    gfsk_modulation = (sx126x_mod_params_gfsk_t){
      msk_configuration.bit_rate_bps,
      msk_configuration.bit_rate_bps / 4U,
      (sx126x_gfsk_pulse_shape_t)msk_configuration.pulse_shape,
      SX126X_GFSK_BW_4800};

    // Native GMSK uses raw packet type 0x03 with h=0.5 and Fdev=Rb/4.
    status = sx126x_set_pkt_type(&radioContext, (sx126x_pkt_type_t)0x03U);
    EAF_ASSERT(status == SX126X_STATUS_OK);

    status = sx126x_set_gfsk_mod_params(&radioContext, &gfsk_modulation);
    EAF_ASSERT(status == SX126X_STATUS_OK);

    status = sx126x_set_gfsk_sync_word(&radioContext,
                                       msk_configuration.sync_word,
                                       RF_TYPES_GENERIC_SYNC_WORD_SIZE);
    EAF_ASSERT(status == SX126X_STATUS_OK);

    status = sx126x_set_gfsk_pkt_address(&radioContext,
                                         msk_configuration.node_address,
                                         msk_configuration.broadcast_address);
    EAF_ASSERT(status == SX126X_STATUS_OK);

    status = sx126x_set_gfsk_crc_seed(&radioContext,
                                      msk_configuration.crc_initial_value);
    EAF_ASSERT(status == SX126X_STATUS_OK);

    status = sx126x_set_gfsk_crc_polynomial(&radioContext,
                                            msk_configuration.crc_polynomial);
    EAF_ASSERT(status == SX126X_STATUS_OK);

    status = sx126x_set_gfsk_whitening_seed(
      &radioContext, msk_configuration.whitening_initial_value);
    EAF_ASSERT(status == SX126X_STATUS_OK);

    status = sx126x_set_tx_params(
      &radioContext,
      configuration->tx_output_power_dbm,
      (sx126x_ramp_time_t)msk_configuration.tx_ramp_time_us);
    EAF_ASSERT(status == SX126X_STATUS_OK);
  }
  else
  {
    EAF_ERROR();
  }

  setPacketParameters(configuration, configuration->payload_length_bytes);
}

void rfManagerHal_calibrate(uint8_t lower, uint8_t upper)
{
  sx126x_status_t status;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(lower >= 0x26U);
  EAF_ASSERT_IN_BLOCK(upper <= 0xF0U);
  EAF_ASSERT_IN_BLOCK(lower <= upper);
  EAF_ASSERT_BLOCK_END();

  status = sx126x_cal(&radioContext, SX126X_CAL_ALL);
  EAF_ASSERT(status == SX126X_STATUS_OK);

  status = sx126x_cal_img(&radioContext, lower, upper);
  EAF_ASSERT(status == SX126X_STATUS_OK);
}

void rfManagerHal_transmit(const rfTypes_configuration_t* configuration,
                           const uint8_t* data,
                           uint8_t length)
{
  sx126x_status_t status;
  uint32_t timeout;

  EAF_ASSERT((configuration != NULL) && (data != NULL));

  timeout = getOperationTimeout(configuration);
  transmitting = true;
  setRfSwitch(true);

  setPacketParameters(configuration, length);

  status = sx126x_write_buffer(&radioContext, 0U, data, length);
  EAF_ASSERT(status == SX126X_STATUS_OK);

  status = sx126x_set_tx_with_timeout_in_rtc_step(&radioContext, timeout);
  EAF_ASSERT(status == SX126X_STATUS_OK);
}

void rfManagerHal_startReceive(const rfTypes_configuration_t* configuration)
{
  rfTypes_rxDutyCycle_t duty_cycle;
  uint32_t timeout;
  uint8_t mode;
  sx126x_status_t status;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(configuration != NULL);
  EAF_ASSERT_IN_BLOCK(
    (configuration->packet_type == RF_TYPES_PACKET_TYPE_LORA) ||
    (configuration->packet_type == RF_TYPES_PACKET_TYPE_FSK));
  EAF_ASSERT_BLOCK_END();

  if (configuration->packet_type == RF_TYPES_PACKET_TYPE_LORA)
  {
    mode = configuration->packet_type_configuration.lora.rx_mode;
    timeout = configuration->packet_type_configuration.lora.rx_timeout_control;
    duty_cycle = configuration->packet_type_configuration.lora.rx_duty_cycle;
  }
  else
  {
    mode = configuration->packet_type_configuration.fsk.rx_mode;
    timeout = configuration->packet_type_configuration.fsk.rx_timeout_control;
    duty_cycle = configuration->packet_type_configuration.fsk.rx_duty_cycle;
  }

  transmitting = false;
  setRfSwitch(false);

  setPacketParameters(configuration, configuration->payload_length_bytes);

  if (mode == 2U)
  {
    status = sx126x_set_rx_duty_cycle_with_timings_in_rtc_step(
      &radioContext, duty_cycle.rx_period, duty_cycle.sleep_period);
    EAF_ASSERT(status == SX126X_STATUS_OK);
  }
  else
  {
    status = sx126x_set_rx_with_timeout_in_rtc_step(
      &radioContext, mode == 1U ? SX126X_RX_CONTINUOUS : timeout);
    EAF_ASSERT(status == SX126X_STATUS_OK);
  }
}

void rfManagerHal_startCad(const rfTypes_configuration_t* configuration)
{
  sx126x_status_t status;

  EAF_ASSERT(configuration != NULL);
  EAF_ASSERT(configuration->packet_type == RF_TYPES_PACKET_TYPE_LORA);

  transmitting = false;
  setRfSwitch(false);

  if (configuration->packet_type_configuration.lora.cad.exit_mode != 0U)
  {
    setPacketParameters(configuration, configuration->payload_length_bytes);
  }

  status = sx126x_set_cad(&radioContext);
  EAF_ASSERT(status == SX126X_STATUS_OK);
}

void rfManagerHal_startContinuousWave(void)
{
  sx126x_status_t status;

  setRfSwitch(true);
  transmitting = true;

  status = sx126x_set_tx_cw(&radioContext);
  EAF_ASSERT(status == SX126X_STATUS_OK);
}

void rfManagerHal_startContinuousPreamble(void)
{
  sx126x_status_t status;

  setRfSwitch(true);
  transmitting = true;

  status = sx126x_set_tx_infinite_preamble(&radioContext);
  EAF_ASSERT(status == SX126X_STATUS_OK);
}

void rfManagerHal_stop(void)
{
  sx126x_status_t status;

  transmitting = false;

  status = sx126x_set_standby(&radioContext, SX126X_STANDBY_CFG_RC);
  EAF_ASSERT(status == SX126X_STATUS_OK);

  setRfSwitch(false);
}

uint8_t rfManagerHal_readReceivedPacket(
  uint8_t* data,
  uint8_t capacity,
  const rfTypes_configuration_t* configuration,
  rfTypes_packetStatus_t* packet_status)
{
  sx126x_rx_buffer_status_t buffer;
  uint8_t packet_status_raw[3];
  sx126x_status_t radio_status;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(configuration != NULL);
  EAF_ASSERT_IN_BLOCK(packet_status != NULL);
  EAF_ASSERT_IN_BLOCK(
    (configuration->packet_type == RF_TYPES_PACKET_TYPE_LORA) ||
    (configuration->packet_type == RF_TYPES_PACKET_TYPE_FSK));
  EAF_ASSERT_BLOCK_END();

  radio_status = sx126x_get_rx_buffer_status(&radioContext, &buffer);
  EAF_ASSERT(radio_status == SX126X_STATUS_OK);
  EAF_ASSERT(buffer.pld_len_in_bytes <= capacity);

  radio_status = sx126x_read_buffer(
    &radioContext, buffer.buffer_start_pointer, data, buffer.pld_len_in_bytes);
  EAF_ASSERT(radio_status == SX126X_STATUS_OK);

  getPacketStatus(packet_status_raw);

  if (configuration->packet_type == RF_TYPES_PACKET_TYPE_LORA)
  {
    packet_status->rssi = packet_status_raw[0];
    packet_status->packet_info = packet_status_raw[1];
  }
  else
  {
    packet_status->rssi = packet_status_raw[2];
    packet_status->packet_info = packet_status_raw[0];
  }

  return buffer.pld_len_in_bytes;
}

uint16_t rfManagerHal_getErrors(void)
{
  sx126x_status_t status;
  sx126x_errors_mask_t errors;

  status = sx126x_get_device_errors(&radioContext, &errors);
  EAF_ASSERT(status == SX126X_STATUS_OK);

  return errors;
}

/** @brief Handles integrated-radio interrupts owned by RF manager. */
void SUBGHZ_Radio_IRQHandler(void)
{
  sx126x_status_t status;
  sx126x_irq_mask_t irq;

  EAF_ASSERT((callbacks != NULL) && (callbackContext != NULL));

  status = sx126x_get_and_clear_irq_status(&radioContext, &irq);
  EAF_ASSERT(status == SX126X_STATUS_OK);

  if ((irq & SX126X_IRQ_TX_DONE) != 0U)
  {
    callbacks->tx_done(callbackContext);
  }
  if ((irq & (SX126X_IRQ_HEADER_ERROR | SX126X_IRQ_CRC_ERROR)) != 0U)
  {
    callbacks->rx_error(callbackContext);
  }
  else if ((irq & SX126X_IRQ_RX_DONE) != 0U)
  {
    callbacks->rx_done(callbackContext);
  }
  if ((irq & SX126X_IRQ_TIMEOUT) != 0U)
  {
    if (transmitting)
    {
      callbacks->tx_timeout(callbackContext);
    }
    else
    {
      callbacks->rx_timeout(callbackContext);
    }
  }
  if ((irq & SX126X_IRQ_CAD_DONE) != 0U)
  {
    callbacks->cad_done(callbackContext, (irq & SX126X_IRQ_CAD_DETECTED) != 0U);
  }

#if (EBF_CORE == EBF_CORE_BAREMETAL)
  EDF_ISRExit();
#endif
}
