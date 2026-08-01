add_library(time_mgr STATIC
  src/time_mgr.c)
target_include_directories(time_mgr 
  PUBLIC inc)
target_link_libraries(time_mgr
  PUBLIC edf # edf.h file is included in time_mgr.h file, so link needs to be public.
  PRIVATE eaf
  PRIVATE emf)

ecf_create_edf_test(
  test_time_event
  src/test_time_event.c
  LINKS
    time_mgr
    emf
  INCLUDES)
