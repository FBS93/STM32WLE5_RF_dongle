if(DEFINED ECF_TARGET_PLATFORM)
  add_subdirectory(arm_cortex_m_preemption_example)
endif()

if(EDF_EXAMPLE_FUZZ)
  add_subdirectory(fuzz_example)
endif()
