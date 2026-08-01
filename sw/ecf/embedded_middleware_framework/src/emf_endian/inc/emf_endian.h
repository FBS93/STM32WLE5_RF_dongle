/*******************************************************************************
 * @brief Endianness conversion utilities.
 *
 * Provides functions to read and write values in big-endian and little-endian
 * formats for multiple data widths.
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

#ifndef EMF_ENDIAN_H
#define EMF_ENDIAN_H

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
 * @brief Reads a big-endian value from a buffer into a generic variable.
 *
 * @param[in] buff Pointer to the input byte buffer.
 * @param[out] out Pointer to the output variable.
 * @param[in] out_size Size (in bytes) of the output variable.
 */
void EMF_endian_readBuffBE(const uint8_t* buff, void* out, uint8_t out_size);

/**
 * @brief Writes a generic variable to a buffer in big-endian format.
 *
 * @param[out] buff Pointer to the output byte buffer.
 * @param[in] in Pointer to the input variable.
 * @param[in] in_size Size (in bytes) of the input variable.
 */
void EMF_endian_writeBuffBE(uint8_t* buff, const void* in, uint8_t in_size);

/**
 * @brief Reads a 16-bit unsigned integer from a buffer in big-endian format.
 *
 * @param[in] buff Pointer to the input byte buffer.
 * @param[out] out Pointer to the output variable.
 */
void EMF_endian_u16ReadBuffBE(const uint8_t* buff, uint16_t* out);

/**
 * @brief Writes a 16-bit unsigned integer to a buffer in big-endian format.
 *
 * @param[out] buff Pointer to the output byte buffer.
 * @param[in] in Pointer to the input variable.
 */
void EMF_endian_u16WriteBuffBE(uint8_t* buff, const uint16_t* in);

/**
 * @brief Reads a 32-bit unsigned integer from a buffer in big-endian format.
 *
 * @param[in] buff Pointer to the input byte buffer.
 * @param[out] out Pointer to the output variable.
 */
void EMF_endian_u32ReadBuffBE(const uint8_t* buff, uint32_t* out);

/**
 * @brief Writes a 32-bit unsigned integer to a buffer in big-endian format.
 *
 * @param[out] buff Pointer to the output byte buffer.
 * @param[in] in Pointer to the input variable.
 */
void EMF_endian_u32WriteBuffBE(uint8_t* buff, const uint32_t* in);

/**
 * @brief Reads a 64-bit unsigned integer from a buffer in big-endian format.
 *
 * @param[in] buff Pointer to the input byte buffer.
 * @param[out] out Pointer to the output variable.
 */
void EMF_endian_u64ReadBuffBE(const uint8_t* buff, uint64_t* out);

/**
 * @brief Writes a 64-bit unsigned integer to a buffer in big-endian format.
 *
 * @param[out] buff Pointer to the output byte buffer.
 * @param[in] in Pointer to the input variable.
 */
void EMF_endian_u64WriteBuffBE(uint8_t* buff, const uint64_t* in);

/**
 * @brief Reads a little-endian value from a buffer into a generic variable.
 *
 * @param[in] buff Pointer to the input byte buffer.
 * @param[out] out Pointer to the output variable.
 * @param[in] out_size Size (in bytes) of the output variable.
 */
void EMF_endian_readBuffLE(const uint8_t* buff, void* out, uint8_t out_size);

/**
 * @brief Writes a generic variable to a buffer in little-endian format.
 *
 * @param[out] buff Pointer to the output byte buffer.
 * @param[in] in Pointer to the input variable.
 * @param[in] in_size Size (in bytes) of the input variable.
 */
void EMF_endian_writeBuffLE(uint8_t* buff, const void* in, uint8_t in_size);

/**
 * @brief Reads a 16-bit unsigned integer from a buffer in little-endian format.
 *
 * @param[in] buff Pointer to the input byte buffer.
 * @param[out] out Pointer to the output variable.
 */
void EMF_endian_u16ReadBuffLE(const uint8_t* buff, uint16_t* out);

/**
 * @brief Writes a 16-bit unsigned integer to a buffer in little-endian format.
 *
 * @param[out] buff Pointer to the output byte buffer.
 * @param[in] in Pointer to the input variable.
 */
void EMF_endian_u16WriteBuffLE(uint8_t* buff, const uint16_t* in);

/**
 * @brief Reads a 32-bit unsigned integer from a buffer in little-endian format.
 *
 * @param[in] buff Pointer to the input byte buffer.
 * @param[out] out Pointer to the output variable.
 */
void EMF_endian_u32ReadBuffLE(const uint8_t* buff, uint32_t* out);

/**
 * @brief Writes a 32-bit unsigned integer to a buffer in little-endian format.
 *
 * @param[out] buff Pointer to the output byte buffer.
 * @param[in] in Pointer to the input variable.
 */
void EMF_endian_u32WriteBuffLE(uint8_t* buff, const uint32_t* in);

/**
 * @brief Reads a 64-bit unsigned integer from a buffer in little-endian format.
 *
 * @param[in] buff Pointer to the input byte buffer.
 * @param[out] out Pointer to the output variable.
 */
void EMF_endian_u64ReadBuffLE(const uint8_t* buff, uint64_t* out);

/**
 * @brief Writes a 64-bit unsigned integer to a buffer in little-endian format.
 *
 * @param[out] buff Pointer to the output byte buffer.
 * @param[in] in Pointer to the input variable.
 */
void EMF_endian_u64WriteBuffLE(uint8_t* buff, const uint64_t* in);

#endif /* EMF_ENDIAN_H */
