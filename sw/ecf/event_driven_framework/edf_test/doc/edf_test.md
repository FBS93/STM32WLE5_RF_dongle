# Event Driven Framework Test (EDF Test) overview

EDF Test is a lightweight, platform-agnostic testing framework designed to run on both embedded systems and host environments, specifically tailored for validating event-driven systems built on top of [EDF](../../doc/edf.md). It enables deterministic and non-intrusive validation of applications based on inversion of control, where the framework owns the main execution loop and never returns control.

Conventional testing approaches are not suitable for the inversion of control execution model, as the execution flow is not based on sequential call-and-return semantics and cannot be tested using standard unit test runners. Existing alternatives typically rely on extensive logging and external analysis tools, which introduce significant maintenance overhead and pollute the source code with test-only instrumentation. EDF Test addresses this limitation by integrating testing directly into the [EDF](../../doc/edf.md) execution model.

EDF Test introduces a dedicated Active Object (AO) with the lowest system priority, reserved for testing purposes. The user is expected to reserve the lowest available AO priority when enabling this testing mode. This test AO is subscribed by default to all system events and is guaranteed to be the last AO to process them. As a result, minimal intrusiveness is ensured, since event processing by application AOs is completed first (guaranteed by [EDF](../../doc/edf.md) priority-based scheduling) and the test logic is executed only after all functional behavior has finished.

Each test case is composed of two phases:
- Init: The test prepares the system with the desired initial conditions and then stimulates it by publishing the event that triggers the behavior under test.
- Verify: Once all other AOs have processed the init event and any subsequent events derived from it, the test AO is invoked, with the verification phase bound exclusively to a specific event and executes the validation logic using the built-in macros. After verification, the framework automatically proceeds to the init phase of the next test case.

The following diagram illustrates the flow of a simple test case.

```mermaid
sequenceDiagram
  participant EDF_PUB_SUB as EDF Publish/Subscribe mechanism
  participant AO1 as Application AO
  participant AO2 as Application AO
  participant EDF_TEST_AO as EDF Test AO

  Note over EDF_TEST_AO: Test case start
  EDF_TEST_AO->>EDF_TEST_AO: Init phase
  EDF_TEST_AO->>EDF_PUB_SUB: Publish new event

  EDF_PUB_SUB-->>AO1: Event delivered
  Note over AO1: Processed first (highest priority)
  AO1->>AO1: Process event

  EDF_PUB_SUB-->>AO2: Event delivered
  Note over AO2: Processed after
  AO2->>AO2: Process event

  EDF_PUB_SUB-->>EDF_TEST_AO: Event delivered
  Note over EDF_TEST_AO: Processed last (lowest priority)
  EDF_TEST_AO->>EDF_TEST_AO: Verify phase

  Note over EDF_TEST_AO: Test case end
```

Additional events may be generated and processed before the event bound to the verify phase reaches the Test AO. Regardless of intermediate event processing, the verify phase executes only after all higher-priority AOs have processed that event and any events derived from it.

This approach enables fully automated, in-system testing within a single self-contained executable, without modifying application logic, relying on external tools, or introducing intrusive logging, while preserving real execution behavior and timing characteristics.

ECF provides CMake functions that encapsulate test executable creation and registration with CTest. See [ECF cmake functions](../../../tools/cmake/functions/ecf_test.cmake)

# Glossary

| Term | Definition |
|------|------------|
|   |   |

# Usage example

```c
#include "edf_test.h"

EDF_TEST_SUITE_NAME(example_suite);
EDF_TEST_EVENT_BUFF(8U);

EDF_TEST_CASE(example_test, MY_SIGNAL);
EDF_TEST_CASE_INIT(example_test)
{
  /* Prepare the system and publish the trigger event. */
}
EDF_TEST_CASE_VERIFY(example_test, e)
{
  EDF_TEST_VERIFY(e->sig == MY_SIGNAL);
}

EDF_TEST_SUITE(
  EDF_TEST_RUN(example_test)
)
```
