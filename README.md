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

When GP22 is pressed, the SPI master emulator is run, the program will request for the chip ID and then the readings from the emulated slave which is the BME280 sensor. Another pico device will be the emulated slave that interacts with the master. 

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
After cloning the repository, build the application from the main folder only and load the uf2 file into the raspberry pi pico. You will then be led to the menu of the application and a button from GP20 - GP22 can be pressed to run its associated feature.

For the SPI emulation, build the bme280_Slave in the SPI_Master_Slave folder and load the uf2 file into another raspberry pico with GP16 to GP 19 connected to each other (one pico's GP16 to the other's GP19 and GP19 to GP16).

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
  - BME280 sensor datasheet (https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf)

### 3rd Feature : I2C Slave Emulator
- Libraries from pico SDK were used


## Task Allocation and Project Details
### Drivers
- SPI Drivers _ Yi Ching
  - Read and write using SPI to flash (NOR flash Winbond W25Q128JV-IQ was used)
- I2C Driver _ Darius
  - Read and write using I2C to flash ( EEPROM flash ATMTC730 24C256N was used)
- MicroSD Card Driver
  - Read and write to MicroSD Card using Maker Pi Pico
### Main Features
- First Feature _ Yi Ching
  - Using SPI interface
  - Program to check what flash memory (part/series) is the connected device of. Also read the flash buffer memory content.
  - Output the manufacturer ID, device ID and series parts #.
  - Information gathered (the IDs and memory content) will be saved to the microsd card attached to the maker pi pico
- Second Feature _ Jordan & Jia Le
  - Using SPI interface
  - Pico device by default is a master, it can be made to emulate a slave to listen and respond to commands by the master device
  - The main idea is to emulate SPI devices using PIO
    - Two buffers will be sent between the devices–master is the transmitter, slave is the receiver.
    - Master to send incrementing data to slave in a loop, which the slave will listen for.
    - Slave then reads and prints the received data.
- Third Feature _ Darius & Manav
  - Using I2C interface
  - Create I2C slave emulator for devices in i2c/*_i2c of pico-examples
  - Another pico device should be able to use the pico example code to interact with the I2C slave emulator pico
  - The slave emulator will return the appropriate outputs when prompted by the master
  - The slave emulator will be based of the datasheets of the different devices in pico_example (e.g. https://www.sgbotic.com/products/datasheets/sensors/BST-BMP280-DS001-11.pdf)

## Future Improvements
- Main General Improvements
  - Attach more RAM to the pico to allow it to perform more functions simultaneously
- I2C Driver
  - Allow for more features to be editable in the config.h to make generic.c more configurable and powerful
  - Improve the capability of the pico by making the generic slave perform specific actions based on the commands that the generic slave receives
- Flash & MicroSD
  - Forensic analysis can be conducted on the buffer memory content that was pulled from the flash.
  - Upgrading the program to detect and identify more manufacture companies, as well as the different series under that company brand
  - Using pico’s WiFi feature to work with Network Time Protocol (NTP) to get the time to save the file with the current timestamp. Currently, the code is coded and has to be changed manually using
- SPI Slave Emulator:
  - Emulate more devices
  - Config file can be improved such that it consists of an array of classes or structs that stores the chip id or memory address of the other devices that can be emulated.
  - Program to allow for scalability so that when other devices were configured to be emulated, the master can iterate through the config file to identity the board.

