/*******************************************************************************
 * @brief Circular FIFO for byte array storage implementation.
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
#include "emf_array_fifo.h"
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
static uint8_t advanceIndex(const EMF_arrayFifo_handler_t* handler,
                            uint8_t idx,
                            uint8_t offset);

/**
 * @brief Reads one slot from FIFO storage into destination buffer.
 *
 * @param[in] handler FIFO handler.
 * @param[in] slot_idx Slot index in storage.
 * @param[out] data Destination buffer.
 */
static void readSlot(const EMF_arrayFifo_handler_t* handler,
                     uint8_t slot_idx,
                     uint8_t* data);

/**
 * @brief Writes one slot into FIFO storage from source buffer.
 *
 * @param[in,out] handler FIFO handler.
 * @param[in] slot_idx Slot index in storage.
 * @param[in] data Source buffer.
 */
static void writeSlot(EMF_arrayFifo_handler_t* handler,
                      uint8_t slot_idx,
                      const uint8_t* data);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static uint8_t advanceIndex(const EMF_arrayFifo_handler_t* handler,
                            uint8_t idx,
                            uint8_t offset)
{
  uint16_t next_idx;

  EAF_ASSERT(handler != NULL);
  EAF_ASSERT(handler->n_slots > 0U);

  next_idx = (uint16_t)idx + (uint16_t)offset;
  if (next_idx >= handler->n_slots)
  {
    next_idx = (uint16_t)(next_idx % handler->n_slots);
  }

  return (uint8_t)next_idx;
}

static void readSlot(const EMF_arrayFifo_handler_t* handler,
                     uint8_t slot_idx,
                     uint8_t* data)
{
  const uint8_t* slot;
  uint32_t offset;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(handler != NULL);
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(slot_idx < handler->n_slots);
  EAF_ASSERT_BLOCK_END();

  offset = slot_idx * handler->slot_size;
  slot = &handler->storage[offset];
  for (uint16_t i = 0U; i < handler->slot_size; ++i)
  {
    data[i] = slot[i];
  }
}

static void writeSlot(EMF_arrayFifo_handler_t* handler,
                      uint8_t slot_idx,
                      const uint8_t* data)
{
  uint8_t* slot;
  uint32_t offset;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(handler != NULL);
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(slot_idx < handler->n_slots);
  EAF_ASSERT_BLOCK_END();

  offset = slot_idx * handler->slot_size;
  slot = &handler->storage[offset];
  for (uint16_t i = 0U; i < handler->slot_size; ++i)
  {
    slot[i] = data[i];
  }
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void EMF_arrayFifo_init(EMF_arrayFifo_handler_t* handler,
                        uint8_t n_slots,
                        uint16_t slot_size,
                        uint8_t* storage)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(handler != NULL);
  EAF_ASSERT_IN_BLOCK(n_slots > 0);
  EAF_ASSERT_IN_BLOCK(slot_size > 0);
  EAF_ASSERT_IN_BLOCK(storage != NULL);
  EAF_ASSERT_BLOCK_END();

  handler->n_slots = n_slots;
  handler->slot_size = slot_size;
  handler->storage = storage;
  handler->head = 0;
  handler->tail = 0;
  handler->full = false;
}

void EMF_arrayFifo_push(EMF_arrayFifo_handler_t* handler, const uint8_t* data)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(handler != NULL);
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(!EMF_arrayFifo_isFull(handler));
  EAF_ASSERT_BLOCK_END();

  writeSlot(handler, handler->head, data);
  handler->head = advanceIndex(handler, handler->head, 1U);

  if (handler->head == handler->tail)
  {
    handler->full = true;
  }
}

void EMF_arrayFifo_pop(EMF_arrayFifo_handler_t* handler, uint8_t* data)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(handler != NULL);
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(!EMF_arrayFifo_isEmpty(handler));
  EAF_ASSERT_BLOCK_END();

  readSlot(handler, handler->tail, data);
  handler->tail = advanceIndex(handler, handler->tail, 1U);
  handler->full = false;
}

void EMF_arrayFifo_peek(const EMF_arrayFifo_handler_t* handler, uint8_t* data)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(handler != NULL);
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(!EMF_arrayFifo_isEmpty(handler));
  EAF_ASSERT_BLOCK_END();

  readSlot(handler, handler->tail, data);
}

void EMF_arrayFifo_drop(EMF_arrayFifo_handler_t* handler)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(handler != NULL);
  EAF_ASSERT_IN_BLOCK(!EMF_arrayFifo_isEmpty(handler));
  EAF_ASSERT_BLOCK_END();

  handler->tail = advanceIndex(handler, handler->tail, 1U);
  handler->full = false;
}

void EMF_arrayFifo_flush(EMF_arrayFifo_handler_t* handler)
{
  EAF_ASSERT(handler != NULL);

  handler->head = handler->tail = 0;
  handler->full = false;
}

bool EMF_arrayFifo_isEmpty(const EMF_arrayFifo_handler_t* handler)
{
  EAF_ASSERT(handler != NULL);

  return (!handler->full && (handler->head == handler->tail));
}

bool EMF_arrayFifo_isFull(const EMF_arrayFifo_handler_t* handler)
{
  EAF_ASSERT(handler != NULL);

  return handler->full;
}

uint8_t EMF_arrayFifo_getUsed(const EMF_arrayFifo_handler_t* handler)
{
  uint8_t used;

  EAF_ASSERT(handler != NULL);

  if (handler->full)
  {
    used = handler->n_slots;
  }
  else if (handler->head >= handler->tail)
  {
    used = (uint8_t)(handler->head - handler->tail);
  }
  else
  {
    used = (uint8_t)(handler->n_slots - (handler->tail - handler->head));
  }

  return used;
}

uint8_t EMF_arrayFifo_getFree(const EMF_arrayFifo_handler_t* handler)
{
  EAF_ASSERT(handler != NULL);

  return (uint8_t)(handler->n_slots - EMF_arrayFifo_getUsed(handler));
}
