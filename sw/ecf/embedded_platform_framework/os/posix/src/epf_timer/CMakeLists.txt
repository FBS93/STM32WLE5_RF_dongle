target_sources(epf PRIVATE src/epf_timer.c)
target_include_directories(epf PUBLIC inc)
target_link_libraries(epf PRIVATE ebf eaf emf)
