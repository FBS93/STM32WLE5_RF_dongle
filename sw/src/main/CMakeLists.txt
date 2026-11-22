ew_add_executable(stm32wle5_rf_dongle src/main.c)
set_property(
  TARGET stm32wle5_rf_dongle
  APPEND
  PROPERTY LINK_DEPENDS ${EW_LINKER_FILES}
)
target_link_libraries(
  stm32wle5_rf_dongle
  PRIVATE error_handler edf emf events host_command_manager rf_manager
)
