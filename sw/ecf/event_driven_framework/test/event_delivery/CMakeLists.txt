add_library(active_object STATIC
  src/active_object.c)
target_include_directories(active_object 
  PUBLIC inc)
target_link_libraries(active_object
  PUBLIC edf # edf.h file is included in active_object.h file, so link needs to be public.
  PRIVATE eaf
  PRIVATE emf)

ecf_create_edf_test(
  test_event_delivery
  src/test_event_delivery.c
  LINKS
    active_object
    emf
  INCLUDES)
