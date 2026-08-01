/*******************************************************************************
 * @brief Generic utility macros and functions.
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

#ifndef EMF_UTILS_H
#define EMF_UTILS_H

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

/**
 * @brief Suppresses compiler warnings for unused parameters.
 *
 * Casts the parameter to void to explicitly mark it as unused.
 *
 * @param[in] param Parameter to suppress warning for.
 */
#define EMF_UTILS_UNUSED_PARAM(param) ((void)(param))

/**
 * @brief Returns the number of elements in a static array.
 *
 * Safe compile-time computation of array length. Only works
 * with actual arrays, not pointers.
 *
 * @param[in] array Array to evaluate.
 * @return Number of elements in the array.
 */
#define EMF_UTILS_SIZEOF_ARRAY(array) (sizeof(array) / sizeof((array)[0U]))

/**
 * @brief Defines a portable, memory-aligned slot.
 *
 * This macro defines a struct containing an array of @c void* large enough
 * to hold a memory slot of the specified @p size.
 *
 * @code
 * // Example: Define a slot for a custom structure
 * EMF_UTILS_MEM_ALIGNED_SLOT(sizeof(myStruct_t)) mySlot;
 *
 * // Example: Array of 10 aligned slots
 * EMF_UTILS_MEM_ALIGNED_SLOT(sizeof(myStruct_t)) myStorage[10];
 * @endcode
 *
 * @param[in] size Requested slot size in bytes.
 */
#define EMF_UTILS_MEM_ALIGNED_SLOT(size)                          \
  struct                                                          \
  {                                                               \
    void* storage[((size) + sizeof(void*) - 1U) / sizeof(void*)]; \
  }

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
 * @brief Clears a memory region by setting all bytes to zero.
 *
 * @param[in,out] start Pointer to the start of the memory region.
 * @param[in] len Number of bytes to clear.
 */
void EMF_utils_clear(void* start, uint32_t len);

#endif /* EMF_UTILS_H */
