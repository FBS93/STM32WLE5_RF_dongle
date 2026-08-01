/*******************************************************************************
 * @brief EDF fuzz example module.
 *
 * Implements a tiny dummy state machine whose only purpose is to demonstrate
 * fuzz-event injection, stateful discovery and reserved crash/hang triggers.
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
#include <stdlib.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "fuzz_example.h"
#include "eaf.h"
#include "edf_fuzz.h"
#include "emf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief Priority assigned to the fuzz example active object.
 */
#define FUZZ_EXAMPLE_AO_PRIO 2U

/**
 * @brief Length of the active object event queue.
 */
#define FUZZ_EXAMPLE_QUEUE_LEN 4U

/**
 * @brief Number of mutable event blocks in the pool.
 */
#define FUZZ_EXAMPLE_POOL_BLOCKS 4U

/**
 * @brief Default accepted slot after reset.
 */
#define FUZZ_EXAMPLE_DEFAULT_SLOT 0x10U

/**
 * @brief Default gate value after reset.
 */
#define FUZZ_EXAMPLE_DEFAULT_GATE 0x20U

/**
 * @brief Default finish target count after reset.
 */
#define FUZZ_EXAMPLE_DEFAULT_TARGET_COUNT 2U

/**
 * @brief Reserved slot value that triggers the direct crash path.
 */
#define FUZZ_EXAMPLE_CRASH_SET_SLOT 0xC7U

/**
 * @brief Reserved gate value that triggers the direct crash path.
 */
#define FUZZ_EXAMPLE_CRASH_SET_GATE 0x00U

/**
 * @brief Reserved target count that triggers the direct crash path.
 */
#define FUZZ_EXAMPLE_CRASH_SET_TARGET 1U

/**
 * @brief Reserved slot value that triggers the direct hang path.
 */
#define FUZZ_EXAMPLE_HANG_SET_SLOT 0xE1U

/**
 * @brief Reserved gate value that triggers the direct hang path.
 */
#define FUZZ_EXAMPLE_HANG_SET_GATE 0x00U

/**
 * @brief Reserved target count that triggers the direct hang path.
 */
#define FUZZ_EXAMPLE_HANG_SET_TARGET 1U

/**
 * @brief Reserved slot used by the finish crash pattern.
 */
#define FUZZ_EXAMPLE_FINISH_CRASH_SLOT 0x11U

/**
 * @brief Reserved gate used by the finish crash pattern.
 */
#define FUZZ_EXAMPLE_FINISH_CRASH_GATE 0x11U

/**
 * @brief Reserved target count used by the finish crash pattern.
 */
#define FUZZ_EXAMPLE_FINISH_CRASH_TARGET 2U

/**
 * @brief Reserved running sum used by the finish crash pattern.
 */
#define FUZZ_EXAMPLE_FINISH_CRASH_SUM 0x33U

/**
 * @brief Adjusted value that triggers the deep collecting hang path.
 */
#define FUZZ_EXAMPLE_STEP_HANG_VALUE 0xFEU

/**
 * @brief Value boost applied when the boost flag is set.
 */
#define FUZZ_EXAMPLE_STEP_BOOST_DELTA 0x20U

/**
 * @brief Maximum size among mutable fuzz event payloads.
 */
#define FUZZ_EXAMPLE_MAX_MUTABLE_EVENT_SIZE                             \
  ((sizeof(fuzzExample_setEvent_t) < sizeof(fuzzExample_stepEvent_t)) ? \
     sizeof(fuzzExample_stepEvent_t) :                                  \
     sizeof(fuzzExample_setEvent_t))

/**
 * @brief Event-pool block size aligned to EDF pool requirements.
 */
#define FUZZ_EXAMPLE_POOL_BLOCK_SIZE                             \
  ((EDF_pool_blockSize_t)((FUZZ_EXAMPLE_MAX_MUTABLE_EVENT_SIZE < \
                           sizeof(void*)) ?                      \
                            sizeof(void*) :                      \
                            FUZZ_EXAMPLE_MAX_MUTABLE_EVENT_SIZE))

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/**
 * @brief Active object backing the fuzz example application.
 */
typedef struct
{
  EDF_activeObject_t super;   //!< EDF active object base type.
  uint32_t ping_count;        //!< Number of ping events processed.
  uint32_t set_count;         //!< Number of setup events processed.
  uint32_t step_count_total;  //!< Number of step events processed.
  uint32_t ignored_count;     //!< Number of ignored events.
  uint32_t rejected_count;    //!< Number of rejected setup events.
  uint32_t transition_count;  //!< Number of state transitions executed.
  uint16_t running_sum;       //!< Running sum of collected step values.
  uint8_t match_slot;         //!< Slot accepted by the dummy app.
  uint8_t gate;               //!< Minimum value required to start collecting.
  uint8_t target_count;       //!< Small counter target used by finish logic.
  uint8_t collected_count;    //!< Number of collected steps.
  uint8_t last_slot;          //!< Last slot processed.
  uint8_t last_value;         //!< Last value processed.
} fuzzExample_t;

/**
 * @brief Bit flags carried by fuzzed step events.
 */
typedef enum
{
  FUZZ_EXAMPLE_STEP_FLAG_BOOST = (1U << 0),  //!< Add a fixed boost to value.
  FUZZ_EXAMPLE_STEP_FLAG_STOP = (1U << 1),   //!< Return to standby after step.
  FUZZ_EXAMPLE_STEP_FLAG_CLEAR = (1U << 2)   //!< Clear the running sum first.
} fuzzExample_stepFlag_t;

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/**
 * @brief Define static file name string for asserts.
 */
EAF_DEFINE_THIS_FILE(__FILE__);

/**
 * @brief Fuzz example active object instance.
 */
static fuzzExample_t fuzzExample;

/**
 * @brief Publish-subscribe subscriber list for all example signals.
 */
static EDF_activeObject_bitmask_t subscriberList[FUZZ_EXAMPLE_LAST_SIG];

/**
 * @brief Storage backing the fuzz example active object queue.
 */
static EDF_event_ptr eventQueueStorage[FUZZ_EXAMPLE_QUEUE_LEN];

/**
 * @brief Storage backing the mutable event memory pool.
 */
static EMF_UTILS_MEM_ALIGNED_SLOT(FUZZ_EXAMPLE_MAX_MUTABLE_EVENT_SIZE)
  eventPoolStorage[FUZZ_EXAMPLE_POOL_BLOCKS];

/**
 * @brief Immutable ping event descriptor payload.
 */
static const EDF_event_t pingEvt =
  EDF_EVENT_IMMUTABLE_INIT(FUZZ_EXAMPLE_PING_SIG);

/**
 * @brief Immutable start event descriptor payload.
 */
static const EDF_event_t startEvt =
  EDF_EVENT_IMMUTABLE_INIT(FUZZ_EXAMPLE_START_SIG);

/**
 * @brief Immutable finish event descriptor payload.
 */
static const EDF_event_t finishEvt =
  EDF_EVENT_IMMUTABLE_INIT(FUZZ_EXAMPLE_FINISH_SIG);

/**
 * @brief Immutable reset event descriptor payload.
 */
static const EDF_event_t resetEvt =
  EDF_EVENT_IMMUTABLE_INIT(FUZZ_EXAMPLE_RESET_SIG);

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/**
 * @brief Fuzzable-event table consumed by the EDF fuzz port.
 */
EDF_FUZZ_EVENT_TABLE(
  EDF_FUZZ_IMMUTABLE_EVENT(FUZZ_EXAMPLE_PING_SIG, &pingEvt),
  EDF_FUZZ_IMMUTABLE_EVENT(FUZZ_EXAMPLE_START_SIG, &startEvt),
  EDF_FUZZ_IMMUTABLE_EVENT(FUZZ_EXAMPLE_FINISH_SIG, &finishEvt),
  EDF_FUZZ_IMMUTABLE_EVENT(FUZZ_EXAMPLE_RESET_SIG, &resetEvt),
  EDF_FUZZ_MUTABLE_EVENT(FUZZ_EXAMPLE_SET_SIG, sizeof(fuzzExample_setEvent_t)),
  EDF_FUZZ_MUTABLE_EVENT(FUZZ_EXAMPLE_STEP_SIG,
                         sizeof(fuzzExample_stepEvent_t)));

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/**
 * @brief Perform the initial transition of the fuzz example HSM.
 *
 * @param[in] me Pointer to the fuzz example active object instance.
 * @param[in] e Pointer to the triggering event.
 * @return Next state-handler return code.
 */
static EDF_hsm_stateReturn_t fuzzExample_initialTransition(
  fuzzExample_t* me,
  const EDF_event_t* e);

/**
 * @brief Handle the operational superstate of the fuzz example HSM.
 *
 * @param[in] me Pointer to the fuzz example active object instance.
 * @param[in] e Pointer to the triggering event.
 * @return Next state-handler return code.
 */
static EDF_hsm_stateReturn_t fuzzExample_operationalState(fuzzExample_t* me,
                                                          const EDF_event_t* e);

/**
 * @brief Handle the standby state of the fuzz example HSM.
 *
 * @param[in] me Pointer to the fuzz example active object instance.
 * @param[in] e Pointer to the triggering event.
 * @return Next state-handler return code.
 */
static EDF_hsm_stateReturn_t fuzzExample_standbyState(fuzzExample_t* me,
                                                      const EDF_event_t* e);

/**
 * @brief Handle the armed state of the fuzz example HSM.
 *
 * @param[in] me Pointer to the fuzz example active object instance.
 * @param[in] e Pointer to the triggering event.
 * @return Next state-handler return code.
 */
static EDF_hsm_stateReturn_t fuzzExample_armedState(fuzzExample_t* me,
                                                    const EDF_event_t* e);

/**
 * @brief Handle the collecting state of the fuzz example HSM.
 *
 * @param[in] me Pointer to the fuzz example active object instance.
 * @param[in] e Pointer to the triggering event.
 * @return Next state-handler return code.
 */
static EDF_hsm_stateReturn_t fuzzExample_collectingState(fuzzExample_t* me,
                                                         const EDF_event_t* e);

/**
 * @brief Restore the runtime state to its default configuration.
 *
 * @param[in] me Pointer to the fuzz example active object instance.
 */
static void fuzzExample_resetRuntime(fuzzExample_t* me);

/**
 * @brief Apply a mutable setup event to the runtime configuration.
 *
 * @param[in] me Pointer to the fuzz example active object instance.
 * @param[in] set_evt Pointer to the setup event payload.
 * @return true if the setup event is accepted, otherwise false.
 */
static bool fuzzExample_applySet(fuzzExample_t* me,
                                 const fuzzExample_setEvent_t* set_evt);

/**
 * @brief Check whether a step slot matches the configured slot.
 *
 * @param[in] me Pointer to the fuzz example active object instance.
 * @param[in] slot Slot carried by the step event.
 * @return true if the slot is accepted, otherwise false.
 */
static bool fuzzExample_acceptsSlot(const fuzzExample_t* me, uint8_t slot);

/**
 * @brief Adjust a step value according to the supplied flags.
 *
 * @param[in] value Raw step value.
 * @param[in] flags Step-processing flags.
 * @return Adjusted step value.
 */
static uint8_t fuzzExample_adjustValue(uint8_t value, uint8_t flags);

/**
 * @brief Initialize runtime data when entering the collecting state.
 *
 * @param[in] me Pointer to the fuzz example active object instance.
 * @param[in] step_evt Pointer to the accepted step event payload.
 * @param[in] adjusted_value Adjusted step value used for collection.
 */
static void fuzzExample_beginCollecting(fuzzExample_t* me,
                                        const fuzzExample_stepEvent_t* step_evt,
                                        uint8_t adjusted_value);

/**
 * @brief Check whether a setup event matches the direct crash trigger.
 *
 * @param[in] set_evt Pointer to the setup event payload.
 * @return true if the direct crash trigger matches, otherwise false.
 */
static bool fuzzExample_shouldCrashOnSet(const fuzzExample_setEvent_t* set_evt);

/**
 * @brief Check whether a setup event matches the direct hang trigger.
 *
 * @param[in] set_evt Pointer to the setup event payload.
 * @return true if the direct hang trigger matches, otherwise false.
 */
static bool fuzzExample_shouldHangOnSet(const fuzzExample_setEvent_t* set_evt);

/**
 * @brief Check whether the runtime matches the finish crash trigger.
 *
 * @param[in] me Pointer to the fuzz example active object instance.
 * @return true if the finish crash trigger matches, otherwise false.
 */
static bool fuzzExample_shouldCrashOnFinish(const fuzzExample_t* me);

/**
 * @brief Check whether a step should trigger the deep collecting hang path.
 *
 * @param[in] me Pointer to the fuzz example active object instance.
 * @param[in] step_evt Pointer to the accepted step event payload.
 * @param[in] adjusted_value Adjusted step value under evaluation.
 * @return true if the deep collecting hang trigger matches, otherwise false.
 */
static bool fuzzExample_shouldHangOnStep(
  const fuzzExample_t* me,
  const fuzzExample_stepEvent_t* step_evt,
  uint8_t adjusted_value);

/**
 * @brief Trigger the intentional crash path used by the fuzz example.
 */
static void fuzzExample_triggerCrash(void);

/**
 * @brief Trigger the intentional hang path used by the fuzz example.
 */
static void fuzzExample_triggerHang(void);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static EDF_hsm_stateReturn_t fuzzExample_initialTransition(fuzzExample_t* me,
                                                           const EDF_event_t* e)
{
  fuzzExample_resetRuntime(me);
  EMF_UTILS_UNUSED_PARAM(e);

  return EDF_HSM_RET_TRAN(fuzzExample_operationalState);
}

static EDF_hsm_stateReturn_t fuzzExample_operationalState(fuzzExample_t* me,
                                                          const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case EDF_HSM_INIT_SIGNAL:
    {
      ++me->transition_count;
      state = EDF_HSM_RET_TRAN(fuzzExample_standbyState);
      break;
    }

    case FUZZ_EXAMPLE_PING_SIG:
    {
      ++me->ping_count;
      state = EDF_HSM_RET_HANDLED();
      break;
    }

    case FUZZ_EXAMPLE_RESET_SIG:
    {
      fuzzExample_resetRuntime(me);
      ++me->transition_count;
      state = EDF_HSM_RET_TRAN(fuzzExample_standbyState);
      break;
    }

    default:
    {
      state = EDF_HSM_RET_SUPER(EDF_hsm_top);
      break;
    }
  }

  return state;
}

static EDF_hsm_stateReturn_t fuzzExample_standbyState(fuzzExample_t* me,
                                                      const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case FUZZ_EXAMPLE_START_SIG:
    {
      me->running_sum = 0U;
      me->collected_count = 0U;
      ++me->transition_count;
      state = EDF_HSM_RET_TRAN(fuzzExample_armedState);
      break;
    }

    case FUZZ_EXAMPLE_FINISH_SIG:
    case FUZZ_EXAMPLE_STEP_SIG:
    {
      ++me->ignored_count;
      state = EDF_HSM_RET_HANDLED();
      break;
    }

    case FUZZ_EXAMPLE_SET_SIG:
    {
      const fuzzExample_setEvent_t* set_evt;

      set_evt = (const fuzzExample_setEvent_t*)e;
      (void)fuzzExample_applySet(me, set_evt);
      state = EDF_HSM_RET_HANDLED();
      break;
    }

    default:
    {
      state = EDF_HSM_RET_SUPER(fuzzExample_operationalState);
      break;
    }
  }

  return state;
}

static EDF_hsm_stateReturn_t fuzzExample_armedState(fuzzExample_t* me,
                                                    const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case FUZZ_EXAMPLE_START_SIG:
    case FUZZ_EXAMPLE_FINISH_SIG:
    {
      ++me->ignored_count;
      state = EDF_HSM_RET_HANDLED();
      break;
    }

    case FUZZ_EXAMPLE_SET_SIG:
    {
      const fuzzExample_setEvent_t* set_evt;

      set_evt = (const fuzzExample_setEvent_t*)e;
      (void)fuzzExample_applySet(me, set_evt);
      state = EDF_HSM_RET_HANDLED();
      break;
    }

    case FUZZ_EXAMPLE_STEP_SIG:
    {
      const fuzzExample_stepEvent_t* step_evt;
      uint8_t adjusted_value;

      step_evt = (const fuzzExample_stepEvent_t*)e;
      ++me->step_count_total;
      me->last_slot = step_evt->slot;
      me->last_value = step_evt->value;

      if (fuzzExample_acceptsSlot(me, step_evt->slot) == false)
      {
        ++me->ignored_count;
        state = EDF_HSM_RET_HANDLED();
      }
      else
      {
        adjusted_value =
          fuzzExample_adjustValue(step_evt->value, step_evt->flags);

        if (adjusted_value < me->gate)
        {
          ++me->ignored_count;
          state = EDF_HSM_RET_HANDLED();
        }
        else
        {
          fuzzExample_beginCollecting(me, step_evt, adjusted_value);
          ++me->transition_count;
          state = EDF_HSM_RET_TRAN(fuzzExample_collectingState);
        }
      }
      break;
    }

    default:
    {
      state = EDF_HSM_RET_SUPER(fuzzExample_operationalState);
      break;
    }
  }

  return state;
}

static EDF_hsm_stateReturn_t fuzzExample_collectingState(fuzzExample_t* me,
                                                         const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case FUZZ_EXAMPLE_FINISH_SIG:
    {
      if (fuzzExample_shouldCrashOnFinish(me) == true)
      {
        // Reserved stateful crash path intentionally left unseeded.
        fuzzExample_triggerCrash();
      }

      ++me->transition_count;
      state = EDF_HSM_RET_TRAN(fuzzExample_standbyState);
      break;
    }

    case FUZZ_EXAMPLE_SET_SIG:
    {
      const fuzzExample_setEvent_t* set_evt;

      set_evt = (const fuzzExample_setEvent_t*)e;
      (void)fuzzExample_applySet(me, set_evt);
      state = EDF_HSM_RET_HANDLED();
      break;
    }

    case FUZZ_EXAMPLE_STEP_SIG:
    {
      const fuzzExample_stepEvent_t* step_evt;
      uint8_t adjusted_value;

      step_evt = (const fuzzExample_stepEvent_t*)e;
      ++me->step_count_total;
      me->last_slot = step_evt->slot;
      me->last_value = step_evt->value;

      if (fuzzExample_acceptsSlot(me, step_evt->slot) == false)
      {
        ++me->ignored_count;
        state = EDF_HSM_RET_HANDLED();
      }
      else
      {
        adjusted_value =
          fuzzExample_adjustValue(step_evt->value, step_evt->flags);

        if (fuzzExample_shouldHangOnStep(me, step_evt, adjusted_value) == true)
        {
          // Reserved stateful hang path intentionally left unseeded.
          fuzzExample_triggerHang();
        }

        if ((step_evt->flags & FUZZ_EXAMPLE_STEP_FLAG_CLEAR) != 0U)
        {
          me->running_sum = adjusted_value;
        }
        else
        {
          me->running_sum = (uint16_t)(me->running_sum + adjusted_value);
        }

        me->last_value = adjusted_value;
        ++me->collected_count;

        if ((step_evt->flags & FUZZ_EXAMPLE_STEP_FLAG_STOP) != 0U)
        {
          ++me->transition_count;
          state = EDF_HSM_RET_TRAN(fuzzExample_standbyState);
        }
        else
        {
          state = EDF_HSM_RET_HANDLED();
        }
      }
      break;
    }

    default:
    {
      state = EDF_HSM_RET_SUPER(fuzzExample_operationalState);
      break;
    }
  }

  return state;
}

static void fuzzExample_resetRuntime(fuzzExample_t* me)
{
  EAF_ASSERT(me != NULL);

  me->running_sum = 0U;
  me->match_slot = FUZZ_EXAMPLE_DEFAULT_SLOT;
  me->gate = FUZZ_EXAMPLE_DEFAULT_GATE;
  me->target_count = FUZZ_EXAMPLE_DEFAULT_TARGET_COUNT;
  me->collected_count = 0U;
  me->last_slot = 0U;
  me->last_value = 0U;
}

static bool fuzzExample_applySet(fuzzExample_t* me,
                                 const fuzzExample_setEvent_t* set_evt)
{
  bool is_valid;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(set_evt != NULL);
  EAF_ASSERT_BLOCK_END();

  ++me->set_count;

  if (fuzzExample_shouldCrashOnSet(set_evt) == true)
  {
    // Reserved single-event crash probe for the fuzz demo.
    fuzzExample_triggerCrash();
  }

  if (fuzzExample_shouldHangOnSet(set_evt) == true)
  {
    // Reserved single-event hang probe for the fuzz demo.
    fuzzExample_triggerHang();
  }

  is_valid = ((set_evt->slot != 0U) && (set_evt->gate < 0xF0U) &&
              (set_evt->target_count > 0U));

  if (is_valid == true)
  {
    me->match_slot = set_evt->slot;
    me->gate = set_evt->gate;
    me->target_count = set_evt->target_count;
    me->running_sum = 0U;
    me->collected_count = 0U;
  }
  else
  {
    ++me->rejected_count;
  }

  return is_valid;
}

static bool fuzzExample_acceptsSlot(const fuzzExample_t* me, uint8_t slot)
{
  EAF_ASSERT(me != NULL);
  return (slot == me->match_slot);
}

static uint8_t fuzzExample_adjustValue(uint8_t value, uint8_t flags)
{
  uint16_t adjusted_value;

  adjusted_value = value;

  if ((flags & FUZZ_EXAMPLE_STEP_FLAG_BOOST) != 0U)
  {
    adjusted_value = (uint16_t)(adjusted_value + FUZZ_EXAMPLE_STEP_BOOST_DELTA);

    if (adjusted_value > 0xFFU)
    {
      adjusted_value = 0xFFU;
    }
  }

  return (uint8_t)adjusted_value;
}

static void fuzzExample_beginCollecting(fuzzExample_t* me,
                                        const fuzzExample_stepEvent_t* step_evt,
                                        uint8_t adjusted_value)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(step_evt != NULL);
  EAF_ASSERT_BLOCK_END();

  me->last_slot = step_evt->slot;
  me->last_value = adjusted_value;
  me->collected_count = 1U;
  me->running_sum = adjusted_value;
}

static bool fuzzExample_shouldCrashOnSet(const fuzzExample_setEvent_t* set_evt)
{
  EAF_ASSERT(set_evt != NULL);

  return ((set_evt->slot == FUZZ_EXAMPLE_CRASH_SET_SLOT) &&
          (set_evt->gate == FUZZ_EXAMPLE_CRASH_SET_GATE) &&
          (set_evt->target_count == FUZZ_EXAMPLE_CRASH_SET_TARGET));
}

static bool fuzzExample_shouldHangOnSet(const fuzzExample_setEvent_t* set_evt)
{
  EAF_ASSERT(set_evt != NULL);

  return ((set_evt->slot == FUZZ_EXAMPLE_HANG_SET_SLOT) &&
          (set_evt->gate == FUZZ_EXAMPLE_HANG_SET_GATE) &&
          (set_evt->target_count == FUZZ_EXAMPLE_HANG_SET_TARGET));
}

static bool fuzzExample_shouldCrashOnFinish(const fuzzExample_t* me)
{
  EAF_ASSERT(me != NULL);

  return ((me->match_slot == FUZZ_EXAMPLE_FINISH_CRASH_SLOT) &&
          (me->gate == FUZZ_EXAMPLE_FINISH_CRASH_GATE) &&
          (me->target_count == FUZZ_EXAMPLE_FINISH_CRASH_TARGET) &&
          (me->collected_count == FUZZ_EXAMPLE_FINISH_CRASH_TARGET) &&
          (me->running_sum == FUZZ_EXAMPLE_FINISH_CRASH_SUM));
}

static bool fuzzExample_shouldHangOnStep(
  const fuzzExample_t* me,
  const fuzzExample_stepEvent_t* step_evt,
  uint8_t adjusted_value)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(step_evt != NULL);
  EAF_ASSERT_BLOCK_END();

  return (((step_evt->flags & FUZZ_EXAMPLE_STEP_FLAG_STOP) == 0U) &&
          ((step_evt->flags & FUZZ_EXAMPLE_STEP_FLAG_BOOST) != 0U) &&
          (me->collected_count >= 2U) &&
          (adjusted_value == FUZZ_EXAMPLE_STEP_HANG_VALUE));
}

static void fuzzExample_triggerCrash(void)
{
  abort();
}

static void fuzzExample_triggerHang(void)
{
  while (true)
  {
  }
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void fuzzExample_init(void)
{
  EDF_init();

  EDF_activeObject_pubSubInit(subscriberList,
                              (EDF_event_signal_t)FUZZ_EXAMPLE_LAST_SIG);

  EDF_pool_init(
    eventPoolStorage, sizeof(eventPoolStorage), FUZZ_EXAMPLE_POOL_BLOCK_SIZE);

  EDF_activeObject_init(EDF_AO_UPCAST(fuzzExample),
                        (EDF_hsm_stateHandler_t)fuzzExample_initialTransition);

  EDF_activeObject_start(EDF_AO_UPCAST(fuzzExample),
                         (EDF_activeObject_prio_t)FUZZ_EXAMPLE_AO_PRIO,
                         eventQueueStorage,
                         (EDF_eventQueue_cnt_t)FUZZ_EXAMPLE_QUEUE_LEN,
                         NULL,
                         0U,
                         NULL);

  EDF_activeObject_subscribe(EDF_AO_UPCAST(fuzzExample), FUZZ_EXAMPLE_PING_SIG);
  EDF_activeObject_subscribe(EDF_AO_UPCAST(fuzzExample),
                             FUZZ_EXAMPLE_START_SIG);
  EDF_activeObject_subscribe(EDF_AO_UPCAST(fuzzExample),
                             FUZZ_EXAMPLE_FINISH_SIG);
  EDF_activeObject_subscribe(EDF_AO_UPCAST(fuzzExample),
                             FUZZ_EXAMPLE_RESET_SIG);
  EDF_activeObject_subscribe(EDF_AO_UPCAST(fuzzExample), FUZZ_EXAMPLE_SET_SIG);
  EDF_activeObject_subscribe(EDF_AO_UPCAST(fuzzExample), FUZZ_EXAMPLE_STEP_SIG);
}
