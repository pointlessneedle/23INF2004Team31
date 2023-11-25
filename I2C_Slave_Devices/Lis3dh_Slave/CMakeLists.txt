add_executable(Lis3dh_Slave
Lis3dh_Slave.c)

target_link_libraries(Lis3dh_Slave
    pico_i2c_slave
    pico_stdlib
    hardware_i2c
    )

# enable usb output, enable uart output
pico_enable_stdio_usb(Lis3dh_Slave 1)
pico_enable_stdio_uart(Lis3dh_Slave 1)

pico_add_extra_outputs(Lis3dh_Slave)

example_auto_set_url(Lis3dh_Slave)