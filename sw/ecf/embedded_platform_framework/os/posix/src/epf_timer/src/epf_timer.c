/*******************************************************************************
 * @brief Platform-specific timer utilities implementation.
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
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "epf_timer.h"
#include "ebf.h"
#include "eaf.h"
#include "emf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief Number of ticks per second.
 *
 * @todo Make this value configurable by the library user via CMake.
 */
#define TICKS_PER_SEC 1000L

/**
 * @brief Number of nanoseconds per second.
 */
#define NSEC_PER_SEC 1000000000L

/**
 * @brief Duration of one tick in nanoseconds (ns/tick).
 */
#define TICK_NS (NSEC_PER_SEC / TICKS_PER_SEC)

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

/**
 * @brief Converts nanoseconds to ticks.
 *
 * @note Implemented as integer ceil division to round up partial ticks instead
 * of truncating.
 */
static uint64_t ns_to_ticks(uint64_t ns);

/**
 * @brief Converts ticks to nanoseconds.
 */
static uint64_t ticks_to_ns(uint64_t ticks);

/**
 * @brief Ticker thread managing all armed timers.
 */
static void* ticker_thread(void* arg);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static uint64_t ns_to_ticks(uint64_t ns)
{
  return (ns + TICK_NS - 1) / TICK_NS;
}

static uint64_t ticks_to_ns(uint64_t ticks)
{
  return ticks * TICK_NS;
}

static void* ticker_thread(void* arg)
{
  EPF_timer_entry_t** t_link;
  EPF_timer_entry_t* t;
  struct timespec next_tick;

  EPF_timer_t* me = (EPF_timer_t*)arg;

  /**
   * @note The implementation uses a pointer-to-pointer
   * (@c EPF_timer_t **t_link)
   * to avoid special-casing the first element of the timers list:
   * - At the start, @c t_link = &me->armed_head.
   * - As we traverse the armed timers list, @c t_link is updated to point to
   *   the @c next field of the current node: @c t_link = &t->next, where
   *   @c t = *t_link.
   * - Removing a node is always @c *t_link = t->next;
   * (works for both the first node and all subsequent nodes).
   *
   * Attaching freshly armed timers (@c me->new_head) is also unified:
   * - When @c *t_link == NULL and @c me->new_head != NULL, simply do
   *   @c *t_link = me->new_head; (this attaches the new timer list either at
   *   the beginning of me->armed_head or after its last node).
   *
   * This way the same code handles head removal, internal removals and
   * attaching new timers without branching on special cases.
   */

  // Align next_tick to the nearest lower tick boundary before starting the loop
  clock_gettime(CLOCK_MONOTONIC, &next_tick);
  next_tick.tv_nsec = (next_tick.tv_nsec / TICK_NS) * TICK_NS;

  while (!me->stop_ticker)
  {
    /* Sleep until the absolute time `next_tick` (prevents drift). */
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_tick, NULL);

    (void)pthread_mutex_lock(&me->mutex);

    // Initialize to head of armed timers list.
    t_link = &me->armed_head;

    // Set the current timer to the first node of armed list.
    t = *t_link;

    while (true)
    {
      if (t == NULL)
      {
        // No more timers in the armed list.
        if (me->new_head == NULL)
        {
          break;  // No more timers in the new list.
        }

        /**
         * Attach new timers to armed list;
         * Updates me->armed_head or me->armed_head[last].next
         * depending on the current t_link.
         */
        *t_link = me->new_head;
        me->new_head = NULL;
        t = *t_link;  // Continue from the newly attached list.
      }

      if (t->cnt == 0U)
      {
        // "t" scheduled for removal.
        *t_link = t->next;  // Unlink.
        t->is_linked = false;
        // Do not advance t_link, because we just unlinked "t".

        (void)pthread_mutex_unlock(&me->mutex);
      }
      else if (t->cnt == 1U)
      {
        if (t->period != 0U)
        {
          t->cnt = t->period;  // Periodic --> reload counter.
          t_link = &t->next;   // Advance t_link to the next node.
        }
        else
        {
          t->cnt = 0U;        // One-shot --> disarm.
          *t_link = t->next;  // Unlink.
          t->is_linked = false;
          // Do not advance t_link, because we just unlinked "t".
        }

        (void)pthread_mutex_unlock(&me->mutex);

        // Execute timer callback
        t->cb();
      }
      else
      {
        t->cnt--;           // Normal countdown.
        t_link = &t->next;  // Advance t_link to the next node.

        (void)pthread_mutex_unlock(&me->mutex);
      }

      // Re-enter critical section to continue loop.
      (void)pthread_mutex_lock(&me->mutex);

      // Update "t" to the current node after any modifications.
      t = *t_link;
    }

    (void)pthread_mutex_unlock(&me->mutex);
  }

  return NULL;
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void EPF_timer_init(EPF_timer_t* me, uint8_t prio)
{
  pthread_attr_t attr;
  int err;
  struct sched_param sched_param;

  // Ensure priority is within the valid SCHED_FIFO range.
  EAF_ASSERT((prio >= sched_get_priority_min(SCHED_FIFO)) &&
             (prio <= sched_get_priority_max(SCHED_FIFO)));

  // Clear timer instance.
  EMF_utils_clear(me, sizeof(EPF_timer_t));

  // Initialize mutex.
  (void)pthread_mutex_init(&me->mutex, NULL);

  // Configure ticker thread scheduling policy and attributes.
  (void)pthread_attr_init(&attr);
  (void)pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  (void)pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  (void)pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

  // Configure ticker thread priority.
  sched_param.sched_priority = prio;
  (void)pthread_attr_setschedparam(&attr, &sched_param);

  // Create ticker thread.
  err = pthread_create(&me->ticker_tid, &attr, ticker_thread, me);
  if (err != 0)
  {
    /**
     * Creating p-thread with the SCHED_FIFO policy failed. Most likely
     * this application has no superuser privileges, so we just fall
     * back to the default SCHED_OTHER policy, which only supports
     * priority 0.
     */
    (void)pthread_attr_setschedpolicy(&attr, SCHED_OTHER);
    sched_param.sched_priority = 0;
    (void)pthread_attr_setschedparam(&attr, &sched_param);
    err = pthread_create(&me->ticker_tid, &attr, ticker_thread, me);
  }

  // Ticker thread shall be created without errors.
  EAF_ASSERT(err == 0);

  // Release thread attribute object resources.
  (void)pthread_attr_destroy(&attr);
}

void EPF_timer_deinit(EPF_timer_t* me)
{
  // Skip only if the ticker thread has not been created.
  if (me->ticker_tid != 0)
  {
    // Stop and wait ticker thread.
    me->stop_ticker = true;
    (void)pthread_join(me->ticker_tid, NULL);
    me->ticker_tid = 0;

    (void)pthread_mutex_lock(&me->mutex);

    // Set timer lists as empty
    me->armed_head = NULL;
    me->new_head = NULL;

    // Unlock and destroy mutex
    (void)pthread_mutex_unlock(&me->mutex);
    (void)pthread_mutex_destroy(&me->mutex);
  }
}

void EPF_timeEvent_new(EPF_timer_entry_t* timer_entry, EPF_timer_callback_t cb)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(timer_entry != NULL);
  EAF_ASSERT_IN_BLOCK(cb != NULL);
  EAF_ASSERT_BLOCK_END();

  // Initialize timer attributes.
  timer_entry->next = NULL;
  timer_entry->cb = cb;
  timer_entry->cnt = 0U;
  timer_entry->period = 0U;
  timer_entry->is_linked = false;
}

void EPF_timer_arm(EPF_timer_t* me,
                   EPF_timer_entry_t* timer_entry,
                   uint64_t start,
                   uint64_t period)
{
  (void)pthread_mutex_lock(&me->mutex);

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me->ticker_tid != 0);
  EAF_ASSERT_IN_BLOCK(timer_entry != NULL);
  EAF_ASSERT_IN_BLOCK(timer_entry->cnt == 0U);
  EAF_ASSERT_IN_BLOCK(timer_entry->cb != NULL);
  EAF_ASSERT_IN_BLOCK(start != 0U);
  EAF_ASSERT_BLOCK_END();

  timer_entry->cnt = ns_to_ticks(start);
  timer_entry->period = ns_to_ticks(period);

  /**
   * Is the timer unlinked? See @ref timer_disarmed_but_linked.
   * This check allows disarming and re-arming within the same tick
   * without duplicating links.
   */
  if (!timer_entry->is_linked)
  {
    timer_entry->is_linked = true;  // Mark as linked.

    // Link into the temporary new list. See @ref timer_new_list.
    timer_entry->next = me->new_head;
    me->new_head = timer_entry;
  }

  (void)pthread_mutex_unlock(&me->mutex);
}

bool EPF_timer_disarm(EPF_timer_t* me, EPF_timer_entry_t* timer_entry)
{
  bool was_armed;

  (void)pthread_mutex_lock(&me->mutex);

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me->ticker_tid != 0);
  EAF_ASSERT_IN_BLOCK(timer_entry != NULL);
  EAF_ASSERT_BLOCK_END();

  // Was the timer actually armed?
  if (timer_entry->cnt != 0U)
  {
    was_armed = true;
    timer_entry->cnt = 0U;  // Schedule removal from the list.
  }
  else
  {
    // The timer was already disarmed.
    was_armed = false;
  }

  (void)pthread_mutex_unlock(&me->mutex);

  return was_armed;
}

bool EPF_timer_rearm(EPF_timer_t* me,
                     EPF_timer_entry_t* timer_entry,
                     uint64_t time)
{
  bool was_armed;

  (void)pthread_mutex_lock(&me->mutex);

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me->ticker_tid != 0);
  EAF_ASSERT_IN_BLOCK(timer_entry != NULL);
  EAF_ASSERT_IN_BLOCK(timer_entry->cb != NULL);
  EAF_ASSERT_IN_BLOCK(time != 0U);
  EAF_ASSERT_BLOCK_END();

  // Was the timer not running?
  if (timer_entry->cnt == 0U)
  {
    was_armed = false;

    /**
     * Is the timer unlinked? See @ref timer_disarmed_but_linked.
     * This check allows disarming and re-arming within the same tick
     * without duplicating links.
     */
    if (!timer_entry->is_linked)
    {
      timer_entry->is_linked = true;  // Mark as linked.

      // Link into the temporary new list. See @ref timer_new_list.
      timer_entry->next = me->new_head;
      me->new_head = timer_entry;
    }
  }
  else
  {
    // The timer was armed.
    was_armed = true;
  }

  // Set new counter value.
  timer_entry->cnt = ns_to_ticks(time);

  (void)pthread_mutex_unlock(&me->mutex);

  return was_armed;
}

uint64_t EPF_timer_currentCounter(EPF_timer_t* me,
                                  EPF_timer_entry_t* timer_entry)
{
  uint64_t cnt;

  (void)pthread_mutex_lock(&me->mutex);

  EAF_ASSERT(timer_entry != NULL);

  cnt = ticks_to_ns(timer_entry->cnt);

  (void)pthread_mutex_unlock(&me->mutex);

  return cnt;
}

/**
 * @anchor timer_disarmed_but_linked
 * @par Timer disarmed but linked
 *
 * A timer can be "disarmed but still linked" for the duration of one
 * tick interval:
 * - @ref EPF_timer_disarm() sets @c cnt = 0, but does not unlink the timer.
 * - Unlinking is performed exclusively in @c ticker_thread().
 */

/**
 * @anchor timer_new_list
 * @par Timer new list
 *
 * The list of armed timers (@c armed_head) is modified
 * exclusively inside @c ticker_thread().
 * Newly armed timers are first linked into a temporary list
 * (`new_head`) and only merged into the main list during
 * the tick processing.
 * This design allows ticker_thread() to enter and exit the critical
 * section multiple times, avoiding long CPU blocking while maintaining armed
 * timer list consistency.
 */
