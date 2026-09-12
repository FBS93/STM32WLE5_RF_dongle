/*******************************************************************************
 * @brief STM32WLE5C8U6 USART2 host command manager HAL.
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
#include <stdint.h>
#include <stdbool.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */
#include "eaf.h"
#include "ebf.h"
#include "edf.h"
#include "emf.h"

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "events.h"
#include "host_command_manager_hal.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief Access a 32-bit memory-mapped register.
 *
 * volatile_use: hardware_interaction
 */
#define REGISTER_32(address) (*(volatile uint32_t*)(address))
/**
 * @brief Access an 8-bit memory-mapped register.
 *
 * volatile_use: hardware_interaction
 */
#define REGISTER_8(address) (*(volatile uint8_t*)(address))

/** @brief RCC register block base address. */
#define RCC_BASE 0x58000000UL
/** @brief RCC AHB2 peripheral clock enable register. */
#define RCC_AHB2ENR REGISTER_32(RCC_BASE + 0x04CUL)
/** @brief RCC APB1 peripheral clock enable register 1. */
#define RCC_APB1ENR1 REGISTER_32(RCC_BASE + 0x058UL)
/** @brief GPIOA clock enable bit. */
#define RCC_AHB2ENR_GPIOAEN (1UL << 0U)
/** @brief USART2 clock enable bit. */
#define RCC_APB1ENR1_USART2EN (1UL << 17U)

/** @brief GPIOA register block base address. */
#define GPIOA_BASE 0x48000000UL
/** @brief GPIOA mode register. */
#define GPIOA_MODER REGISTER_32(GPIOA_BASE + 0x000UL)
/** @brief GPIOA pull-up/pull-down register. */
#define GPIOA_PUPDR REGISTER_32(GPIOA_BASE + 0x00CUL)
/** @brief GPIOA alternate-function low register. */
#define GPIOA_AFRL REGISTER_32(GPIOA_BASE + 0x020UL)
/** @brief PA2 and PA3 mode field mask. */
#define GPIOA_MODE_2_3_MASK ((3UL << 4U) | (3UL << 6U))
/** @brief PA2 and PA3 alternate-function mode. */
#define GPIOA_MODE_2_3_ALTERNATE ((2UL << 4U) | (2UL << 6U))
/** @brief PA2 and PA3 pull field mask. */
#define GPIOA_PUPD_2_3_MASK ((3UL << 4U) | (3UL << 6U))
/** @brief PA2 and PA3 alternate-function field mask. */
#define GPIOA_AF_2_3_MASK ((0xFUL << 8U) | (0xFUL << 12U))
/** @brief AF7 selection for PA2 and PA3. */
#define GPIOA_AF_2_3_USART2 ((7UL << 8U) | (7UL << 12U))

/** @brief USART2 register block base address. */
#define USART2_BASE 0x40004400UL
/** @brief USART2 control register 1. */
#define USART2_CR1 REGISTER_32(USART2_BASE + 0x000UL)
/** @brief USART2 control register 2. */
#define USART2_CR2 REGISTER_32(USART2_BASE + 0x004UL)
/** @brief USART2 control register 3. */
#define USART2_CR3 REGISTER_32(USART2_BASE + 0x008UL)
/** @brief USART2 baud-rate register. */
#define USART2_BRR REGISTER_32(USART2_BASE + 0x00CUL)
/** @brief USART2 interrupt and status register. */
#define USART2_ISR REGISTER_32(USART2_BASE + 0x01CUL)
/** @brief USART2 interrupt flag clear register. */
#define USART2_ICR REGISTER_32(USART2_BASE + 0x020UL)
/** @brief USART2 receive data register. */
#define USART2_RDR REGISTER_32(USART2_BASE + 0x024UL)
/** @brief USART2 transmit data register. */
#define USART2_TDR REGISTER_32(USART2_BASE + 0x028UL)
/** @brief USART enable bit. */
#define USART_CR1_UE (1UL << 0U)
/** @brief Receiver enable bit. */
#define USART_CR1_RE (1UL << 2U)
/** @brief Transmitter enable bit. */
#define USART_CR1_TE (1UL << 3U)
/** @brief RX-not-empty interrupt enable bit. */
#define USART_CR1_RXNEIE (1UL << 5U)
/** @brief TX-empty interrupt enable bit. */
#define USART_CR1_TXEIE (1UL << 7U)
/** @brief USART framing, noise, and overrun error interrupt enable bit. */
#define USART_CR3_EIE (1UL << 0U)
/** @brief Framing error flag. */
#define USART_ISR_FE (1UL << 1U)
/** @brief Noise error flag. */
#define USART_ISR_NE (1UL << 2U)
/** @brief Overrun error flag. */
#define USART_ISR_ORE (1UL << 3U)
/** @brief Receive-data-register-not-empty flag. */
#define USART_ISR_RXNE (1UL << 5U)
/** @brief Transmit-data-register-empty flag. */
#define USART_ISR_TXE (1UL << 7U)
/** @brief Transmitter acknowledgement flag. */
#define USART_ISR_TEACK (1UL << 21U)
/** @brief Receiver acknowledgement flag. */
#define USART_ISR_REACK (1UL << 22U)
/** @brief USART receive error flags. */
#define USART_ISR_ERROR_MASK (USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE)
/** @brief USART error clear bits. */
#define USART_ICR_ERROR_MASK ((1UL << 1U) | (1UL << 2U) | (1UL << 3U))
/** @brief BRR value for 115200 baud from a 32 MHz PCLK1. */
#define USART2_BRR_115200_AT_32MHZ 278UL

/** @brief USART2 external interrupt number. */
#define USART2_IRQ_NUMBER 37U
/** @brief USART2 NVIC enable register. */
#define NVIC_ISER1 REGISTER_32(0xE000E104UL)
/** @brief USART2 NVIC enable bit. */
#define NVIC_ISER1_USART2 (1UL << (USART2_IRQ_NUMBER - 32U))
/** @brief USART2 byte-addressed NVIC priority register. */
#define NVIC_IPR_USART2 REGISTER_8(0xE000E400UL + USART2_IRQ_NUMBER)
/** @brief Architecture priority 1 encoded in four implemented priority bits. */
#define NVIC_PRIORITY_USART2 0x10U

/** @brief RX FIFO capacity defined by the detailed design. */
#define RX_FIFO_CAPACITY 264U
/** @brief TX FIFO capacity defined by the detailed design. */
#define TX_FIFO_CAPACITY 267U

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/** @brief Define static file name string for asserts. */
EAF_DEFINE_THIS_FILE(__FILE__);
/** @brief RX FIFO state shared by USART2 ISR and active-object context. */
static EMF_byteFifo_handler_t rxFifo;
/** @brief TX FIFO state shared by USART2 ISR and active-object context. */
static EMF_byteFifo_handler_t txFifo;
/** @brief RX FIFO storage. */
static uint8_t rxStorage[RX_FIFO_CAPACITY];
/** @brief TX FIFO storage. */
static uint8_t txStorage[TX_FIFO_CAPACITY];
/** @brief Immutable ISR-to-active-object notification event. */
static const events_hostDataAvailable_t hostDataAvailableEvent = {
  .super = EDF_EVENT_IMMUTABLE_INIT(EVENTS_HOST_DATA_AVAILABLE)};
/** @brief True after the target HAL has been initialized. */
static bool initialized;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

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

void hostCommandManagerHal_init(void)
{
  EAF_ASSERT(!initialized);

  EMF_byteFifo_init(&rxFifo, sizeof(rxStorage), rxStorage);
  EMF_byteFifo_init(&txFifo, sizeof(txStorage), txStorage);

  RCC_AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
  RCC_APB1ENR1 |= RCC_APB1ENR1_USART2EN;
  GPIOA_MODER = (GPIOA_MODER & ~GPIOA_MODE_2_3_MASK) | GPIOA_MODE_2_3_ALTERNATE;
  GPIOA_PUPDR &= ~GPIOA_PUPD_2_3_MASK;
  GPIOA_AFRL = (GPIOA_AFRL & ~GPIOA_AF_2_3_MASK) | GPIOA_AF_2_3_USART2;

  USART2_CR1 = 0U;
  USART2_CR2 = 0U;
  USART2_CR3 = USART_CR3_EIE;
  USART2_BRR = USART2_BRR_115200_AT_32MHZ;
  // Error flags are cleared by writing one to their ICR bits.
  USART2_ICR = USART_ICR_ERROR_MASK;
  USART2_CR1 = USART_CR1_UE | USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE;
  while ((USART2_ISR & (USART_ISR_TEACK | USART_ISR_REACK)) !=
         (USART_ISR_TEACK | USART_ISR_REACK))
  {
    // Wait until both USART directions acknowledge enablement.
  }

  NVIC_IPR_USART2 = NVIC_PRIORITY_USART2;
  initialized = true;  // Mark initialization complete before an enabled IRQ.
  NVIC_ISER1 = NVIC_ISER1_USART2;
}

uint32_t hostCommandManagerHal_getAvailableBytes(void)
{
  uint32_t available;

  EAF_ASSERT(initialized);

  EBF_CRITICAL_SECTION_ENTRY();
  available = EMF_byteFifo_getUsed(&rxFifo);
  EBF_CRITICAL_SECTION_EXIT();

  return available;
}

void hostCommandManagerHal_read(uint8_t* data, uint32_t length)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(initialized);
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(length > 0U);
  EAF_ASSERT_BLOCK_END();

  EBF_CRITICAL_SECTION_ENTRY();
  EAF_ASSERT_IN_CRITICAL_SECTION(length <= EMF_byteFifo_getUsed(&rxFifo));
  EMF_byteFifo_pop(&rxFifo, data, length);
  EBF_CRITICAL_SECTION_EXIT();
}

void hostCommandManagerHal_write(const uint8_t* data, uint32_t length)
{
  bool was_empty;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(initialized);
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(length > 0U);
  EAF_ASSERT_IN_BLOCK(length <= TX_FIFO_CAPACITY);
  EAF_ASSERT_BLOCK_END();

  // Keep enqueue and transmitter kick atomic with respect to the ISR.
  EBF_CRITICAL_SECTION_ENTRY();
  EAF_ASSERT_IN_CRITICAL_SECTION(length <= EMF_byteFifo_getFree(&txFifo));
  was_empty = EMF_byteFifo_isEmpty(&txFifo);
  EMF_byteFifo_push(&txFifo, data, length);
  if (was_empty)
  {
    USART2_CR1 |= USART_CR1_TXEIE;
  }
  EBF_CRITICAL_SECTION_EXIT();
}

/** @brief Handles USART2 RX and TX interrupts owned by host command manager. */
void USART2_IRQHandler(void)
{
  uint32_t status;
  uint8_t byte;
  bool notify;

  EAF_ASSERT(initialized);

  status = USART2_ISR;
  if ((status & USART_ISR_ERROR_MASK) != 0U)
  {
    // Error flags are cleared by writing one to their ICR bits.
    USART2_ICR = USART_ICR_ERROR_MASK;
    EAF_ERROR();
  }

  if ((status & USART_ISR_RXNE) != 0U)
  {
    // Reading RDR consumes the byte; notify only on empty-to-non-empty.
    byte = (uint8_t)USART2_RDR;
    EAF_ASSERT(!EMF_byteFifo_isFull(&rxFifo));
    notify = EMF_byteFifo_isEmpty(&rxFifo);
    EMF_byteFifo_push(&rxFifo, &byte, 1U);
    if (notify)
    {
      EDF_activeObject_publish(&hostDataAvailableEvent.super);
    }
  }

  // Drain ready bytes and stop TX-empty interrupts once the FIFO is empty.
  while (((USART2_ISR & USART_ISR_TXE) != 0U) && !EMF_byteFifo_isEmpty(&txFifo))
  {
    EMF_byteFifo_pop(&txFifo, &byte, 1U);
    USART2_TDR = byte;
  }
  if (EMF_byteFifo_isEmpty(&txFifo))
  {
    USART2_CR1 &= ~USART_CR1_TXEIE;
  }

  EDF_ISRExit();
}
