/*******************************************************************************
 * @brief Host command manager active object implementation.
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
#include <stddef.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */
#include "eaf.h"
#include "edf.h"

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "events.h"
#include "host_command_manager.h"
#include "host_command_manager_activities.h"
#include "host_command_manager_hal.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/** @brief Architecture-defined active-object priority. */
#define ACTIVE_OBJECT_PRIORITY EDF_AO_PRIO(2U, 2U)
/** @brief Queue capacity for the five subscribed event signals. */
#define EVENT_QUEUE_CAPACITY 5U

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/** @brief Define static file name string for asserts. */
EAF_DEFINE_THIS_FILE(__FILE__);

/** @brief Active-object event queue storage. */
static EDF_event_ptr eventQueueStorage[EVENT_QUEUE_CAPACITY];

/** @brief Event used to execute the top-most HSM initial transition. */
static const EDF_event_t initializationEvent =
  EDF_EVENT_IMMUTABLE_INIT(EDF_HSM_INIT_SIGNAL);

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
 * @brief Performs the initial transition to idle.
 *
 * @param[in,out] me Pointer to the host command manager instance.
 * @param[in] e Pointer to the initialization event.
 * @return HSM transition result.
 */
static EDF_hsm_stateReturn_t initialTransition(hostCommandManager_t* me,
                                               const EDF_event_t* e);

/**
 * @brief Handles events in the idle state.
 *
 * @param[in,out] me Pointer to the host command manager instance.
 * @param[in] e Pointer to the event being dispatched.
 * @return HSM dispatch result.
 */
static EDF_hsm_stateReturn_t idle(hostCommandManager_t* me,
                                  const EDF_event_t* e);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static EDF_hsm_stateReturn_t initialTransition(hostCommandManager_t* me,
                                               const EDF_event_t* e)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  return EDF_HSM_RET_TRAN(idle);
}

static EDF_hsm_stateReturn_t idle(hostCommandManager_t* me,
                                  const EDF_event_t* e)
{
  EDF_hsm_stateReturn_t result;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(me != NULL);
  EAF_ASSERT_IN_BLOCK(e != NULL);
  EAF_ASSERT_BLOCK_END();

  switch (e->sig)
  {
    case EVENTS_HOST_DATA_AVAILABLE:
    {
      hostCommandManagerActivities_handleHostDataAvailable(
        me, (const events_hostDataAvailable_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    }
    case EVENTS_GET_CONFIG_RESPONSE:
    {
      hostCommandManagerActivities_handleGetConfigResponse(
        me, (const events_getConfigResponse_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    }
    case EVENTS_GET_DIAGNOSTICS_RESPONSE:
    {
      hostCommandManagerActivities_handleGetDiagnosticsResponse(
        me, (const events_getDiagnosticsResponse_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    }
    case EVENTS_RX_PACKET:
    {
      hostCommandManagerActivities_handleRxPacket(me,
                                                  (const events_rxPacket_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    }
    case EVENTS_CAD_DETECTED:
    {
      hostCommandManagerActivities_handleCadDetected(
        me, (const events_cadDetected_t*)e);
      result = EDF_HSM_RET_TRAN(idle);
      break;
    }
    default:
    {
      result = EDF_HSM_RET_SUPER(EDF_hsm_top);
      break;
    }
  }

  return result;
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void hostCommandManager_init(hostCommandManager_t* me)
{
  EAF_ASSERT(me != NULL);

  me->rx_frame_length = 0U;
  EDF_activeObject_init(EDF_AO_UPCAST(*me),
                        (EDF_hsm_stateHandler_t)initialTransition);
  EDF_activeObject_start(EDF_AO_UPCAST(*me),
                         ACTIVE_OBJECT_PRIORITY,
                         eventQueueStorage,
                         EVENT_QUEUE_CAPACITY,
                         NULL,
                         0U,
                         &initializationEvent);
  EDF_activeObject_subscribe(EDF_AO_UPCAST(*me), EVENTS_GET_CONFIG_RESPONSE);
  EDF_activeObject_subscribe(EDF_AO_UPCAST(*me),
                             EVENTS_GET_DIAGNOSTICS_RESPONSE);
  EDF_activeObject_subscribe(EDF_AO_UPCAST(*me), EVENTS_RX_PACKET);
  EDF_activeObject_subscribe(EDF_AO_UPCAST(*me), EVENTS_CAD_DETECTED);
  EDF_activeObject_subscribe(EDF_AO_UPCAST(*me), EVENTS_HOST_DATA_AVAILABLE);
}

void hostCommandManager_startup(void)
{
  hostCommandManagerHal_init();
}
