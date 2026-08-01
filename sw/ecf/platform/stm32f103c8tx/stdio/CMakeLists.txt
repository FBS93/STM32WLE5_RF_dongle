# This library provides strong symbol definitions that override weak ones from other libraries.
# It is implemented as an OBJECT + INTERFACE library to ensure strong symbols are never discarded.
# See: <project_root_dir>/src/doc/technical_notes/CMake_weak_symbol_oddities.md
add_library(stdio_obj OBJECT
    src/stdio.c
)
target_link_libraries(stdio_obj
    PRIVATE ebf
    PRIVATE eaf
)
add_library(stdio INTERFACE)
target_link_libraries(stdio INTERFACE
    stdio_obj
    $<TARGET_OBJECTS:stdio_obj>
)
