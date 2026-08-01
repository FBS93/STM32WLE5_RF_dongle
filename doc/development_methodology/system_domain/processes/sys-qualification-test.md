# sys-qualification-test

## Purpose

Define and implement system qualification tests against system requirements to validate that the system implementation correctly realizes them.

## Input work products

- System requirements
- System architecture
- System implementation (all engineering domain work products)
- Tools

## Output work products

- System qualification test

## Steps

1. Review the system requirements.
2. Review the system architecture and system implementation to understand how the system is realized and how it can be validated from an external system level perspective.
3. Review available tools required to implement and execute system qualification tests.
4. Identify system requirements requiring validation.
5. Define and implement the system qualification tests.
6. Define traceability between system qualification tests and the corresponding system requirements.
7. Check the system qualification tests for completeness, consistency and correctness.

## Guidelines

### System qualification test work product

The system qualification test work product shall follow the [System test definition](../resources/system_test_definition.md).

In addition to the [System test definition](../resources/system_test_definition.md), the system qualification tests shall:
- Be derived from the system requirements to validate that the system architecture and system implementation correctly realize them.
- Use the system architecture as complementary input only to understand system interactions and to support test implementation.
- Use the system implementation only as the element under test.
- Define at least one qualification test for each system requirement, ensuring full coverage of all system requirements.
- Have a unique and well-defined objective. Multiple tests may be defined when required to validate different aspects of the same system requirement.
- Validate the system behavior from an external perspective, using only external system interfaces. Internal interfaces between system components shall not be used for qualification testing.

At the end of the system qualification test process, it shall be ensured that all system requirements are validated by at least one system qualification test and that the complete system behaves as specified.
