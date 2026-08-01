/*******************************************************************************
 * @brief Event Driven Framework (EDF) event implementation.
 *
 * @copyright
 * Copyright (c) 2026 FBS93.
 * See the LICENSE file of this project for license details.
 * This notice shall be retained in all copies or substantial portions
 * of the software.
 *
 * @note
 * This file is a derivative work based on:
 * QP/C (c) Quantum Leaps, LLC.
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
#include <stddef.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "edf_event.h"
#include "edf_framework.h"
#include "edf_active_object.h"
#include "edf_core.h"
#include "edf.h"
#include "eaf.h"
#include "ebf.h"
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

void EDF_event_initImmutable(EDF_event_t* me, EDF_event_signal_t sig)
{
  EAF_ASSERT(me != NULL);

  me->sig = (EDF_event_signal_t)sig;
  me->pool_num = 0x00U;
  me->ref_cnt = EDF_EVENT_REF_CNT_IMMUTABLE;
}

EDF_event_t* EDF_event_initMutable(uint_fast16_t e_size, int sig)
{
  uint8_t pool_idx;
  EDF_event_t* e;

  // Search the pool index that fits the requested event size.
  for (pool_idx = 0; pool_idx < EDF_framework.pool_registry.max_pool;
       ++pool_idx)
  {
    if (e_size <=
        ((uint_fast16_t)EDF_framework.pool_registry.list[pool_idx].block_size))
    {
      break;
    }
  }

  // Cannot run out of registered pools.
  EAF_ASSERT(pool_idx < EDF_framework.pool_registry.max_pool);

  // Get pool slot.
  e =
    (EDF_event_t*)EDF_pool_acquire(&EDF_framework.pool_registry.list[pool_idx]);

  // New event allocated correctly?
  if (e != NULL)
  {
    // Clear full acquired pool slot to ensure deterministic initial state.
    EMF_utils_clear(e, EDF_framework.pool_registry.list[pool_idx].block_size);

    // Initialize event attributes.
    e->sig = (EDF_event_signal_t)sig;
    e->pool_num = pool_idx + 1;
    e->ref_cnt = 0U;
  }
  else
  {
    /**
     * @note This assertion means that the event allocation failed,
     * and this failure cannot be tolerated.
     */
    EAF_ERROR();
  }

  return e;
}

void EDF_event_incrementRefCnt_(EDF_event_t* me)
{
  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);

  me->ref_cnt++;
}

void EDF_event_decrementRefCnt_(EDF_event_t* me)
{
  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);

  me->ref_cnt--;
}

void EDF_event_gc(EDF_event_t* e)
{
  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(e != NULL);

  // Is it a pool event (mutable event)?
  if (e->pool_num != 0U)
  {
    // Isn't this the last reference?
    if (e->ref_cnt > 1U)
    {
      EAF_ASSERT_IN_CRITICAL_SECTION(e->ref_cnt > 0U);

      EDF_event_decrementRefCnt_(e);

      EBF_CRITICAL_SECTION_EXIT();
    }
    else
    {
      // Pool number must be in range.
      EAF_ASSERT_IN_CRITICAL_SECTION(
        (e->pool_num <= EDF_framework.pool_registry.max_pool) &&
        (e->pool_num <= EDF_MAX_POOL));

      EBF_CRITICAL_SECTION_EXIT();

      // This is the last reference to this event, so recycle it.
      EDF_pool_release(&EDF_framework.pool_registry.list[e->pool_num - 1U], e);
    }
  }
  else
  {
    EBF_CRITICAL_SECTION_EXIT();
  }
}
