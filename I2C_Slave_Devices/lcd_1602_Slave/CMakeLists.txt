add_executable(lcd_1602_Slave
    lcd_1602_Slave.c)

target_link_libraries(lcd_1602_Slave
    pico_i2c_slave
    pico_stdlib
    hardware_i2c
    )

# enable usb output, enable uart output
pico_enable_stdio_usb(lcd_1602_Slave 1)
pico_enable_stdio_uart(lcd_1602_Slave 1)

pico_add_extra_outputs(lcd_1602_Slave)

example_auto_set_url(lcd_1602_Slave)