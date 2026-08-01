/*******************************************************************************
 * @brief Event Driven Framework (EDF) pool implementation.
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
#include "edf_pool.h"
#include "eaf.h"
#include "edf_active_object.h"
#include "edf_core.h"
#include "edf_framework.h"
#include "ebf.h"

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

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void EDF_pool_init(void* pool_storage,
                   uint_fast32_t pool_size,
                   EDF_pool_blockSize_t block_size)
{
  uint8_t pool_num;
  EDF_pool_t* new_pool;
  uint_fast16_t next_block_idx;
  void** free_block;
  uint32_t n_total;

  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(pool_storage != NULL);
  EAF_ASSERT_IN_CRITICAL_SECTION(EDF_framework.pool_registry.max_pool <
                                 EDF_MAX_POOL);
  // Minimum block_size. See @ref free_list.
  EAF_ASSERT_IN_CRITICAL_SECTION(block_size >= sizeof(void*));
  EAF_ASSERT_IN_CRITICAL_SECTION(pool_size >= block_size);

  // Ensure pools are initialized in ascending block size order.
  pool_num = EDF_framework.pool_registry.max_pool;
  if (pool_num > 0U)
  {
    EAF_ASSERT_IN_CRITICAL_SECTION(
      EDF_framework.pool_registry.list[pool_num - 1U].block_size < block_size);
  }
  EDF_framework.pool_registry.max_pool = pool_num + 1U;  // One more pool.

  // Set new pool pointer to be initialized.
  new_pool = &EDF_framework.pool_registry.list[pool_num];

  // Init free list head block at pool start.
  new_pool->free_head = (void**)pool_storage;

  // Round up requested block_size to multiples of sizeof(void*).
  new_pool->block_size = 0U;
  next_block_idx = 0U;
  while (new_pool->block_size < block_size)
  {
    new_pool->block_size += (EDF_pool_blockSize_t)sizeof(void*);
    next_block_idx++;
  }

  // The pool buffer must fit at least one rounded-up block.
  EAF_ASSERT_IN_CRITICAL_SECTION(pool_size >= new_pool->block_size);

  // Start at the head of the free list.
  free_block = new_pool->free_head;  // Pointer to the head free block.
  n_total = 1U;                      // Head free block already in the list.
  // Chain all blocks together in the free list.
  for (uint_fast32_t size = pool_size - new_pool->block_size;
       size >= (uint_fast32_t)new_pool->block_size;
       size -= (uint_fast32_t)new_pool->block_size)
  {
    // Set the next link to next free block.
    free_block[0] = (void*)&free_block[next_block_idx];

    // Advance to the next block.
    free_block = (void**)free_block[0];

    // One more free block in the pool.
    ++n_total;
  }
  free_block[0] = NULL;  // The last link points to NULL.

  /**
   * Dynamic range check.
   * Only needed when EDF_pool_blockCnt_t is 8 or 16 bits;
   * for 32-bit, n_total always fits.
   */
#if (EDF_MAX_POOL_BLOCK == 1U)
  EAF_ASSERT_IN_CRITICAL_SECTION(n_total < 0xFFU);
#elif (EDF_MAX_POOL_BLOCK == 2U)
  EAF_ASSERT_IN_CRITICAL_SECTION(n_total < 0xFFFFU);
#endif

  // Set pool init values
  new_pool->n_total = (EDF_pool_blockCnt_t)n_total;
  new_pool->n_free = new_pool->n_total;
  new_pool->start = (void**)pool_storage;
  new_pool->end = free_block;
  new_pool->n_min = new_pool->n_total;

  EBF_CRITICAL_SECTION_EXIT();
}

void* EDF_pool_acquire(EDF_pool_t* me)
{
  void** free_block;
  void** next_free_block;

  EBF_CRITICAL_SECTION_ENTRY();

  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);

  free_block = me->free_head;

  // Is there at least one free block?
  if (me->n_free > 0)
  {
    next_free_block = (void**)free_block[0];
    me->n_free--;

    // No more free blocks?
    if (me->n_free == 0U)
    {
      // Pool is becoming empty, so the next free block must be NULL.
      EAF_ASSERT_IN_CRITICAL_SECTION(next_free_block == NULL);
      me->n_min = 0U;  // Remember that the pool got empty.
    }
    else
    {
      // The pool is not empty so the next free-block pointer must be in range.
      EAF_ASSERT_IN_CRITICAL_SECTION((me->start <= next_free_block) &&
                                     (next_free_block <= me->end));

      // Is this the new minimum?
      if (me->n_min > me->n_free)
      {
        me->n_min = me->n_free;  // Remember the minimum so far.
      }
    }

    // Set the head list block to the next free block.
    me->free_head = next_free_block;

    // Mark acquired block as allocated; break the free list link.
    free_block[0] = NULL;
  }
  else
  {
    // Don't have enough free blocks at this point.
    free_block = NULL;
  }

  EBF_CRITICAL_SECTION_EXIT();

  return (void*)free_block;
}

void EDF_pool_release(EDF_pool_t* me, void* block)
{
  void** free_block;

  EBF_CRITICAL_SECTION_ENTRY();

  free_block = (void**)block;  // Pointer to released block.

  EAF_ASSERT_IN_CRITICAL_SECTION(me != NULL);
  EAF_ASSERT_IN_CRITICAL_SECTION(me->n_free < me->n_total);
  EAF_ASSERT_IN_CRITICAL_SECTION((me->start <= free_block) &&
                                 (free_block <= me->end));

  free_block[0] = (void*)me->free_head;  // Link into the free list.
  me->free_head = free_block;            // Set as new head of the free list.
  me->n_free++;                          // One more free block in this pool.

  EBF_CRITICAL_SECTION_EXIT();
}

/**
 * @anchor free_list
 * @par Free list
 *
 * This note explains how the free list is managed.
 * When a block is free, its first sizeof(void*) bytes are used to store
 * the pointer to the next free block, forming a linked list.
 * When the block is acquired, this space is returned to the user,
 * since the block is no longer part of the free list.
 * Upon release, the same first bytes are reused to relink the block
 * back into the free list.
 *
 * @code{.unparsed}
 * me->free_block
 *       |
 *       v
 * +--------------------+     +--------------------+     +--------------------+
 * | block1[0] = block2 | --> | block2[0] = block3 | --> | block3[0] = NULL   |
 * +--------------------+     +--------------------+     +--------------------+
 * @endcode
 */
