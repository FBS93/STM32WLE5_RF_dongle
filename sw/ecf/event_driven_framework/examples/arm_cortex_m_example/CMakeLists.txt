if(EBF_PORT STREQUAL "EBF_PORT_ARM_CORTEX_M_PREEMPTIVE")
  set(variant preemption)
else()
  set(variant non_preemption)
endif()

set(example_target arm_cortex_m_${variant}_example)

ew_add_executable(
  ${example_target}
  src/main.c
  src/tick_timer.c
  src/exec_timer.c
  src/${variant}/src/app.c
  src/${variant}/src/gpio_trace.c
  src/${variant}/src/tick_timer.c
)
target_include_directories(${example_target} PRIVATE inc src/${variant}/inc)
target_link_libraries(${example_target} PRIVATE edf eaf emf)
