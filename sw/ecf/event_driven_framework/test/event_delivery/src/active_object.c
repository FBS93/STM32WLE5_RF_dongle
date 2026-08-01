/*******************************************************************************
 * @brief Active objects implementation, used for test validation.
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

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "active_object.h"
#include "edf.h"
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

/**
 * @brief Subscriber lists used by the publish–subscribe mechanism.
 */
static EDF_activeObject_bitmask_t subscriberList[LAST_EVENT_SIGNAL];

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

activeObject_t activeObject[ACTIVE_OBJECT_N_TOTAL];

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/**
 * @brief Initial transition of the activeObject HSM.
 *
 * @param[in] me Pointer to the activeObject instance.
 * @param[in] e Pointer to the event.
 */
static EDF_hsm_stateReturn_t initialTransition(activeObject_t* me,
                                               const EDF_event_t* e);

/**
 * @brief readyState state of the activeObject HSM.
 *s
 * @param[in] me Pointer to the activeObject instance.
 * @param[in] e Pointer to the event.
 */
static EDF_hsm_stateReturn_t readyState(activeObject_t* me,
                                        const EDF_event_t* e);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static EDF_hsm_stateReturn_t initialTransition(activeObject_t* me,
                                               const EDF_event_t* e)
{
  EMF_UTILS_UNUSED_PARAM(me);
  EMF_UTILS_UNUSED_PARAM(e);

  return EDF_HSM_RET_TRAN(readyState);
}

static EDF_hsm_stateReturn_t readyState(activeObject_t* me,
                                        const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  if ((e->sig >= S1) && (e->sig <= LAST_EVENT_SIGNAL))
  {
    me->event_received = true;
  }

  if (me->event_received)
  {
    state = EDF_HSM_RET_HANDLED();
  }
  else
  {
    state = EDF_HSM_RET_SUPER(&EDF_hsm_top);
  }

  return state;
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @todo Evaluate adding test validation for correct reception order of
 * events taking into account the AOs priority. This could be done by setting
 * an array within the HSM states to record the sequence of state activity
 * executions.
 */

void activeObject_init(void)
{
  EDF_activeObject_prio_t ao_prio;

  // Init EDF.
  EDF_init();

  // Init EDF publish and subscribe functionality.
  EDF_activeObject_pubSubInit(subscriberList,
                              (EDF_event_signal_t)LAST_EVENT_SIGNAL);

  /**
   * Subscribes each active object to specific event signals:
   *   - All but the last active object subscribe to a unique signal.
   *   - The last active object subscribes to all signals.
   */
  ao_prio = 2;
  for (uint8_t ao_idx = 0; ao_idx < ACTIVE_OBJECT_N_TOTAL; ao_idx++)
  {
    EDF_activeObject_init(EDF_AO_UPCAST(activeObject[ao_idx]),
                          (EDF_hsm_stateHandler_t)initialTransition);
    EDF_activeObject_start(
      EDF_AO_UPCAST(activeObject[ao_idx]), ao_prio, NULL, 0, NULL, 0, NULL);

    if (ao_idx < (ACTIVE_OBJECT_N_TOTAL - 1))
    {
      EDF_activeObject_subscribe(EDF_AO_UPCAST(activeObject[ao_idx]),
                                 ACTIVE_OBJECT_PRIO_TO_SIGNAL(ao_prio));
    }
    else
    {
      for (activeObject_eventSignal_t e_sig = S1; e_sig <= LAST_EVENT_SIGNAL;
           e_sig++)
      {
        EDF_activeObject_subscribe(EDF_AO_UPCAST(activeObject[ao_idx]), e_sig);
      }
    }

    ao_prio++;
  }
}

void activeObject_resetEventReceivedFlag(void)
{
  for (uint8_t ao_idx = 0; ao_idx < ACTIVE_OBJECT_N_TOTAL; ao_idx++)
  {
    activeObject[ao_idx].event_received = false;
  }
}