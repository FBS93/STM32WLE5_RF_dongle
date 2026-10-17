if(ECF_TARGET_PLATFORM STREQUAL "stm32wle5c8u6_platform")
  # Add project-specific target platform CMake configuration.
  include(src/${ECF_TARGET_PLATFORM}/cmake/platform.cmake)
  add_subdirectory(src/${ECF_TARGET_PLATFORM})

  # Add third-party source files.
  set(SX126X_ENABLE_BPSK ON)
  add_subdirectory(third_party)
endif()

add_subdirectory(ecf)

if(
  NOT DEFINED ECF_TARGET_PLATFORM
  OR ECF_TARGET_PLATFORM STREQUAL "stm32wle5c8u6_platform"
)
  # Add project-specific source files.
  add_subdirectory(src/rf_types)
  add_subdirectory(src/events)
  add_subdirectory(src/host_command_manager)
  add_subdirectory(src/error_handler)
  add_subdirectory(src/rf_manager)
  add_subdirectory(src/main)
endif()
