/*******************************************************************************
 * @brief Event Driven Framework (EDF) active object implementation.
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
#include "edf_active_object.h"
#include "edf_framework.h"
#include "edf_hsm.h"
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

void EDF_activeObject_init(EDF_activeObject_t* me,
                           EDF_hsm_stateHandler_t initial)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(initial != NULL);
  EAF_ASSERT_BLOCK_END();

  // Clear the whole active object data structure.
  EMF_utils_clear(me, sizeof(*me));

  // Initialize the active object's HSM.
  EDF_hsm_init((EDF_hsm_t*)(me), initial);
}

void EDF_activeObject_pubSubInit(EDF_activeObject_bitmask_t* subs_list,
                                 EDF_event_signal_t max_signal)
{
  EAF_ASSERT(subs_list != NULL);

  EDF_framework.subscr_registry.list = subs_list;
  EDF_framework.subscr_registry.max_signal = max_signal;

  // Initialize the subscriber list.
  for (EDF_event_signal_t idx = 0; idx < max_signal; ++idx)
  {
    EMF_bitmask_clearAll(subs_list[idx].bitmask,
                         EMF_UTILS_SIZEOF_ARRAY(subs_list[idx].bitmask));
  }
}

void EDF_activeObject_publish(const EDF_event_t* e)
{
  EDF_activeObject_bitmask_t subs_list;
  EDF_activeObject_t* ao;
  uint8_t prio;

  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(e != NULL);
  EAF_ASSERT_IN_CRITICAL_SECTION(e->sig <=
                                 EDF_framework.subscr_registry.max_signal);

  // Is it a mutable event?
  if (e->pool_num != 0U)
  {
    /**
     * @note If a lower-priority AO publishes to a higher-priority one,
     * it can preempt and recycle the event before multicasting finishes,
     * prematurely freeing it from the pool. To prevent this, the reference
     * counter of a mutable event is incremented before multicasting.
     * At the end, @ref EDF_event_gc() decrements the counter and recycles the
     * event if it drops to zero.
     */
    // Safety margin allows up to 2 refs per AO (normal queue + deferred queue).
    EAF_ASSERT_IN_CRITICAL_SECTION(e->ref_cnt < (2U * EDF_MAX_ACTIVE_OBJECT));

    // Typecast to discard const qualifier.
    EDF_event_incrementRefCnt_((EDF_event_t*)e);
  }

  // Make a local, modifiable copy of the signal subscribers list.
  subs_list = EDF_framework.subscr_registry.list[e->sig - 1];

  EBF_CRITICAL_SECTION_EXIT();

  // Any subscribers?
  if (EMF_bitmask_isAnySet(subs_list.bitmask,
                           EMF_UTILS_SIZEOF_ARRAY(subs_list.bitmask)))
  {
    // Get highest-prio subscriber.
    prio = EMF_bitmask_findMax(subs_list.bitmask,
                               EMF_UTILS_SIZEOF_ARRAY(subs_list.bitmask));

    EDF_CORE_SCHED_STATUS
    EDF_CORE_SCHED_LOCK(prio);  // Lock the scheduler up to AO's prio.

    while (EMF_bitmask_isAnySet(subs_list.bitmask,
                                EMF_UTILS_SIZEOF_ARRAY(subs_list.bitmask)))
    {
      // Get highest-prio subscriber.
      prio = EMF_bitmask_findMax(subs_list.bitmask,
                                 EMF_UTILS_SIZEOF_ARRAY(subs_list.bitmask));

      ao = EDF_framework.ao_registry[prio];  // Get active object reference.
      EDF_activeObject_postFIFO(ao, e);

      // Remove the handled subscriber.
      EMF_bitmask_clearBit(subs_list.bitmask, prio);
    }

    EDF_CORE_SCHED_UNLOCK();  // Unlock the scheduler.
  }

  EDF_event_gc((EDF_event_t*)e);  // Typecast to discard const qualifier.
}

void EDF_activeObject_subscribe(const EDF_activeObject_t* me,
                                EDF_event_signal_t sig)
{
  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);
  EAF_ASSERT_IN_CRITICAL_SECTION(EDF_HSM_USER_SIGNAL <= sig);
  EAF_ASSERT_IN_CRITICAL_SECTION(sig <=
                                 EDF_framework.subscr_registry.max_signal);
  EAF_ASSERT_IN_CRITICAL_SECTION(me->prio > 0U);
  EAF_ASSERT_IN_CRITICAL_SECTION(me->prio <= EDF_MAX_ACTIVE_OBJECT);
  EAF_ASSERT_IN_CRITICAL_SECTION(EDF_framework.ao_registry[me->prio] == me);

  // Insert the priority into the subscriber set.
  EMF_bitmask_setBit(EDF_framework.subscr_registry.list[sig - 1].bitmask,
                     me->prio);

  EBF_CRITICAL_SECTION_EXIT();
}

void EDF_activeObject_subscribeAll(const EDF_activeObject_t* me)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(me->prio > 0U);
  EAF_ASSERT_IN_BLOCK(me->prio <= EDF_MAX_ACTIVE_OBJECT);
  EAF_ASSERT_IN_BLOCK(EDF_framework.ao_registry[me->prio] == me);
  EAF_ASSERT_BLOCK_END();

  for (EDF_event_signal_t idx = (EDF_HSM_USER_SIGNAL - 1);
       idx < EDF_framework.subscr_registry.max_signal;
       ++idx)
  {
    EBF_CRITICAL_SECTION_ENTRY();

    if (!EMF_bitmask_isBitSet(EDF_framework.subscr_registry.list[idx].bitmask,
                              me->prio))
    {
      // Insert the priority into the subscriber set.
      EMF_bitmask_setBit(EDF_framework.subscr_registry.list[idx].bitmask,
                         me->prio);
    }
    EBF_CRITICAL_SECTION_EXIT();

    EBF_NOP();  // Prevent merging critical sections.
  }
}

void EDF_activeObject_unsubscribe(const EDF_activeObject_t* me,
                                  EDF_event_signal_t sig)
{
  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);
  EAF_ASSERT_IN_CRITICAL_SECTION(EDF_HSM_USER_SIGNAL <= sig);
  EAF_ASSERT_IN_CRITICAL_SECTION(sig <=
                                 EDF_framework.subscr_registry.max_signal);
  EAF_ASSERT_IN_CRITICAL_SECTION(me->prio > 0U);
  EAF_ASSERT_IN_CRITICAL_SECTION(me->prio <= EDF_MAX_ACTIVE_OBJECT);
  EAF_ASSERT_IN_CRITICAL_SECTION(EDF_framework.ao_registry[me->prio] == me);

  // Remove the priority from the subscriber set.
  EMF_bitmask_clearBit(EDF_framework.subscr_registry.list[sig - 1].bitmask,
                       me->prio);

  EBF_CRITICAL_SECTION_EXIT();
}

void EDF_activeObject_unsubscribeAll(const EDF_activeObject_t* me)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(me->prio > 0U);
  EAF_ASSERT_IN_BLOCK(me->prio <= EDF_MAX_ACTIVE_OBJECT);
  EAF_ASSERT_IN_BLOCK(EDF_framework.ao_registry[me->prio] == me);
  EAF_ASSERT_BLOCK_END();

  for (EDF_event_signal_t idx = (EDF_HSM_USER_SIGNAL - 1);
       idx < EDF_framework.subscr_registry.max_signal;
       ++idx)
  {
    EBF_CRITICAL_SECTION_ENTRY();

    if (EMF_bitmask_isBitSet(EDF_framework.subscr_registry.list[idx].bitmask,
                             me->prio))
    {
      // Remove the priority from the subscriber set.
      EMF_bitmask_clearBit(EDF_framework.subscr_registry.list[idx].bitmask,
                           me->prio);
    }
    EBF_CRITICAL_SECTION_EXIT();

    EBF_NOP();  // Prevent merging critical sections.
  }
}

void EDF_activeObject_defer(const EDF_activeObject_t* me,
                            EDF_eventQueue_t* e_queue,
                            const EDF_event_t* e)
{
  EMF_UTILS_UNUSED_PARAM(me);

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(e_queue != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  // Store the event in the deferred queue.
  EDF_eventQueue_postFIFO(e_queue, e);
}

void EDF_activeObject_recall(EDF_activeObject_t* me, EDF_eventQueue_t* e_queue)
{
  EDF_event_t* e;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e_queue != NULL);
  EAF_ASSERT_BLOCK_END();

  // Typecast to discard const qualifier.
  e = (EDF_event_t*)EDF_eventQueue_get(e_queue);

  EAF_ASSERT(e != NULL);             // Queue shall not be empty when recalling.
  EDF_activeObject_postLIFO(me, e);  // Post recalled event.

  EBF_CRITICAL_SECTION_ENTRY();

  // Is it a mutable event?
  if (e->pool_num != 0U)
  {
    /**
     * @note After posting to the AO's queue the event must be referenced
     * at least twice: once in the deferred event queue (e_queue->get()
     * did NOT decrement the reference counter) and once in the
     * AO's event queue.
     */
    EAF_ASSERT_IN_CRITICAL_SECTION(e->ref_cnt >= 2U);

    // Decrement ref counter for removing event from deferred queue.
    EDF_event_decrementRefCnt_(e);
  }

  EBF_CRITICAL_SECTION_EXIT();
}

uint_fast16_t EDF_activeObject_flushDeferred(const EDF_activeObject_t* me,
                                             EDF_eventQueue_t* e_queue,
                                             uint_fast16_t num)
{
  uint_fast16_t n;
  EDF_event_t* e;

  EMF_UTILS_UNUSED_PARAM(me);

  EAF_ASSERT(e_queue != NULL);

  n = 0U;
  while (n < num)
  {
    // Typecast to discard const qualifier.
    e = (EDF_event_t*)EDF_eventQueue_get(e_queue);

    if (e != NULL)
    {
      ++n;
      EDF_event_gc(e);
    }
    else
    {
      break;
    }
  }

  return n;
}

void EDF_activeObject_register(EDF_activeObject_t* me)
{
  uint8_t prev_thre;
  uint8_t next_thre;

  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);
  EAF_ASSERT_IN_CRITICAL_SECTION(me->prio > 0U);
  EAF_ASSERT_IN_CRITICAL_SECTION(me->prio <= EDF_MAX_ACTIVE_OBJECT);
  EAF_ASSERT_IN_CRITICAL_SECTION(EDF_framework.ao_registry[me->prio] == NULL);

  // Preemption-threshold not defined?
  if (me->pthre == 0U)
  {
    me->pthre = me->prio;  // Apply the default.
  }

  EAF_ASSERT_IN_CRITICAL_SECTION(me->prio <= me->pthre);

  prev_thre = me->pthre;
  next_thre = me->pthre;

  // Find nearest lower-priority AO and get its threshold.
  for (uint8_t prio = (uint8_t)(me->prio - 1U); prio > 0U; --prio)
  {
    if (EDF_framework.ao_registry[prio] != NULL)
    {
      prev_thre = EDF_framework.ao_registry[prio]->pthre;
      break;
    }
  }

  // Find nearest higher-priority AO and get its threshold.
  for (uint8_t prio = (uint8_t)me->prio + 1U; prio <= EDF_MAX_ACTIVE_OBJECT;
       ++prio)
  {
    if (EDF_framework.ao_registry[prio] != NULL)
    {
      next_thre = EDF_framework.ao_registry[prio]->pthre;
      break;
    }
  }

  // Threshold consistency check. See @ref preemption_threshold.
  EAF_ASSERT_IN_CRITICAL_SECTION(prev_thre <= me->pthre);
  EAF_ASSERT_IN_CRITICAL_SECTION(me->pthre <= next_thre);

  // Register the AO at the QF-prio.
  EDF_framework.ao_registry[me->prio] = me;

  EBF_CRITICAL_SECTION_EXIT();
}

void EDF_activeObject_postFIFO(EDF_activeObject_t* me, const EDF_event_t* e)
{
  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);
  EAF_ASSERT_IN_CRITICAL_SECTION(e != NULL);

  // Can the queue accept the event?
  if (me->e_queue.n_free > 0U)
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

    me->e_queue.n_free--;
    if (me->e_queue.n_min > me->e_queue.n_free)
    {
      me->e_queue.n_min = me->e_queue.n_free;  // Update minimum so far.
    }

    // Is the queue empty?
    if (me->e_queue.front_e == NULL)
    {
      // Deliver event directly.
      me->e_queue.front_e = e;
      EDF_CORE_NOTIFY_EVENT(me);  // Notify the active object.
    }
    else
    {  // Queue was not empty, insert event into the ring-buffer.
      me->e_queue.ring[me->e_queue.head] = e;

      if (me->e_queue.head == 0U)
      {  // Need to wrap the head?
        me->e_queue.head = me->e_queue.end;
      }
      me->e_queue.head--;  // Advance the head (counter-clockwise).
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

void EDF_activeObject_postLIFO(EDF_activeObject_t* me, const EDF_event_t* e)
{
  EDF_event_t* front_e;

  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);
  EAF_ASSERT_IN_CRITICAL_SECTION(e != NULL);

  // Can the queue accept the event?
  if (me->e_queue.n_free > 0U)
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

    me->e_queue.n_free--;
    if (me->e_queue.n_min > me->e_queue.n_free)
    {
      me->e_queue.n_min = me->e_queue.n_free;  // Update minimum so far.
    }

    // Typecast to discard const qualifier.
    front_e = (EDF_event_t*)me->e_queue.front_e;

    // Deliver the event directly to the front.
    me->e_queue.front_e = e;

    if (front_e != NULL)
    {  // Was the queue NOT empty?
      me->e_queue.tail++;
      if (me->e_queue.tail == me->e_queue.end)
      {
        me->e_queue.tail = 0U;  // Wrap around.
      }
      me->e_queue.ring[me->e_queue.tail] = front_e;
    }
    else
    {
      // Queue was empty.
      EDF_CORE_NOTIFY_EVENT(me);  // Notify the active object.
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

const EDF_event_t* EDF_activeObject_get(EDF_activeObject_t* me)
{
  EDF_event_t* e;
  EDF_event_t* front_e;

  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);

  EDF_CORE_WAIT_FOR_EVENT(me);

  // Typecast to discard const qualifier.
  e = (EDF_event_t*)me->e_queue.front_e;

  EAF_ASSERT_IN_CRITICAL_SECTION(e != NULL);  // Queue must NOT be empty.
  me->e_queue.n_free++;  // One more free event in the queue.

  // Any events in the ring buffer?
  if (me->e_queue.n_free <= me->e_queue.end)
  {
    // Remove event from the tail.
    // Typecast to discard const qualifier.
    front_e = (EDF_event_t*)me->e_queue.ring[me->e_queue.tail];

    EAF_ASSERT_IN_CRITICAL_SECTION(front_e != NULL);
    me->e_queue.front_e = front_e;  // Update the original.

    // Need to wrap the tail?
    if (me->e_queue.tail == 0U)
    {
      me->e_queue.tail = me->e_queue.end;
    }
    me->e_queue.tail--;  // Advance the tail (counter-clockwise).
  }
  else
  {
    me->e_queue.front_e = NULL;  // Queue becomes empty.

    // All entries in the queue must be free (+1 for fronEvt).
    EAF_ASSERT_IN_CRITICAL_SECTION(me->e_queue.n_free ==
                                   (me->e_queue.end + 1U));
  }

  EBF_CRITICAL_SECTION_EXIT();

  return e;
}

/**
 * @anchor duplicated_code_in_post_functions
 * @par Duplicated code in post functions
 *
 * The implementations of @ref EDF_activeObject_postFIFO() and
 * @ref EDF_activeObject_postLIFO() contain code that is largely duplicated
 * from @ref EDF_eventQueue_postFIFO() and @ref EDF_eventQueue_postLIFO().
 * This duplication is intentional and necessary.
 *
 * The difference is that the active-object variants must call
 * @ref EDF_CORE_NOTIFY_EVENT(me) when posting to an empty queue. This
 * notification must occur *within the same critical section* to ensure
 * atomicity and prevent race conditions.
 *
 * Attempting to unify the functions (e.g. by calling into a shared function in
 * the edf_event_queue module) would require exiting and re-entering the
 * critical section, leaving a race condition window.
 *
 * Therefore, although the logic looks duplicated, the implementations are
 * deliberately kept separate to preserve correctness and avoid race conditions.
 */

/**
 * @anchor duplicated_code_in_get_functions
 * @par Duplicated code in get functions
 *
 * The implementations of @ref EDF_activeObject_get() and
 * @ref EDF_eventQueue_get() contain code that is largely duplicated. This
 * duplication is intentional.
 *
 * The difference is that @ref EDF_activeObject_get() must additionally call
 * @ref EDF_CORE_WAIT_FOR_EVENT(me) to wait until an event arrives. This step
 * must occur within the same critical section to ensure atomicity and prevent
 * race conditions.
 *
 * Attempting to unify the functions (e.g. by calling into a shared function in
 * the edf_event_queue module) would require exiting and re-entering the
 * critical section, leaving a race condition window.
 *
 * Therefore, although the logic looks duplicated, the implementations are
 * deliberately kept separate to preserve correctness and avoid race conditions.
 */
