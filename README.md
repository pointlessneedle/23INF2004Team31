# Embedded Systems Team 31
## Brief Project Introduction
This project is about the use of I2C and SPI to create emulations of both slave and master devices along with the use of Flash and MicroSD Cards to obtain the flash information which can be used for forensics work or experimentations.

## Project Objectives
This project’s objective is to create 3 separate subsystems with the documented features which can be selected using a maker pi pico. The 3 features that will be developed are generic I2C slave emulation, flash reading through SPI interface and MicroSD, and SPI slave emulation.

When the application is loaded onto a maker pi pico, these features can be accessed by pressing the buttons, GP20 - GP22 on the maker pi pico which will run the features stated above respectively. 

## Overview of Project's Program
When the application is first run, you will be led to a menu which offers 3 choices on what feature that you will want to run. When a button from GP20 - GP22 is pressed, its respective feature is then run.

When GP20 is pressed, the generic I2C slave emulation feature is run, it will first read the file config.h which is used to set the configuration of the slave device on the maker pi pico. If the configuration allows the creation of the master, the current device can connect to itself to conduct testing to see if the slave is properly configured. Another maker pi pico device can also be used to interact with the emulated slave device.

When GP21 is pressed, the feature for the flash and MicroSD program is run, the pico will transmit (send) in defined instructions into the connected flash to get the manufacturing, device and chip identification (ID) numbers. The manufacturer ID will identify what company manufactured the flash, and the device ID will be used to check against a data structure list within the program to see what is the flash’s type/series, and these information will be stored in a file in the sdcard. Additionally, the flash’s buffer memory content will be read and saved into the same file created in the sdcard.

When GP22 is pressed, the SPI slave emulator is run, the program will load the memory map of the emulated evidence which is the BME280 sensor and initialization of the SPI pins. Another pico device will be the spi master that interacts with the emulated slave device.

## Requirements to Run
> i.e. what libraries are needed/were used</i>
<table>
  <tr>
    <td>stdio.h</td>
    <td>stdlib.h</td>
    <td>string.h</td>
    <td>pico/stdlib.h</td>
    <td>pico/binary_info.h</td>
    <td>pico/i2c_slave.h</td>
  </tr>
  <tr>
    <td>pico/time.h</td>
    <td>ff.h</td>
    <td>sd_card.h</td>
    <td>hardware/i2c.h</td>
    <td>hardware/spi.h</td>
    <td>no-OS-FatFS-SD-SPI-RPi-Pico(https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico)</td>
  </tr>
</table>

## How to Execute the Program
After cloning the repository, build the application and load the uf2 file into the maker pi pico. You will then be led to the menu of the application and a button from GP20 - GP22 can be pressed to run its associated feature.

## Citations on any Referenced Code Used
### 1st Feature : Flash & MicroSD Card
- pico-example spi_flash.c
- Various Winbond datasheet (included in the repository)
- Reference for sdcard read and write: https://www.digikey.com/en/maker/projects/raspberry-pi-pico-rp2040-sd-card-example-with-micropython-and-cc/e472c7f578734bfd96d437e68e670050
- Key to Value Mapping:
  - https://www.geeksforgeeks.org/implementation-on-map-or-dictionary-data-structure-in-c/
  - https://www.14core.com/wiring-the-winbond-w25qxx-spi-serial-flash-memory-with-microcontroller/ 
### 2nd Feature : SPI Slave Emulator
- Example codes from pico SDK
  - Spi -> bme280 spi
- Code references for SPI between 2 picos:
  - https://www.circuitstate.com/tutorials/making-two-raspberry-pi-pico-boards-communicate-through-spi-using-c-cpp-sdk/
### 3rd Feature : I2C Slave Emulator







