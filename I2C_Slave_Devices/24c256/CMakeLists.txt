add_executable(24c256
    24c256.c)

target_link_libraries(24c256
    pico_i2c_slave
    pico_stdlib
    hardware_i2c
    )

# enable usb output, enable uart output
pico_enable_stdio_usb(24c256 1)
pico_enable_stdio_uart(24c256 1)

pico_add_extra_outputs(24c256)

example_auto_set_url(24c256)