/*******************************************************************************
 * @brief Dummy module API used to validate EFF mock generation.
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

#ifndef EFF_DUMMY_H
#define EFF_DUMMY_H

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
 * @brief Stores a sample buffer into an output location.
 *
 * @param[in] input_buffer Pointer to the input sample buffer.
 * @param[out] output_value Pointer to the output storage location.
 */
void effDummy_storeSample(const uint8_t* input_buffer, uint16_t* output_value);

/**
 * @brief Measures a value using a sample counter and sample window.
 *
 * @param[in,out] sample_count Pointer to the sample counter.
 * @param[in] sample_window Pointer to the input sample window.
 * @return Measured value.
 */
uint8_t effDummy_measure(uint8_t* sample_count, const uint8_t* sample_window);

/**
 * @brief Selects a candidate buffer using a reference buffer.
 *
 * @param[in] candidate_buffers Pointer to the candidate buffer array.
 * @param[in] reference_buffer Pointer to the reference buffer.
 * @return Pointer to the selected candidate buffer.
 */
uint16_t* effDummy_selectBuffer(uint16_t* candidate_buffers,
                                const uint16_t* reference_buffer);

/**
 * @brief Registers a callback used by the dummy module.
 *
 * @param[in] callback Callback function to register.
 */
void effDummy_registerCallback(void (*callback)(uint8_t status,
                                                const uint8_t* payload));

#endif /* EFF_DUMMY_H */
