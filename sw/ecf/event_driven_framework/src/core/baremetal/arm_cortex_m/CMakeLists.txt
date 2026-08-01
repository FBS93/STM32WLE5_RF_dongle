target_sources(edf PRIVATE
    src/edf_core.c
    src/edf_core.S)
target_include_directories(edf 
    PRIVATE inc)
target_link_libraries(edf
    PRIVATE ebf
    PRIVATE eaf
    PRIVATE emf)

