# Python test definition

## Purpose

Define the common test definition for tests implemented in Python.

## Test structure

Each test implemented in Python shall:
- Be documented with a Markdown document named `<test_name>.md`, following the template defined in [python_coding_guidelines.md](python_coding_guidelines.md#test-documentation), with the following additional information:
  - A unique identifier defined as a top Markdown heading (all other headings defined in the [python_coding_guidelines.md](python_coding_guidelines.md#test-documentation) template shall be shifted one level below this top heading), following the pattern `SYS_INTEGRATION_TEST_<X>` for system integration tests or `QUALIFICATION_TEST_<X>` for qualification tests.
    - `<X>` is a monotonically increasing number that shall never be reused.
  - The `<Test name> test overview` chapter placeholder shall be replaced with the test name used in the document name, using the following pattern `<Test name> overview`. The chapter shall contain the following additional information:
    - Upstream traceability to the traceable elements being tested. This shall be defined as a list of Markdown links referencing the corresponding system architecture elements for integration tests or requirements for qualification tests.
  - A status indicating the approval state of the test. Shall use exactly one of these values: `not approved`, `approved`.
- Be implemented in Python using [pytest](https://docs.pytest.org/).
- Follow the rules defined in [python_coding_guidelines.md](python_coding_guidelines.md).
- Be designed to validate in a fully automated way whenever possible. When full automation is not feasible, tests shall define a structured sequence of manual steps, where each step requires human confirmation (`OK`/`NOK`) for the observed result.
- Define automated and manual-assisted tests separately to allow independent execution.

The following additions to the Markdown document template shall be applied for Python tests, in addition to the template defined in [python_coding_guidelines.md](python_coding_guidelines.md#test-documentation):

`<TEST_UNIQUE_ID>` shall use the identifier pattern defined above for the corresponding test type.

```md
# <TEST_UNIQUE_ID>

## <Test name> overview

<Description of the purpose of the test>.

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)

Status: <status>
```
