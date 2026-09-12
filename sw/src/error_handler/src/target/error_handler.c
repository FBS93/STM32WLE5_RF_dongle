/*******************************************************************************
 * @brief Project-specific fatal-error handling.
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
#include "eaf.h"
#include "emf.h"

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief Access a 32-bit memory-mapped register.
 *
 * volatile_use: hardware_interaction
 */
#define REGISTER_32(address) (*(volatile uint32_t*)(address))

/** @brief RCC AHB2 peripheral clock-enable register. */
#define RCC_AHB2ENR REGISTER_32(0x5800004CUL)
/** @brief GPIOA clock-enable bit. */
#define RCC_AHB2ENR_GPIOAEN (1UL << 0U)
/** @brief GPIOA mode register. */
#define GPIOA_MODER REGISTER_32(0x48000000UL)
/** @brief GPIOA bit set/reset register. */
#define GPIOA_BSRR REGISTER_32(0x48000018UL)
/** @brief PA7 mode field mask. */
#define GPIOA_MODE_7_MASK (3UL << 14U)
/** @brief PA7 general-purpose output mode. */
#define GPIOA_MODE_7_OUTPUT (1UL << 14U)
/** @brief PA7 reset bit, which turns on the active-low user LED. */
#define GPIOA_BSRR_BR7 (1UL << 23U)

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

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

/** @brief Handles fatal errors by asserting the user LED and halting. */
void EAF_onError(const char* file, int line, int id)
{
  EMF_UTILS_UNUSED_PARAM(file);
  EMF_UTILS_UNUSED_PARAM(line);
  EMF_UTILS_UNUSED_PARAM(id);

  RCC_AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

  // Readback ensures the GPIOA clock-enable write completes before GPIO access.
  (void)RCC_AHB2ENR;

  // Latch PA7 low before output mode so the active-low LED asserts immediately.
  GPIOA_BSRR = GPIOA_BSRR_BR7;
  GPIOA_MODER = (GPIOA_MODER & ~GPIOA_MODE_7_MASK) | GPIOA_MODE_7_OUTPUT;

  while (1)
  {
    // Forever. This function shall not return.
  }
}
