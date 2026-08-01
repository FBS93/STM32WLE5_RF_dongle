/*******************************************************************************
 * @brief Event Driven Framework (EDF) event queue.
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

#ifndef EDF_EVENT_QUEUE_H
#define EDF_EVENT_QUEUE_H

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
#include "edf_config.h"
#include "edf_event.h"

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief Event queue counter type.
 */
#if (EDF_EVENT_QUEUE_SIZE == 1U)
typedef uint8_t EDF_eventQueue_cnt_t;
#elif (EDF_EVENT_QUEUE_SIZE == 2U)
typedef uint16_t EDF_eventQueue_cnt_t;
#endif

/**
 * @brief Event queue structure.
 */
typedef struct EDF_eventQueue_t
{
  const EDF_event_t* front_e;  /**< Event at the front of the queue. */
  const EDF_event_t** ring;    /**< Ring buffer storage. */
  EDF_eventQueue_cnt_t end;    /**< Index of the end of the ring buffer. */
  EDF_eventQueue_cnt_t head;   /**< Head index for inserting events. */
  EDF_eventQueue_cnt_t tail;   /**< Tail index for removing events. */
  EDF_eventQueue_cnt_t n_free; /**< Number of free entries in the queue. */
  EDF_eventQueue_cnt_t
    n_min; /**< Minimum number of free entries ever in the queue. */
} EDF_eventQueue_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes an event queue.
 *
 * @note If @p q_storage is NULL and @p q_len is 0, the queue will have no
 * ring buffer and can only hold one event at a time in @c front_e.
 *
 * @param[in,out] me Pointer to the event queue instance.
 * @param[in] q_storage Pointer to the ring buffer storage
 * (can be NULL if @p q_len is 0).
 * @param[in] q_len Length of the ring buffer
 * (can be 0 if @p q_storage is NULL).
 */
void EDF_eventQueue_init(EDF_eventQueue_t* me,
                         const EDF_event_t** q_storage,
                         EDF_eventQueue_cnt_t q_len);

/**
 * @brief Posts an event to the back of the event queue.
 *
 * @param[in,out] me Pointer to the event queue instance.
 * @param[in] e Pointer to the event to post.
 */
void EDF_eventQueue_postFIFO(EDF_eventQueue_t* me, const EDF_event_t* e);

/**
 * @brief Posts an event to the front of the event queue.
 *
 * @param[in,out] me Pointer to the event queue instance.
 * @param[in] e Pointer to the event to post.
 */
void EDF_eventQueue_postLIFO(EDF_eventQueue_t* me, const EDF_event_t* e);

/**
 * @brief Retrieves the next event from the queue (FIFO order).
 *
 * @param[in,out] me Pointer to the event queue instance.
 * @return Pointer to the retrieved event, or NULL if the queue is empty.
 */
const EDF_event_t* EDF_eventQueue_get(EDF_eventQueue_t* me);

/**
 * @brief Gets the current number of free entries in the event queue.
 *
 * @param[in] me Pointer to the event queue instance.
 * @return Number of free entries.
 */
static inline EDF_eventQueue_cnt_t EDF_eventQueue_getNumFree(
  const EDF_eventQueue_t* me)
{
  return me->n_free;
}

/**
 * @brief Gets the current number of occupied entries in the event queue.
 *
 * @param[in] me Pointer to the event queue instance.
 * @return Number of occupied entries.
 */
static inline EDF_eventQueue_cnt_t EDF_eventQueue_getNumOccupied(
  const EDF_eventQueue_t* me)
{
  // Number of occupied entries = total capacity - free slots
  return (EDF_eventQueue_cnt_t)((me->end + 1U) - me->n_free);
}

/**
 * @brief Checks whether the event queue is empty.
 *
 * @param[in] me Pointer to the event queue instance.
 * @return true if queue is empty, false otherwise.
 */
static inline bool EDF_eventQueue_isEmpty(const EDF_eventQueue_t* me)
{
  return (me->front_e == NULL);
}

/**
 * @brief Gets the minimum number of free entries ever in the event queue.
 *
 * @param[in] me Pointer to the event queue instance.
 * @return Minimum number of free entries recorded.
 */
static inline EDF_eventQueue_cnt_t EDF_eventQueue_getNumMin(
  const EDF_eventQueue_t* me)
{
  return me->n_min;
}

#endif /* EDF_EVENT_QUEUE_H */
