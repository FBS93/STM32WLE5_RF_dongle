target_sources(edf PRIVATE
    src/edf_core.c)
target_include_directories(edf 
    PUBLIC inc)
target_link_libraries(edf
    PRIVATE ebf
    PRIVATE eaf
    PRIVATE emf)
