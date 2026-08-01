/*******************************************************************************
 * @brief Circular FIFO for byte array storage.
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

#ifndef EMF_ARRAY_FIFO_H
#define EMF_ARRAY_FIFO_H

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

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief Handler for managing a circular FIFO buffer.
 *
 * This structure acts as a handle to an internal FIFO context.
 *
 * It is not meant to be accessed or modified directly by user code;
 * all operations must be performed via the API functions.
 */
typedef struct
{
  uint8_t n_slots;     ///< Number of available slots in the FIFO.
  uint16_t slot_size;  ///< Size (in bytes) of each individual slot.
  /**
   * @brief Pointer to the FIFO storage.
   *
   * @note The FIFO storage is not intended to be accessed directly outside this
   * module's API. However, if direct access to the FIFO storage memory is
   * required, it is the user's responsibility to ensure that the storage is
   * properly memory-aligned. It is recommended to define the FIFO storage using
   * the @ref EMF_UTILS_MEM_ALIGNED_SLOT macro or an equivalent alignment-safe
   * definition for portability across architectures.
   */
  uint8_t* storage;
  uint8_t head;  ///< Index of the next write position.
  uint8_t tail;  ///< Index of the next read position.
  bool full;     ///< Indicates whether the FIFO is currently full.
} EMF_arrayFifo_handler_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the FIFO.
 *
 * Sets up the FIFO with the specified sizes.
 *
 * @param[in,out] handler Pointer to the FIFO handler structure to initialize.
 * @param[in] n_slots Number of FIFO slots.
 * @param[in] slot_size Total size of one FIFO slot (size in bytes).
 * @param[in] storage Pointer to the FIFO storage.
 */
void EMF_arrayFifo_init(EMF_arrayFifo_handler_t* handler,
                        uint8_t n_slots,
                        uint16_t slot_size,
                        uint8_t* storage);

/**
 * @brief Pushes data to the FIFO.
 *
 * The length of data pushed is equal to the slot size defined during
 * initialization.
 *
 * @param[in,out] handler Pointer to the FIFO handler structure.
 * @param[in] data Pointer to the buffer containing data to push.
 */
void EMF_arrayFifo_push(EMF_arrayFifo_handler_t* handler, const uint8_t* data);

/**
 * @brief Pops data from the FIFO.
 *
 * The length of data popped is equal to the slot size defined during
 * initialization.
 *
 * @param[in,out] handler Pointer to the FIFO handler structure.
 * @param[out] data Pointer to the buffer where the popped data will be stored.
 */
void EMF_arrayFifo_pop(EMF_arrayFifo_handler_t* handler, uint8_t* data);

/**
 * @brief Peeks data from the FIFO without removing it.
 *
 * The length of data copied is equal to the slot size defined during
 * initialization.
 *
 * @param[in] handler Pointer to the FIFO handler structure.
 * @param[out] data Pointer to the buffer where the peeked data will be stored.
 */
void EMF_arrayFifo_peek(const EMF_arrayFifo_handler_t* handler, uint8_t* data);

/**
 * @brief Drops the first element from the FIFO without copying it.
 *
 * Advances the FIFO tail index to remove the first element without copying it,
 * effectively discarding the oldest data in the FIFO.
 *
 * @param[in,out] handler Pointer to the FIFO handler structure.
 */
void EMF_arrayFifo_drop(EMF_arrayFifo_handler_t* handler);

/**
 * @brief Flushes the FIFO.
 *
 * Clears the FIFO by resetting the head and tail indices to 0.
 *
 * @param[in,out] handler Pointer to the FIFO handler structure.
 */
void EMF_arrayFifo_flush(EMF_arrayFifo_handler_t* handler);

/**
 * @brief Checks if the FIFO is empty.
 *
 * @param[in] handler Pointer to the FIFO handler structure.
 * @return true if the FIFO is empty, false otherwise.
 */
bool EMF_arrayFifo_isEmpty(const EMF_arrayFifo_handler_t* handler);

/**
 * @brief Checks if the FIFO is full.
 *
 * @param[in] handler Pointer to the FIFO handler structure.
 * @return true if the FIFO is full, false otherwise.
 */
bool EMF_arrayFifo_isFull(const EMF_arrayFifo_handler_t* handler);

/**
 * @brief Returns used capacity in FIFO slots.
 *
 * @param[in] handler Pointer to the FIFO handler structure.
 * @return Number of used slots.
 */
uint8_t EMF_arrayFifo_getUsed(const EMF_arrayFifo_handler_t* handler);

/**
 * @brief Returns free capacity in FIFO slots.
 *
 * @param[in] handler Pointer to the FIFO handler structure.
 * @return Number of free slots.
 */
uint8_t EMF_arrayFifo_getFree(const EMF_arrayFifo_handler_t* handler);

#endif /* EMF_ARRAY_FIFO_H */
