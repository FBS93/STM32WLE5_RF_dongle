cmake_minimum_required(VERSION 3.22)
set(CMAKE_C_STANDARD 11)
project(embedded_workbench C ASM)

# ******************************************************************************
# Build options and configuration.
# ******************************************************************************

if(DEFINED ECF_TARGET_PLATFORM)
  # Add target platform specific cmake configurations.
  include(sw/ecf/platform/${ECF_TARGET_PLATFORM}/cmake/platform.cmake)
endif()

add_link_options(
  # Keep EBF custom syscalls required by newlib-nano.
  -Wl,--undefined=_write
  -Wl,--undefined=_read
  -Wl,--undefined=_sbrk
  -Wl,--undefined=_close
  -Wl,--undefined=_lseek
  -Wl,--undefined=_fstat
  -Wl,--undefined=_isatty
  -Wl,--undefined=_exit
  -Wl,--undefined=_kill
  -Wl,--undefined=_getpid
)

# Add custom cmake function to add executable.
include(tools/cmake/functions/ew_add_executable.cmake)

# Set ECF_TEST to OFF if not defined.
option(ECF_TEST "Build Embedded C Framework tests" OFF)

if(ECF_TEST)
  # Enable CTest.
  enable_testing()

  # Add ECF custom cmake functions related to test and mock.
  include(sw/ecf/tools/cmake/functions/ecf_test.cmake)
  include(sw/ecf/tools/cmake/functions/ecf_mock.cmake)
endif()

option(EW_ENABLE_CLANG_TIDY "Run clang-tidy during compilation" OFF)

if(EW_ENABLE_CLANG_TIDY)
  include(tools/cmake/functions/ew_enable_clang_tidy.cmake)
  ew_enable_clang_tidy()
endif()

# Set EDF_EXAMPLE_FUZZ to OFF if not defined.
option(EDF_EXAMPLE_FUZZ "Build the EDF fuzz example application" OFF)

# ******************************************************************************
# ECF configuration.
#
# These variables can be overridden using `set(...)` before adding ECF through
# CMake.
#
# Example:
#     set(EBF_CORE "EBF_CORE_BAREMETAL")
#     add_subdirectory(ecf)
# ******************************************************************************

# ==============================================================================
# EBF default configuration.
# ==============================================================================

if(NOT DEFINED EBF_CORE)
  set(
    EBF_CORE
    "EBF_CORE_OS"
    CACHE STRING
    "Target core: one of {EBF_CORE_BAREMETAL, EBF_CORE_RTOS, EBF_CORE_OS}"
  )
endif()

if(NOT DEFINED EBF_PORT)
  set(
    EBF_PORT
    "EBF_PORT_POSIX_NON_PREEMPTIVE"
    CACHE STRING
    "Target port: one of {EBF_PORT_ARM_CORTEX_M, EBF_PORT_FREERTOS, EBF_PORT_POSIX_NON_PREEMPTIVE, EBF_PORT_POSIX_NON_PREEMPTIVE_FUZZ, EBF_PORT_POSIX_PREEMPTIVE}"
  )
endif()

if(EBF_PORT STREQUAL "EBF_PORT_POSIX_NON_PREEMPTIVE_FUZZ" AND ECF_TEST)
  message(
    FATAL_ERROR
    "Invalid configuration: ECF_TEST=ON is not supported with EBF_PORT=EBF_PORT_POSIX_NON_PREEMPTIVE_FUZZ. Disable ECF_TEST or select a non-fuzz port."
  )
endif()

if(
  EDF_EXAMPLE_FUZZ
  AND NOT EBF_PORT STREQUAL "EBF_PORT_POSIX_NON_PREEMPTIVE_FUZZ"
)
  message(
    FATAL_ERROR
    "Invalid configuration: EDF_EXAMPLE_FUZZ=ON requires EBF_PORT=EBF_PORT_POSIX_NON_PREEMPTIVE_FUZZ. Disable EDF_EXAMPLE_FUZZ or select the fuzz port."
  )
endif()

if(EBF_PORT MATCHES "^EBF_PORT_POSIX")
  # Expose features from the 2008 POSIX standard (IEEE 1003.1-2008) for all
  # POSIX-based ECF ports before including any POSIX-related headers.
  add_compile_definitions(_POSIX_C_SOURCE=200809L)
endif()

# ==============================================================================
# EMF default configuration.
# ==============================================================================

if(NOT DEFINED EMF_PRINT_MAX_LEN)
  set(
    EMF_PRINT_MAX_LEN
    "256"
    CACHE STRING
    "Maximum EMF formatted print message length in bytes (1-65535)"
  )
endif()

# ==============================================================================
# EDF default configuration.
# ==============================================================================

if(NOT DEFINED EDF_MAX_SIGNAL)
  set(
    EDF_MAX_SIGNAL
    "2"
    CACHE STRING
    "Max number of signals: one of {1, 2, 4} (1=2^8, 2=2^16, 4=2^32)"
  )
endif()

if(NOT DEFINED EDF_EVENT_QUEUE_SIZE)
  set(
    EDF_EVENT_QUEUE_SIZE
    "1"
    CACHE STRING
    "Event queue size: one of {1, 2} (1=2^8, 2=2^16)"
  )
endif()

if(NOT DEFINED EDF_MAX_ACTIVE_OBJECT)
  set(
    EDF_MAX_ACTIVE_OBJECT
    "32"
    CACHE STRING
    "Max number of active objects (1-64)"
  )
endif()

if(NOT DEFINED EDF_MAX_TICK_RATE)
  set(
    EDF_MAX_TICK_RATE
    "1"
    CACHE STRING
    "Max number of tick rates for time events (1-15)"
  )
endif()

if(NOT DEFINED EDF_MAX_POOL)
  set(EDF_MAX_POOL "3" CACHE STRING "Max number of memory pools (1-15)")
endif()

if(NOT DEFINED EDF_TIME_EVENT_MAX_TICK)
  set(
    EDF_TIME_EVENT_MAX_TICK
    "4"
    CACHE STRING
    "Max ticks per time event: one of {1, 2, 4} (1=2^8, 2=2^16, 4=2^32)"
  )
endif()

if(NOT DEFINED EDF_POOL_BLOCK_SIZE)
  set(
    EDF_POOL_BLOCK_SIZE
    "2"
    CACHE STRING
    "Pool block size (bytes): one of {1, 2, 4} (1=2^8, 2=2^16, 4=2^32)"
  )
endif()

if(NOT DEFINED EDF_MAX_POOL_BLOCK)
  set(
    EDF_MAX_POOL_BLOCK
    "2"
    CACHE STRING
    "Max number of blocks per pool: one of {1, 2, 4} (1=2^8, 2=2^16, 4=2^32)"
  )
endif()

if(NOT DEFINED EDF_HSM_MAX_DEPTH)
  set(
    EDF_HSM_MAX_DEPTH
    "6"
    CACHE STRING
    "Max HSM state nesting depth (>=3, including top level)"
  )
endif()

# ******************************************************************************
# Include software directory.
# ******************************************************************************

add_subdirectory(sw)
