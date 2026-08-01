/*******************************************************************************
 * @brief Event Driven Framework (EDF) HSM implementation.
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
#include "edf_hsm.h"
#include "eaf.h"
#include "edf_config.h"
#include "edf_event.h"
#include "edf_core.h"
#include "ebf.h"
#include "emf.h"

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief Dispatches a reserved HSM event to the given state handler.
 *
 * @note The caller shall ensure that @p state is a valid state handler.
 *
 * @param state Pointer to the state handler function.
 * @param sig Reserved signal to dispatch. Must be one of the following:
 * @ref EDF_HSM_EMPTY_SIGNAL, @ref EDF_HSM_ENTRY_SIGNAL,
 * @ref EDF_HSM_EXIT_SIGNAL or @ref EDF_HSM_INIT_SIGNAL.
 */
#define DISPATCH_RESERVED_EVENT(state, sig)                 \
  /* State handlers contract is validated by assertions. */ \
  /* NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage) */  \
  ((*(state))(me, &reservedEvents[(sig)]))

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

/**
 * @brief Array of HSM internal reserved events.
 */
static const EDF_event_t reservedEvents[4] = {
  EDF_EVENT_IMMUTABLE_INIT(EDF_HSM_EMPTY_SIGNAL),
  EDF_EVENT_IMMUTABLE_INIT(EDF_HSM_ENTRY_SIGNAL),
  EDF_EVENT_IMMUTABLE_INIT(EDF_HSM_EXIT_SIGNAL),
  EDF_EVENT_IMMUTABLE_INIT(EDF_HSM_INIT_SIGNAL)};

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
 * @brief Computes the transition path between two states in the HSM.
 *
 * Determines which states must be exited and entered when transitioning
 * from @p source to @p target. Supports the following transition types:
 *
 * Case 1: Self-Transition (source == target)
 * Transition begins and ends in the same state.
 * The source state executes its exit action and then its entry action.
 *
 * Case 2: Ancestor → Descendant
 * Transition goes from an ancestor state to a descendant state.
 * Only entry actions are executed for each state on the path down to the
 * descendant.
 *
 * Case 3: Descendant → Ancestor
 * Transition ascends from a deep descendant to one of its ancestors.
 * Exit actions are executed upward until (but not including) the ancestor
 * target.
 *
 * Case 4: Between Branches (via Lowest Common Ancestor)
 * Transition between two states in different branches sharing a Lowest Common
 * Ancestor (LCA). Exit actions are executed up to (but not including) the LCA,
 * then entry actions down to the target.
 *
 * Key Definitions:
 * -Ancestor: A state that is higher in the hierarchy relative to another state
 *  (e.g., a parent, grandparent, etc.).
 * -Descendant: A state that is nested within another state (e.g., a child,
 *  grandchild, etc.).
 * -Sibling: States that share the same immediate parent in the hierarchy.
 * -Parent: The direct enclosing state of a child state.
 * -Child: A state that is directly nested within a parent.
 * -Self-transition: A transition where the source and target are the same
 *  state.
 * -Lowest Common Ancestor (LCA): The deepest (lowest) state in the hierarchy
 *  that is a common ancestor of both the source and the target states.
 * -Exit action: The behavior executed when leaving a state.
 * -Entry action: The behavior executed when entering a state.
 *
 * @param[in] me Pointer to the HSM instance.
 * @param[in] source Source state handler.
 * @param[in] target Target state handler.
 * @param[out] e_path Ordered list of states to enter, from @p target
 * upward in the hierarchy (excluding already active LCA). The array must be
 * at least EDF_HSM_MAX_DEPTH in size.
 * @param[out] e_idx Last valid index in @p e_path (−1 if no entry
 * actions needed).
 * @param[out] x_path Ordered list of states to exit, from @p source
 * upward in the hierarchy (excluding already active LCA). The array must be
 * at least EDF_HSM_MAX_DEPTH in size.
 * @param[out] x_idx Last valid index in @p x_path (−1 if no exit
 * actions needed).
 */
static void EDF_hsm_getTransitionPath(EDF_hsm_t* me,
                                      EDF_hsm_stateHandler_t source,
                                      EDF_hsm_stateHandler_t target,
                                      EDF_hsm_stateHandler_t* e_path,
                                      int_fast8_t* e_idx,
                                      EDF_hsm_stateHandler_t* x_path,
                                      int_fast8_t* x_idx);

/**
 * @brief Executes transition along the given entry and exit paths in the HSM.
 *
 * Performs the exit actions for all states in @p x_path, then the entry actions
 * for all states in @p e_path. Handles initial transitions recursively if
 * the target state defines them, updating the current active state accordingly.
 *
 * @param[in,out] me Pointer to the HSM instance.
 * @param[in] e_path Ordered list of states to enter, from target upward.
 * The array must be at least EDF_HSM_MAX_DEPTH in size.
 * @param[in] e_idx Last valid index in @p e_path (−1 if no entry actions).
 * @param[in] x_path Ordered list of states to exit, from source upward.
 * The array must be at least EDF_HSM_MAX_DEPTH in size.
 * @param[in] x_idx Last valid index in @p x_path (−1 if no exit actions).
 */
static void EDF_hsm_executeTransition(EDF_hsm_t* me,
                                      EDF_hsm_stateHandler_t* e_path,
                                      int_fast8_t e_idx,
                                      EDF_hsm_stateHandler_t* x_path,
                                      int_fast8_t x_idx);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static void EDF_hsm_getTransitionPath(EDF_hsm_t* me,
                                      EDF_hsm_stateHandler_t source,
                                      EDF_hsm_stateHandler_t target,
                                      EDF_hsm_stateHandler_t* e_path,
                                      int_fast8_t* e_idx,
                                      EDF_hsm_stateHandler_t* x_path,
                                      int_fast8_t* x_idx)
{
  EDF_hsm_stateReturn_t state_ret;
  bool t_found;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(source != NULL);
  EAF_ASSERT_IN_BLOCK(target != NULL);
  EAF_ASSERT_IN_BLOCK(e_path != NULL);
  EAF_ASSERT_IN_BLOCK(e_idx != NULL);
  EAF_ASSERT_IN_BLOCK(x_path != NULL);
  EAF_ASSERT_IN_BLOCK(x_idx != NULL);
  EAF_ASSERT_BLOCK_END();

  t_found = false;

  /**
   * Case 1: Check is self transition.
   */
  if (source == target)
  {
    *x_idx = 0;
    x_path[*x_idx] = target;
    *e_idx = 0;
    e_path[*e_idx] = target;
    t_found = true;
  }

  if (!t_found)
  {
    /**
     * Case 2: Check is ancestor to descendant transition and build the
     * entry path along the way.
     */
    *e_idx = 0;
    e_path[*e_idx] = target;

    // Get target superstate.
    state_ret = DISPATCH_RESERVED_EVENT(e_path[*e_idx], EDF_HSM_EMPTY_SIGNAL);

    // -1: exclude top state.
    while ((state_ret == RET_SUPER) && (*e_idx < (EDF_HSM_MAX_DEPTH - 1)))
    {
      (*e_idx)++;
      e_path[*e_idx] = me->temp_state;  // Store the entry path.

      // Transition source found?
      if (me->temp_state == source)
      {
        *x_idx = -1;  // Do not execute any exit action.
        (*e_idx)--;   // Do not enter the source.
        t_found = true;
        break;
      }

      // Get next ancestor.
      state_ret = DISPATCH_RESERVED_EVENT(me->temp_state, EDF_HSM_EMPTY_SIGNAL);
    }
  }

  if (!t_found)
  {
    /**
     * Case 3: Check is descendant to ancestor transition and build
     * the exit path along the way.
     */
    *x_idx = 0;
    x_path[*x_idx] = source;

    // Get source superstate.
    state_ret = DISPATCH_RESERVED_EVENT(x_path[*x_idx], EDF_HSM_EMPTY_SIGNAL);

    // -1: exclude top state.
    while ((state_ret == RET_SUPER) && (*x_idx < (EDF_HSM_MAX_DEPTH - 1)))
    {
      (*x_idx)++;

      // Transition source found?
      if (me->temp_state == target)
      {
        *e_idx = -1;  // Do not execute any entry action.
        (*x_idx)--;   // Do not exit the target.
        t_found = true;
        break;
      }

      x_path[*x_idx] = me->temp_state;  // Store the exit path.

      // Get next ancestor.
      state_ret = DISPATCH_RESERVED_EVENT(me->temp_state, EDF_HSM_EMPTY_SIGNAL);
    }
  }

  if (!t_found)
  {
    /**
     * Case 4: Check is lowest common ancestor transition and trim paths
     * accordingly.
     */
    for (int_fast8_t x_i = *x_idx; (x_i >= 0) && !t_found; x_i--)
    {
      for (int_fast8_t e_i = *e_idx; e_i >= 0; e_i--)
      {
        if (x_path[x_i] == e_path[e_i])
        {
          *x_idx = (int_fast8_t)(x_i - 1);  // Do not exit the LCA.
          *e_idx = (int_fast8_t)(e_i - 1);  // Do not enter the LCA.
          t_found = true;
          break;
        }
      }
    }
  }

  // Ensure valid HSM transition.
  EAF_ASSERT(t_found);
}

static void EDF_hsm_executeTransition(EDF_hsm_t* me,
                                      EDF_hsm_stateHandler_t* e_path,
                                      int_fast8_t e_idx,
                                      EDF_hsm_stateHandler_t* x_path,
                                      int_fast8_t x_idx)
{
  EDF_hsm_stateHandler_t aux_state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e_path != NULL);
  EAF_ASSERT_IN_BLOCK(e_idx < EDF_HSM_MAX_DEPTH);
  EAF_ASSERT_IN_BLOCK(x_path != NULL);
  EAF_ASSERT_IN_BLOCK(x_idx < EDF_HSM_MAX_DEPTH);
  EAF_ASSERT_BLOCK_END();

  // Execute exit actions.
  for (int_fast8_t i = 0; i <= x_idx; i++)
  {
    DISPATCH_RESERVED_EVENT(x_path[i], EDF_HSM_EXIT_SIGNAL);
  }

  // Execute entry actions.
  while (e_idx >= 0)
  {
    DISPATCH_RESERVED_EVENT(e_path[e_idx], EDF_HSM_ENTRY_SIGNAL);
    e_idx--;
  }

  // Set target state of the transition.
  aux_state = e_path[0];
  me->temp_state = aux_state;

  // Check if there are initial transitions in the target state to execute.
  while (DISPATCH_RESERVED_EVENT(aux_state, EDF_HSM_INIT_SIGNAL) ==
         RET_TRANSITION)
  {
    e_idx = 0;
    e_path[0] = me->temp_state;  // Update with the new target state.

    // Get superstate of the new target state.
    (void)DISPATCH_RESERVED_EVENT(me->temp_state, EDF_HSM_EMPTY_SIGNAL);

    // Build the entry path of the initial transition.
    while ((me->temp_state != aux_state) && (e_idx < (EDF_HSM_MAX_DEPTH - 1)))
    {
      e_idx++;
      e_path[e_idx] = me->temp_state;

      // Get next ancestor.
      (void)DISPATCH_RESERVED_EVENT(me->temp_state, EDF_HSM_EMPTY_SIGNAL);
    }

    // Execute entry actions.
    while (e_idx >= 0)
    {
      DISPATCH_RESERVED_EVENT(e_path[e_idx], EDF_HSM_ENTRY_SIGNAL);
      e_idx--;
    }

    // Target state becomes the new source.
    aux_state = e_path[0];
    me->temp_state = aux_state;
  }

  me->current_state = e_path[0];  // Set the current active state.
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void EDF_hsm_init(EDF_hsm_t* me, EDF_hsm_stateHandler_t initial)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(initial != NULL);
  EAF_ASSERT_BLOCK_END();

  me->current_state = (EDF_hsm_stateHandler_t)(&EDF_hsm_top);
  me->temp_state = initial;
}

void EDF_hsm_start(EDF_hsm_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state_ret;
  EDF_hsm_stateHandler_t e_path[EDF_HSM_MAX_DEPTH];
  int_fast8_t e_idx;
  EDF_hsm_stateHandler_t x_path[1];  // Dummy for EDF_hsm_executeTransition.
  int_fast8_t x_idx;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(me->temp_state != NULL);
  EAF_ASSERT_IN_BLOCK(me->current_state ==
                      ((EDF_hsm_stateHandler_t)(&EDF_hsm_top)));
  EAF_ASSERT_BLOCK_END();

  // Execute the top-most initial transition.
  // State handlers contract is validated by assertions.
  // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
  state_ret = me->temp_state(me, e);

  // The top-most initial transition shall be taken.
  EAF_ASSERT(state_ret == RET_TRANSITION);

  // Drill up to EDF_hsm_top to build the entry path of the initial transition.
  e_path[0] = me->temp_state;  // Set initial transition target state.

  // Get parent of the initial transition target state.
  (void)DISPATCH_RESERVED_EVENT(me->temp_state, EDF_HSM_EMPTY_SIGNAL);

  e_idx = 1;  // Initialize entry path index; index 0 is already set.
  while ((me->temp_state != ((EDF_hsm_stateHandler_t)(&EDF_hsm_top))) &&
         (e_idx < EDF_HSM_MAX_DEPTH))
  {
    // Store next ancestor state in the entry path.
    e_path[e_idx] = me->temp_state;

    // Get next ancestor until the EDF_hsm_top state is reached.
    (void)DISPATCH_RESERVED_EVENT(me->temp_state, EDF_HSM_EMPTY_SIGNAL);

    e_idx++;
  }

  // Entry to initial transition target state.
  e_idx--;     // -1: exclude top state.
  x_idx = -1;  // Do not execute any exit action.
  EDF_hsm_executeTransition(me, e_path, e_idx, x_path, x_idx);
}

void EDF_hsm_dispatch(EDF_hsm_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state_ret;
  EDF_hsm_stateHandler_t aux_state;
  EDF_hsm_stateHandler_t t_source;
  EDF_hsm_stateHandler_t t_target;
  EDF_hsm_stateHandler_t e_path[EDF_HSM_MAX_DEPTH];
  int_fast8_t e_idx;
  EDF_hsm_stateHandler_t x_path[EDF_HSM_MAX_DEPTH];
  int_fast8_t x_idx;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(me->current_state != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  state_ret = RET_SUPER;

  // Process the event hierarchically.
  me->temp_state = me->current_state;  // Set current state.
  for (int_fast8_t i = EDF_HSM_MAX_DEPTH; i > 0; i--)
  {
    aux_state = me->temp_state;

    /**
     * Invoke state handler (updates me->temp_state on RET_SUPER or
     * RET_TRANSITION).
     */
    // State handlers contract is validated by assertions.
    // NOLINTNEXTLINE(clang-analyzer-core.CallAndMessage)
    state_ret = aux_state(me, e);

    // Unhandled due to a guard?
    if (state_ret == RET_UNHANDLED)
    {
      // Get superstate, if any.
      state_ret = DISPATCH_RESERVED_EVENT(aux_state, EDF_HSM_EMPTY_SIGNAL);
    }
    if (state_ret != RET_SUPER)
    {
      break;  // Stop hierarchical propagation of the event.
    }
  }

  // Transition taken?
  if (state_ret == RET_TRANSITION)
  {
    t_source = aux_state;       // Transition source.
    t_target = me->temp_state;  // Transition target.

    // Exit states from current state up to the transition source.
    aux_state = me->current_state;  // Set to current state.
    while (aux_state != t_source)
    {
      // Exit state.
      state_ret = DISPATCH_RESERVED_EVENT(aux_state, EDF_HSM_EXIT_SIGNAL);

      if (state_ret == RET_HANDLED)
      {
        // Get next superstate.
        (void)DISPATCH_RESERVED_EVENT(aux_state, EDF_HSM_EMPTY_SIGNAL);
      }
      /**
       * else: Exit event not defined in the invoked state handler.
       * Default case executed, so me->temp_state already points to the
       * next superstate.
       */

      aux_state = me->temp_state;
    }

    // Take the transition.
    EDF_hsm_getTransitionPath(
      me, t_source, t_target, e_path, &e_idx, x_path, &x_idx);
    EDF_hsm_executeTransition(me, e_path, e_idx, x_path, x_idx);
  }
}

EDF_hsm_stateHandler_t EDF_hsm_getCurrentState(const EDF_hsm_t* me)
{
  EAF_ASSERT(me != NULL);

  return me->current_state;
}

EDF_hsm_stateReturn_t EDF_hsm_top(const EDF_hsm_t* me, const EDF_event_t* e)
{
  EMF_UTILS_UNUSED_PARAM(me);
  EMF_UTILS_UNUSED_PARAM(e);

  return RET_IGNORED;  // The top state ignores all events.
}
