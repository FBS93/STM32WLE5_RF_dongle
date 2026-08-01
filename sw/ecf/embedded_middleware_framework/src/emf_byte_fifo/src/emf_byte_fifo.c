/*******************************************************************************
 * @brief Circular FIFO for byte streams implementation.
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
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "emf_byte_fifo.h"
#include "eaf.h"

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
 * @brief Advances an index with wrap-around.
 *
 * @param[in] handler FIFO handler.
 * @param[in] idx Base index.
 * @param[in] offset Offset to add.
 * @return Wrapped index.
 */
static uint32_t advanceIndex(const EMF_byteFifo_handler_t* handler,
                             uint32_t idx,
                             uint32_t offset);

/**
 * @brief Reads bytes from FIFO storage into destination buffer.
 *
 * @param[in] handler FIFO handler.
 * @param[in] start_index Read start index in storage.
 * @param[out] data Destination buffer.
 * @param[in] len Number of bytes to read.
 */
static void readBytes(const EMF_byteFifo_handler_t* handler,
                      uint32_t start_index,
                      uint8_t* data,
                      uint32_t len);

/**
 * @brief Writes bytes into FIFO storage from source buffer.
 *
 * @param[in,out] handler FIFO handler.
 * @param[in] start_index Write start index in storage.
 * @param[in] data Source buffer.
 * @param[in] len Number of bytes to write.
 */
static void writeBytes(EMF_byteFifo_handler_t* handler,
                       uint32_t start_index,
                       const uint8_t* data,
                       uint32_t len);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static uint32_t advanceIndex(const EMF_byteFifo_handler_t* handler,
                             uint32_t idx,
                             uint32_t offset)
{
  EAF_ASSERT(handler != NULL);
  EAF_ASSERT(handler->size > 0U);

  idx += offset;
  if (idx >= handler->size)
  {
    idx %= handler->size;
  }

  return idx;
}

static void readBytes(const EMF_byteFifo_handler_t* handler,
                      uint32_t start_index,
                      uint8_t* data,
                      uint32_t len)
{
  uint32_t idx;

  EAF_ASSERT(handler != NULL);
  EAF_ASSERT((data != NULL) || (len == 0U));

  idx = start_index;
  for (uint32_t i = 0U; i < len; ++i)
  {
    data[i] = handler->storage[idx];
    idx = advanceIndex(handler, idx, 1U);
  }
}

static void writeBytes(EMF_byteFifo_handler_t* handler,
                       uint32_t start_index,
                       const uint8_t* data,
                       uint32_t len)
{
  uint32_t idx;

  EAF_ASSERT(handler != NULL);
  EAF_ASSERT((data != NULL) || (len == 0U));

  idx = start_index;
  for (uint32_t i = 0U; i < len; ++i)
  {
    handler->storage[idx] = data[i];
    idx = advanceIndex(handler, idx, 1U);
  }
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void EMF_byteFifo_init(EMF_byteFifo_handler_t* handler,
                       uint32_t size,
                       uint8_t* storage)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(handler != NULL);
  EAF_ASSERT_IN_BLOCK(size > 0U);
  EAF_ASSERT_IN_BLOCK(storage != NULL);
  EAF_ASSERT_BLOCK_END();

  handler->storage = storage;
  handler->size = size;
  handler->head = 0U;
  handler->tail = 0U;
  handler->used = 0U;
}

void EMF_byteFifo_push(EMF_byteFifo_handler_t* handler,
                       const uint8_t* data,
                       uint32_t len)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(handler != NULL);
  EAF_ASSERT_IN_BLOCK((data != NULL) || (len == 0U));
  EAF_ASSERT_IN_BLOCK(len <= EMF_byteFifo_getFree(handler));
  EAF_ASSERT_BLOCK_END();

  writeBytes(handler, handler->head, data, len);
  handler->head = advanceIndex(handler, handler->head, len);
  handler->used += len;
}

void EMF_byteFifo_pop(EMF_byteFifo_handler_t* handler,
                      uint8_t* data,
                      uint32_t len)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(handler != NULL);
  EAF_ASSERT_IN_BLOCK((data != NULL) || (len == 0U));
  EAF_ASSERT_IN_BLOCK(len <= EMF_byteFifo_getUsed(handler));
  EAF_ASSERT_BLOCK_END();

  readBytes(handler, handler->tail, data, len);
  handler->tail = advanceIndex(handler, handler->tail, len);
  handler->used -= len;
}

void EMF_byteFifo_peek(const EMF_byteFifo_handler_t* handler,
                       uint8_t* data,
                       uint32_t len)
{
  EMF_byteFifo_peekAt(handler, 0U, data, len);
}

void EMF_byteFifo_peekAt(const EMF_byteFifo_handler_t* handler,
                         uint32_t offset,
                         uint8_t* data,
                         uint32_t len)
{
  uint32_t start_index;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(handler != NULL);
  EAF_ASSERT_IN_BLOCK((data != NULL) || (len == 0U));
  EAF_ASSERT_IN_BLOCK(offset <= EMF_byteFifo_getUsed(handler));
  EAF_ASSERT_IN_BLOCK(len <= (EMF_byteFifo_getUsed(handler) - offset));
  EAF_ASSERT_BLOCK_END();

  start_index = advanceIndex(handler, handler->tail, offset);
  readBytes(handler, start_index, data, len);
}

void EMF_byteFifo_drop(EMF_byteFifo_handler_t* handler, uint32_t len)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(handler != NULL);
  EAF_ASSERT_IN_BLOCK(len <= EMF_byteFifo_getUsed(handler));
  EAF_ASSERT_BLOCK_END();

  handler->tail = advanceIndex(handler, handler->tail, len);
  handler->used -= len;
}

void EMF_byteFifo_flush(EMF_byteFifo_handler_t* handler)
{
  EAF_ASSERT(handler != NULL);

  handler->head = 0U;
  handler->tail = 0U;
  handler->used = 0U;
}

uint32_t EMF_byteFifo_getUsed(const EMF_byteFifo_handler_t* handler)
{
  EAF_ASSERT(handler != NULL);
  return handler->used;
}

uint32_t EMF_byteFifo_getFree(const EMF_byteFifo_handler_t* handler)
{
  EAF_ASSERT(handler != NULL);
  return (uint32_t)(handler->size - handler->used);
}

bool EMF_byteFifo_isEmpty(const EMF_byteFifo_handler_t* handler)
{
  EAF_ASSERT(handler != NULL);
  return (handler->used == 0U);
}

bool EMF_byteFifo_isFull(const EMF_byteFifo_handler_t* handler)
{
  EAF_ASSERT(handler != NULL);
  return (handler->used == handler->size);
}
