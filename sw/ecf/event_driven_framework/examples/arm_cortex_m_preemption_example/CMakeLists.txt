ew_add_executable(arm_cortex_m_preemption_example
  src/main.c
  src/app.c
  src/gpio_trace.c
  src/tick_timer.c
  src/exec_timer.c)
target_include_directories(arm_cortex_m_preemption_example
  PRIVATE inc)
target_link_libraries(arm_cortex_m_preemption_example
  PRIVATE edf
  PRIVATE eaf
  PRIVATE emf)
