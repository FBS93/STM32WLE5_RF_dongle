target_sources(ebf 
    PRIVATE src/ebf_core.c)
target_include_directories(ebf 
    PRIVATE inc)
target_link_libraries(ebf
  PRIVATE eaf
  PRIVATE emf)