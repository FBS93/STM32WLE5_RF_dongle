# sw-unit-test

## Purpose

Define and implement software unit tests against software detailed design to validate that the software implementation correctly realizes it.

## Input work products

- Software detailed design
- Software implementation
- Tools

## Output work products

- Software unit test

## Steps

1. Review the software detailed design.
2. Review available tools required to implement and execute software unit tests.
3. Identify which parts of the software detailed design require unit testing.
4. Define and implement the software unit tests.
5. Define traceability between software unit tests and the corresponding software detailed design.
6. Check the software unit tests for completeness, consistency and correctness.

## Guidelines

### Software unit test work product

The software unit test work product shall follow the [Software test definition](../resources/software_test_definition.md).

In addition to the [Software test definition](../resources/software_test_definition.md), the software unit tests shall:
- Be derived exclusively from the software detailed design content to validate that the software implementation correctly realizes it.
- Use the software implementation only as the element under test.
- Be defined only for software detailed design functions whose logic or algorithmic behavior justifies isolated validation for early error detection.
- Have a unique and well-defined objective. Multiple tests may be defined when required to validate different aspects of the same software detailed design.
- Be implemented in C using [Embedded Test Framework (ETF)](../../../../sw/ecf/embedded_test_framework/doc/etf.md). 
  - The [Embedded Middleware Framework (EMF)](../../../../sw/ecf/embedded_middleware_framework/doc/emf.md) software unit tests shall be used as a reference for how to use the [Embedded Test Framework (ETF)](../../../../sw/ecf/embedded_test_framework/doc/etf.md).

#### Active object unit test

For software units implemented as active objects, software unit tests shall only validate individual HSM function activities defined in the software detailed design. The Hierarchical State Machine (HSM) behavior as a complete active object shall not be validated in the software unit test process.

#### Platform unit test

Platform package software units shall not have dedicated software unit tests. The correct implementation of the platform package is indirectly validated from all software unit tests that can be executed in target environment (HiL).
