/*******************************************************************************
 * @brief Event Driven Framework (EDF) event.
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

#ifndef EDF_EVENT_H
#define EDF_EVENT_H

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */
#include <stdint.h>
#include <stddef.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "edf_config.h"

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/**
 * @brief Reserved value for immutable events (marks them as constant).
 */
#define EDF_EVENT_REF_CNT_IMMUTABLE (0xFFU)

/**
 * @brief Initializer for an immutable event.
 */
#define EDF_EVENT_IMMUTABLE_INIT(sig)                             \
  {                                                               \
    (EDF_event_signal_t)(sig), 0x00U, EDF_EVENT_REF_CNT_IMMUTABLE \
  }

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief Event signal type.
 */
#if (EDF_MAX_SIGNAL == 1U)
typedef uint8_t EDF_event_signal_t;
#elif (EDF_MAX_SIGNAL == 2U)
typedef uint16_t EDF_event_signal_t;
#elif (EDF_MAX_SIGNAL == 4U)
typedef uint32_t EDF_event_signal_t;
#endif

/**
 * @brief Event structure.
 */
typedef struct EDF_event_t
{
  EDF_event_signal_t sig; /**< Event signal. */
  uint8_t pool_num;       /**< Pool number. */
  uint8_t ref_cnt;        /**< Reference counter. */
} EDF_event_t;

/**
 * @brief Pointer to an event.
 */
typedef const EDF_event_t* EDF_event_ptr;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes an immutable event.
 *
 * @param[in,out] me Pointer to the event instance to initialize.
 * @param[in] sig Event signal.
 */
void EDF_event_initImmutable(EDF_event_t* me, EDF_event_signal_t sig);

/**
 * @brief Create and initialize a mutable event.
 *
 * @param[in] e_size Size of the event in bytes.
 * @param[in] sig Event signal.
 * @return Pointer to the newly created mutable event, or NULL if no pool
 * space is available.
 */
EDF_event_t* EDF_event_initMutable(uint_fast16_t e_size, int sig);

/**
 * @brief Increments the reference counter of an event.
 *
 * @note This function shall be called inside a critical section.
 *
 * @param[in,out] me Pointer to the event instance.
 */
void EDF_event_incrementRefCnt_(EDF_event_t* me);

/**
 * @brief Decrements the reference counter of an event.
 *
 * @note This function shall be called inside a critical section.
 *
 * @param[in,out] me Pointer to the event instance.
 */
void EDF_event_decrementRefCnt_(EDF_event_t* me);

/**
 * @brief Garbage collector for a mutable event.
 *
 * Releases the pool if the event is no longer referenced.
 *
 * @param[in] e Pointer to the mutable event to be recycled.
 */
void EDF_event_gc(EDF_event_t* e);

#endif  // EDF_EVENT_H
