add_library(etf STATIC
  src/etf.c)
target_include_directories(etf 
  PUBLIC inc)
target_link_libraries(etf
  PRIVATE ebf
  PRIVATE eaf
  PRIVATE emf)
