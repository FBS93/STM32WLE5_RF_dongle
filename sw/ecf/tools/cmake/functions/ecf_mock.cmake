#------------------------------------------------------------------------------
# Function: ecf_create_mock_from_file
#
# Description:
#   Generates a mock library from a module’s header file using the Embedded Fake
#   Framework (EFF) auto-generator.
#
#   The generated mock will:
#     - Derive the mock name automatically from the header filename.
#     - Generate the following files inside a dedicated mock directory:
#         <mock_directory>/mock/<mock_name>/src/<mock_name>.c
#         <mock_directory>/mock/<mock_name>/inc/<mock_name>.h
#     - Create a CMake library target named <mock_name>.
#     - Add include directories for:
#         - The original header’s directory.
#         - The generated mock include folder.
#     - Link the mock library against:
#         - Embedded Fake Framework.
#         - Any additional libraries passed via optional arguments.
#
# Parameters:
#   header_path - Path to the module header file (relative or absolute).
#   mock_directory_path - Root directory where the mock structure will be created.
#
# Optional Arguments:
#   Additional arguments are forwarded as PRIVATE dependencies to
#   target_link_libraries for the generated mock target.
#
# Example:
#   ecf_create_mock_from_file(inc/foo.h test lib_a lib_b)
#
#   This will:
#     - Generate:
#         test/mock/foo_mock/src/foo_mock.c
#         test/mock/foo_mock/inc/foo_mock.h
#     - Create a CMake target named "foo_mock".
#     - Add include paths for:
#         inc/
#         test/mock/foo_mock/inc
#     - Link "foo_mock" against:
#         - eff
#         - lib_a
#         - lib_b
#------------------------------------------------------------------------------
function(ecf_create_mock_from_file header_path mock_directory_path)
  set(eff_gen_tool ${PROJECT_SOURCE_DIR}/sw/ecf/tools/eff_gen/eff_gen.py)

  # Locate the Python executable required for the mock generator if not already defined
  if(NOT DEFINED PYTHON_EXECUTABLE)
    find_program(PYTHON_EXECUTABLE NAMES python3 python REQUIRED)
  endif()

  # Get absolute/real paths
  get_filename_component(header_abs_path ${header_path} REALPATH)
  get_filename_component(header_folder ${header_abs_path} DIRECTORY)
  get_filename_component(mock_directory_abs ${mock_directory_path} ABSOLUTE)

  # Use the header filename without extension as base for mock_name
  get_filename_component(header_filename ${header_abs_path} NAME_WE)

  # Compose the mock target name by appending "_mock"
  set(mock_name "${header_filename}_mock")

  # Create folder structure for the mock - per mock subdirectory
  file(MAKE_DIRECTORY
    ${mock_directory_abs}/mock/${mock_name}
    ${mock_directory_abs}/mock/${mock_name}/src
    ${mock_directory_abs}/mock/${mock_name}/inc)

  # Define expected output files
  set(mock_src ${mock_directory_abs}/mock/${mock_name}/src/${mock_name}.c)
  set(mock_hdr ${mock_directory_abs}/mock/${mock_name}/inc/${mock_name}.h)

  # Generate the mock source and header using the EFF Python script
  add_custom_command(
    OUTPUT ${mock_src} ${mock_hdr}
    COMMAND ${PYTHON_EXECUTABLE}
            ${eff_gen_tool}
            -i ${header_abs_path}
            -o ${mock_directory_abs}/mock/${mock_name}
    DEPENDS ${header_abs_path} ${eff_gen_tool})

  # Create the mock library
  add_library(${mock_name} ${mock_src})
  target_include_directories(${mock_name} 
    PUBLIC ${header_folder}
    PUBLIC ${mock_directory_abs}/mock/${mock_name}/inc)
  target_link_libraries(${mock_name} 
    PUBLIC eff
    PRIVATE ${ARGN})
endfunction()

#------------------------------------------------------------------------------
# Function: ecf_create_mock_from_dir
#
# Description:
#   Generates a mock library from all header files contained in a directory,
#   using the Embedded Fake Framework (EFF) auto-generator.
#
#   The generated mock library will:
#     - Use the given mock_lib_name as the CMake target name.
#     - Generate one mock source/header pair per header file found in:
#         <headers_dir>/*.h
#     - Place all generated files inside:
#         <mock_directory>/mock/<mock_lib_name>/src/
#         <mock_directory>/mock/<mock_lib_name>/inc/
#     - Include each generated mock <base_name>_mock.c in the library.
#     - Add include directories for:
#         - The directory containing the original headers.
#         - The generated mock include directory.
#     - Link the library against:
#         - Embedded Fake Framework.
#         - Any additional libraries passed via optional arguments.
#
# Parameters:
#   mock_lib_name - Name of the mock library target to create.
#   headers_dir - Directory containing the headers to mock (relative or absolute).
#   mock_directory_path - Root directory where the mock structure will be created.
#
# Optional Arguments:
#   Additional arguments are forwarded as PRIVATE dependencies to
#   target_link_libraries for the generated mock library.
#
# Example:
#   ecf_create_mock_from_dir(generated_mocks modules/inc tests lib_a lib_b)
#
#   This will:
#     - For every *.h file in modules/inc/, generate a mock pair:
#         tests/mock/generated_mocks/src/<header_base>_mock.c
#         tests/mock/generated_mocks/inc/<header_base>_mock.h
#     - Create a CMake library named "generated_mocks".
#     - Add include paths:
#         modules/inc/
#         tests/mock/generated_mocks/inc
#     - Link "generated_mocks" against:
#         - eff
#         - lib_a
#         - lib_b
#------------------------------------------------------------------------------
function(ecf_create_mock_from_dir mock_lib_name headers_dir mock_directory_path)
  set(eff_gen_tool ${PROJECT_SOURCE_DIR}/sw/ecf/tools/eff_gen/eff_gen.py)

  # Locate the Python executable required for the mock generator if not already defined
  if(NOT DEFINED PYTHON_EXECUTABLE)
    find_program(PYTHON_EXECUTABLE NAMES python3 python REQUIRED)
  endif()

  # Get absolute/real paths
  get_filename_component(headers_dir_abs ${headers_dir} REALPATH)
  get_filename_component(mock_directory_abs ${mock_directory_path} ABSOLUTE)

  # Create output directories (if they don't exist)
  file(MAKE_DIRECTORY
    ${mock_directory_abs}/mock/${mock_lib_name}
    ${mock_directory_abs}/mock/${mock_lib_name}/src
    ${mock_directory_abs}/mock/${mock_lib_name}/inc)

  # Glob all header files (*.h) in the absolute headers directory
  file(GLOB header_files "${headers_dir_abs}/*.h")

  # Initialize empty lists to store the expected mock outputs and source files
  set(mock_outputs "")
  set(mock_sources "")

  # Loop over each header file found
  foreach(header_file IN LISTS header_files)
    # Extract the filename without extension (base name) from the header file path
    get_filename_component(header_filename ${header_file} NAME_WE)

    # Construct the mock target name by appending "_mock" suffix
    set(mock_name "${header_filename}_mock")

    # Define the expected paths for the generated mock source (.c) and header (.h) files
    set(mock_src "${mock_directory_abs}/mock/${mock_lib_name}/src/${mock_name}.c")
    set(mock_hdr "${mock_directory_abs}/mock/${mock_lib_name}/inc/${mock_name}.h")

    # Append the expected output files to the list of outputs
    list(APPEND mock_outputs ${mock_src} ${mock_hdr})

    # Append the generated mock source file to the list of source files for the library
    list(APPEND mock_sources ${mock_src})
  endforeach()

  # Generate the mock source and header using the EFF Python script
  add_custom_command(
    OUTPUT ${mock_outputs}
    COMMAND ${PYTHON_EXECUTABLE}
            ${eff_gen_tool}
            -i ${headers_dir_abs}
            -o ${mock_directory_abs}/mock/${mock_lib_name}
    DEPENDS ${header_files} ${eff_gen_tool}
  )

  # Create the mock library
  add_library(${mock_lib_name} ${mock_sources})
  target_include_directories(${mock_lib_name} 
    PUBLIC ${headers_dir_abs}
    PUBLIC ${mock_directory_abs}/mock/${mock_lib_name}/inc)
  target_link_libraries(${mock_lib_name} 
    PUBLIC eff 
    PRIVATE ${ARGN})
endfunction()
