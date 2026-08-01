/*******************************************************************************
 * @brief Bitmask utility for managing and querying bit-level flags.
 *
 * This module is priority-based:
 * - Lower bit index means higher priority.
 * - bit0 is the highest priority bit.
 * - Public API bit positions use 1-based indexing (bit0 <-> bit_pos = 1).
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

#ifndef EMF_BITMASK_H
#define EMF_BITMASK_H

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

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Clears all bits in the bitmask.
 *
 * @param[in,out] bitmask Pointer to the bitmask.
 * @param[in] len Length of the bitmask in bytes.
 */
void EMF_bitmask_clearAll(uint8_t* bitmask, uint8_t len);

/**
 * @brief Checks if any bit is set in the bitmask.
 *
 * @param[in] bitmask Pointer to the bitmask.
 * @param[in] len Length of the bitmask in bytes.
 * @return true if at least one bit is set, false otherwise.
 */
bool EMF_bitmask_isAnySet(const uint8_t* bitmask, uint8_t len);

/**
 * @brief Checks if a specific bit is set in the bitmask.
 *
 * @param[in] bitmask Pointer to the bitmask.
 * @param[in] bit_pos Bit position (1-based index).
 * @return true if the bit is set, false otherwise.
 */
bool EMF_bitmask_isBitSet(const uint8_t* bitmask, uint8_t bit_pos);

/**
 * @brief Sets a specific bit in the bitmask.
 *
 * @param[in,out] bitmask Pointer to the bitmask.
 * @param[in] bit_pos Bit position (1-based index).
 */
void EMF_bitmask_setBit(uint8_t* bitmask, uint8_t bit_pos);

/**
 * @brief Clears a specific bit in the bitmask.
 *
 * @param[in,out] bitmask Pointer to the bitmask.
 * @param[in] bit_pos Bit position (1-based index).
 */
void EMF_bitmask_clearBit(uint8_t* bitmask, uint8_t bit_pos);

/**
 * @brief Finds the set bit with the highest priority.
 *
 * @param[in] bitmask Pointer to the bitmask.
 * @param[in] len Length of the bitmask in bytes.
 * @return Position (1-based index) of the highest-priority set bit,
 * or 0 if empty.
 */
uint8_t EMF_bitmask_findMax(const uint8_t* bitmask, uint8_t len);

#endif /* EMF_BITMASK_H */
