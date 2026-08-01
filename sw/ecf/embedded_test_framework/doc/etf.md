# Embedded Test Framework (ETF) overview

ETF is a lightweight, platform-agnostic test framework designed to run on both embedded systems and host environments. Its purpose is to provide:
- Conventional test framework structure for C code.
- Single verification macro to validate all test conditions, avoiding multiple specialized checks when properly configured compiler warnings already catch comparison issues.

ECF provides CMake functions that encapsulate test executable creation and registration with CTest. See [ECF cmake functions](../../tools/cmake/functions/ecf_test.cmake)

This framework is a derivative work based on:
- Embedded-Test (c) Quantum Leaps, LLC.

# Glossary

| Term | Definition |
|------|------------|
|   |   |

# Usage example

```c
#include "etf.h"

ETF_TEST_SUITE(example_suite)
{
  ETF_TEST(example_pass)
  {
    ETF_VERIFY(1 == 1);
  }

  ETF_SKIP_TEST(example_skip)
  {
  }
}
```
