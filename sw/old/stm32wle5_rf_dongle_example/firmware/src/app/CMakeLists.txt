add_library(app src/app.c)
target_include_directories(app PUBLIC inc)
target_link_libraries(app
    PRIVATE command_manager
    PRIVATE uart_hal
    PRIVATE radio_hal
    PRIVATE stm32cubemx
)