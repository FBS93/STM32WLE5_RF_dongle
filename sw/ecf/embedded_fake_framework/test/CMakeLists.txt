ecf_create_mock_from_file(
  inc/eff_dummy.h
  ${CMAKE_CURRENT_BINARY_DIR})

ecf_create_etf_test(
  test_eff
  src/test_eff.c
  LINKS
    eff_dummy_mock)
