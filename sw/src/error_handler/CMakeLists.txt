if(PROJECT_TARGET_PLATFORM STREQUAL "stm32wle5c8u6_platform")
  set(error_handler_source src/target/error_handler.c)
elseif(NOT DEFINED PROJECT_TARGET_PLATFORM)
  set(error_handler_source src/host/error_handler.c)
else()
  message(
    FATAL_ERROR
    "error_handler is not supported on ${PROJECT_TARGET_PLATFORM}"
  )
endif()

# This library provides strong symbol definitions that override weak ones from other libraries.
# It is implemented as an OBJECT + INTERFACE library to ensure strong symbols are used.
# See: doc/technical_notes/CMake_weak_symbol_oddities.md
add_library(error_handler_obj OBJECT ${error_handler_source})
target_link_libraries(error_handler_obj PRIVATE eaf emf)
add_library(error_handler INTERFACE)
target_link_libraries(
  error_handler
  INTERFACE error_handler_obj $<TARGET_OBJECTS:error_handler_obj>
)
