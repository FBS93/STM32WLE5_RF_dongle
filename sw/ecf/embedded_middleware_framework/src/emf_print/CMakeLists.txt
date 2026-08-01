target_sources(emf PRIVATE
    src/emf_print.c)
target_include_directories(emf 
    PUBLIC inc)
target_link_libraries(emf
    PRIVATE ebf
    PRIVATE eaf)