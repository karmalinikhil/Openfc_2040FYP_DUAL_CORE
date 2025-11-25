cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(openfc2040_peripheral_test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Add executable
add_executable(peripheral_test
    src/main.c
    src/led.c
    src/spi.c
    src/imu.c
    src/baro.c
    src/pwm.c
    src/adc.c
    src/uart.c
    src/sd_card.c
)

target_include_directories(peripheral_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(peripheral_test
    pico_stdlib
    hardware_spi
    hardware_i2c
    hardware_pwm
    hardware_adc
    hardware_gpio
    hardware_uart
    hardware_dma
)

# Enable USB and UART output
pico_enable_stdio_usb(peripheral_test 1)
pico_enable_stdio_uart(peripheral_test 1)

# Create UF2 output
pico_add_extra_outputs(peripheral_test)
