/*******************************************************************************
 * @brief Event Driven Framework (EDF) event queue implementation.
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
#include <stdbool.h>
#include <stddef.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "edf_event_queue.h"
#include "eaf.h"
#include "edf_event.h"
#include "edf_active_object.h"
#include "edf_core.h"
#include "edf.h"
#include "ebf.h"

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

void EDF_eventQueue_init(EDF_eventQueue_t* me,
                         const EDF_event_t** q_storage,
                         EDF_eventQueue_cnt_t q_len)
{
  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);

  me->front_e = NULL;  // No events in the queue.
  me->ring = q_storage;
  me->end = (EDF_eventQueue_cnt_t)q_len;
  me->head = 0U;
  me->tail = 0U;
  me->n_free = (EDF_eventQueue_cnt_t)(q_len + 1U);  // +1 for front_e.
  me->n_min = me->n_free;

  EBF_CRITICAL_SECTION_EXIT();
}

void EDF_eventQueue_postFIFO(EDF_eventQueue_t* me, const EDF_event_t* e)
{
  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);
  EAF_ASSERT_IN_CRITICAL_SECTION(e != NULL);

  // Can the queue accept the event?
  if (me->n_free > 0U)
  {
    // Is it a mutable event?
    if (e->pool_num != 0U)
    {
      /**
       * Safety margin allows up to 2 refs per AO (normal queue + deferred
       * queue).
       */
      EAF_ASSERT_IN_CRITICAL_SECTION(e->ref_cnt < (2U * EDF_MAX_ACTIVE_OBJECT));

      // Typecast to discard const qualifier.
      EDF_event_incrementRefCnt_((EDF_event_t*)e);
    }

    me->n_free--;
    if (me->n_min > me->n_free)
    {
      me->n_min = me->n_free;  // Update minimum so far.
    }

    // Is the queue empty?
    if (me->front_e == NULL)
    {
      // Deliver event directly.
      me->front_e = e;
    }
    else
    {  // Queue was not empty, insert event into the ring-buffer.
      me->ring[me->head] = e;

      if (me->head == 0U)
      {  // Need to wrap the head?
        me->head = me->end;
      }
      me->head--;  // Advance head (counter-clockwise).
    }
  }
  else
  {
    /**
     * @note This assertion means that the post failed,
     * and this failure cannot be tolerated.
     */
    EAF_ERROR_IN_CRITICAL_SECTION();
  }

  EBF_CRITICAL_SECTION_EXIT();
}

void EDF_eventQueue_postLIFO(EDF_eventQueue_t* me, const EDF_event_t* e)
{
  EDF_event_t* front_e;

  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);
  EAF_ASSERT_IN_CRITICAL_SECTION(e != NULL);

  // Can the queue accept the event?
  if (me->n_free > 0U)
  {
    // Is it a mutable event?
    if (e->pool_num != 0U)
    {
      /**
       * Safety margin allows up to 2 refs per AO (normal queue + deferred
       * queue).
       */
      EAF_ASSERT_IN_CRITICAL_SECTION(e->ref_cnt < (2U * EDF_MAX_ACTIVE_OBJECT));

      // Typecast to discard const qualifier.
      EDF_event_incrementRefCnt_((EDF_event_t*)e);
    }

    --me->n_free;
    if (me->n_min > me->n_free)
    {
      me->n_min = me->n_free;  // Update minimum so far.
    }

    // Typecast to discard const qualifier.
    front_e = (EDF_event_t*)me->front_e;

    me->front_e = e;  // Deliver the event directly to the front.

    if (front_e != NULL)
    {
      me->tail++;
      if (me->tail == me->end)
      {
        me->tail = 0U;  // Wrap around.
      }
      me->ring[me->tail] = front_e;
    }
  }
  else
  {
    /**
     * @note This assertion means that the post failed,
     * and this failure cannot be tolerated.
     */
    EAF_ERROR_IN_CRITICAL_SECTION();
  }

  EBF_CRITICAL_SECTION_EXIT();
}

const EDF_event_t* EDF_eventQueue_get(EDF_eventQueue_t* me)
{
  EDF_event_t* e;
  EDF_event_t* front_e;

  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);

  // Always remove event from the front.
  e = (EDF_event_t*)me->front_e;  // Typecast to discard const qualifier.

  if (e != NULL)
  {
    me->n_free++;  // One more free event in the queue.

    // Any events in the ring buffer?
    if (me->n_free <= me->end)
    {
      // Remove event from the tail.
      // Typecast to discard const qualifier.
      front_e = (EDF_event_t*)me->ring[me->tail];

      EAF_ASSERT_IN_CRITICAL_SECTION(front_e != NULL);
      me->front_e = front_e;  // Update the original.

      // Need to wrap the tail?
      if (me->tail == 0U)
      {
        me->tail = me->end;
      }
      me->tail--;  // Advance the tail (counter-clockwise).
    }
    else
    {
      me->front_e = NULL;  // Queue becomes empty.

      // All entries in the queue must be free (+1 for front_e).
      EAF_ASSERT_IN_CRITICAL_SECTION(me->n_free == (me->end + 1U));
    }
  }

  EBF_CRITICAL_SECTION_EXIT();

  return e;
}
