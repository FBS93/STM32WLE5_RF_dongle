if(DEFINED PROJECT_TARGET_PLATFORM AND DEFINED ECF_TARGET_PLATFORM)
  message(
    FATAL_ERROR
    "PROJECT_TARGET_PLATFORM and ECF_TARGET_PLATFORM are mutually exclusive"
  )
endif()

if(DEFINED PROJECT_TARGET_PLATFORM)
  if(NOT PROJECT_TARGET_PLATFORM STREQUAL "stm32wle5c8u6_platform")
    message(
      FATAL_ERROR
      "Unsupported PROJECT_TARGET_PLATFORM: ${PROJECT_TARGET_PLATFORM}"
    )
  endif()

  # Add project-specific target platform CMake configuration.
  include(src/${PROJECT_TARGET_PLATFORM}/cmake/platform.cmake)
  add_subdirectory(src/${PROJECT_TARGET_PLATFORM})

  # Add third-party source files.
  set(SX126X_ENABLE_BPSK ON)
  add_subdirectory(third_party)
  target_compile_options(sx126x_driver PRIVATE -w) # Disable warnings for third-party code.
endif()

add_subdirectory(ecf)

if(NOT DEFINED ECF_TARGET_PLATFORM)
  # Add project-specific source files.
  add_subdirectory(src/rf_types)
  add_subdirectory(src/events)
  add_subdirectory(src/host_command_manager)
  add_subdirectory(src/error_handler)
  add_subdirectory(src/rf_manager)
  add_subdirectory(src/main)
endif()
