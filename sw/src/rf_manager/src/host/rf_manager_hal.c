/*******************************************************************************
 * @brief Minimal host implementation of the RF manager HAL.
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
#include <stddef.h>
#include <stdint.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */
#include "eaf.h"

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "rf_manager_hal.h"

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/** @brief Define static file name string for asserts. */
EAF_DEFINE_THIS_FILE(__FILE__);

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void rfManagerHal_init(const rfManagerHal_callbacks_t* registered_callbacks,
                       void* context)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(registered_callbacks != NULL);
  EAF_ASSERT_IN_BLOCK(registered_callbacks->tx_done != NULL);
  EAF_ASSERT_IN_BLOCK(registered_callbacks->tx_timeout != NULL);
  EAF_ASSERT_IN_BLOCK(registered_callbacks->rx_done != NULL);
  EAF_ASSERT_IN_BLOCK(registered_callbacks->rx_timeout != NULL);
  EAF_ASSERT_IN_BLOCK(registered_callbacks->rx_error != NULL);
  EAF_ASSERT_IN_BLOCK(registered_callbacks->cad_done != NULL);
  EAF_ASSERT_IN_BLOCK(context != NULL);
  EAF_ASSERT_BLOCK_END();
}

void rfManagerHal_setConfiguration(const rfTypes_configuration_t* configuration)
{
  EAF_ASSERT(configuration != NULL);
}

void rfManagerHal_calibrate(uint8_t lower_frequency, uint8_t upper_frequency)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(lower_frequency >= 0x26U);
  EAF_ASSERT_IN_BLOCK(upper_frequency <= 0xF0U);
  EAF_ASSERT_IN_BLOCK(lower_frequency <= upper_frequency);
  EAF_ASSERT_BLOCK_END();
}

void rfManagerHal_transmit(const rfTypes_configuration_t* configuration,
                           const uint8_t* data,
                           uint8_t length)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(configuration != NULL);
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_BLOCK_END();

  (void)length;
}

void rfManagerHal_startReceive(const rfTypes_configuration_t* configuration)
{
  EAF_ASSERT(configuration != NULL);
}

void rfManagerHal_startCad(const rfTypes_configuration_t* configuration)
{
  EAF_ASSERT(configuration != NULL);
}

void rfManagerHal_startContinuousWave(void)
{
}

void rfManagerHal_startContinuousPreamble(void)
{
}

void rfManagerHal_stop(void)
{
}

uint8_t rfManagerHal_readReceivedPacket(
  uint8_t* data,
  uint8_t capacity,
  const rfTypes_configuration_t* configuration,
  rfTypes_packetStatus_t* packet_status)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(data != NULL);
  EAF_ASSERT_IN_BLOCK(configuration != NULL);
  EAF_ASSERT_IN_BLOCK(packet_status != NULL);
  EAF_ASSERT_BLOCK_END();

  (void)capacity;
  *packet_status = (rfTypes_packetStatus_t){0};

  return 0U;
}

uint16_t rfManagerHal_getErrors(void)
{
  return 0U;
}
