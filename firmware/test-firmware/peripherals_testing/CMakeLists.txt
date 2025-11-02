cmake_minimum_required(VERSION 3.13)

# Include the Pico SDK
include(pico-sdk/pico_sdk_init.cmake)

# Project name and language
project(openfc2040_firmware C CXX ASM)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the Pico SDK
pico_sdk_init()

# Add include directories
include_directories(include)

# Create the executable
add_executable(openfc2040_firmware
    src/main.c
)

# Link libraries
target_link_libraries(openfc2040_firmware
    pico_stdlib
    hardware_gpio
    hardware_spi
    hardware_uart
    hardware_pwm
)

# Enable USB output, disable UART output
pico_enable_stdio_usb(openfc2040_firmware 1)
pico_enable_stdio_uart(openfc2040_firmware 0)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(openfc2040_firmware)

# Print memory usage (commented out as it may not be available in all SDK versions)
# if (PICO_SDK_VERSION_STRING VERSION_GREATER_EQUAL "1.3.0")
#     pico_print_memory_usage(openfc2040_firmware)
# endif()
