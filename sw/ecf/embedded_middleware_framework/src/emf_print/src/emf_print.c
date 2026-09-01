/*******************************************************************************
 * @brief Print utilities implementation.
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
#include <stdbool.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "emf_print.h"
#include "emf_config.h"
#include "ebf.h"
#include "eaf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief Maximum buffer size to hold a 32-bit unsigned integer as a string,
 * including the terminating null character.
 */
#define UINT32_STR_MAX_LEN 11U

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/**
 * @brief Complete formatted message under construction.
 */
typedef struct
{
  uint8_t data[EMF_PRINT_MAX_LEN];  //!< Formatted message bytes.
  uint16_t len;                     //!< Number of formatted message bytes.
  bool overflow;  //!< Indicates that the message exceeded buffer capacity.
} printBuffer_t;

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

/**
 * @brief Appends a single character to a formatted message.
 *
 * @param[in,out] print_buffer Message under construction.
 * @param[in] c Character to append.
 */
static void writeChar(printBuffer_t* print_buffer, char c);

/**
 * @brief Appends a null-terminated string to a formatted message.
 *
 * @param[in,out] print_buffer Message under construction.
 * @param[in] str Pointer to a null-terminated string.
 */
static void writeStr(printBuffer_t* print_buffer, const char* str);

/**
 * @brief Converts an unsigned integer and appends it to a formatted message.
 *
 * @param[in,out] print_buffer Message under construction.
 * @param[in] val Unsigned 32-bit integer to append.
 */
static void writeUint(printBuffer_t* print_buffer, uint32_t val);

/**
 * @brief Converts a signed integer and appends it to a formatted message.
 *
 * @param[in,out] print_buffer Message under construction.
 * @param[in] val Signed 32-bit integer to append.
 */
static void writeInt(printBuffer_t* print_buffer, int32_t val);

/**
 * @brief Converts a float and appends it with two decimal digits.
 *
 * @param[in,out] print_buffer Message under construction.
 * @param[in] fval Float value to append.
 */
static void writeFloat(printBuffer_t* print_buffer, float fval);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static void writeChar(printBuffer_t* print_buffer, char c)
{
  if (print_buffer->len < EMF_PRINT_MAX_LEN)
  {
    print_buffer->data[print_buffer->len] = (uint8_t)c;
    print_buffer->len++;
  }
  else
  {
    print_buffer->overflow = true;
  }
}

static void writeStr(printBuffer_t* print_buffer, const char* str)
{
  EAF_ASSERT(str != NULL);

  while ((*str != '\0') && !print_buffer->overflow)
  {
    writeChar(print_buffer, *str);
    str++;
  }
}

static void writeUint(printBuffer_t* print_buffer, uint32_t val)
{
  char tmp[UINT32_STR_MAX_LEN];
  int i;

  i = UINT32_STR_MAX_LEN - 1;
  tmp[i] = '\0';

  if (val == 0U)
  {
    i--;
    tmp[i] = '0';
  }
  else
  {
    while ((val != 0U) && (i > 0))
    {
      i--;
      tmp[i] = (char)('0' + (val % 10U));
      val /= 10U;
    }
  }

  writeStr(print_buffer, &tmp[i]);
}

static void writeInt(printBuffer_t* print_buffer, int32_t val)
{
  uint32_t abs_val;

  if (val < 0)
  {
    writeChar(print_buffer, '-');
    // Avoid signed overflow for int32_t minimum value.
    abs_val = (uint32_t)(-(val + 1));
    abs_val++;
    writeUint(print_buffer, abs_val);
  }
  else
  {
    writeUint(print_buffer, (uint32_t)val);
  }
}

static void writeFloat(printBuffer_t* print_buffer, float fval)
{
  uint32_t int_part;
  uint32_t frac;
  float frac_part;

  if (fval < 0.0f)
  {
    writeChar(print_buffer, '-');
    fval = -fval;
  }

  int_part = (uint32_t)fval;
  frac_part = fval - (float)int_part;

  writeUint(print_buffer, int_part);
  writeChar(print_buffer, '.');

  frac_part *= 100.0f;
  frac = (uint32_t)(frac_part + 0.5f);
  if (frac < 10U)
  {
    // Add 0 if single digit.
    writeChar(print_buffer, '0');
  }
  writeUint(print_buffer, frac);
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void EMF_print(const char* fmt, const uintptr_t* args)
{
  printBuffer_t print_buffer;
  int arg_idx;
  union
  {
    float f;     //!< Floating-point representation.
    uint32_t u;  //!< Unsigned integer representation.
  } conv;

  EAF_ASSERT(fmt != NULL);

  print_buffer.len = 0U;
  print_buffer.overflow = false;
  arg_idx = 0;
  while ((*fmt != '\0') && !print_buffer.overflow)
  {
    if ((*fmt == '%') && (*(fmt + 1) != '\0'))
    {
      fmt++;
      switch (*fmt)
      {
        case 'u':
        {
          EAF_ASSERT(args != NULL);
          writeUint(&print_buffer, (uint32_t)args[arg_idx]);
          arg_idx++;
          break;
        }
        case 'i':
        {
          EAF_ASSERT(args != NULL);
          writeInt(&print_buffer, (int32_t)args[arg_idx]);
          arg_idx++;
          break;
        }
        case 'f':
        {
          EAF_ASSERT(args != NULL);
          conv.u = (uint32_t)args[arg_idx];
          writeFloat(&print_buffer, conv.f);
          arg_idx++;
          break;
        }
        case 'c':
        {
          EAF_ASSERT(args != NULL);
          writeChar(&print_buffer, (char)args[arg_idx]);
          arg_idx++;
          break;
        }
        case 's':
        {
          EAF_ASSERT(args != NULL);
          writeStr(&print_buffer, (const char*)args[arg_idx]);
          arg_idx++;
          break;
        }
        case '%':
        {
          writeChar(&print_buffer, '%');
          break;
        }
        default:
        {
          writeChar(&print_buffer, '?');
          break;
        }
      }
    }
    else
    {
      writeChar(&print_buffer, *fmt);
    }
    fmt++;
  }

  EAF_ASSERT(!print_buffer.overflow);

  if ((print_buffer.len > 0U) &&
      !EBF_stdoutWrite(print_buffer.data, print_buffer.len))
  {
    EAF_ERROR();  // Stdout failed to write the complete message.
  }
}
