ew_add_executable(fuzz_example src/main.c src/fuzz_example.c)
target_include_directories(fuzz_example PRIVATE inc)
target_link_libraries(fuzz_example PRIVATE ebf edf eaf emf)
