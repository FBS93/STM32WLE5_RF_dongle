/*******************************************************************************
 * @brief Circular FIFO for byte streams.
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

#ifndef EMF_BYTE_FIFO_H
#define EMF_BYTE_FIFO_H

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
 * @brief Handler for a byte-oriented circular FIFO.
 *
 * This structure is managed through module APIs and should not be modified
 * directly by user code.
 */
typedef struct
{
  uint8_t* storage;  ///< FIFO storage memory.
  uint32_t size;     ///< Storage capacity in bytes.
  uint32_t head;     ///< Next write index.
  uint32_t tail;     ///< Next read index.
  uint32_t used;     ///< Occupied bytes.
} EMF_byteFifo_handler_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the byte FIFO.
 *
 * @param[in,out] handler FIFO handler to initialize.
 * @param[in] size Storage capacity in bytes.
 * @param[in] storage Pointer to FIFO storage.
 */
void EMF_byteFifo_init(EMF_byteFifo_handler_t* handler,
                       uint32_t size,
                       uint8_t* storage);

/**
 * @brief Pushes bytes into the FIFO.
 *
 * @param[in,out] handler FIFO handler.
 * @param[in] data Data to push.
 * @param[in] len Number of bytes to push.
 */
void EMF_byteFifo_push(EMF_byteFifo_handler_t* handler,
                       const uint8_t* data,
                       uint32_t len);

/**
 * @brief Pops bytes from the FIFO.
 *
 * @param[in,out] handler FIFO handler.
 * @param[out] data Output buffer.
 * @param[in] len Number of bytes to pop.
 */
void EMF_byteFifo_pop(EMF_byteFifo_handler_t* handler,
                      uint8_t* data,
                      uint32_t len);

/**
 * @brief Copies bytes from the FIFO without removing them.
 *
 * Equivalent to @ref EMF_byteFifo_peekAt with offset equal to 0.
 *
 * @param[in] handler FIFO handler.
 * @param[out] data Output buffer.
 * @param[in] len Number of bytes to copy.
 */
void EMF_byteFifo_peek(const EMF_byteFifo_handler_t* handler,
                       uint8_t* data,
                       uint32_t len);

/**
 * @brief Copies bytes from the FIFO without removing them.
 *
 * @param[in] handler FIFO handler.
 * @param[in] offset Byte offset from current FIFO tail.
 * @param[out] data Output buffer.
 * @param[in] len Number of bytes to copy.
 */
void EMF_byteFifo_peekAt(const EMF_byteFifo_handler_t* handler,
                         uint32_t offset,
                         uint8_t* data,
                         uint32_t len);

/**
 * @brief Drops bytes from the FIFO without copying them.
 *
 * @param[in,out] handler FIFO handler.
 * @param[in] len Number of bytes to drop.
 */
void EMF_byteFifo_drop(EMF_byteFifo_handler_t* handler, uint32_t len);

/**
 * @brief Flushes the FIFO.
 *
 * @param[in,out] handler FIFO handler.
 */
void EMF_byteFifo_flush(EMF_byteFifo_handler_t* handler);

/**
 * @brief Returns used bytes in FIFO.
 *
 * @param[in] handler FIFO handler.
 * @return Number of used bytes.
 */
uint32_t EMF_byteFifo_getUsed(const EMF_byteFifo_handler_t* handler);

/**
 * @brief Returns free bytes in FIFO.
 *
 * @param[in] handler FIFO handler.
 * @return Number of free bytes.
 */
uint32_t EMF_byteFifo_getFree(const EMF_byteFifo_handler_t* handler);

/**
 * @brief Checks whether FIFO is empty.
 *
 * @param[in] handler FIFO handler.
 * @return true if empty, false otherwise.
 */
bool EMF_byteFifo_isEmpty(const EMF_byteFifo_handler_t* handler);

/**
 * @brief Checks whether FIFO is full.
 *
 * @param[in] handler FIFO handler.
 * @return true if full, false otherwise.
 */
bool EMF_byteFifo_isFull(const EMF_byteFifo_handler_t* handler);

#endif /* EMF_BYTE_FIFO_H */
