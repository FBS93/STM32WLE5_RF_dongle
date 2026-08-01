/*******************************************************************************
 * @brief Minimal STM32F103C8Tx register definitions.
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

#ifndef STM32F103C8TX_REGISTERS_H
#define STM32F103C8TX_REGISTERS_H

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

/**
 * @brief Base address of the RCC register block.
 */
#define RCC_BASE 0x40021000UL

/**
 * @brief RCC APB2 peripheral clock enable register.
 *
 * volatile_use: hardware_interaction
 */
#define RCC_APB2ENR (*(volatile uint32_t*)(RCC_BASE + 0x18U))

/**
 * @brief RCC APB1 peripheral clock enable register.
 *
 * volatile_use: hardware_interaction
 */
#define RCC_APB1ENR (*(volatile uint32_t*)(RCC_BASE + 0x1CU))

/**
 * @brief GPIOB clock enable bit in RCC_APB2ENR.
 */
#define RCC_APB2ENR_IOPBEN (1U << 3U)

/**
 * @brief TIM2 clock enable bit in RCC_APB1ENR.
 */
#define RCC_APB1ENR_TIM2EN (1U << 0U)

/**
 * @brief Base address of the GPIOB register block.
 */
#define GPIOB_BASE 0x40010C00UL

/**
 * @brief GPIOB control register high (pins 8-15).
 *
 * volatile_use: hardware_interaction
 */
#define GPIOB_CRH (*(volatile uint32_t*)(GPIOB_BASE + 0x04U))

/**
 * @brief GPIOB bit set/reset register.
 *
 * volatile_use: hardware_interaction
 */
#define GPIOB_BSRR (*(volatile uint32_t*)(GPIOB_BASE + 0x10U))

/**
 * @brief Base address of the TIM2 register block.
 */
#define TIM2_BASE 0x40000000UL

/**
 * @brief TIM2 control register 1.
 *
 * volatile_use: hardware_interaction
 */
#define TIM2_CR1 (*(volatile uint32_t*)(TIM2_BASE + 0x00U))

/**
 * @brief TIM2 event generation register.
 *
 * volatile_use: hardware_interaction
 */
#define TIM2_EGR (*(volatile uint32_t*)(TIM2_BASE + 0x14U))

/**
 * @brief TIM2 counter register.
 *
 * volatile_use: hardware_interaction
 */
#define TIM2_CNT (*(volatile uint32_t*)(TIM2_BASE + 0x24U))

/**
 * @brief TIM2 prescaler register.
 *
 * volatile_use: hardware_interaction
 */
#define TIM2_PSC (*(volatile uint32_t*)(TIM2_BASE + 0x28U))

/**
 * @brief TIM2 auto-reload register.
 *
 * volatile_use: hardware_interaction
 */
#define TIM2_ARR (*(volatile uint32_t*)(TIM2_BASE + 0x2CU))

/**
 * @brief TIM enable bit.
 */
#define TIM_CR1_CEN (1U << 0U)

/**
 * @brief TIM update generation bit.
 */
#define TIM_EGR_UG (1U << 0U)

/**
 * @brief SysTick control and status register.
 *
 * volatile_use: hardware_interaction
 */
#define SYST_CSR (*(volatile uint32_t*)0xE000E010UL)

/**
 * @brief SysTick reload value register.
 *
 * volatile_use: hardware_interaction
 */
#define SYST_RVR (*(volatile uint32_t*)0xE000E014UL)

/**
 * @brief SysTick current value register.
 *
 * volatile_use: hardware_interaction
 */
#define SYST_CVR (*(volatile uint32_t*)0xE000E018UL)

/**
 * @brief SysTick enable bit.
 */
#define SYST_CSR_ENABLE (1U << 0U)

/**
 * @brief SysTick interrupt enable bit.
 */
#define SYST_CSR_TICKINT (1U << 1U)

/**
 * @brief SysTick processor clock source bit.
 */
#define SYST_CSR_CLKSOURCE (1U << 2U)

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

#endif /* STM32F103C8TX_REGISTERS_H */
