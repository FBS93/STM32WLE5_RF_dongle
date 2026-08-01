/*******************************************************************************
 * @brief Active object and HSM implementation, used for test validation.
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
#include "hsm.h"
#include "edf.h"
#include "eaf.h"
#include "emf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief hsm active object priority.
 */
#define HSM_AO_PRIO 2U

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

hsm_t hsm;

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
 * @todo Verify that the defined HSM state depth allows reaching the top level
 * with the default value of EDF_HSM_MAX_DEPTH (6).
 */

/**
 * @todo Evaluate adding test validation for correct execution order of
 * init/entry/exit activities. This could be done by setting an array or
 * log within the HSM states to record the sequence of state activity
 * executions.
 */

void hsm_init(void)
{
  // Init EDF.
  EDF_init();

  // Init EDF publish and subscribe functionality.
  EDF_activeObject_pubSubInit(subscriberList,
                              (EDF_event_signal_t)LAST_EVENT_SIGNAL);

  // Init hsm active object.
  EDF_activeObject_init(EDF_AO_UPCAST(hsm),
                        (EDF_hsm_stateHandler_t)initialTransition);

  // Start active object.
  EDF_activeObject_start(EDF_AO_UPCAST(hsm),
                         (EDF_activeObject_prio_t)HSM_AO_PRIO,
                         NULL,
                         0,
                         NULL,
                         0,
                         NULL);

  // Subscribe to event.
  EDF_activeObject_subscribe(EDF_AO_UPCAST(hsm), TRANSITION);
}

/**
 * @todo Validate that the initial transition correctly receives event
 * parameters passed by pointer in the EDF_activeObject_start function, in
 * accordance with the API specification.
 */
EDF_hsm_stateReturn_t initialTransition(hsm_t* me, const EDF_event_t* e)
{
  EMF_UTILS_UNUSED_PARAM(me);
  EMF_UTILS_UNUSED_PARAM(e);

  return EDF_HSM_RET_TRAN(S1);
}

EDF_hsm_stateReturn_t S1(hsm_t* me, const EDF_event_t* e)
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
      state = EDF_HSM_RET_TRAN(S11);
      break;
    }
    case TRANSITION:
    {
      state = EDF_HSM_RET_TRAN(S211);
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

EDF_hsm_stateReturn_t S11(hsm_t* me, const EDF_event_t* e)
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
      state = EDF_HSM_RET_TRAN(S111);
      break;
    }
    default:
    {
      state = EDF_HSM_RET_SUPER(S1);
      break;
    }
  }

  return state;
}

EDF_hsm_stateReturn_t S111(hsm_t* me, const EDF_event_t* e)
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
      state = EDF_HSM_RET_TRAN(S1111);
      break;
    }
    default:
    {
      state = EDF_HSM_RET_SUPER(S11);
      break;
    }
  }

  return state;
}

EDF_hsm_stateReturn_t S1111(hsm_t* me, const EDF_event_t* e)
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
      state = EDF_HSM_RET_TRAN(S11111);
      break;
    }
    default:
    {
      state = EDF_HSM_RET_SUPER(S111);
      break;
    }
  }

  return state;
}

EDF_hsm_stateReturn_t S11111(hsm_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    default:
    {
      state = EDF_HSM_RET_SUPER(S1111);
      break;
    }
  }

  return state;
}

EDF_hsm_stateReturn_t S2(hsm_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case TRANSITION:
    {
      state = EDF_HSM_RET_TRAN(S21);
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

EDF_hsm_stateReturn_t S21(hsm_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case TRANSITION:
    {
      state = EDF_HSM_RET_TRAN(S31);
      break;
    }
    default:
    {
      state = EDF_HSM_RET_SUPER(S2);
      break;
    }
  }

  return state;
}

EDF_hsm_stateReturn_t S211(hsm_t* me, const EDF_event_t* e)
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
      state = EDF_HSM_RET_TRAN(S2111);
      break;
    }
    default:
    {
      state = EDF_HSM_RET_SUPER(S21);
      break;
    }
  }

  return state;
}

EDF_hsm_stateReturn_t S2111(hsm_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case TRANSITION:
    {
      state = EDF_HSM_RET_TRAN(S2112);
      break;
    }
    default:
    {
      state = EDF_HSM_RET_SUPER(S211);
      break;
    }
  }

  return state;
}

EDF_hsm_stateReturn_t S2112(hsm_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case TRANSITION:
    {
      state = EDF_HSM_RET_TRAN(S2);
      break;
    }
    default:
    {
      state = EDF_HSM_RET_SUPER(S211);
      break;
    }
  }

  return state;
}

EDF_hsm_stateReturn_t S3(hsm_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case TRANSITION:
    {
      state = EDF_HSM_RET_TRAN(S311);
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

EDF_hsm_stateReturn_t S31(hsm_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case TRANSITION:
    {
      state = EDF_HSM_RET_TRAN(S3);
      break;
    }
    default:
    {
      state = EDF_HSM_RET_SUPER(S3);
      break;
    }
  }

  return state;
}

EDF_hsm_stateReturn_t S311(hsm_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case TRANSITION:
    {
      state = EDF_HSM_RET_TRAN(S41);
      break;
    }
    default:
    {
      state = EDF_HSM_RET_SUPER(S31);
      break;
    }
  }

  return state;
}

EDF_hsm_stateReturn_t S4(hsm_t* me, const EDF_event_t* e)
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
      state = EDF_HSM_RET_TRAN(S41);
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

EDF_hsm_stateReturn_t S41(hsm_t* me, const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t state;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case TRANSITION:
    {
      if (me->self_transition)
      {
        state = EDF_HSM_RET_TRAN(S41);
      }
      else
      {
        state = EDF_HSM_RET_TRAN(S4);
      }
      break;
    }
    default:
    {
      state = EDF_HSM_RET_SUPER(S4);
      break;
    }
  }

  return state;
}