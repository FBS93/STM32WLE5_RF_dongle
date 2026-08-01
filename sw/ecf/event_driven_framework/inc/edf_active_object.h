/*******************************************************************************
 * @brief Event Driven Framework (EDF) active object.
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

#ifndef EDF_ACTIVE_OBJECT_H
#define EDF_ACTIVE_OBJECT_H

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */
#include <stdint.h>
#include <stdbool.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "edf_config.h"
#include "edf_hsm.h"
#include "edf_event_queue.h"
#include "ebf_config.h"

/**
 * Conditional library headers include depending on EBF_CORE
 * (defined in ebf_config.h)
 */
#if EBF_CORE == EBF_CORE_RTOS
#include "FreeRTOS.h"  // FreeRTOS master include file.
#include "task.h"      // FreeRTOS task management.
#include "queue.h"     // FreeRTOS queue management.
#elif (EBF_CORE == EBF_CORE_OS)
#include <pthread.h>  // POSIX-thread API.
#endif

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/**
 * @brief Cast to EDF_activeObject_t base type.
 *
 * @param[in] ao Active object instance.
 */
#define EDF_AO_UPCAST(ao) ((EDF_activeObject_t*)(&(ao)))

/**
 * @brief Compose the priority of an active object.
 *
 * @param[in] prio Base priority of the active object.
 * @param[in] pthre Preemption-threshold of the active object.
 */
#define EDF_AO_PRIO(prio, pthre) \
  ((EDF_activeObject_prio_t)((prio) | ((pthre) << 8U)))

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief Active object structure.
 */
typedef struct
{
  EDF_hsm_t super; /**< Base type. */
  uint8_t prio;    /**< Priority of the active object. */
  uint8_t pthre;   /**< Preemption threshold. */
#if (EBF_CORE == EBF_CORE_BAREMETAL)
  EDF_eventQueue_t e_queue; /**< Event queue. */
#elif (EBF_CORE == EBF_CORE_RTOS)
  StaticTask_t thread;         /**< RTOS task control block. */
  StaticQueue_t queue_storage; /**< RTOS event queue storage. */
  QueueHandle_t e_queue;       /**< RTOS event queue handle. */
#elif (EBF_CORE == EBF_CORE_OS)
#if (EBF_PORT == EBF_PORT_POSIX_PREEMPTIVE)
  pthread_cond_t e_cond; /**< Condition variable for event notification. */
#endif
  EDF_eventQueue_t e_queue; /**< Event queue. */
#endif
} EDF_activeObject_t;

/**
 * @brief Bitmask of active objects.
 */
typedef struct
{
  /**
   * @brief Bitmask indicating active objects using their priority.
   */
  uint8_t bitmask[(EDF_MAX_ACTIVE_OBJECT + 7U) / 8U];
} EDF_activeObject_bitmask_t;

/**
 * @brief Active object priority.
 *
 * The priority is composed of two parts packed into a 16-bit value:
 * - The lower 8 bits represent the base priority.
 * - The upper 8 bits represent the preemption-threshold.
 *
 * Use the macro @ref EDF_AO_PRIO(prio, pthre) to compose this value.
 *
 * @note Priority numbering is ascending (1 = lowest), up to
 * @c EDF_MAX_ACTIVE_OBJECT.
 *
 * @todo Consider separating base priority and preemption-threshold into
 * distinct typedefs and removing the @c EDF_activeObject_prio_t type from all
 * usage sites, to improve clarity and avoid packing both values into a single
 * type.
 */
typedef uint16_t EDF_activeObject_prio_t;

/**
 * @anchor preemption_threshold
 * @par Preemption threshold
 *
 * Each AO has a priority (prio) and an optional preemption-threshold (pthre).
 *   - prio decides scheduling order.
 *   - pthre limits which higher-priority AOs can preempt this AO.
 *   - if not specified, pthre = prio (default).
 *
 * AOs with prio ≤ pthre cannot preempt it (same group).
 *
 * Only AOs with prio > pthre can preempt.
 *
 * This allows grouping AOs to avoid unnecessary preemptions
 * while still preserving responsiveness to truly higher priorities.
 *
 * For consistency, each AO’s pthre must not be smaller than its lower
 * prio neighbor and must not be greater than its higher prio neighbor.
 */

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Public API functions for EDF library users
 * -------------------------------------------------------------------------- */
/**
 * @brief Initializes an active object.
 *
 * @param[in,out] me Pointer to the active object instance.
 * @param[in] initial Initial state handler function.
 */
void EDF_activeObject_init(EDF_activeObject_t* me,
                           EDF_hsm_stateHandler_t initial);

/**
 * @brief Sets implementation-specific attributes for an active object.
 *
 * This function can only be called after @ref EDF_activeObject_init() and
 * before @ref EDF_activeObject_start(), only if specific attributes need to be
 * configured,
 * depending on the selected core and port.
 *
 * @param[in,out] me Pointer to the active object instance.
 * @param[in] attr1 Attribute identifier.
 * @param[in] attr2 Pointer to attribute data.
 */
void EDF_activeObject_setAttr(EDF_activeObject_t* me,
                              uint32_t attr1,
                              const void* attr2);

/**
 * @brief Starts the execution of an active object.
 *
 * @note If the active object has no event queue, it can only retain
 * one event at a time; subsequent events will be lost.
 *
 * @param[in,out] me Pointer to the active object instance.
 * @param[in] prio Priority of the active object.
 * @param[in] q_storage Pointer to the event queue storage buffer. NULL if no
 * queue
 * @param[in] q_len Length of the event queue. 0 if no queue.
 * @param[in] stack_storage Pointer to the stack storage.
 * @param[in] stack_size Size of the stack storage.
 * @param[in] e Event passed to the AO’s HSM initial state. NULL if not used.
 * If NULL is provided, the AO initial state implementation shall not
 * dereference or rely on this parameter.
 */
void EDF_activeObject_start(EDF_activeObject_t* me,
                            EDF_activeObject_prio_t prio,
                            EDF_event_ptr* q_storage,
                            EDF_eventQueue_cnt_t q_len,
                            void* stack_storage,
                            uint_fast16_t stack_size,
                            const EDF_event_t* e);

/**
 * @brief Initializes the publish-subscribe feature.
 *
 * Shall be called before any other function related to the publish-subscribe
 * feature.
 *
 * @note The subscriberList array shall provide exactly [max_signal] entries.
 *
 * @note The @p subscriberList array shall also include the reserved entries for
 * signals below @ref EDF_HSM_USER_SIGNAL.
 *
 * Example:
 * @code
 * typedef enum {
 *   SIGNAL_1 = EDF_HSM_USER_SIGNAL,
 *   SIGNAL_2,
 *   LAST_SIGNAL
 * } hsm_eventSignal_t;
 *
 * static EDF_activeObject_bitmask_t subscriberList[LAST_SIGNAL];
 *
 * EDF_activeObject_pubSubInit(subscriberList, LAST_SIGNAL);
 * @endcode
 *
 * The EDF reserved signal entries remain intentionally unused for simplicity of
 * the implementation, ensuring consistent indexing between EDF and user
 * signals.
 *
 * @todo Evaluate whether it is worth optimizing memory usage by avoiding
 * reserved entries for EDF signals (those below @ref EDF_HSM_USER_SIGNAL).
 * These entries remain intentionally unused in the current implementation
 * for indexing simplicity.
 *
 * @param[in] subscriber_list Pointer to the array of subscriber lists.
 * @param[in] max_signal Highest user event signal defined.
 */
void EDF_activeObject_pubSubInit(EDF_activeObject_bitmask_t* subscriber_list,
                                 EDF_event_signal_t max_signal);

/**
 * @brief Publishes an event to all subscribed active objects.
 *
 * @param[in] e Pointer to the event to be published.
 */
void EDF_activeObject_publish(const EDF_event_t* e);

/**
 * @brief Subscribes an active object to a specific event signal.
 *
 * @param[in] me Pointer to the active object instance.
 * @param[in] sig Event signal to subscribe to.
 */
void EDF_activeObject_subscribe(const EDF_activeObject_t* me,
                                EDF_event_signal_t sig);

/**
 * @brief Subscribes an active object to all event signals.
 *
 * @param[in] me Pointer to the active object instance.
 */
void EDF_activeObject_subscribeAll(const EDF_activeObject_t* me);

/**
 * @brief Unsubscribes an active object from a specific event signal.
 *
 * @param[in] me Pointer to the active object instance.
 * @param[in] sig Event signal to unsubscribe from.
 */
void EDF_activeObject_unsubscribe(const EDF_activeObject_t* me,
                                  EDF_event_signal_t sig);

/**
 * @brief Unsubscribes an active object from all event signals.
 *
 * @param[in] me Pointer to the active object instance.
 */
void EDF_activeObject_unsubscribeAll(const EDF_activeObject_t* me);

/**
 * @brief Defers an event by placing it into a deferred event queue.
 *
 * @param[in] me Pointer to the active object instance.
 * @param[in,out] e_queue Pointer to the deferred event queue.
 * @param[in] e Pointer to the event to defer.
 */
void EDF_activeObject_defer(const EDF_activeObject_t* me,
                            EDF_eventQueue_t* e_queue,
                            const EDF_event_t* e);

/**
 * @brief Recalls (retrieves) a deferred event from the deferred event queue.
 *
 * @param[in,out] me Pointer to the active object instance.
 * @param[in,out] e_queue Pointer to the deferred event queue.
 */
void EDF_activeObject_recall(EDF_activeObject_t* me, EDF_eventQueue_t* e_queue);

/**
 * @brief Removes a number of deferred events from the deferred event queue.
 *
 * @param[in] me Pointer to the active object instance.
 * @param[in,out] e_queue Pointer to the deferred event queue.
 * @param[in] num Maximum number of events to flush.
 * @return The actual number of events flushed from the queue.
 */
uint_fast16_t EDF_activeObject_flushDeferred(const EDF_activeObject_t* me,
                                             EDF_eventQueue_t* e_queue,
                                             uint_fast16_t num);

/* -----------------------------------------------------------------------------
 * Internal functions for EDF library use only
 * -------------------------------------------------------------------------- */

/**
 * @brief Registers an active object within the EDF framework.
 *
 * @note For internal EDF library use only.
 *
 * @param[in,out] me Pointer to the active object instance.
 */
void EDF_activeObject_register(EDF_activeObject_t* me);

/**
 * @brief Posts an event to the back of an active object's queue.
 *
 * @note For internal EDF library use only.
 *
 * @param[in,out] me Pointer to the active object instance.
 * @param[in] e Pointer to the event to post.
 */
void EDF_activeObject_postFIFO(EDF_activeObject_t* me, const EDF_event_t* e);

/**
 * @brief Posts an event to the front of an active object's queue.
 *
 * @note For internal EDF library use only.
 *
 * @param[in,out] me Pointer to the active object instance.
 * @param[in] e Pointer to the event to post.
 */
void EDF_activeObject_postLIFO(EDF_activeObject_t* me, const EDF_event_t* e);

/**
 * @brief Retrieves the next event from the active object's event queue.
 *
 * @note For internal EDF library use only.
 *
 * @param[in,out] me Pointer to the active object instance.
 * @return Pointer to the next event.
 */
const EDF_event_t* EDF_activeObject_get(EDF_activeObject_t* me);

#endif /* EDF_ACTIVE_OBJECT_H */
