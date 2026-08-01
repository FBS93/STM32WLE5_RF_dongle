target_sources(emf PRIVATE
    src/emf_byte_fifo.c)
target_include_directories(emf
    PUBLIC inc)
target_link_libraries(emf
    PRIVATE eaf)

if(ECF_TEST)
    add_subdirectory(test)
endif()
