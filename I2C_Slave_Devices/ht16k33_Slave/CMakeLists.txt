add_executable(ht16k33_Slave
    ht16k33_Slave.c)

target_link_libraries(ht16k33_Slave
    pico_i2c_slave
    pico_stdlib
    hardware_i2c
    )

# enable usb output, enable uart output
pico_enable_stdio_usb(ht16k33_Slave 1)
pico_enable_stdio_uart(ht16k33_Slave 1)

pico_add_extra_outputs(ht16k33_Slave)

example_auto_set_url(ht16k33_Slave)