add_subdirectory(no-OS-FatFS-SD-SPI-RPi-Pico/FatFs_SPI build)
add_executable(integrated integrated.c Generic.c BMP280_Slave.c ht16k33_Slave.c lcd_1602_Slave.c flashmicro.c spi_master.c)

# pull in common dependencies
target_link_libraries(
    integrated
    pico_i2c_slave
    pico_stdlib
    pico_cyw43_arch_none
    hardware_pwm
    hardware_adc
    hardware_i2c
    hardware_spi 
    hardware_gpio
    hardware_adc
    FatFs_SPI
    ) 

# enable usb output, enable uart output
pico_enable_stdio_usb(integrated 1)
pico_enable_stdio_uart(integrated 1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(integrated)

# add url via pico_set_program_url
example_auto_set_url(integrated)