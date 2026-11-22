add_test(
  NAME i_test_stack_watermark
  COMMAND
    ${CMAKE_COMMAND} -E env
    "STACK_PRODUCT_ELF=$<TARGET_FILE:stm32wle5_rf_dongle>" python3 -m pytest -q
    -s ${CMAKE_CURRENT_SOURCE_DIR}/src/stack_watermark.py
)
set_tests_properties(
  i_test_stack_watermark
  PROPERTIES RESOURCE_LOCK hil_target TIMEOUT 300
)
