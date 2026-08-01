/*******************************************************************************
 * @brief Consistent Overhead Byte Stuffing utilities.
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

#ifndef EMF_COBS_H
#define EMF_COBS_H

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

/**
 * @brief Special reserved value used as packet delimiter in COBS encoding.
 *
 * @note Must remain 0x00. Changing this requires reviewing all encode/decode
 * logic.
 */
#define EMF_COBS_PACKET_DELIMITER 0x00U

/**
 * @brief Size in bytes of the COBS packet delimiter.
 */
#define EMF_COBS_PACKET_DELIMITER_SIZE 1U

/**
 * @brief Maximum number of non-EMF_COBS_PACKET_DELIMITER bytes per COBS block.
 *        The next byte after this is used to store the code.
 */
#define EMF_COBS_BLOCK_MAX 0xFEU

/**
 * @brief Computes the COBS encoding overhead for a given input size.
 */
#define EMF_COBS_OVERHEAD(size) ((size) / EMF_COBS_BLOCK_MAX + 1U)

/**
 * @brief Computes the worst-case size of the encoded buffer for a given input
 * size.
 */
#define EMF_COBS_ENCODED_SIZE(size) \
  ((size) + EMF_COBS_OVERHEAD(size) + EMF_COBS_PACKET_DELIMITER_SIZE)

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
 * @brief COBS encode data to buffer
 *
 * @note The output includes a trailing EMF_COBS_PACKET_DELIMITER.
 *
 * @note The input and output buffers must not overlap.
 *
 * @param[in] buff_in Pointer to input data to encode
 * @param[in] len_in Number of bytes to encode
 * @param[out] buff_out Pointer to encoded output buffer. The output buffer must
 * be at least len_in + (len_in / EMF_COBS_BLOCK_MAX) + 2 bytes to accommodate
 * worst-case COBS expansion.
 * @param[out] len_out Pointer to a variable that will be set to the encoded
 * length.
 */
void EMF_cobs_encode(const uint8_t* buff_in,
                     uint16_t len_in,
                     uint8_t* buff_out,
                     uint16_t* len_out);

/**
 * @brief  Decodes a COBS‑encoded buffer.
 *
 * @note Decoding stops when a EMF_COBS_PACKET_DELIMITER is encountered,
 * or when len_in bytes have been processed.
 *
 * @note The minimum size required for the buff_out size is: len_in - 2 bytes.
 * This is because, in the best case, COBS encoding only adds one byte for the
 * length code and one byte for the final EMF_COBS_PACKET_DELIMITER.
 *
 * @note The input and output buffers must not overlap.
 *
 * @param[in] buff_in Pointer to the COBS‑encoded input buffer.
 * @param[in] len_in Length of the input buffer, in bytes.
 * @param[out] buff_out Pointer to the buffer to receive decoded data.
 * @param[out] len_out Pointer to a variable that will be set to the decoded
 * length.
 * @return true if a COBS packet delimiter (EMF_COBS_PACKET_DELIMITER) was
 * found, so the output is valid. false otherwise.
 */
bool EMF_cobs_decode(const uint8_t* buff_in,
                     uint16_t len_in,
                     uint8_t* buff_out,
                     uint16_t* len_out);

#endif /* EMF_COBS_H */
