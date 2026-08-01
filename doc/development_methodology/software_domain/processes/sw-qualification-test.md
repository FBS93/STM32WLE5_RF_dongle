# sw-qualification-test

## Purpose

Define and implement software qualification tests against software requirements to validate that the software architecture, software detailed design and software implementation correctly realizes them.

## Input work products

- Software requirements
- Software architecture
- Software detailed design
- Software implementation
- Tools

## Output work products

- Software qualification test

## Steps

1. Review the software requirements.
2. Review available tools required to implement and execute software qualification tests.
3. Identify software requirements requiring validation.
4. Define and implement the software qualification tests.
5. Define traceability between software qualification tests and the corresponding software requirements.
6. Check the software qualification tests for completeness, consistency and correctness.

## Guidelines

### Software qualification test work product

The software qualification test work product shall follow the [Software test definition](../resources/software_test_definition.md).

In addition to the [Software test definition](../resources/software_test_definition.md), the software qualification tests shall:
- Be derived from the software requirements to validate that the software architecture, software detailed design and software implementation correctly realizes them.
- Use the software architecture and software detailed design as complementary input only to understand how software interacts and to support test implementation.
- Use the software implementation only as the element under test.
- Define at least one qualification test for each software requirement, ensuring full coverage of all software requirements.
- Have a unique and well-defined objective. Multiple tests may be defined when required to validate different aspects of the same software requirement.
- Be implemented in C using [Embedded Test Framework (ETF)](../../../../sw/ecf/embedded_test_framework/doc/etf.md) or [EDF Test Framework (EDFTest)](../../../../sw/ecf/event_driven_framework/edf_test/doc/edf_test.md), depending on the implemented architecture model.
  - Event-driven architecture model implemented using [Event Driven Framework (EDF)](../../../../sw/ecf/event_driven_framework/doc/edf.md) shall be tested using [EDF Test Framework (EDFTest)](../../../../sw/ecf/event_driven_framework/edf_test/doc/edf_test.md).
    - The [Event Driven Framework (EDF)](../../../../sw/ecf/event_driven_framework/doc/edf.md) software tests shall be used as a reference for how to use the [EDF Test Framework (EDFTest)](../../../../sw/ecf/event_driven_framework/edf_test/doc/edf_test.md).
  - All non event-driven architecture models shall be tested using [Embedded Test Framework (ETF)](../../../../sw/ecf/embedded_test_framework/doc/etf.md).
    - The [Embedded Middleware Framework (EMF)](../../../../sw/ecf/embedded_middleware_framework/doc/emf.md) software tests shall be used as a reference for how to use the [Embedded Test Framework (ETF)](../../../../sw/ecf/embedded_test_framework/doc/etf.md).
- Validate the software behavior using the complete software executable from an external perspective, without relying on internal implementation details.

At the end of the software qualification test process, it shall be ensured that all software requirements are validated by at least one software qualification test and that the complete software behaves as specified.
