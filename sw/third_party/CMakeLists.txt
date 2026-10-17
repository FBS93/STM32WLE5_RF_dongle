# Exclude third-party software from static analysis and compiler warnings.
set(CMAKE_C_CLANG_TIDY "")
add_compile_options(-w)

# Use the CMake configuration provided by the SX126x driver.
add_subdirectory(sx126x_driver-2.5.0)

# Define a local CMake target because SEGGER RTT provides no CMake integration.
add_library(segger_rtt STATIC segger_rtt-8.58.0/RTT/SEGGER_RTT.c)
target_include_directories(
  segger_rtt
  SYSTEM
  PUBLIC segger_rtt-8.58.0/RTT segger_rtt-8.58.0/Config
)
target_compile_definitions(
  segger_rtt
  PUBLIC
    RTT_USE_ASM=0
    SEGGER_RTT_MAX_NUM_UP_BUFFERS=1
    SEGGER_RTT_MAX_NUM_DOWN_BUFFERS=1
    BUFFER_SIZE_UP=2049
    BUFFER_SIZE_DOWN=16
    SEGGER_RTT_MODE_DEFAULT=SEGGER_RTT_MODE_NO_BLOCK_SKIP
)
add_library(segger_rtt::segger_rtt ALIAS segger_rtt)
