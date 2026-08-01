# sys-integration-test

## Purpose

Define and implement system integration tests against system architecture to validate that the system implementation correctly realizes it.

## Input work products

- System architecture
- System implementation (all engineering domain work products)
- Tools

## Output work products

- System integration test

## Steps

1. Review the system architecture.
2. Review system implementation to understand how it is implemented and how the interfaces between system components can be validated at system level.
3. Review available tools required to implement and execute system integration tests.
4. Identify system architecture elements requiring validation.
5. Define and implement the system integration tests.
6. Define traceability between system integration tests and the corresponding system architecture elements.
7. Check the system integration tests for completeness, consistency and correctness.

## Guidelines

### System integration test work product

The system integration test work product shall follow the [System test definition](../resources/system_test_definition.md).

In addition to the [System test definition](../resources/system_test_definition.md), the system integration tests shall:
- Be derived from the system architecture to validate that the system implementation correctly realizes it.
- Use the system implementation as complementary input only to understand system interactions and to support test implementation.
- Use the system implementation only as the element under test.
- Define at least one integration test for each dynamic view represented as a sequence diagram, ensuring that all interactions defined through interfaces between system components are validated.
- Have a unique and well-defined objective. Multiple tests may be defined when required to validate the same interaction scenario.
- Use observable system interfaces between system components or external system interfaces to stimulate the system and observe its behavior.

At the end of the system integration test process, it shall be ensured that the complete system operates correctly as a whole, validating that all system components are properly integrated and interact as defined in the system architecture. However, individual system integration tests may be implemented using only the subset of system components required to validate the defined interactions and may not require the complete system.

#### System modes integration test

The system modes system design shall be validated through system integration tests.

The integration tests shall ensure that:
- All system modes behave as defined in the state diagram.
- All transitions between states are triggered correctly by the defined events.
- All relevant system modes and transitions are covered.
- The behavior associated with each state is validated.

Tests shall be implemented by stimulating the system through the defined interfaces and events, using the available project tools and observing the resulting system behavior.
