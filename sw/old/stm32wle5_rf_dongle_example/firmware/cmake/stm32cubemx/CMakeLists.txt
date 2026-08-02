cmake_minimum_required(VERSION 3.22)

project(stm32cubemx)
add_library(stm32cubemx INTERFACE)

# Enable CMake support for ASM and C languages
enable_language(C ASM)

target_compile_definitions(stm32cubemx INTERFACE 
	CORE_CM4 
	USE_HAL_DRIVER 
	STM32WLE5xx
    $<$<CONFIG:Debug>:DEBUG>
)

target_include_directories(stm32cubemx INTERFACE
    ../../Core/Inc
    ../../SubGHz_Phy/App
    ../../SubGHz_Phy/Target
    ../../Utilities/trace/adv_trace
    ../../Drivers/STM32WLxx_HAL_Driver/Inc
    ../../Drivers/STM32WLxx_HAL_Driver/Inc/Legacy
    ../../Utilities/misc
    ../../Utilities/sequencer
    ../../Utilities/timer
    ../../Utilities/lpm/tiny_lpm
    ../../Drivers/CMSIS/Device/ST/STM32WLxx/Include
    ../../Middlewares/Third_Party/SubGHz_Phy
    ../../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver
    ../../Drivers/CMSIS/Include
)

target_sources(stm32cubemx INTERFACE
    ../../Core/Src/main.c
    ../../Core/Src/stm32_lpm_if.c
    ../../Core/Src/sys_app.c
    ../../Core/Src/timer_if.c
    ../../Core/Src/stm32_adv_trace_if.c
    ../../Core/Src/stm32wlxx_it.c
    ../../Core/Src/stm32wlxx_hal_msp.c
    ../../SubGHz_Phy/Target/radio_board_if.c
    ../../SubGHz_Phy/App/app_subghz_phy.c
    ../../SubGHz_Phy/App/subghz_phy_app.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_rcc.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_rcc_ex.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_flash.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_flash_ex.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_gpio.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_dma.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_dma_ex.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_pwr.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_pwr_ex.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_cortex.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_exti.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_subghz.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_tim.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_tim_ex.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_uart.c
    ../../Drivers/STM32WLxx_HAL_Driver/Src/stm32wlxx_hal_uart_ex.c
    ../../Core/Src/system_stm32wlxx.c
    ../../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_driver.c
    ../../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio_fw.c
    ../../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/lr_fhss_mac.c
    ../../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/wl_lr_fhss.c
    ../../Middlewares/Third_Party/SubGHz_Phy/stm32_radio_driver/radio.c
    ../../Utilities/trace/adv_trace/stm32_adv_trace.c
    ../../Utilities/misc/stm32_mem.c
    ../../Utilities/misc/stm32_systime.c
    ../../Utilities/misc/stm32_tiny_sscanf.c
    ../../Utilities/misc/stm32_tiny_vsnprintf.c
    ../../Utilities/sequencer/stm32_seq.c
    ../../Utilities/timer/stm32_timer.c
    ../../Utilities/lpm/tiny_lpm/stm32_lpm.c
    ../../Core/Src/sysmem.c
    ../../Core/Src/syscalls.c
    ../../startup_stm32wle5xx.s
)

target_link_directories(stm32cubemx INTERFACE
)

target_link_libraries(stm32cubemx INTERFACE
)

# Validate that STM32CubeMX code is compatible with C standard
if(CMAKE_C_STANDARD LESS 11)
    message(ERROR "Generated code requires C11 or higher")
endif()


