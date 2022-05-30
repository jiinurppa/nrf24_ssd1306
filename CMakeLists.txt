# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "C:/pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
include(lib/RF24/CMakeLists.txt)

project(nrf24_ssd1306 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(nrf24_ssd1306 nrf24_ssd1306.cpp )

pico_set_program_name(nrf24_ssd1306 "nrf24_ssd1306")
pico_set_program_version(nrf24_ssd1306 "0.1")

pico_enable_stdio_uart(nrf24_ssd1306 1)
pico_enable_stdio_usb(nrf24_ssd1306 0)

# Add the standard library to the build
target_link_libraries(nrf24_ssd1306 pico_stdlib hardware_sleep)

# Add any user requested libraries
target_link_libraries(nrf24_ssd1306
        hardware_spi
        hardware_i2c
        hardware_timer
        RF24
        )

pico_add_extra_outputs(nrf24_ssd1306)

