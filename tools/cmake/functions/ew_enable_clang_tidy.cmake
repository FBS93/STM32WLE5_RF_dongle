#------------------------------------------------------------------------------
# Function: ew_enable_clang_tidy
#
# Description:
#   Configures clang-tidy integration for C compilation in the current CMake
#   scope.
#
#   The configured clang-tidy command will:
#     - Use the repository .clang-tidy configuration file.
#     - Restrict header diagnostics to files under the sw/ directory.
#     - Add the compiler target triple when available.
#     - Add the compiler sysroot when available and valid.
#
# Parameters:
#   None.
#------------------------------------------------------------------------------
function(ew_enable_clang_tidy)
    # Define the clang-tidy configuration inputs.
    set(ew_clang_tidy_config "${CMAKE_SOURCE_DIR}/.clang-tidy")
    set(ew_clang_tidy_header_filter "^${CMAKE_SOURCE_DIR}/sw/.*")

    # Ensure the repository clang-tidy configuration exists.
    if(NOT EXISTS "${ew_clang_tidy_config}")
        message(FATAL_ERROR "clang-tidy config file not found: ${ew_clang_tidy_config}")
    endif()

    # Locate the clang-tidy executable.
    find_program(EW_CLANG_TIDY_EXECUTABLE NAMES clang-tidy REQUIRED)

    # Build the base clang-tidy command.
    set(ew_clang_tidy_command
        "${EW_CLANG_TIDY_EXECUTABLE}"
        "--config-file=${ew_clang_tidy_config}"
        "--header-filter=${ew_clang_tidy_header_filter}"
    )

    # Query the compiler sysroot for toolchain-aware linting.
    execute_process(
        COMMAND "${CMAKE_C_COMPILER}" -print-sysroot
        OUTPUT_VARIABLE ew_clang_tidy_sysroot
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # Query the compiler target triple for toolchain-aware linting.
    execute_process(
        COMMAND "${CMAKE_C_COMPILER}" -dumpmachine
        OUTPUT_VARIABLE ew_clang_tidy_target
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )

    # Propagate the compiler target triple when available.
    if(ew_clang_tidy_target)
        list(APPEND ew_clang_tidy_command "--extra-arg-before=--target=${ew_clang_tidy_target}")
    endif()

    # Propagate the compiler sysroot when available.
    if(ew_clang_tidy_sysroot AND IS_DIRECTORY "${ew_clang_tidy_sysroot}")
        list(APPEND ew_clang_tidy_command "--extra-arg-before=--sysroot=${ew_clang_tidy_sysroot}")
    endif()

    # Enable clang-tidy for C sources in the caller scope.
    set(CMAKE_C_CLANG_TIDY "${ew_clang_tidy_command}" PARENT_SCOPE)
endfunction()
