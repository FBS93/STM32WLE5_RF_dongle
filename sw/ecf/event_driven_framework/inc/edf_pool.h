/*******************************************************************************
 * @brief Event Driven Framework (EDF) pool.
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

#ifndef EDF_POOL_H
#define EDF_POOL_H

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

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief Pool block size type.
 */
#if (EDF_POOL_BLOCK_SIZE == 1U)
typedef uint8_t EDF_pool_blockSize_t;
#elif (EDF_POOL_BLOCK_SIZE == 2U)
typedef uint16_t EDF_pool_blockSize_t;
#elif (EDF_POOL_BLOCK_SIZE == 4U)
typedef uint32_t EDF_pool_blockSize_t;
#endif

/**
 * @brief Pool block counter type.
 */
#if (EDF_MAX_POOL_BLOCK == 1U)
typedef uint8_t EDF_pool_blockCnt_t;
#elif (EDF_MAX_POOL_BLOCK == 2U)
typedef uint16_t EDF_pool_blockCnt_t;
#elif (EDF_MAX_POOL_BLOCK == 4U)
typedef uint32_t EDF_pool_blockCnt_t;
#endif

/**
 * @brief Pool structure.
 *
 * @note Pool storage is declared as array of void*. Therefore,
 * pointers to pool blocks are manipulated as void**.
 */
typedef struct
{
  void** start;                    /**< Pointer to the first block. */
  void** end;                      /**< Pointer past the last block. */
  void** free_head;                /**< Head of the free block list. */
  EDF_pool_blockSize_t block_size; /**< Size of each block in bytes. */
  EDF_pool_blockCnt_t n_total;     /**< Total number of blocks in the pool. */
  EDF_pool_blockCnt_t n_free;      /**< Number of free blocks available. */
  EDF_pool_blockCnt_t n_min;       /**< Minimum number of free blocks ever
                                        observed. */
} EDF_pool_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes a pool.
 *
 * @note Proper memory alignment of the pool storage is required to ensure safe
 * pointer access. As described in @ref free_list, the first bytes of each block
 * store a pointer to the next free block; if the storage is not properly
 * aligned, this may lead to undefined behavior or hardware faults on some
 * architectures. It is recommended to use @ref EMF_UTILS_MEM_ALIGNED_SLOT or an
 * equivalent alignment-safe definition. This not only guarantees correct
 * pointer alignment, but also ensures that the computed number of blocks
 * matches the ratio of pool_size to block_size as intended.
 *
 * @todo Currently, pools must be initialized in ascending block size order.
 * Future implementations shall allow pools to be initialized in any order.
 *
 * @param[in] pool_storage Pointer to the storage for the pool.
 * @param[in] pool_size Total size of pool_storage in bytes.
 * @param[in] block_size Size of each block of the pool in bytes.
 */
void EDF_pool_init(void* pool_storage,
                   uint_fast32_t pool_size,
                   EDF_pool_blockSize_t block_size);

/**
 * @brief Acquires a free block from a pool.
 *
 * @note The returned pool block contains undefined data.
 *
 * @param[in,out] me Pointer to the pool structure.
 * @return Pointer to the allocated block, or NULL if no free blocks.
 */
void* EDF_pool_acquire(EDF_pool_t* me);

/**
 * @brief Releases a pool block.
 *
 * @param[in,out] me Pointer to the pool structure.
 * @param[in] block Pointer to the block to release.
 */
void EDF_pool_release(EDF_pool_t* me, void* block);

#endif /* EDF_POOL_H */
