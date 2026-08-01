
add_library(eaf STATIC
  src/eaf.c)
target_include_directories(eaf 
  PUBLIC inc)
target_link_libraries(eaf
  PRIVATE ebf
  PRIVATE emf)

