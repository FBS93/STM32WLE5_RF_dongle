/*******************************************************************************
 * @brief Test report utilities implementation.
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
#include <string.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "emf_test_report.h"
#include "eaf.h"
#include "emf_print.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/**
 * @brief Maximum width of a printed line (in characters).
 */
#define LINE_WIDTH 80U

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

/**
 * @brief Prints a character repeated a specified number of times.
 *
 * @param[in] c Character to print repeatedly.
 * @param[in] cnt Number of times to print the character.
 */
static void printRepeated(char c, uint8_t cnt);

/**
 * @brief Prints a null-terminated string centered on a line, padded with a
 * specified fill character on both sides.
 *
 * @param[in] text Null-terminated string to center.
 * @param[in] fill Character used for padding on both sides.
 */
static void printCenteredLine(const char* text, char fill);

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

static void printRepeated(char c, uint8_t cnt)
{
  for (uint8_t i = 0; i < cnt; i++)
  {
    EMF_PRINT("%c", EMF_PRINT_ARG_C(c));
  }
}

static void printCenteredLine(const char* text, char fill)
{
  size_t len;
  uint8_t pad;

  EAF_ASSERT(text != NULL);

  len = strlen(text) + 2;
  pad = (uint8_t)((LINE_WIDTH - len) / 2);

  EAF_ASSERT(len <= LINE_WIDTH);

  printRepeated(fill, pad);
  EMF_PRINT(" %s ", EMF_PRINT_ARG_S(text));
  printRepeated(fill, (uint8_t)(LINE_WIDTH - pad - len));
  EMF_PRINT("\n");
}

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

void EMF_testReport_printHeader(const char* frameworkName,
                                const char* suiteName)
{
  const char* prefix;
  size_t len;
  uint8_t pad;

  EAF_ASSERT_BLOCK_BEGIN();
  EAF_ASSERT_IN_BLOCK(frameworkName != NULL);
  EAF_ASSERT_IN_BLOCK(suiteName != NULL);
  EAF_ASSERT_BLOCK_END();

  EMF_PRINT("\n");
  printRepeated('*', LINE_WIDTH);
  EMF_PRINT("\n");
  printCenteredLine(frameworkName, '*');
  printRepeated('*', LINE_WIDTH);
  EMF_PRINT("\n");

  prefix = "Test suite: ";
  len = strlen(prefix) + strlen(suiteName) + 2;
  pad = (uint8_t)((LINE_WIDTH - len) / 2);

  EAF_ASSERT(len <= LINE_WIDTH);

  printRepeated('-', pad);
  EMF_PRINT(" %s%s ", EMF_PRINT_ARG_S(prefix), EMF_PRINT_ARG_S(suiteName));
  printRepeated('-', (uint8_t)(LINE_WIDTH - pad - len));
  EMF_PRINT("\n");
}

void EMF_testReport_printTestIntro(uint16_t test_num, const char* test_name)
{
  EAF_ASSERT(test_name != NULL);

  EMF_PRINT("%u", EMF_PRINT_ARG_U(test_num));
  EMF_PRINT("] ");
  EMF_PRINT("%s", EMF_PRINT_ARG_S(test_name));
  EMF_PRINT(" ...");
}

void EMF_testReport_printTestResult(EMF_testReport_testResult_t test_result,
                                    const char* fail_cond,
                                    const char* fail_file,
                                    int fail_line)
{
  switch (test_result)
  {
    case TEST_SKIPPED:
    {
      EMF_PRINT(" SKIPPED\n");
      break;
    }
    case TEST_FAILED:
    {
      EMF_PRINT(" FAILED --> ");
      EMF_PRINT("%s", EMF_PRINT_ARG_S(fail_file));
      EMF_PRINT(":");
      EMF_PRINT("%i", EMF_PRINT_ARG_I(fail_line));
      EMF_PRINT(" --> ");
      EMF_PRINT("VERIFY(");
      EMF_PRINT("%s", EMF_PRINT_ARG_S(fail_cond));
      EMF_PRINT(")\n");
      break;
    }
    case TEST_PASSED:
    {
      EMF_PRINT(" PASSED\n");
      break;
    }
    default:
    {
      EAF_ERROR();
      break;
    }
  }
}

void EMF_testReport_printSummary(uint16_t totalTests,
                                 uint16_t skippedTests,
                                 uint16_t failedTests)
{
  printRepeated('-', LINE_WIDTH);
  EMF_PRINT("\n");
  EMF_PRINT(" Summary: %u test(s), %u skipped, %u failed\n",
            EMF_PRINT_ARG_U(totalTests),
            EMF_PRINT_ARG_U(skippedTests),
            EMF_PRINT_ARG_U(failedTests));
  printRepeated('-', LINE_WIDTH);
  EMF_PRINT("\n\n");
}
