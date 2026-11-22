if(
  ECF_TARGET_PLATFORM STREQUAL "stm32f103c8tx"
  AND EBF_CORE STREQUAL "EBF_CORE_BAREMETAL"
  AND
    (
      EBF_PORT STREQUAL "EBF_PORT_ARM_CORTEX_M_PREEMPTIVE"
      OR EBF_PORT STREQUAL "EBF_PORT_ARM_CORTEX_M_NON_PREEMPTIVE"
    )
)
  add_subdirectory(arm_cortex_m_example)
endif()

if(EDF_EXAMPLE_FUZZ)
  add_subdirectory(fuzz_example)
endif()
