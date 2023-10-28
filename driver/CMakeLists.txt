add_executable(spi_driver spi_driver.c)

# pull in common dependencies
target_link_libraries(spi_driver pico_stdlib hardware_spi)

# enable usb output
pico_enable_stdio_usb(spi_driver 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(spi_driver)

# add url via pico_set_program_url
example_auto_set_url(spi_driver)