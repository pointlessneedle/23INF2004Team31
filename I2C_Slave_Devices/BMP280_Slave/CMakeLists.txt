add_executable(BMP280_Slave
    BMP280_Slave.c)

target_link_libraries(BMP280_Slave
    pico_i2c_slave
    pico_stdlib
    hardware_i2c
    )

# enable usb output, enable uart output
pico_enable_stdio_usb(BMP280_Slave 1)
pico_enable_stdio_uart(BMP280_Slave 1)

pico_add_extra_outputs(BMP280_Slave)

example_auto_set_url(BMP280_Slave)