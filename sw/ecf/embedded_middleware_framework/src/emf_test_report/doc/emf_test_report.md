# EMF test report overview

EMF test report provides the shared text report utilities used by [ETF](../../../../embedded_test_framework/doc/etf.md) and [EDF Test](../../../../event_driven_framework/edf_test/doc/edf_test.md).

# Glossary

| Term | Definition |
|------|------------|
|   |   |

# Usage example

```c
#include "emf_test_report.h"

void example(void)
{
  EMF_testReport_printHeader("Example framework", "example_suite");
  EMF_testReport_printTestIntro(1U, "example_test");
  EMF_testReport_printTestResult(TEST_PASSED, NULL, NULL, 0);
  EMF_testReport_printSummary(1U, 0U, 0U);
}
```
