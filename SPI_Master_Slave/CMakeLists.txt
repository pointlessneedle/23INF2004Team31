add_executable(spi_master
        spi_master.c
        )

# Pull in basic dependencies
target_link_libraries(spi_master pico_stdlib hardware_spi)

# create map/bin/hex file etc.
pico_add_extra_outputs(spi_master)

# add url via pico_set_program_url
example_auto_set_url(spi_master)

# enable usb output, disable uart output
pico_enable_stdio_usb(spi_master 1)
pico_enable_stdio_uart(spi_master 1)
 


add_executable(bme280_Slave
bme280_Slave.c
        )

# pull in common dependencies and additional spi hardware support
target_link_libraries(bme280_Slave pico_stdlib hardware_spi)
 
# create map/bin/hex file etc.
pico_add_extra_outputs(bme280_Slave)

# add url via pico_set_program_url
example_auto_set_url(bme280_Slave)

# enable usb output, disable uart output
pico_enable_stdio_usb(bme280_Slave 1)
pico_enable_stdio_uart(bme280_Slave 1)