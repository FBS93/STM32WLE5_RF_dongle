#ifndef SUBGHZ_REGISTERS_H_
#define SUBGHZ_REGISTERS_H_

/**
 * @brief  Sub-GHz radio RAM definition
 * @note   The sub-GHz radio peripheral RAM address can be accessed by sub-GHz radio command
 *         SUBGRF_WriteRegisters() and SUBGRF_ReadRegisters() "
 * @note   These RAM addresses are used to control accurately ramp-up, ramp-down and length of a frame
 */
/*Sub-GHz radio ramp-up High register*/
#define SUBGHZ_RAM_RAMPUPH ((uint16_t)0x00F0)
/*Sub-GHz radio ramp-up Low register*/
#define SUBGHZ_RAM_RAMPUPL ((uint16_t)0x00F1)
/*Sub-GHz radio ramp-down High register*/
#define SUBGHZ_RAM_RAMPDNH ((uint16_t)0x00F2)
/*Sub-GHz radio ramp-down Low register*/
#define SUBGHZ_RAM_RAMPDNL ((uint16_t)0x00F3)
/*Sub-GHz radio frame limit High register*/
#define SUBGHZ_RAM_FRAMELIMH ((uint16_t)0x00F4)
/*Sub-GHz radio frame limit Low register*/
#define SUBGHZ_RAM_FRAMELIML ((uint16_t)0x00F5)

/**
 * @brief  Sub-GHz radio register definition
 * @note   The sub-GHz radio peripheral registers can be accessed by sub-GHz radio command
 *         SUBGRF_WriteRegisters() and SUBGRF_ReadRegisters() "
 */
/*Sub-GHz radio generic bit synchronization register*/
#define SUBGHZ_GBSYNCR ((uint16_t)0x06AC)
/*Sub-GHz radio generic CFO High register */
#define SUBGHZ_GCFORH ((uint16_t)0x06B0)
/*Sub-GHz radio generic CFO Low register */
#define SUBGHZ_GCFORL ((uint16_t)0x06B1)
/*Sub-GHz radio generic pktCtl1 register*/
#define SUBGHZ_GPKTCTL1R ((uint16_t)0x06B4)
/*Sub-GHz radio generic packet control 1A register*/
#define SUBGHZ_GPKTCTL1AR ((uint16_t)0x06B8)
/*Sub-GHz radio generic whitening LSB register*/
#define SUBGHZ_GWHITEINIRL ((uint16_t)0x06B9)
/*Sub-GHz radio generic rtx register*/
#define SUBGHZ_GRTXPLDLEN ((uint16_t)0x06BB)
/*Sub-GHz radio generic CRC initial High register*/
#define SUBGHZ_GCRCINIRH ((uint16_t)0x06BC)
/*Sub-GHz radio generic CRC initial Low register*/
#define SUBGHZ_GCRCINIRL ((uint16_t)0x06BD)
/*Sub-GHz radio generic CRC polynomial High register*/
#define SUBGHZ_GCRCPOLRH ((uint16_t)0x06BE)
/*Sub-GHz radio generic CRC polynomial Low register*/
#define SUBGHZ_GCRCPOLRL ((uint16_t)0x06BF)
/*Sub-GHz radio generic synchronization word control register 0*/
#define SUBGHZ_GSYNCR0 ((uint16_t)0x06C0)
/*Sub-GHz radio generic synchronization word control register 1*/
#define SUBGHZ_GSYNCR1 ((uint16_t)0x06C1)
/*Sub-GHz radio generic synchronization word control register 2*/
#define SUBGHZ_GSYNCR2 ((uint16_t)0x06C2)
/*Sub-GHz radio generic synchronization word control register 3*/
#define SUBGHZ_GSYNCR3 ((uint16_t)0x06C3)
/*Sub-GHz radio generic synchronization word control register 4*/
#define SUBGHZ_GSYNCR4 ((uint16_t)0x06C4)
/*Sub-GHz radio generic synchronization word control register 5*/
#define SUBGHZ_GSYNCR5 ((uint16_t)0x06C5)
/*Sub-GHz radio generic synchronization word control register 6*/
#define SUBGHZ_GSYNCR6 ((uint16_t)0x06C6)
/*Sub-GHz radio generic synchronization word control register 7*/
#define SUBGHZ_GSYNCR7 ((uint16_t)0x06C7)
/*Sub-GHz radio generic node address register*/
#define SUBGHZ_GNODEADDR ((uint16_t)0x06CD)
/*Sub-GHz radio generic broadacst address register*/
#define SUBGHZ_GBCASTADDR ((uint16_t)0x06CE)
/*Sub-GHz radio generic Afc register*/
#define SUBGHZ_GAFCR ((uint16_t)0x06D1)
/*Sub-GHz radio Lora Payload Length */
#define SUBGHZ_LPLDLENR ((uint16_t)0x0702)
/*Sub-GHz radio Lora synchro timeout */
#define SUBGHZ_LSYNCTIMEOUTR ((uint16_t)0x0706)
/*Sub-GHz radio Lora IQ polarity High register*/
#define SUBGHZ_LIQPOLRH ((uint16_t)0x0735)
/*Sub-GHz radio Lora IQ polarity Low register*/
#define SUBGHZ_LIQPOLRL ((uint16_t)0x0736)
/*Sub-GHz radio LoRa synchronization word High register*/
#define SUBGHZ_LSYNCRH ((uint16_t)0x0740)
/*Sub-GHz radio LoRa synchronization word Low register*/
#define SUBGHZ_LSYNCRL ((uint16_t)0x0741)
/*Sub-GHz radio txAddrPtr register*/
#define SUBGHZ_TXADRPTR ((uint16_t)0x0802)
/*Sub-GHz radio rxAddrPtr register*/
#define SUBGHZ_RXADRPTR ((uint16_t)0x0803)
/*Sub-GHz radio Rx Bandwidth selector register */
#define SUBGHZ_BWSELR ((uint16_t)0x0807)
/*Sub-GHz radio random number register 3*/
#define SUBGHZ_RNGR3 ((uint16_t)0x0819)
/*Sub-GHz radio  random number register 2*/
#define SUBGHZ_RNGR2 ((uint16_t)0x081A)
/*Sub-GHz radio  random number register 1*/
#define SUBGHZ_RNGR1 ((uint16_t)0x081B)
/*Sub-GHz radio  random number register 0*/
#define SUBGHZ_RNGR0 ((uint16_t)0x081C)
/*Sub-GHz radio SD resolution*/
#define SUBGHZ_SDCFG0R ((uint16_t)0x0889)
/*Sub-GHz radio Agc Gfo Reset Rssi Control register*/
#define SUBGHZ_AGCRSSICTL0R ((uint16_t)0x089B)
/*Sub-GHz radio receiver gain control register*/
#define SUBGHZ_RXGAINCR ((uint16_t)0x08AC)
/*Sub-GHz radio Agc Gfo Reset Config register*/
#define SUBGHZ_AGCGFORSTCFGR ((uint16_t)0x08B8)
/*Sub-GHz radio Agc Gfo Reset Power Threshold register*/
#define SUBGHZ_AGCGFORSTPOWTHR ((uint16_t)0x08B9)
/*Sub-GHz radio Tx clamp register*/
#define SUBGHZ_TXCLAMPR ((uint16_t)0x08D8)
/*Sub-GHz radio disable LNA register*/
#define REG_ANA_LNA ((uint16_t)0x08E2)
/*Sub-GHz radio disable mixer register*/
#define REG_ANA_MIXER ((uint16_t)0x08E5)
/*Sub-GHz radio PA over current protection register*/
#define SUBGHZ_PAOCPR ((uint16_t)0x08E7)
/*Sub-GHz radio rtc control register*/
#define SUBGHZ_RTCCTLR ((uint16_t)0x0902)
/*Sub-GHz radio rtc period High register*/
#define SUBGHZ_RTCPRDR2 ((uint16_t)0x0903)
/*Sub-GHz radio rtc period Medium register*/
#define SUBGHZ_RTCPRDR1 ((uint16_t)0x0904)
/*Sub-GHz radio rtc period Low register*/
#define SUBGHZ_RTCPRDR0 ((uint16_t)0x0905)
/*Sub-GHz radio HSE32 OSC_IN capacitor trim register*/
#define SUBGHZ_HSEINTRIMR ((uint16_t)0x0911)
/*Sub-GHz radio HSE32 OSC_OUT capacitor trim register*/
#define SUBGHZ_HSEOUTTRIMR ((uint16_t)0x0912)
/*Sub-GHz radio SMPS control 0 register */
#define SUBGHZ_SMPSC0R ((uint16_t)0x0916)
/*Sub-GHz radio power control register*/
#define SUBGHZ_PCR ((uint16_t)0x091A)
/*Sub-GHz radio regulator drive control register*/
#define SUBGHZ_REGDRVCR ((uint16_t)0x091F)
/*Sub-GHz radio SMPS control 2 register */
#define SUBGHZ_SMPSC2R ((uint16_t)0x0923)

/*Sub-GHz radio Agc LoRa register. Used in SubGHz_Phy example software workaround.
 * This register is not specified in the reference manual
 */
#define SUBGHZ_AGCCFG 0x08A3

/*Sub-GHz event mask register. Used in SubGHz_Phy example software workaround.
 * This register is not specified in the reference manual
 */
#define SUBGHZ_EVENTMASKR 0x0944

// TODO: use EVENTMASKR in rx ISR
/* WORKAROUND - Implicit Header Mode Timeout Behavior, see STM32WL Erratasheet */
// SUBGRF_WriteRegister( SUBGHZ_RTCCTLR, 0x00 );
// SUBGRF_WriteRegister( SUBGHZ_EVENTMASKR, SUBGRF_ReadRegister( SUBGHZ_EVENTMASKR ) | ( 1 << 1 ) );
/* WORKAROUND END */

/* SubGHz module critical section to avoid deadelock in irq management
 *
 * See -> https://github.com/STMicroelectronics/STM32CubeWL/issues/43
 *        https://stackoverflow.com/questions/71626597/what-are-the-various-ways-to-disable-and-re-enable-interrupts-in-stm32-microcont
 */
#define SUBGHZ_CRITICAL_SECTION_BEGIN() HAL_NVIC_DisableIRQ(SUBGHZ_Radio_IRQn);
#define SUBGHZ_CRITICAL_SECTION_END() HAL_NVIC_EnableIRQ(SUBGHZ_Radio_IRQn);

/* TODO:
 * for the moment try if disable the subghz interrupts is enough to avoid the deadlock in the subghz irqs.
 * If the issue persist disable global interrupts with the following macros as it's done in SubGhz_Phy example software.
 * If it's not needed delete the following commented code
 */
/*
#define CRITICAL_SECTION_BEGIN( ) uint32_t primask_bit= __get_PRIMASK();\
  __disable_irq()

#define CRITICAL_SECTION_END( ) __set_PRIMASK(primask_bit)
*/

/*!
 * \brief Defines the TCXO and HSE32 ready timeout.
 *
 * Time-out duration = Timeout x 15.625 μs (maximum time-out duration = 262.14 s)
 *
 * See "Set_TcxoMode()" command definition in reference manual
 */
#define TCXO_AND_HSE32_READY_TIMEOUT_100ms ((uint32_t)0x40) // 100ms

/*!
 * \brief HSE32 XTAL mode OSC_IN load capacitor trimming value
 *
 * from 0x00 to 0x2F starting 11.3pF with 0.47pF steps
 *
 * See "SUBGHZ_HSEINTRIMR" register definition in reference manual
 */
#define LOAD_CAPACITOR_TRIMMING_VALUE_11_3_pF ((uint8_t)0x00)
#define LOAD_CAPACITOR_TRIMMING_VALUE_26_34_pF ((uint8_t)0x20)

/*!
 * \brief Masks to enable the available calibration blocks
 */
#define IMAGE_CALIBRATION ((uint8_t)(1 << 6))
#define RF_ADC_BULK_P_CALIBRATION ((uint8_t)(1 << 5))
#define RF_ADC_BULK_N_CALIBRATION ((uint8_t)(1 << 4))
#define RF_ADC_PULSE_CALIBRATION ((uint8_t)(1 << 3))
#define RF_PLL_CALIBRATION ((uint8_t)(1 << 2))
#define SUB_GHZ_RADIO_RC_13_MHz_CALIBRATION ((uint8_t)(1 << 1))
#define SUB_GHZ_RADIO_RC_64_KHz_CALIBRATION ((uint8_t)(1 << 0))
#define ALL_CALIBRATION ((uint8_t)0xFF)

/*!
 * \brief Represents the interruption masks available for the radio
 *
 * \remark Note that not all these interruptions are available for all packet types
 */
#define IRQ_RADIO_NONE ((uint16_t)(0 << 0))
#define IRQ_TX_DONE ((uint16_t)(1 << 0))
#define IRQ_RX_DONE ((uint16_t)(1 << 1))
#define IRQ_PREAMBLE_DETECTED ((uint16_t)(1 << 2))
#define IRQ_SYNCWORD_VALID ((uint16_t)(1 << 3))
#define IRQ_HEADER_VALID ((uint16_t)(1 << 4))
#define IRQ_HEADER_ERROR ((uint16_t)(1 << 5))
#define IRQ_CRC_ERROR ((uint16_t)(1 << 6))
#define IRQ_CAD_CLEAR ((uint16_t)(1 << 7))
#define IRQ_CAD_DETECTED ((uint16_t)(1 << 8))
#define IRQ_RX_TX_TIMEOUT ((uint16_t)(1 << 9))
#define IRQ_LR_FHSS_HOP ((uint16_t)(1 << 10))
#define IRQ_RADIO_ALL ((uint16_t)0xFFFF)

/*!
 * \brief Regulator drive trimming
 *
 * Regulator drive timming enabled as per WORKAROUND in ST "SubGHz_Phy" example software
 *
 * "WORKAROUND - Trimming the output voltage power_ldo to 3.3V"
 *
 * See also "SUBGHZ_REGDRVCR" register definition in reference manual
 */
#define REGULATOR_DRIVE_TRIMMING ((uint8_t)(0x7 << 1))

/*!
 * \brief Set_PaConfig() command optimal parameters for LP PA selection
 *
 * This parameters are optimized for the HW design (maximum power of 15dBm with the LP PA)
 *
 * See "Set_PaConfig()" command definition in reference manual
 */
#define LP_MAX_POWER_dBm ((uint8_t)0x0F) // max power with LP PA, defined by HW design
#define LP_PA_DUTY_CYCLE ((uint8_t)0x07)
#define LP_HP_MAX ((uint8_t)0x00)
#define LP_PA_SEL ((uint8_t)0x01)
#define LP_PA_LUT ((uint8_t)0x01)
#define LP_POWER_OFFSET_dBm ((int8_t)(0x0E - LP_MAX_POWER_dBm))
#define LP_OVER_CURRENT_PROTECTION_VALUE_FOR_PAOCPR_REGISTER ((uint8_t)0x18)

/*!
 * \brief Set_PaConfig() command optimal parameters for HP PA selection
 *
 * This parameters are optimized for the HW design (maximum power of 22dBm with the HP PA)
 *
 * See "Set_PaConfig()" command definition in reference manual
 */
#define HP_MAX_POWER_dBm ((uint8_t)0x16) // max power with HP PA, defined by HW design
#define HP_PA_DUTY_CYCLE ((uint8_t)0x04)
#define HP_HP_MAX ((uint8_t)0x07)
#define HP_PA_SEL ((uint8_t)0x00)
#define HP_PA_LUT ((uint8_t)0x01)
#define HP_POWER_OFFSET_dBm ((int8_t)(0x16 - HP_MAX_POWER_dBm))
#define HP_OVER_CURRENT_PROTECTION_VALUE_FOR_PAOCPR_REGISTER ((uint8_t)0x38)

/*!
 * Masks for Set_sleep() command
 */
#define SLEEP_CFG_START_COLD ((uint8_t)(0 << 2)) // Cold startup when exiting Sleep mode, configuration registers reset
/* Warm startup when exiting Sleep mode, configuration registers kept in retention
 *
 * Note: Only the configuration of the activated modem, before going to Sleep
 * mode, is retained. The configuration of the other modes is lost and must
 * be re-configured when exiting Sleep mode.
 */
#define SLEEP_CFG_START_WARM ((uint8_t)(1 << 2))

// Sub-GHz radio RTC wake-up disabled
#define SLEEP_CFG_RTC_WAKE_UP_DISABLED ((uint8_t)(0 << 0))

// Sub-GHz radio RTC wake-up enabled
#define SLEEP_CFG_RTC_WAKE_UP_ENABLED ((uint8_t)(1 << 0))

/*!
 * Masks for SUBGHZ_SMPSC2R register. SMPS maximum drive capability.
 */
#define SMPS_DRV_20_mA ((uint8_t)(0x00 << 1))  // 20 mA
#define SMPS_DRV_40_mA ((uint8_t)(0x01 << 1))  // 40 mA
#define SMPS_DRV_60_mA ((uint8_t)(0x02 << 1))  // 60 mA
#define SMPS_DRV_100_mA ((uint8_t)(0x03 << 1)) // 100 mA
#define SMPS_DRV_MASK ((uint8_t)(0x03 << 1))

/*!
 * \brief Payload max length
 *
 * See "Set_PacketParams()" command definition in reference manual
 */
#define MAX_PAYLOAD_SIZE ((uint8_t)0xFF)

/**
 * @brief Provides the frequency of the chip running on the radio and the frequency step
 * @remark These defines are used for computing the frequency divider to set the RF frequency
 */
#define XTAL_FREQ (uint32_t)(32000000)

/*!
 * \brief Represents the voltage used to control the TCXO on/off VDD_TCXO
 */
typedef enum
{
  TCXO_CTRL_1_6V = 0x00,
  TCXO_CTRL_1_7V = 0x01,
  TCXO_CTRL_1_8V = 0x02,
  TCXO_CTRL_2_2V = 0x03,
  TCXO_CTRL_2_4V = 0x04,
  TCXO_CTRL_2_7V = 0x05,
  TCXO_CTRL_3_0V = 0x06,
  TCXO_CTRL_3_3V = 0x07,
} loraRadio_tcxoCtrlVoltage_t;

/*!
 * \brief Declares the power regulation used to power the SubGhz module
 *
 * This command allows the user to specify if DC-DC or LDO is used for power regulation.
 * Using only LDO implies that the Rx or Tx current is doubled
 *
 * For the best power performance, use the SMPS configuration.
 * For the lowest cost, the LDO configuration can be used.
 */
typedef enum
{
  LDO = 0x00,
  SMPS = 0x01,
} loraRadio_regulatorMode_t;

/*!
 * \brief Declares the oscillator in use while in standby mode
 *
 * Using the STDBY_RC standby mode allow to reduce the energy consumption
 * STDBY_XOSC should be used for time critical applications
 */
typedef enum
{
  STDBY_RC = 0x00,
  STDBY_XOSC = 0x01,
} loraRadio_standbyModes_t;

/*!
 * \brief Represents the ramping time for power amplifier
 */
typedef enum
{
  RAMP_10_us = 0x00,
  RAMP_20_us = 0x01,
  RAMP_40_us = 0x02,
  RAMP_80_us = 0x03,
  RAMP_200_us = 0x04,
  RAMP_800_us = 0x05,
  RAMP_1700_us = 0x06,
  RAMP_3400_us = 0x07,
} loraRadio_rampTimes_t;

/*!
 * \brief Represents the possible packet type (i.e. modem) used
 */
typedef enum
{
  PACKET_TYPE_GFSK = 0x00,
  PACKET_TYPE_LORA = 0x01,
  PACKET_TYPE_BPSK = 0x02,
  PACKET_TYPE_GMSK = 0x03,
  PACKET_TYPE_LR_FHSS = 0x03,
  PACKET_TYPE_NONE = 0x0F,
} loraRadio_packetTypes_t;

/*!
 * \brief TODO
 */
typedef enum
{
  LDRO_DISABLED = 0x00,
  LDRO_ENABLED = 0x01,
} loraRadio_lowDataRateOptimization_t;

#endif /* SUBGHZ_REGISTERS_H_ */
