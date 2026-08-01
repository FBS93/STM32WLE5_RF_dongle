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
 * Private function declarations
 * -------------------------------------------------------------------------- */

/**
 * @brief Writes a single character to the output.
 *
 * @param[in] c Character to write.
 */
static void writeChar(char c);

/**
 * @brief Writes a null-terminated string to the output.
 *
 * @param[in] str Pointer to a null-terminated string.
 */
static void writeStr(const char* str);

/**
 * @brief Converts an unsigned integer to string and writes it to the output.
 *
 * @param[in] val Unsigned 32-bit integer to write.
 */
static void writeUint(uint32_t val);

/**
 * @brief Converts a signed integer to string and writes it to the output.
 *
 * @param[in] val Signed 32-bit integer to write.
 */
static void writeInt(int32_t val);

/**
 * @brief Converts a floating-point number to string and writes it to the output
 *        with 2 decimal digits.
 *
 * @param[in] fval Float value to write.
 */
static void writeFloat(float fval);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static void writeChar(char c)
{
  while (!EBF_stdoutIsReadyToWrite(1))
  {
    /**
     * @warning This function may block depending on the user stdout
     * implementation. See EBF documentation for details.
     */
  };

  EBF_stdoutWrite((const uint8_t*)&c, 1);
}

static void writeStr(const char* str)
{
  EAF_ASSERT(str != NULL);

  while (*str != '\0')
  {
    while (!EBF_stdoutIsReadyToWrite(1))
    {
      /**
       * @warning This function may block depending on the user stdout
       * implementation. See EBF documentation for details.
       */
    };

    EBF_stdoutWrite((const uint8_t*)str, 1);
    str++;
  }
}

static void writeUint(uint32_t val)
{
  char tmp[UINT32_STR_MAX_LEN];
  int i;

  i = UINT32_STR_MAX_LEN - 1;
  tmp[i] = '\0';

  if (val == 0)
  {
    i--;
    tmp[i] = '0';
  }
  else
  {
    while ((val != 0) && (i > 0))
    {
      i--;
      tmp[i] = (char)('0' + (val % 10));
      val /= 10;
    }
  }

  writeStr(&tmp[i]);
}

static void writeInt(int32_t val)
{
  uint32_t abs_val;

  if (val < 0)
  {
    writeChar('-');
    // Avoid signed overflow for int32_t minimum value.
    abs_val = (uint32_t)(-(val + 1));
    abs_val++;
    writeUint(abs_val);
  }
  else
  {
    writeUint((uint32_t)val);
  }
}

static void writeFloat(float val)
{
  uint32_t int_part;
  uint32_t frac;
  float frac_part;

  if (val < 0)
  {
    writeChar('-');
    val = -val;
  }

  int_part = (uint32_t)val;
  frac_part = val - (float)int_part;

  writeUint(int_part);
  writeChar('.');

  frac_part *= 100.0f;
  frac = (uint32_t)(frac_part + 0.5f);  // round to nearest
  if (frac < 10)
  {  // Add 0 if single digit
    writeChar('0');
  }
  writeUint(frac);
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void EMF_print(const char* fmt, const uintptr_t* args)
{
  int arg_idx;
  union
  {
    float f;
    uint32_t u;
  } conv;

  EAF_ASSERT(fmt != NULL);

  arg_idx = 0;
  while (*fmt != '\0')
  {
    if ((*fmt == '%') && (*(fmt + 1) != '\0'))
    {
      fmt++;
      switch (*fmt)
      {
        case 'u':
        {
          EAF_ASSERT(args != NULL);
          writeUint((uint32_t)args[arg_idx]);
          arg_idx++;
          break;
        }
        case 'i':
        {
          EAF_ASSERT(args != NULL);
          writeInt((int32_t)args[arg_idx]);
          arg_idx++;
          break;
        }
        case 'f':
        {
          EAF_ASSERT(args != NULL);
          conv.u = (uint32_t)args[arg_idx];
          writeFloat(conv.f);
          arg_idx++;
          break;
        }
        case 'c':
        {
          EAF_ASSERT(args != NULL);
          writeChar((char)args[arg_idx]);
          arg_idx++;
          break;
        }
        case 's':
        {
          EAF_ASSERT(args != NULL);
          writeStr((const char*)args[arg_idx]);
          arg_idx++;
          break;
        }
        case '%':
        {
          writeChar('%');
          break;
        }
        default:
        {
          writeChar('?');
          break;
        }
      }
    }
    else
    {
      writeChar(*fmt);
    }
    fmt++;
  }
}
