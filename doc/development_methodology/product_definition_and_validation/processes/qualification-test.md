# qualification-test

## Purpose

Define and implement qualification tests against requirements to validate that the product implementation correctly realizes them.

## Input work products

- Requirements
- System implementation (all engineering domain work products)
- Tools

## Output work products

- Qualification test

## Steps

1. Review the requirements.
2. Review all engineering domain work products to understand how the system is realized and how the requirements can be validated.
3. Review available tools required to implement and execute qualification tests.
4. Identify requirements requiring validation.
5. Define and implement the qualification tests.
6. Define traceability between qualification tests and the corresponding requirements.
7. Check the qualification tests for completeness, consistency and correctness.

## Guidelines

### Qualification test work product

Qualification tests implemented in Python shall follow the [Python test definition](../../resources/python_test_definition.md). Qualification tests implemented in C shall follow the [Embedded C test definition](../../resources/embedded_c_test_definition.md).

Qualification tests shall:
- Be derived from the requirements and their validation criteria to validate that the system implementation correctly realizes them.
- Use architectures and detailed designs as complementary input only to understand interactions and to support test implementation.
- Use the implementation only as the element under test.
- Define at least one qualification test for each requirement, ensuring full coverage of all requirements.
- Have a unique and well-defined objective. Multiple tests may be defined when required to validate different aspects of the same requirement.
- Select the verification scope, method, technology and execution environment according to the nature and validation criteria of the requirement. Tests may validate the complete system through external interfaces or use domain-specific interfaces, tools or inspection when appropriate.
- Be documented with a Markdown document named `<qualification_test_name>.md`, with a unique identifier defined as a top Markdown heading following the pattern `QUALIFICATION_TEST_<X>`. `<X>` is a monotonically increasing number that shall never be reused and shall be unique across all qualification tests.

At the end of the qualification test process, it shall be ensured that all requirements are validated by at least one qualification test and that the system behaves as specified.
