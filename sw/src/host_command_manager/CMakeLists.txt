if(PROJECT_TARGET_PLATFORM STREQUAL "stm32wle5c8u6_platform")
  set(host_command_manager_hal_source src/target/host_command_manager_hal.c)
elseif(NOT DEFINED PROJECT_TARGET_PLATFORM)
  set(host_command_manager_hal_source src/host/host_command_manager_hal.c)
else()
  message(
    FATAL_ERROR
    "host_command_manager is not supported on ${PROJECT_TARGET_PLATFORM}"
  )
endif()

add_library(
  host_command_manager
  STATIC
  src/host_command_manager.c
  src/host_command_manager_activities.c
  ${host_command_manager_hal_source}
)
target_include_directories(host_command_manager PUBLIC inc)
target_link_libraries(
  host_command_manager
  PUBLIC events edf
  PRIVATE eaf ebf emf
)
