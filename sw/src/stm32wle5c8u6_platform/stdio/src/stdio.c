/*******************************************************************************
 * @brief Non-blocking ITM/TPIU/SWO standard output for STM32WLE5C8U6.
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
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */
#include "eaf.h"
#include "ebf.h"

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

/** @brief Access an 8-bit memory-mapped register.
 *
 * volatile_use: hardware_interaction
 */
#define REGISTER_8(address) (*(volatile uint8_t*)(address))

/**
 * @name RCC register and field used to enable TIM2.
 *
 * @c RCC_BASE is the register-block base byte address, @c RCC_APB1ENR1 is its
 * 32-bit APB1 peripheral clock enable register 1 alias, and
 * @c RCC_APB1ENR1_TIM2EN is the TIM2 clock-enable mask.
 * @{
 */
#define RCC_BASE 0x58000000UL
#define RCC_APB1ENR1 REGISTER_32(RCC_BASE + 0x058UL)
#define RCC_APB1ENR1_TIM2EN (1UL << 0U)
/** @} */

/**
 * @name TIM2 register map used by the stdout FIFO consumer.
 *
 * @c TIM2_BASE is the register-block base byte address. @c TIM2_CR1,
 * @c TIM2_DIER, @c TIM2_SR, @c TIM2_EGR, @c TIM2_CNT, @c TIM2_PSC and
 * @c TIM2_ARR are 32-bit register aliases at their respective byte offsets
 * from that base.
 * @{
 */
#define TIM2_BASE 0x40000000UL
#define TIM2_CR1 REGISTER_32(TIM2_BASE + 0x000UL)
#define TIM2_DIER REGISTER_32(TIM2_BASE + 0x00CUL)
#define TIM2_SR REGISTER_32(TIM2_BASE + 0x010UL)
#define TIM2_EGR REGISTER_32(TIM2_BASE + 0x014UL)
#define TIM2_CNT REGISTER_32(TIM2_BASE + 0x024UL)
#define TIM2_PSC REGISTER_32(TIM2_BASE + 0x028UL)
#define TIM2_ARR REGISTER_32(TIM2_BASE + 0x02CUL)
/** @} */

/**
 * @name TIM2 control, interrupt, status and event masks.
 *
 * @c TIM_CR1_CEN enables the counter, while @c TIM_CR1_URS restricts update
 * requests to counter overflow or underflow. @c TIM_DIER_UIE enables the
 * update interrupt, @c TIM_SR_UIF identifies a pending update interrupt, and
 * @c TIM_EGR_UG generates an update event to load the timer configuration.
 * @{
 */
#define TIM_CR1_CEN (1UL << 0U)
#define TIM_CR1_URS (1UL << 2U)
#define TIM_DIER_UIE (1UL << 0U)
#define TIM_SR_UIF (1UL << 0U)
#define TIM_EGR_UG (1UL << 0U)
/** @} */

/**
 * @name NVIC register addresses and TIM2 interrupt configuration.
 *
 * @c NVIC_ISER0 is the 32-bit interrupt set-enable register 0 alias, and
 * @c NVIC_IPR_BASE is the interrupt-priority register array base byte address.
 * TIM2 uses external interrupt number @c TIM2_IRQ_NUMBER and logical priority
 * @c TIM2_IRQ_PRIORITY. @c NVIC_IMPLEMENTED_PRIORITY_BITS defines the number
 * of most-significant priority bits implemented by the target.
 * @{
 */
#define NVIC_ISER0 REGISTER_32(0xE000E100UL)
#define NVIC_IPR_BASE 0xE000E400UL
#define TIM2_IRQ_NUMBER 27U
#define TIM2_IRQ_PRIORITY 2U
#define NVIC_IMPLEMENTED_PRIORITY_BITS 4U
/** @} */

/**
 * @name Core debug register and trace-enable mask.
 *
 * @c CORE_DEBUG_DEMCR is the 32-bit Debug Exception and Monitor Control
 * Register alias. @c CORE_DEBUG_DEMCR_TRCENA enables the trace subsystem.
 * @{
 */
#define CORE_DEBUG_DEMCR REGISTER_32(0xE000EDFCUL)
#define CORE_DEBUG_DEMCR_TRCENA (1UL << 24U)
/** @} */

/**
 * @name TPIU registers and asynchronous NRZ protocol encoding.
 *
 * @c TPI_ACPR is the 32-bit asynchronous clock prescaler register alias, and
 * @c TPI_SPPR is the 32-bit selected pin protocol register alias.
 * @c TPI_SPPR_NRZ encodes asynchronous NRZ output.
 * @{
 */
#define TPI_ACPR REGISTER_32(0xE0040010UL)
#define TPI_SPPR REGISTER_32(0xE00400F0UL)
#define TPI_SPPR_NRZ 2UL
/** @} */

/**
 * @name ITM stimulus port and control registers used for stdout.
 *
 * @c ITM_STIMULUS_PORT_0_32 reads the 32-bit port-zero status, while
 * @c ITM_STIMULUS_PORT_0_8 writes one byte to that port. @c ITM_TER and
 * @c ITM_TCR are the 32-bit trace-enable and trace-control register aliases.
 * @c ITM_TCR_ITMENA enables ITM, @c ITM_TCR_TRACE_BUS_ID selects trace bus ID
 * 1, @c ITM_TER_PORT_0 enables stimulus port zero, and
 * @c ITM_STIMULUS_FIFOREADY identifies immediate byte acceptance.
 * @{
 */
#define ITM_STIMULUS_PORT_0_32 REGISTER_32(0xE0000000UL)
#define ITM_STIMULUS_PORT_0_8 REGISTER_8(0xE0000000UL)
#define ITM_TER REGISTER_32(0xE0000E00UL)
#define ITM_TCR REGISTER_32(0xE0000E80UL)
#define ITM_TCR_ITMENA (1UL << 0U)
#define ITM_TCR_TRACE_BUS_ID (1UL << 16U)
#define ITM_TER_PORT_0 (1UL << 0U)
#define ITM_STIMULUS_FIFOREADY (1UL << 0U)
/** @} */

/** @brief Stdout transmit FIFO capacity in bytes. */
#define STDOUT_TX_BUFFER_CAPACITY 1024U

/** @brief Trace clock frequency in hertz. */
#define PLATFORM_TRACE_CLOCK_HZ 32000000UL

/** @brief Nominal SWO asynchronous bitrate in bits per second. */
#define STDOUT_SWO_BITRATE 2000000UL

/** @brief TPIU prescaler encoding for the configured SWO bitrate. */
#define TPI_ASYNC_PRESCALER \
  ((PLATFORM_TRACE_CLOCK_HZ / STDOUT_SWO_BITRATE) - 1UL)

/** @brief TIM2 input clock frequency in hertz. */
#define TIM2_CLOCK_HZ 32000000UL

/** @brief TIM2 prescaled counter frequency in hertz. */
#define TIM2_COUNTER_HZ 1000000UL

/** @brief TIM2 update-event frequency in hertz. */
#define TIM2_PERIOD_HZ 1000UL

/** @brief TIM2 prescaler encoding for the configured counter frequency. */
#define TIM2_PRESCALER ((TIM2_CLOCK_HZ / TIM2_COUNTER_HZ) - 1UL)

/** @brief TIM2 auto-reload encoding for the configured update frequency. */
#define TIM2_AUTO_RELOAD ((TIM2_COUNTER_HZ / TIM2_PERIOD_HZ) - 1UL)

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/** @brief Asynchronous stdout FIFO state. */
typedef struct
{
  uint8_t buffer[STDOUT_TX_BUFFER_CAPACITY]; /**< FIFO byte storage. */
  volatile uint16_t head;                    /**< Next transmit byte index.
                                                  volatile_use: asynchronous_interaction */
  volatile uint16_t tail;                    /**< Next byte-storage index.
                                                  volatile_use: asynchronous_interaction */
  volatile uint16_t count;                   /**< Number of queued bytes.
                                                  volatile_use: asynchronous_interaction */
  bool initialized; /**< Whether trace and TIM2 initialization is complete. */
} stdio_t;

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/** @brief Define static file name string for asserts. */
EAF_DEFINE_THIS_FILE(__FILE__);

/** @brief Module state, initialized to zero by startup. */
static stdio_t stdio;

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
 * @brief Initialize asynchronous NRZ trace output and its TIM2 consumer.
 *
 * Configures ITM stimulus port zero for 2 Mbit/s SWO and TIM2 for 1 ms update
 * interrupts. The timer remains stopped until stdout data is queued.
 */
static void initialize(void);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static void initialize(void)
{
  // The core trace gate must be open before configuring TPIU and ITM.
  CORE_DEBUG_DEMCR |= CORE_DEBUG_DEMCR_TRCENA;

  // PB3 retains its reset debug assignment; select asynchronous NRZ output.
  TPI_ACPR = TPI_ASYNC_PRESCALER;
  TPI_SPPR = TPI_SPPR_NRZ;

  ITM_TCR = ITM_TCR_TRACE_BUS_ID | ITM_TCR_ITMENA;
  ITM_TER = ITM_TER_PORT_0;

  RCC_APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
  // Read back the clock gate so it is active before accessing TIM2.
  (void)RCC_APB1ENR1;

  // URS prevents the update used to latch PSC and ARR from requesting an IRQ.
  TIM2_CR1 = TIM_CR1_URS;
  TIM2_DIER = TIM_DIER_UIE;
  TIM2_PSC = TIM2_PRESCALER;
  TIM2_ARR = TIM2_AUTO_RELOAD;
  TIM2_EGR = TIM_EGR_UG;
  TIM2_SR &= ~TIM_SR_UIF;

  // Priority occupies the implemented MSBs; ISER uses write-one-to-enable.
  REGISTER_8(NVIC_IPR_BASE + TIM2_IRQ_NUMBER) =
    (uint8_t)(TIM2_IRQ_PRIORITY << (8U - NVIC_IMPLEMENTED_PRIORITY_BITS));
  NVIC_ISER0 = (1UL << TIM2_IRQ_NUMBER);

  stdio.initialized = true;
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Queue a complete byte sequence for non-blocking stdout transmission.
 *
 * Initializes the trace path on first use. Admission is all-or-nothing; no
 * bytes are queued when the FIFO lacks capacity.
 *
 * @param[in] data Pointer to the byte sequence to queue.
 * @param[in] len Sequence length in bytes, from 1 through 1024.
 * @return true if every byte was queued; false if capacity was insufficient.
 */
bool EBF_stdoutWrite(const uint8_t* data, uint16_t len)
{
  uint16_t free_space;
  uint16_t index;
  bool accepted;
  bool was_empty;

  // Serialize with the ISR so admission and FIFO publication are atomic.
  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(data != NULL);
  EAF_ASSERT_IN_CRITICAL_SECTION(len > 0U);
  EAF_ASSERT_IN_CRITICAL_SECTION(len <= STDOUT_TX_BUFFER_CAPACITY);

  if (!stdio.initialized)
  {
    initialize();
  }

  accepted = false;
  free_space = (uint16_t)(STDOUT_TX_BUFFER_CAPACITY - stdio.count);
  if (len <= free_space)
  {
    was_empty = (stdio.count == 0U);
    for (index = 0U; index < len; ++index)
    {
      stdio.buffer[stdio.tail] = data[index];
      stdio.tail = (uint16_t)((stdio.tail + 1U) % STDOUT_TX_BUFFER_CAPACITY);
    }
    stdio.count = (uint16_t)(stdio.count + len);

    // Restart the periodic consumer only on the empty-to-nonempty transition.
    if (was_empty)
    {
      TIM2_CNT = 0UL;
      TIM2_SR &= ~TIM_SR_UIF;
      TIM2_CR1 |= TIM_CR1_CEN;
    }
    accepted = true;
  }

  EBF_CRITICAL_SECTION_EXIT();

  return accepted;
}

/**
 * @brief Service the TIM2 update interrupt for stdout transmission.
 *
 * Clears the update condition, transmits queued bytes in FIFO order while ITM
 * stimulus port zero accepts them immediately, and stops TIM2 when the queue
 * becomes empty.
 */
void TIM2_IRQHandler(void)
{
  // Acknowledge this update before draining so a later update remains pending.
  TIM2_SR &= ~TIM_SR_UIF;

  // Leave bytes queued whenever ITM cannot accept another byte immediately.
  while ((stdio.count > 0U) &&
         ((ITM_STIMULUS_PORT_0_32 & ITM_STIMULUS_FIFOREADY) != 0UL))
  {
    ITM_STIMULUS_PORT_0_8 = stdio.buffer[stdio.head];
    stdio.head = (uint16_t)((stdio.head + 1U) % STDOUT_TX_BUFFER_CAPACITY);
    stdio.count--;
  }

  if (stdio.count == 0U)
  {
    TIM2_CR1 &= ~TIM_CR1_CEN;
  }
}
