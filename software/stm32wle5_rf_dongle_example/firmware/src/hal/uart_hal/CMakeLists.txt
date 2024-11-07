add_library(uart_hal src/uart_hal.c)
target_include_directories(uart_hal PUBLIC inc)
target_link_libraries(uart_hal
    PRIVATE stm32cubemx
    PRIVATE fifo
)