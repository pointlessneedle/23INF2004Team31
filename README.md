# Embedded Systems Team 31
In this repository, it includes the following:
* Drivers
* SPI Emulator
* I2C Emulator
* Flash Memory Type Probing
* MicroSD Reading and Writing

## Drivers are inclusive of:
* I2C Flash
* SPI Flash

### SPI Flash
#### Wiring Information
Wiring up the SPI Flash to the pico with 6 jumper wires are as follows:
| PICO -> SPI External Flash |
* 3.3v (pin __) -> VCC
* GPIO 16 (pin 21) -> DO
* GPIO 17 (pin 22) -> CS
* GPIO 18 (pin 24) -> SCK
* GPIO 19 (pin 25) -> DI
* GND (pin 23) -> GND
<i>Note: The pins used are under the SPI default.</i>

