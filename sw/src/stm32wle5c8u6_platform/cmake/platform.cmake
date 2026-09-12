# Apply the project-specific platform package to every target executable.
set(EW_GLOBAL_LIBS stdio)
set(
  EW_STARTUP_FILES
  ${CMAKE_CURRENT_LIST_DIR}/../startup/startup.c
  ${CMAKE_CURRENT_LIST_DIR}/../startup/startup.S
)
set(EW_LINKER_FILES ${CMAKE_CURRENT_LIST_DIR}/../linker/linker.ld)

add_compile_options(-mcpu=cortex-m4 -mfloat-abi=soft)
add_link_options(-mcpu=cortex-m4 -mfloat-abi=soft)
