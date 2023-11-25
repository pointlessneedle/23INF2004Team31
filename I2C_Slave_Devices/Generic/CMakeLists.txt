add_executable(Generic
    Generic.c)

target_link_libraries(Generic
    pico_i2c_slave
    pico_stdlib
    hardware_i2c
    )

# enable usb output, enable uart output
pico_enable_stdio_usb(Generic 1)
pico_enable_stdio_uart(Generic 1)

pico_add_extra_outputs(Generic)

example_auto_set_url(Generic)