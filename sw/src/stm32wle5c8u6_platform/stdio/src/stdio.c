/*******************************************************************************
 * @brief Non-blocking SEGGER RTT standard output for STM32WLE5C8U6.
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
#include "SEGGER_RTT.h"
#include "ebf.h"

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/** @brief SEGGER RTT terminal channel used for standard output. */
#define STDOUT_RTT_CHANNEL 0U

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/** @brief Whether the RTT control block has been initialized. */
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

/**
 * @brief Attempt to queue a complete byte sequence in RTT channel zero.
 *
 * The configured no-block/skip mode preserves all-or-nothing admission: the
 * complete sequence is queued when capacity is available, otherwise no bytes
 * are queued.
 *
 * @param[in] data Pointer to data to be written.
 * @param[in] len Number of bytes to attempt.
 * @return true if every byte was accepted; false otherwise.
 */
bool EBF_stdoutWrite(const uint8_t* data, uint16_t len)
{
  unsigned int written;
  bool accepted;

  accepted = false;
  if ((data != NULL) && (len > 0U))
  {
    EBF_CRITICAL_SECTION_ENTRY();

    if (!initialized)
    {
      SEGGER_RTT_Init();
      initialized = true;
    }

    written = SEGGER_RTT_WriteNoLock(STDOUT_RTT_CHANNEL, data, len);
    accepted = (written == (unsigned int)len);

    EBF_CRITICAL_SECTION_EXIT();
  }

  return accepted;
}
