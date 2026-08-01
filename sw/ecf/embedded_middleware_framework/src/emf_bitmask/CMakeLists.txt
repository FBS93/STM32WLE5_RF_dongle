target_sources(emf PRIVATE
    src/emf_bitmask.c)
target_include_directories(emf 
    PUBLIC inc)
target_link_libraries(emf
    PRIVATE eaf
    PRIVATE ebf)

if(ECF_TEST)
    add_subdirectory(test)
endif()
