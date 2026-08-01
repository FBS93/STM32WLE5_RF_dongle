include(CMakeParseArguments)

#------------------------------------------------------------------------------
# Function: ecf_create_etf_test
#
# Description:
#   Creates a test executable and configures it for compilation and execution
#   with CTest, running either locally on the host or on the embedded target
#   via run_target_test.py when the global CMake variable ECF_TARGET_PLATFORM
#   is defined.
#
#   For target tests, the test binary path is passed to the run_target_test.py
#   script, which handles flashing, execution, and result capture to report
#   pass/fail status to CTest.
#
#   The generated test will:
#     - Build an executable target from the provided source file
#     - Link it against:
#         - Embedded Test Framework library (ETF).
#         - Any additional libraries passed via the LINKS argument.
#     - Add include directories specified via the INCLUDES argument.
#     - Register the executable as a test (using add_test).
#
# Parameters:
#   test_name - Name of the test target (used for the executable and CTest).
#   test_src - Path to the test source file (relative or absolute).
#
# Keyword Arguments:
#   LINKS - Additional libraries or targets to link against.
#   INCLUDES - Include directories required by the test.
#
# Example:
#   ecf_create_etf_test(my_test test/test_my_module.c
#     LINKS lib_a lib_b
#     INCLUDES lib_c/inc lib_d/inc)
#
#   This will:
#     - Create an executable target named "my_test" from test/test_my_module.c
#     - Link it against:
#         - etf
#         - lib_a
#         - lib_b
#     - Add the include directories:
#         - lib_c/inc
#         - lib_d/inc
#     - Register "my_test" as a CTest test.
#------------------------------------------------------------------------------
function(ecf_create_etf_test test_name test_src)
  # Parse optional keyword arguments LINKS and INCLUDES
  set(options)
  set(oneValueArgs)
  set(multiValueArgs LINKS INCLUDES)
  cmake_parse_arguments(CT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Resolve the absolute path of the test source file
  get_filename_component(test_src_absolute ${test_src} REALPATH)

  # Create the test executable target
  ew_add_executable(${test_name} ${test_src_absolute})
  target_link_libraries(${test_name} PRIVATE etf ${CT_LINKS})
  target_include_directories(${test_name} PRIVATE ${CT_INCLUDES})

  # Register the test with CTest
  if(DEFINED ECF_TARGET_PLATFORM)
    # Target tests: run on embedded target via runner script.
    add_test(
      NAME ${test_name}
      COMMAND
        python3
        ${PROJECT_SOURCE_DIR}/tools/run_target_test/run_target_test.py
        $<TARGET_FILE:${test_name}>
    )
    set_tests_properties(${test_name} PROPERTIES
      RESOURCE_LOCK hil_target
    )
  else()
    # Host tests: run directly on host machine.
    add_test(
      NAME ${test_name}
      COMMAND ${test_name})
  endif()
endfunction()

#------------------------------------------------------------------------------
# Function: ecf_create_edf_test
#
# Description:
#   Creates a test executable and configures it for compilation and execution
#   with CTest, running either locally on the host or on the embedded target
#   via run_target_test.py when the global CMake variable ECF_TARGET_PLATFORM
#   is defined.
#
#   For target tests, the test binary path is passed to the run_target_test.py
#   script, which handles flashing, execution, and result capture to report
#   pass/fail status to CTest.
#
#   The generated test will:
#     - Build an executable target from the provided source file.
#     - Link it against:
#         - EDF Test library (EDF_test).
#         - Any additional libraries passed via the LINKS argument.
#     - Add include directories specified via the INCLUDES argument.
#     - Register the executable as a test (using add_test).
#
# Parameters:
#   test_name - Name of the test target (used for the executable and CTest).
#   test_src - Path to the test source file (relative or absolute).
#
# Keyword Arguments:
#   LINKS - Additional libraries or targets to link against.
#   INCLUDES - Include directories required by the test.
#
# Example:
#   ecf_create_edf_test(my_test test/test_my_module.c
#     LINKS lib_a lib_b
#     INCLUDES lib_c/inc lib_d/inc)
#
#   This will:
#     - Create an executable target named "my_test" from test/test_my_module.c
#     - Link it against:
#         - edf_test
#         - lib_a
#         - lib_b
#     - Add the include directories:
#         - lib_c/inc
#         - lib_d/inc
#     - Register "my_test" as a CTest test.
#------------------------------------------------------------------------------
function(ecf_create_edf_test test_name test_src)
  # Parse optional keyword arguments LINKS and INCLUDES
  set(options)
  set(oneValueArgs)
  set(multiValueArgs LINKS INCLUDES)
  cmake_parse_arguments(CT "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Resolve the absolute path of the test source file
  get_filename_component(test_src_absolute ${test_src} REALPATH)

  # Create the test executable target
  ew_add_executable(${test_name} ${test_src_absolute})
  target_link_libraries(${test_name} PRIVATE edf_test ${CT_LINKS})
  target_include_directories(${test_name} PRIVATE ${CT_INCLUDES})

  # Register the test with CTest
  if(DEFINED ECF_TARGET_PLATFORM)
    # Target tests: run on embedded target via runner script.
    add_test(
      NAME ${test_name}
      COMMAND
        python3
        ${PROJECT_SOURCE_DIR}/tools/run_target_test/run_target_test.py
        $<TARGET_FILE:${test_name}>
    )
    set_tests_properties(${test_name} PROPERTIES
      RESOURCE_LOCK hil_target
    )
  else()
    # Host tests: run directly on host machine.
    add_test(
      NAME ${test_name}
      COMMAND ${test_name})
  endif()
endfunction()
