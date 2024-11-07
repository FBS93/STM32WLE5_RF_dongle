add_library(radio_hal src/radio_hal.c)
target_include_directories(radio_hal PUBLIC inc)
target_link_libraries(radio_hal
    PRIVATE stm32cubemx
    PRIVATE fifo
)