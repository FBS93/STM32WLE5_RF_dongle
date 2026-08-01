/*******************************************************************************
 * @brief Event Driven Framework (EDF) private.
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

#ifndef EDF_FRAMEWORK_H
#define EDF_FRAMEWORK_H

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */
#include <stdint.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "edf_config.h"
#include "edf_active_object.h"
#include "edf_event.h"
#include "edf_pool.h"
#include "edf_time_event.h"

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief Subscriber registry.
 */
typedef struct
{
  /**
   * @brief Subscriber list, one entry per user event signal defined.
   *
   * Each entry corresponds to a signal event (index == signal value)
   * and stores the bitmask of active objects subscribed to that signal,
   * indexed by their priority.
   */
  EDF_activeObject_bitmask_t* list;

  /**
   * @brief Highest event signal supported in publish-subscribe.
   */
  EDF_event_signal_t max_signal;
} EDF_subscriberRegistry_t;

/**
 * @brief Pool registry.
 */
typedef struct
{
#if (EDF_MAX_POOL > 0U)
  EDF_pool_t list[EDF_MAX_POOL]; /**< Array of memory pools. */
#else
  EDF_pool_t list[1]; /**< Dummy entry to avoid conditional compilations. */
#endif
  uint8_t max_pool; /**< Number of memory pools in use. */
} EDF_poolRegistry_t;

/**
 * @brief Time event registry
 *
 * Registry of time events for a specific tick rate.
 */
typedef struct
{
  EDF_timeEvent_t* armed_head; /**< Points to the list of armed time events. */
  EDF_timeEvent_t* new_head; /**< Points to the list of newly armed time events.
                                  See @ref time_event_new_list. */
} EDF_timeEventRegistry_t;

/**
 * @brief EDF framework structure.
 */
typedef struct
{
  /**
   * @brief Active objects registry, indexed by their priority.
   *
   * Index 0 is unused; valid entries are 1 ... @c EDF_MAX_ACTIVE_OBJECT.
   *
   * @note The framework does not support unregistering; entries shall be
   * only modified to add new active objects, never removed or replaced.
   */
  EDF_activeObject_t* ao_registry[EDF_MAX_ACTIVE_OBJECT + 1U];
  EDF_subscriberRegistry_t subscr_registry; /**< Subscriber registry. */
  EDF_poolRegistry_t pool_registry;         /**< Pool registry. */
  /**
   * @brief Registry of time events for all tick rates.
   *
   * Each index of this array corresponds to a specific tick rate.
   * The user is responsible for calling @ref EDF_timeEvent_tick()
   * with the corresponding @p tick_rate index at the correct periodicity
   * to represent the desired tick rate.
   */
  EDF_timeEventRegistry_t te_registry[EDF_MAX_TICK_RATE];
} EDF_framework_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/**
 * @brief Instance of EDF_framework.
 *
 * This variable is public to allow core/port specific implementations.
 */
extern EDF_framework_t EDF_framework;

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initialize the EDF framework registries.
 *
 * @note This function must be called once before using any EDF framework
 * feature.
 */
void EDF_framework_init(void);

#endif /* EDF_FRAMEWORK_H */
