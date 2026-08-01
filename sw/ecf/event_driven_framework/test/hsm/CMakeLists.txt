add_library(hsm STATIC
  src/hsm.c)
target_include_directories(hsm 
  PUBLIC inc)
target_link_libraries(hsm
  PUBLIC edf # edf.h file is included in hsm.h file, so link needs to be public.
  PRIVATE eaf
  PRIVATE emf)

ecf_create_edf_test(
  test_hsm
  src/test_hsm.c
  LINKS
    hsm
    emf
  INCLUDES)
