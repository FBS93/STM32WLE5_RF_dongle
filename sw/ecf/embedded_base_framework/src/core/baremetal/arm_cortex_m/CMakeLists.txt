target_sources(ebf PRIVATE
    src/ebf_core.c
    src/ebf_core.S
    src/syscalls.c)
target_link_libraries(ebf
    PRIVATE eaf
    PRIVATE emf)
target_include_directories(ebf 
    PUBLIC inc)