set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc)

add_compile_options(
    -mthumb
    -ffunction-sections
    -fdata-sections
)

add_link_options(
    -mthumb
    -Wl,--gc-sections

    # Set newlib-nano and nosys specs.
    --specs=nano.specs
    --specs=nosys.specs
)
