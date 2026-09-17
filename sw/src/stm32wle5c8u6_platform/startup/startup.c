/*******************************************************************************
 * @brief Early system initialization for STM32WLE5C8U6.
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

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/** @brief Access a 32-bit memory-mapped register.
 *
 * volatile_use: hardware_interaction
 */
#define REGISTER_32(address) (*(volatile uint32_t*)(address))

/**
 * @name RCC register map used for early clock initialization.
 *
 * @c RCC_BASE is the register-block base byte address. @c RCC_CR,
 * @c RCC_CFGR and @c RCC_EXTCFGR are 32-bit register aliases at their
 * respective byte offsets from that base.
 * @{
 */
#define RCC_BASE 0x58000000UL
#define RCC_CR REGISTER_32(RCC_BASE + 0x000UL)
#define RCC_CFGR REGISTER_32(RCC_BASE + 0x008UL)
#define RCC_EXTCFGR REGISTER_32(RCC_BASE + 0x108UL)
/** @} */

/**
 * @name PWR register map used for voltage scaling.
 *
 * @c PWR_BASE is the register-block base byte address. @c PWR_CR1 and
 * @c PWR_SR2 are 32-bit register aliases at their respective byte offsets from
 * that base.
 * @{
 */
#define PWR_BASE 0x58000400UL
#define PWR_CR1 REGISTER_32(PWR_BASE + 0x000UL)
#define PWR_SR2 REGISTER_32(PWR_BASE + 0x014UL)
/** @} */

/**
 * @name Flash interface register map used for latency configuration.
 *
 * @c FLASH_BASE is the register-block base byte address. @c FLASH_ACR is a
 * 32-bit register alias at its byte offset from that base.
 * @{
 */
#define FLASH_BASE 0x58004000UL
#define FLASH_ACR REGISTER_32(FLASH_BASE + 0x000UL)
/** @} */

/**
 * @name RCC_CR fields used to select the 32 MHz MSI range.
 *
 * @c RCC_CR_MSIRDY indicates that MSI is ready. @c RCC_CR_MSIRGSEL selects
 * RCC_CR as the MSI range source. @c RCC_CR_MSIRANGE_MASK covers the range
 * field, and @c RCC_CR_MSIRANGE_32_MHZ encodes range 10 (32 MHz).
 * @{
 */
#define RCC_CR_MSIRDY (1UL << 1U)
#define RCC_CR_MSIRGSEL (1UL << 3U)
#define RCC_CR_MSIRANGE_MASK (0xFUL << 4U)
#define RCC_CR_MSIRANGE_32_MHZ (10UL << 4U)
/** @} */

/**
 * @name RCC_CFGR clock-source and prescaler fields.
 *
 * Zero in @c RCC_CFGR_SW_MASK selects MSI, and zero in @c RCC_CFGR_SWS_MASK
 * reports MSI as SYSCLK. Zero in the @c RCC_CFGR_HPRE_MASK,
 * @c RCC_CFGR_PPRE1_MASK and @c RCC_CFGR_PPRE2_MASK fields encodes
 * divide-by-one. @c RCC_CFGR_HPREF indicates that the AHB prescaler is ready.
 * @{
 */
#define RCC_CFGR_SW_MASK (0x3UL << 0U)
#define RCC_CFGR_SWS_MASK (0x3UL << 2U)
#define RCC_CFGR_HPRE_MASK (0xFUL << 4U)
#define RCC_CFGR_PPRE1_MASK (0x7UL << 8U)
#define RCC_CFGR_PPRE2_MASK (0x7UL << 11U)
#define RCC_CFGR_HPREF (1UL << 16U)
/** @} */

/**
 * @name RCC_EXTCFGR shared-domain prescaler fields.
 *
 * Zero in @c RCC_EXTCFGR_SHDHPRE_MASK encodes divide-by-one.
 * @c RCC_EXTCFGR_SHDHPREF indicates that the shared AHB prescaler is ready.
 * @{
 */
#define RCC_EXTCFGR_SHDHPRE_MASK (0xFUL << 0U)
#define RCC_EXTCFGR_SHDHPREF (1UL << 16U)
/** @} */

/**
 * @name PWR voltage-scaling fields used for 32 MHz operation.
 *
 * @c PWR_CR1_VOS_MASK covers the voltage-range field.
 * @c PWR_CR1_VOS_RANGE_1 encodes range 1 (typically 1.2 V, up to 64 MHz), and
 * @c PWR_SR2_VOSF indicates that a voltage-scaling transition is in progress.
 * @{
 */
#define PWR_CR1_VOS_MASK (0x3UL << 9U)
#define PWR_CR1_VOS_RANGE_1 (1UL << 9U)
#define PWR_SR2_VOSF (1UL << 10U)
/** @} */

/**
 * @name Flash access latency field and two-wait-state encoding.
 *
 * @c FLASH_ACR_LATENCY_MASK covers the latency field, and
 * @c FLASH_ACR_LATENCY_2 encodes two wait states.
 * @{
 */
#define FLASH_ACR_LATENCY_MASK (0x7UL << 0U)
#define FLASH_ACR_LATENCY_2 (0x2UL << 0U)
/** @} */

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/** @brief Cortex-M4 core clock frequency after @ref SystemInit. */
uint32_t SystemCoreClock = 32000000UL;

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Configure the CPU and bus clocks before the application starts.
 *
 * MSI range 10 supplies 32 MHz SYSCLK without taking ownership of the HSE32
 * and PB0/VDDTCXO resources reserved for the integrated radio.
 */
void SystemInit(void)
{
  // Select voltage range 1 before increasing the clock frequency.
  PWR_CR1 = (PWR_CR1 & ~PWR_CR1_VOS_MASK) | PWR_CR1_VOS_RANGE_1;
  while ((PWR_SR2 & PWR_SR2_VOSF) != 0UL)
  {
    // Wait for voltage scaling to become ready.
  }

  // Program and verify two flash wait states before increasing HCLK3.
  FLASH_ACR = (FLASH_ACR & ~FLASH_ACR_LATENCY_MASK) | FLASH_ACR_LATENCY_2;
  while ((FLASH_ACR & FLASH_ACR_LATENCY_MASK) != FLASH_ACR_LATENCY_2)
  {
    // Wait for the flash latency setting to be applied.
  }

  // Keep all CPU, APB and shared-domain prescalers at divide-by-one.
  RCC_CFGR &= ~(RCC_CFGR_HPRE_MASK | RCC_CFGR_PPRE1_MASK | RCC_CFGR_PPRE2_MASK);
  while ((RCC_CFGR & RCC_CFGR_HPREF) == 0UL)
  {
    // Wait for the HCLK1 prescaler setting to be applied.
  }

  RCC_EXTCFGR &= ~RCC_EXTCFGR_SHDHPRE_MASK;
  while ((RCC_EXTCFGR & RCC_EXTCFGR_SHDHPREF) == 0UL)
  {
    // Wait for the shared HCLK3 prescaler setting to be applied.
  }

  // MSI is ready and is the reset SYSCLK, so its range can be changed in place.
  while ((RCC_CR & RCC_CR_MSIRDY) == 0UL)
  {
    // Wait for MSI readiness.
  }
  RCC_CR =
    (RCC_CR & ~RCC_CR_MSIRANGE_MASK) | RCC_CR_MSIRGSEL | RCC_CR_MSIRANGE_32_MHZ;

  // Explicitly select MSI as SYSCLK and wait for the switch status.
  RCC_CFGR &= ~RCC_CFGR_SW_MASK;
  while ((RCC_CFGR & RCC_CFGR_SWS_MASK) != 0UL)
  {
    // Wait until MSI is used as SYSCLK.
  }
}
