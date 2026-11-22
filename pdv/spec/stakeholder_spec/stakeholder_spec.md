# Stakeholder specification

## Product purpose

The product shall be a laboratory RF dongle based on the selected STM32WLE5 microcontroller (MCU) named `STM32WLE5_RF_dongle`. The `STM32WLE5_RF_dongle` hardware and software shall both be developed within this project.

## Host control and RF operation

The `STM32WLE5_RF_dongle` shall be operated from a USB-connected host and support both manual control and automated scripting.

The host shall be able to configure RF operation of `STM32WLE5_RF_dongle` using all modulation and configuration capabilities provided by the selected STM32WLE5 MCU.

The host shall be able to request RF transmission and reception to `STM32WLE5_RF_dongle`.

## Operational status and diagnostics

The `STM32WLE5_RF_dongle` shall provide the host with the active RF operation and radio operational errors.

## External documentation

- [RM0461: STM32WLEx advanced Arm®-based 32-bit MCUs with sub-GHz radio solution - Reference manual](external_doc/rm0461-stm32wle5xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [DS13105: STM32WLE5xx STM32WLE4xx - Multiprotocol LPWAN 32-bit Arm® Cortex®-M4 MCUs](external_doc/stm32wle5jc.pdf)
