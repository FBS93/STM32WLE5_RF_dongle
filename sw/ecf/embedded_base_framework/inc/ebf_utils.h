/*******************************************************************************
 * @brief Base utilities.
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

#ifndef EBF_UTILS_H
#define EBF_UTILS_H

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
 *  @brief Returns the position of the most significant set bit in a byte.
 *
 * Returns the 1-based index of the most significant bit set in
 * the input byte. If @p byte is fully clear, the function returns 0.
 *
 * The default weak implementation is software-only (lookup table + shifts).
 * On cores/ports that provide a hardware-specific instruction (e.g., CLZ in
 * ARM), this function can be overridden for improved performance.
 *
 * @param[in] byte input byte.
 * @return Position (1–8) of highest set bit, or 0 if no bits are set.
 */
uint8_t EBF_utils_log2(uint8_t byte);

#endif /* EBF_UTILS_H */
