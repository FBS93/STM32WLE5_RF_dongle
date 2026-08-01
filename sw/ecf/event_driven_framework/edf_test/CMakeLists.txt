add_library(edf_test STATIC
  src/edf_test.c)
target_include_directories(edf_test 
  PUBLIC inc)
target_link_libraries(edf_test
  PRIVATE ebf
  PUBLIC edf # edf_test is designed specifically to validate edf, so propagate edf API
  PRIVATE eaf
  PRIVATE emf)
