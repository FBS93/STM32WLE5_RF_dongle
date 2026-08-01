/*******************************************************************************
 * @brief Standard input/output implementation for STM32F103C8Tx series.
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
#include <stdbool.h>
#include <stddef.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "ebf.h"
#include "eaf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief NVIC Interrupt Set-Enable Register 1.
 *
 * volatile_use: hardware_interaction
 */
#define NVIC_ISER1 (*(volatile uint32_t*)0xE000E104UL)

/**
 * @brief NVIC interrupt priority registers base.
 *
 * volatile_use: hardware_interaction
 */
#define NVIC_IPR_BASE ((volatile uint8_t*)0xE000E400UL)

/**
 * @brief Base address of the RCC (Reset and Clock Control) register block.
 */
#define RCC_BASE 0x40021000UL

/**
 * @brief RCC APB2 peripheral clock enable register.
 *
 * volatile_use: hardware_interaction
 */
#define RCC_APB2ENR (*(volatile uint32_t*)(RCC_BASE + 0x18U))

/**
 * @brief GPIOA clock enable bit in RCC_APB2ENR.
 */
#define RCC_APB2ENR_IOPAEN (1U << 2U)

/**
 * @brief USART1 clock enable bit in RCC_APB2ENR.
 */
#define RCC_APB2ENR_USART1EN (1U << 14U)

/**
 * @brief Base address of the GPIO port A register block.
 */
#define GPIOA_BASE 0x40010800UL

/**
 * @brief GPIOA control register high (pins 8–15).
 *
 * volatile_use: hardware_interaction
 */
#define GPIOA_CRH (*(volatile uint32_t*)(GPIOA_BASE + 0x04U))

/**
 * @brief Base address of the USART1 peripheral register block.
 */
#define USART1_BASE 0x40013800UL

/**
 * @brief USART1 status register.
 *
 * volatile_use: hardware_interaction
 */
#define USART1_SR (*(volatile uint32_t*)(USART1_BASE + 0x00U))

/**
 * @brief USART1 data register.
 *
 * volatile_use: hardware_interaction
 */
#define USART1_DR (*(volatile uint32_t*)(USART1_BASE + 0x04U))

/**
 * @brief USART1 baud rate register.
 *
 * volatile_use: hardware_interaction
 */
#define USART1_BRR (*(volatile uint32_t*)(USART1_BASE + 0x08U))

/**
 * @brief USART1 control register 1.
 *
 * volatile_use: hardware_interaction
 */
#define USART1_CR1 (*(volatile uint32_t*)(USART1_BASE + 0x0CU))

/**
 * @brief Transmit data register empty flag.
 */
#define USART_SR_TXE (1U << 7U)

/**
 * @brief Receive data register not empty flag.
 */
#define USART_SR_RXNE (1U << 5U)

/**
 * @brief USART enable bit.
 */
#define USART_CR1_UE (1U << 13U)

/**
 * @brief Transmitter enable bit.
 */
#define USART_CR1_TE (1U << 3U)

/**
 * @brief Receiver enable bit.
 */
#define USART_CR1_RE (1U << 2U)

/**
 * @brief RX not empty interrupt enable bit.
 */
#define USART_CR1_RXNEIE (1U << 5U)

/**
 * @brief NVIC IRQ number for USART1.
 */
#define USART1_IRQ_NUM 37U

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/**
 * @brief Define static file name string for asserts.
 */
EAF_DEFINE_THIS_FILE(__FILE__);

/**
 * @brief Indicates whether the USART has been initialized.
 */
static bool uartInitialized = false;

/**
 * @brief Registered standard input listener.
 *
 * volatile_use: asynchronous_interaction
 */
static volatile EBF_stdin_t stdinListener = NULL;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/**
 * @brief Initializes USART1.
 */
static void initUART(void);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static void initUART(void)
{
  // Enable GPIOA and USART1 peripheral clocks.
  RCC_APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_USART1EN;

  /**
   * Configure PA9 as USART1 TX (alternate function push-pull, high-speed
   * output).
   */
  GPIOA_CRH &= ~(0xFU << 4U);
  GPIOA_CRH |= (0xBU << 4U);

  // Configure PA10 as USART1 RX (floating input).
  GPIOA_CRH &= ~(0xFU << 8U);
  GPIOA_CRH |= (0x4U << 8U);

  // USART1 baud rate = 115200 assuming APB2 clock = 8 MHz (HSI, no PLL).
  USART1_BRR = (4U << 4U) | 5U;

  // Enable USART TX, RX and RX interrupt.
  USART1_CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE | USART_CR1_UE;

  // Enable USART1 interrupt in NVIC (IRQs 32–63 --> ISER1).
  NVIC_ISER1 |= (1U << (USART1_IRQ_NUM - 32U));

  // Set USART1 RX interrupt to highest priority (0 = highest).
  NVIC_IPR_BASE[USART1_IRQ_NUM] = 0x00U;

  uartInitialized = true;
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void EBF_setStdinListener(EBF_stdin_t listener)
{
  if (!uartInitialized)
  {
    initUART();
  }

  stdinListener = listener;
}

bool EBF_stdoutIsReadyToWrite(uint16_t len)
{
  if (!uartInitialized)
  {
    initUART();
  }

  // No TX buffer --> only 1 Byte allowed.
  return (len == 1) && (USART1_SR & USART_SR_TXE);
}

void EBF_stdoutWrite(const uint8_t* data, uint16_t len)
{
  // No TX buffer --> only 1 Byte allowed.
  EAF_ASSERT(len == 1);

  if (!uartInitialized)
  {
    initUART();
  }

  // No blocking, so TX status shall be ready.
  EAF_ASSERT(USART1_SR & USART_SR_TXE);

  // Write the byte to the USART transmit data register (starts transmission).
  USART1_DR = data[0];
}

void USART1_IRQHandler(void)
{
  uint8_t byte;

  if (USART1_SR & USART_SR_RXNE)
  {
    // RXNE (Receive Data Register Not Empty) is cleared by reading USART1_DR.
    byte = (uint8_t)USART1_DR;

    if (stdinListener != NULL)
    {
      stdinListener(&byte, 1);
    }
  }
}
