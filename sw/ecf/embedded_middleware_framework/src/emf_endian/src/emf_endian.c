/*******************************************************************************
 * @brief Endianness conversion utilities implementation.
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
#include <stddef.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "emf_endian.h"
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

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void EMF_endian_readBuffBE(const uint8_t* buff, void* out, uint8_t out_size)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff != NULL);
  EAF_ASSERT_IN_BLOCK(out != NULL);
  EAF_ASSERT_IN_BLOCK(out_size <= sizeof(uint64_t));
  EAF_ASSERT_BLOCK_END();

  if (out_size == sizeof(uint8_t))
  {
    *(uint8_t*)out = buff[0];
  }
  else if (out_size == sizeof(uint16_t))
  {
    *(uint16_t*)out = (buff[0] << 8) | buff[1];
  }
  else if (out_size == sizeof(uint32_t))
  {
    *(uint32_t*)out =
      (buff[0] << 24) | (buff[1] << 16) | (buff[2] << 8) | buff[3];
  }
  else if (out_size == sizeof(uint64_t))
  {
    *(uint64_t*)out = ((uint64_t)buff[0] << 56) | ((uint64_t)buff[1] << 48) |
                      ((uint64_t)buff[2] << 40) | ((uint64_t)buff[3] << 32) |
                      ((uint64_t)buff[4] << 24) | ((uint64_t)buff[5] << 16) |
                      ((uint64_t)buff[6] << 8) | ((uint64_t)buff[7]);
  }
}

void EMF_endian_writeBuffBE(uint8_t* buff, const void* in, uint8_t in_size)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff != NULL);
  EAF_ASSERT_IN_BLOCK(in != NULL);
  EAF_ASSERT_IN_BLOCK(in_size <= sizeof(uint64_t));
  EAF_ASSERT_BLOCK_END();

  if (in_size == sizeof(uint8_t))
  {
    buff[0] = *(const uint8_t*)in;
  }
  else if (in_size == sizeof(uint16_t))
  {
    buff[0] = (uint8_t)(*(const uint16_t*)in >> 8);
    buff[1] = (uint8_t)(*(const uint16_t*)in & 0xFF);
  }
  else if (in_size == sizeof(uint32_t))
  {
    buff[0] = (uint8_t)(*(const uint32_t*)in >> 24);
    buff[1] = (uint8_t)(*(const uint32_t*)in >> 16);
    buff[2] = (uint8_t)(*(const uint32_t*)in >> 8);
    buff[3] = (uint8_t)(*(const uint32_t*)in & 0xFF);
  }
  else if (in_size == sizeof(uint64_t))
  {
    buff[0] = (uint8_t)(*(const uint64_t*)in >> 56);
    buff[1] = (uint8_t)(*(const uint64_t*)in >> 48);
    buff[2] = (uint8_t)(*(const uint64_t*)in >> 40);
    buff[3] = (uint8_t)(*(const uint64_t*)in >> 32);
    buff[4] = (uint8_t)(*(const uint64_t*)in >> 24);
    buff[5] = (uint8_t)(*(const uint64_t*)in >> 16);
    buff[6] = (uint8_t)(*(const uint64_t*)in >> 8);
    buff[7] = (uint8_t)(*(const uint64_t*)in & 0xFF);
  }
}

void EMF_endian_u16ReadBuffBE(const uint8_t* buff, uint16_t* out)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff != NULL);
  EAF_ASSERT_IN_BLOCK(out != NULL);
  EAF_ASSERT_BLOCK_END();

  *out = (buff[0] << 8) | buff[1];
}

void EMF_endian_u16WriteBuffBE(uint8_t* buff, const uint16_t* in)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff != NULL);
  EAF_ASSERT_IN_BLOCK(in != NULL);
  EAF_ASSERT_BLOCK_END();

  buff[0] = (uint8_t)(*in >> 8);
  buff[1] = (uint8_t)(*in & 0xFF);
}

void EMF_endian_u32ReadBuffBE(const uint8_t* buff, uint32_t* out)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff != NULL);
  EAF_ASSERT_IN_BLOCK(out != NULL);
  EAF_ASSERT_BLOCK_END();

  *out = (buff[0] << 24) | (buff[1] << 16) | (buff[2] << 8) | buff[3];
}

void EMF_endian_u32WriteBuffBE(uint8_t* buff, const uint32_t* in)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff != NULL);
  EAF_ASSERT_IN_BLOCK(in != NULL);
  EAF_ASSERT_BLOCK_END();

  buff[0] = (uint8_t)(*in >> 24);
  buff[1] = (uint8_t)(*in >> 16);
  buff[2] = (uint8_t)(*in >> 8);
  buff[3] = (uint8_t)(*in & 0xFF);
}

void EMF_endian_u64ReadBuffBE(const uint8_t* buff, uint64_t* out)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff != NULL);
  EAF_ASSERT_IN_BLOCK(out != NULL);
  EAF_ASSERT_BLOCK_END();

  *out = ((uint64_t)buff[0] << 56) | ((uint64_t)buff[1] << 48) |
         ((uint64_t)buff[2] << 40) | ((uint64_t)buff[3] << 32) |
         ((uint64_t)buff[4] << 24) | ((uint64_t)buff[5] << 16) |
         ((uint64_t)buff[6] << 8) | ((uint64_t)buff[7]);
}

void EMF_endian_u64WriteBuffBE(uint8_t* buff, const uint64_t* in)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff != NULL);
  EAF_ASSERT_IN_BLOCK(in != NULL);
  EAF_ASSERT_BLOCK_END();

  buff[0] = (uint8_t)(*in >> 56);
  buff[1] = (uint8_t)(*in >> 48);
  buff[2] = (uint8_t)(*in >> 40);
  buff[3] = (uint8_t)(*in >> 32);
  buff[4] = (uint8_t)(*in >> 24);
  buff[5] = (uint8_t)(*in >> 16);
  buff[6] = (uint8_t)(*in >> 8);
  buff[7] = (uint8_t)(*in & 0xFF);
}

void EMF_endian_readBuffLE(const uint8_t* buff, void* out, uint8_t out_size)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff != NULL);
  EAF_ASSERT_IN_BLOCK(out != NULL);
  EAF_ASSERT_IN_BLOCK(out_size <= sizeof(uint64_t));
  EAF_ASSERT_BLOCK_END();

  if (out_size == sizeof(uint8_t))
  {
    *(uint8_t*)out = buff[0];
  }
  else if (out_size == sizeof(uint16_t))
  {
    *(uint16_t*)out = (buff[1] << 8) | buff[0];
  }
  else if (out_size == sizeof(uint32_t))
  {
    *(uint32_t*)out =
      (buff[3] << 24) | (buff[2] << 16) | (buff[1] << 8) | buff[0];
  }
  else if (out_size == sizeof(uint64_t))
  {
    *(uint64_t*)out = ((uint64_t)buff[0]) | ((uint64_t)buff[1] << 8) |
                      ((uint64_t)buff[2] << 16) | ((uint64_t)buff[3] << 24) |
                      ((uint64_t)buff[4] << 32) | ((uint64_t)buff[5] << 40) |
                      ((uint64_t)buff[6] << 48) | ((uint64_t)buff[7] << 56);
  }
}

void EMF_endian_writeBuffLE(uint8_t* buff, const void* in, uint8_t in_size)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff != NULL);
  EAF_ASSERT_IN_BLOCK(in != NULL);
  EAF_ASSERT_IN_BLOCK(in_size <= sizeof(uint64_t));
  EAF_ASSERT_BLOCK_END();

  if (in_size == sizeof(uint8_t))
  {
    buff[0] = *(const uint8_t*)in;
  }
  else if (in_size == sizeof(uint16_t))
  {
    buff[0] = (uint8_t)(*(const uint16_t*)in & 0xFF);
    buff[1] = (uint8_t)(*(const uint16_t*)in >> 8);
  }
  else if (in_size == sizeof(uint32_t))
  {
    buff[0] = (uint8_t)(*(const uint32_t*)in & 0xFF);
    buff[1] = (uint8_t)(*(const uint32_t*)in >> 8);
    buff[2] = (uint8_t)(*(const uint32_t*)in >> 16);
    buff[3] = (uint8_t)(*(const uint32_t*)in >> 24);
  }
  else if (in_size == sizeof(uint64_t))
  {
    buff[0] = (uint8_t)(*(const uint64_t*)in & 0xFF);
    buff[1] = (uint8_t)(*(const uint64_t*)in >> 8);
    buff[2] = (uint8_t)(*(const uint64_t*)in >> 16);
    buff[3] = (uint8_t)(*(const uint64_t*)in >> 24);
    buff[4] = (uint8_t)(*(const uint64_t*)in >> 32);
    buff[5] = (uint8_t)(*(const uint64_t*)in >> 40);
    buff[6] = (uint8_t)(*(const uint64_t*)in >> 48);
    buff[7] = (uint8_t)(*(const uint64_t*)in >> 56);
  }
}

void EMF_endian_u16ReadBuffLE(const uint8_t* buff, uint16_t* out)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff != NULL);
  EAF_ASSERT_IN_BLOCK(out != NULL);
  EAF_ASSERT_BLOCK_END();

  *out = (buff[1] << 8) | buff[0];
}

void EMF_endian_u16WriteBuffLE(uint8_t* buff, const uint16_t* in)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff != NULL);
  EAF_ASSERT_IN_BLOCK(in != NULL);
  EAF_ASSERT_BLOCK_END();

  buff[0] = (uint8_t)(*in & 0xFF);
  buff[1] = (uint8_t)(*in >> 8);
}

void EMF_endian_u32ReadBuffLE(const uint8_t* buff, uint32_t* out)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff != NULL);
  EAF_ASSERT_IN_BLOCK(out != NULL);
  EAF_ASSERT_BLOCK_END();

  *out = (buff[3] << 24) | (buff[2] << 16) | (buff[1] << 8) | buff[0];
}

void EMF_endian_u32WriteBuffLE(uint8_t* buff, const uint32_t* in)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff != NULL);
  EAF_ASSERT_IN_BLOCK(in != NULL);
  EAF_ASSERT_BLOCK_END();

  buff[0] = (uint8_t)(*in & 0xFF);
  buff[1] = (uint8_t)(*in >> 8);
  buff[2] = (uint8_t)(*in >> 16);
  buff[3] = (uint8_t)(*in >> 24);
}

void EMF_endian_u64ReadBuffLE(const uint8_t* buff, uint64_t* out)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff != NULL);
  EAF_ASSERT_IN_BLOCK(out != NULL);
  EAF_ASSERT_BLOCK_END();

  *out = ((uint64_t)buff[0]) | ((uint64_t)buff[1] << 8) |
         ((uint64_t)buff[2] << 16) | ((uint64_t)buff[3] << 24) |
         ((uint64_t)buff[4] << 32) | ((uint64_t)buff[5] << 40) |
         ((uint64_t)buff[6] << 48) | ((uint64_t)buff[7] << 56);
}

void EMF_endian_u64WriteBuffLE(uint8_t* buff, const uint64_t* in)
{
  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(buff != NULL);
  EAF_ASSERT_IN_BLOCK(in != NULL);
  EAF_ASSERT_BLOCK_END();

  buff[0] = (uint8_t)(*in & 0xFF);
  buff[1] = (uint8_t)(*in >> 8);
  buff[2] = (uint8_t)(*in >> 16);
  buff[3] = (uint8_t)(*in >> 24);
  buff[4] = (uint8_t)(*in >> 32);
  buff[5] = (uint8_t)(*in >> 40);
  buff[6] = (uint8_t)(*in >> 48);
  buff[7] = (uint8_t)(*in >> 56);
}
