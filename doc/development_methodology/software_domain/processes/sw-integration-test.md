# sw-integration-test

## Purpose

Define and implement software integration tests against software architecture to validate that the software detailed design and software implementation correctly realizes it.

## Input work products

- Software architecture
- Software detailed design
- Software implementation
- Tools

## Output work products

- Software integration test

## Steps

1. Review the software architecture.
2. Review available tools required to implement and execute software integration tests.
3. Identify software architecture elements requiring validation.
4. Define and implement the software integration tests.
5. Define traceability between software integration tests and the corresponding software architecture elements.
6. Check the software integration tests for completeness, consistency and correctness.

## Guidelines

### Software integration test work product

The software integration test work product shall follow the [Software test definition](../resources/software_test_definition.md).

In addition to the [Software test definition](../resources/software_test_definition.md), the software integration tests shall:
- Be derived from the software architecture to validate that the software detailed design and software implementation correctly realizes it.
- Use the software detailed design as complementary input only to understand how software interacts and to support test implementation.
- Use the software implementation only as the element under test.
- Define at least one integration test for each dynamic view represented as a sequence diagram, ensuring that all interactions defined through interfaces between software components are validated.
- Have a unique and well-defined objective. Multiple tests may be defined when required to validate the same interaction scenario.
- Be implemented in C using [Embedded Test Framework (ETF)](../../../../sw/ecf/embedded_test_framework/doc/etf.md) or [EDF Test Framework (EDFTest)](../../../../sw/ecf/event_driven_framework/edf_test/doc/edf_test.md), depending on the interaction model:
  - Event-driven interactions implemented using [Event Driven Framework (EDF)](../../../../sw/ecf/event_driven_framework/doc/edf.md) shall be tested using [EDF Test Framework (EDFTest)](../../../../sw/ecf/event_driven_framework/edf_test/doc/edf_test.md).
    - The [Event Driven Framework (EDF)](../../../../sw/ecf/event_driven_framework/doc/edf.md) software tests shall be used as a reference for how to use the [EDF Test Framework (EDFTest)](../../../../sw/ecf/event_driven_framework/edf_test/doc/edf_test.md).
  - All other interactions shall be tested using [Embedded Test Framework (ETF)](../../../../sw/ecf/embedded_test_framework/doc/etf.md).
    - The [Embedded Middleware Framework (EMF)](../../../../sw/ecf/embedded_middleware_framework/doc/emf.md) software tests shall be used as a reference for how to use the [Embedded Test Framework (ETF)](../../../../sw/ecf/embedded_test_framework/doc/etf.md).

At the end of the software integration test process, the complete software implementation shall build as a single executable without errors or warnings, using the toolchain and build configuration defined in the platform software design. This validation shall be performed for all defined build presets, including both host and target. However, individual software integration tests may be implemented using only the subset of software components required to validate the defined interactions and may not require a complete software executable.

#### Active object integration test

For software components implemented as active objects, software integration tests shall also validate their behavior as implemented by the Hierarchical State Machine (HSM), including all event-driven interactions defined in the software architecture.

The integration tests shall ensure that:
- All event-driven interactions between active objects behave as defined.
- The complete Hierarchical State Machine (HSM) behavior is validated through event sequences.
- All relevant use cases are covered.

#### Platform integration test

The platform software design shall be validated through software integration tests for all aspects that can be verified at this level.

The integration tests shall validate:
- Correct application of toolchains and build configuration, including successful compilation and linking using all defined build presets.
- Correct integration of platform-specific startup code and linker configuration, ensuring proper initialization and memory placement.
- Correct implementation of the defined memory layout:
  - Verification that memory regions are correctly mapped according to the linker script.
  - Validation of stack usage when applicable (e.g., using stack watermarking or equivalent techniques).
- Correct behavior of stdin/stdout integration defined by the Embedded C Framework (ECF), including buffering and ISR-based execution when applicable.
- Correct configuration and accessibility of debug interfaces defined in the platform software design.

Tests shall be executed in the target environment (HiL) when validation depends on hardware-specific behavior or execution environment characteristics that cannot be reliably validated in host environments.
