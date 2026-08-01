/*******************************************************************************
 * @brief Event Driven Framework (EDF) time event implementation.
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
#include "edf_time_event.h"
#include "eaf.h"
#include "edf_framework.h"
#include "edf_active_object.h"
#include "edf.h"
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

void EDF_timeEvent_new(EDF_timeEvent_t* me,
                       EDF_activeObject_t* ao,
                       EDF_event_signal_t sig,
                       uint8_t tick_rate)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(ao != NULL);
  EAF_ASSERT_IN_BLOCK(sig != 0);
  EAF_ASSERT_IN_BLOCK(tick_rate < EDF_MAX_TICK_RATE);
  EAF_ASSERT_BLOCK_END();

  // Initialize base type.
  EDF_event_initImmutable(&me->super, sig);

  // Initialize time event attributes.
  me->next = NULL;
  me->ao = ao;
  me->cnt = 0U;
  me->period = 0U;
  me->tick_rate = tick_rate;
  me->is_linked = false;
}

void EDF_timeEvent_arm(EDF_timeEvent_t* me,
                       EDF_timeEvent_cnt_t start,
                       EDF_timeEvent_cnt_t period)
{
  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);
  EAF_ASSERT_IN_CRITICAL_SECTION(me->cnt == 0U);
  EAF_ASSERT_IN_CRITICAL_SECTION(me->ao != NULL);
  EAF_ASSERT_IN_CRITICAL_SECTION(me->tick_rate < EDF_MAX_TICK_RATE);
  EAF_ASSERT_IN_CRITICAL_SECTION(start != 0U);

  me->cnt = start;
  me->period = period;

  /**
   * Is the time event unlinked? See @ref time_event_disarmed_but_linked.
   * This check allows disarming and re-arming within the same tick
   * without duplicating links.
   */
  if (!me->is_linked)
  {
    me->is_linked = true;  // Mark as linked.

    // Link into the temporary new list. See @ref time_event_new_list.
    me->next = EDF_framework.te_registry[me->tick_rate].new_head;
    EDF_framework.te_registry[me->tick_rate].new_head = me;
  }

  EBF_CRITICAL_SECTION_EXIT();
}

bool EDF_timeEvent_disarm(EDF_timeEvent_t* me)
{
  bool was_armed;

  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);

  // Was the time event actually armed?
  if (me->cnt != 0U)
  {
    was_armed = true;
    me->cnt = 0U;  // Schedule removal from the list.
  }
  else
  {
    // The time event was already disarmed.
    was_armed = false;
  }

  EBF_CRITICAL_SECTION_EXIT();

  return was_armed;
}

bool EDF_timeEvent_rearm(EDF_timeEvent_t* me, EDF_timeEvent_cnt_t time)
{
  bool was_armed;

  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);
  EAF_ASSERT_IN_CRITICAL_SECTION(me->ao != NULL);
  EAF_ASSERT_IN_CRITICAL_SECTION(me->tick_rate < EDF_MAX_TICK_RATE);
  EAF_ASSERT_IN_CRITICAL_SECTION(time != 0U);

  // Was the time event not running?
  if (me->cnt == 0U)
  {
    was_armed = false;

    /**
     * Is the time event unlinked? See @ref time_event_disarmed_but_linked.
     * This check allows disarming and re-arming within the same tick
     * without duplicating links.
     */
    if (!me->is_linked)
    {
      me->is_linked = true;  // Mark as linked.

      // Link into the temporary new list. See @ref time_event_new_list.
      me->next = EDF_framework.te_registry[me->tick_rate].new_head;
      EDF_framework.te_registry[me->tick_rate].new_head = me;
    }
  }
  else
  {
    // The time event was armed.
    was_armed = true;
  }

  // Set new counter value.
  me->cnt = time;

  EBF_CRITICAL_SECTION_EXIT();

  return was_armed;
}

EDF_timeEvent_cnt_t EDF_timeEvent_currentCounter(const EDF_timeEvent_t* me)
{
  EDF_timeEvent_cnt_t cnt;

  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);

  cnt = me->cnt;

  EBF_CRITICAL_SECTION_EXIT();

  return cnt;
}

void EDF_timeEvent_tick(uint8_t tick_rate)
{
  EDF_timeEventRegistry_t* registry;
  EDF_timeEvent_t** te_link;
  EDF_timeEvent_t* te;

  /**
   * @note
   * The implementation uses a pointer-to-pointer
   * (@c EDF_timeEvent_t **te_link)
   * to avoid special-casing the first element of the time event registry:
   * - At the start, @c te_link = &registry->armed_head.
   * - As we traverse the armed time events list, @c te_link is updated to point
   *   to the @c next field of the current node: @c te_link = &te->next, where
   *   @c te = *te_link.
   * - Removing a node is always @c *te_link = te->next; (works for both the
   * first node and all subsequent nodes).
   *
   * Attaching freshly armed time events (@c registry->new_head) is also
   * unified:
   * - When @c *te_link == NULL and @c registry->new_head != NULL, simply do
   *   @c *te_link = registry->new_head; (this attaches the new time events list
   * either at the beginning of registry->armed_head or after its last node).
   *
   * This way the same code handles head removal, internal removals and
   * attaching new time events without branching on special cases.
   */

  EBF_CRITICAL_SECTION_ENTRY();

  // Make sure tick_rate is valid.
  EAF_ASSERT_IN_CRITICAL_SECTION(
    tick_rate < EMF_UTILS_SIZEOF_ARRAY(EDF_framework.te_registry));

  // Local reference to EDF_framework.te_registry[tick_rate] for readability.
  registry = &EDF_framework.te_registry[tick_rate];

  // Initialize to head of armed time events list.
  te_link = &registry->armed_head;

  // Set the current time event to the first node of armed list.
  te = *te_link;

  while (true)
  {
    if (te == NULL)
    {
      // No more time events in the armed list.
      if (registry->new_head == NULL)
      {
        break;  // No more time events in the new list.
      }

      /**
       * Attach new time events to armed list;
       * Updates registry->armed_head or registry->armed_head[last].next
       * depending on the current te_link.
       */
      *te_link = registry->new_head;
      registry->new_head = NULL;
      te = *te_link;  // Continue from the newly attached list.
    }

    if (te->cnt == 0U)
    {
      // "te" scheduled for removal.
      *te_link = te->next;  // Unlink.
      te->is_linked = false;
      // Do not advance te_link, because we just unlinked "te".

      EBF_CRITICAL_SECTION_EXIT();
    }
    else if (te->cnt == 1U)
    {
      if (te->period != 0U)
      {
        te->cnt = te->period;  // Periodic --> reload counter.

        // Advance te_link to the next node.
        te_link = &te->next;
      }
      else
      {
        te->cnt = 0U;         // One-shot --> disarm.
        *te_link = te->next;  // Unlink.
        te->is_linked = false;
        // Do not advance te_link, because we just unlinked "te".
      }

      EBF_CRITICAL_SECTION_EXIT();

      // Post the event to its active object.
      EDF_activeObject_postFIFO(te->ao, &te->super);
    }
    else
    {
      te->cnt--;  // Normal countdown.

      // Advance te_link to the next node.
      te_link = &te->next;

      EBF_CRITICAL_SECTION_EXIT();
    }

    // Re-enter critical section to continue loop.
    EBF_CRITICAL_SECTION_ENTRY();

    // Update "te" to the current node after any modifications.
    te = *te_link;
  }

  EBF_CRITICAL_SECTION_EXIT();
}

/**
 * @anchor time_event_disarmed_but_linked
 * @par Time event disarmed but linked
 *
 * A time event can be "disarmed but still linked" for the duration of one
 * tick interval:
 * - @ref EDF_timeEvent_disarm() sets @c cnt=0 but does not unlink the time
 *   event.
 * - Unlinking is performed exclusively in @ref EDF_timeEvent_tick().
 */

/**
 * @anchor time_event_new_list
 * @par Time event new list
 *
 * The list of armed time events
 * (@c EDF_framework.te_registry[tick_rate].armed_head) is modified exclusively
 * inside @ref EDF_timeEvent_tick(). Newly armed time events are first linked
 * into a temporary list (@c EDF_framework.te_registry[tick_rate].new_head) and
 * only merged into the main list during the tick processing. This design
 * allows @ref EDF_timeEvent_tick() to enter and exit the critical section
 * multiple times, avoiding long CPU blocking while maintaining armed time
 * event list consistency.
 */
