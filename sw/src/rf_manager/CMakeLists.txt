if(PROJECT_TARGET_PLATFORM STREQUAL "stm32wle5c8u6_platform")
  set(rf_manager_hal_source src/target/rf_manager_hal.c)
  set(rf_manager_hal_dependency sx126x_driver)
elseif(NOT DEFINED PROJECT_TARGET_PLATFORM)
  set(rf_manager_hal_source src/host/rf_manager_hal.c)
  set(rf_manager_hal_dependency)
else()
  message(
    FATAL_ERROR
    "rf_manager is not supported on ${PROJECT_TARGET_PLATFORM}"
  )
endif()

add_library(
  rf_manager
  STATIC
  src/rf_manager.c
  src/rf_manager_activities.c
  ${rf_manager_hal_source}
)
target_include_directories(rf_manager PUBLIC inc)
target_link_libraries(
  rf_manager
  PRIVATE ${rf_manager_hal_dependency}
  PUBLIC events edf rf_types
  PRIVATE eaf ebf emf
)
