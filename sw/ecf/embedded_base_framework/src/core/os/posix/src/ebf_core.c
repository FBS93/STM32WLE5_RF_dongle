/*******************************************************************************
 * @brief Core/port-specific base implementation.
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
#include <pthread.h>
#include <stdio.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "ebf_core.h"
#include "ebf_utils.h"
#include "ebf.h"
#include "eaf.h"
#include "emf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

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

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/**
 * @todo If an init function is added to this module, move initialization there.
 */
pthread_mutex_t EBF_critSecMutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @todo If an init function is added to this module, move initialization there.
 */
int8_t EBF_critSecNestCnt = 0;

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

EBF_WEAK void EBF_setStdinListener(EBF_stdin_t listener)
{
  EMF_UTILS_UNUSED_PARAM(listener);
}

EBF_WEAK void EBF_stdoutWrite(const uint8_t* data, uint16_t len)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(len > 0);
  EAF_ASSERT_BLOCK_END();

  for (uint16_t i = 0; i < len; ++i)
  {
    fputc((int)data[i], stdout);
  }

  fflush(stdout);  // Flush to force immediate output
}

EBF_WEAK bool EBF_stdoutIsReadyToWrite(uint16_t len)
{
  EMF_UTILS_UNUSED_PARAM(len);

  return true;
}

void EBF_entryCriticalSection(void)
{
  (void)pthread_mutex_lock(&EBF_critSecMutex);

  // Nested critical sections are not supported.
  EAF_ASSERT_IN_CRITICAL_SECTION(EBF_critSecNestCnt == 0);

  EBF_critSecNestCnt++;
}

void EBF_exitCriticalSection(void)
{
  // Nested critical sections are not supported.
  EAF_ASSERT_IN_CRITICAL_SECTION(EBF_critSecNestCnt == 1);

  EBF_critSecNestCnt--;
  (void)pthread_mutex_unlock(&EBF_critSecMutex);
}

bool EBF_isInCriticalSection(void)
{
  return (EBF_critSecNestCnt > 0);
}

void EBF_noOperation(void)
{
  __asm__ volatile("nop");
}