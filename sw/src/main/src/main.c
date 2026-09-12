/*******************************************************************************
 * @brief Project composition and startup entry point.
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
#include "edf.h"
#include "emf.h"

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "events.h"
#include "host_command_manager.h"
#include "rf_manager.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/** @brief Number of mutable-event pool slots. */
#define MUTABLE_EVENT_POOL_SLOT_COUNT 10U

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/** @brief Host command manager instance. */
static hostCommandManager_t hostCommandManager;

/** @brief RF manager instance. */
static rfManager_t rfManager;

/** @brief EDF publish-subscribe storage. */
static EDF_activeObject_bitmask_t subscriberList[EVENTS_LAST_SIGNAL];

/**
 * @brief Mutable-event pool using the largest current mutable event.
 *
 * @todo Evaluate splitting mutable events into pools of different block sizes
 * to optimize memory.
 */
static EMF_UTILS_MEM_ALIGNED_SLOT(sizeof(events_rxPacket_t))
  mutableEventPool[MUTABLE_EVENT_POOL_SLOT_COUNT];

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

/** @brief Starts application event-producing sources. */
void EDF_onStartup(void)
{
  rfManager_startup();
  hostCommandManager_startup();
}

/** @brief Initializes the application and transfers control to EDF. */
int main(void)
{
  EDF_init();
  EDF_activeObject_pubSubInit(subscriberList,
                              (EDF_event_signal_t)EVENTS_LAST_SIGNAL);
  EDF_pool_init(mutableEventPool,
                sizeof(mutableEventPool),
                (EDF_pool_blockSize_t)sizeof(events_rxPacket_t));
  rfManager_init(&rfManager);
  hostCommandManager_init(&hostCommandManager);

  return EDF_run();
}
