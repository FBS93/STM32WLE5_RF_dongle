if(
  EBF_PORT STREQUAL "EBF_PORT_ARM_CORTEX_M_PREEMPTIVE"
  OR EBF_PORT STREQUAL "EBF_PORT_ARM_CORTEX_M_NON_PREEMPTIVE"
)
  add_subdirectory(arm_cortex_m)
else()
  message(FATAL_ERROR "Invalid EBF_PORT value: ${EBF_PORT}")
endif()
