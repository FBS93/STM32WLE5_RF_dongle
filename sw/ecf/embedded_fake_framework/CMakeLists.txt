add_library(eff INTERFACE)
target_include_directories(eff 
  INTERFACE inc)

if(ECF_TEST)
  add_subdirectory(test)
endif()
