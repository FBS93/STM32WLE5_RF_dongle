# Software test definition

## Purpose

Define the common test definition across the software domain.

## Test structure

Each software test shall:
- Be documented with a Markdown document named `<sw_test_name>.md`, following the template defined in [embedded_c_coding_guidelines.md](embedded_c_coding_guidelines.md#test-documentation), with the following additional information:
  - A unique identifier defined as a top Markdown heading (all other headings defined in the [embedded_c_coding_guidelines.md](embedded_c_coding_guidelines.md#test-documentation) template shall be shifted one level below this top heading), following the pattern `SW_<TEST_TYPE>_TEST_<X>`.
    - `<TEST_TYPE>` identifies the test type in upper case. Possible values are `UNIT` for unit test, `INTEGRATION` for integration test and `QUALIFICATION` for qualification test.
    - `<X>` is a monotonically increasing number that shall never be reused.
  - The `<Test name> test overview` chapter placeholder shall be replaced with the software test name used in the document name, using the following pattern `<Software test name> overview`. The chapter shall contain the following additional information:
    - Upstream traceability to the software traceable elements being tested. This shall be defined as a list of Markdown links referencing the corresponding software traceable elements.
  - A status indicating the approval state of the test. Shall use exactly one of these values: `not approved`, `approved`.
- Be implemented in C using [Embedded Test Framework (ETF)](../../../../sw/ecf/embedded_test_framework/doc/etf.md) or [EDF Test Framework (EDF Test)](../../../../sw/ecf/event_driven_framework/edf_test/doc/edf_test.md).
- Follow the rules defined in [embedded_c_coding_guidelines.md](embedded_c_coding_guidelines.md).
- Use [Embedded C Framework (ECF) CMake functions](../../../../sw/ecf/tools/cmake/functions/ecf_test.cmake) to register software tests.
- Be designed to run on both host (SiL) and target (HiL) environments whenever possible. If execution is restricted to a specific environment, CMake variables defined in the build presets related to the execution environment shall be used to condition the `add_subdirectory(<test_name>)` CMake command to include the test directory.
- Prioritize test definition in host environments (SiL) to enable faster and more efficient testing. Tests that depend on hardware-specific behavior, architecture, or execution environment characteristics that cannot be reliably emulated on host shall be implemented as target tests (HiL) only.

A project-specific CMake option named `<TEST_TYPE_ABBREVIATION>_TEST` shall be defined to enable software test build integration. The root `CMakeLists.txt` shall set the default value of this option to `OFF` when it is not explicitly defined by the selected build preset. All defined build presets shall explicitly set this option to `ON` unless otherwise specified. The `add_subdirectory(<test_type_abbreviation>_test)` CMake command to include the software test directories shall be conditioned by this option and shall only be applied when it is set to `ON`.
- `<TEST_TYPE_ABBREVIATION>` identifies the test type using its abbreviation in upper case. Possible values are `U` for unit test, `I` for integration test and `Q` for qualification test.
- `<test_type_abbreviation>` identifies the test type using its abbreviation in lower case. Possible values are `u` for unit test, `i` for integration test and `q` for qualification test.

The [Embedded C Framework (ECF)](../../../../sw/ecf/doc/ecf.md) shall be used as a reference for the implementation of the software tests.

The following additions to the Markdown document template shall be applied for software tests, in addition to the template defined in [embedded_c_coding_guidelines.md](embedded_c_coding_guidelines.md#test-documentation):

```md
# SW_<TEST_TYPE>_TEST_<X>

## <Software test name> overview

<Description of the purpose of the test>.

Upstream traceability:
- [<UPSTREAM_ELEMENT_UNIQUE_ID>](#...)

Status: <status>
```
