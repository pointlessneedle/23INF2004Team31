# Minimum CMake version and project name
cmake_minimum_required(VERSION 3.13)
set(PROJECT_NAME flashid)
# Set the board you're using
set(PICO_BOARD pico_w)

# Import the Pico SDK
include(pico_sdk_import.cmake)

project(flashid)

# Initialize the SDK
pico_sdk_init()

# Create your executable
add_executable(${PROJECT_NAME} flashid.c)

# Include any directories for header files here
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Link your libraries
target_link_libraries(${PROJECT_NAME} 
    pico_stdlib
    hardware_spi 
    hardware_gpio
)

# Enable stdio functionalities
pico_enable_stdio_usb(${PROJECT_NAME} TRUE)
pico_enable_stdio_uart(${PROJECT_NAME} TRUE)

# Generate extra outputs (.uf2, .hex, .bin etc.)
pico_add_extra_outputs(${PROJECT_NAME})



