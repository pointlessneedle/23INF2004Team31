#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <pico/i2c_slave.h>

#define SLAVE_ADDR _u(0x27)
static const uint I2C_BAUDRATE = 100 * 1000;

#define SLAVE_SDA_PIN 0 
#define SLAVE_SCL_PIN 1
#define MASTER_SDA_PIN 6
#define MASTER_SCL_PIN 7

// commands
const int LCD_CLEARDISPLAY = 0x01;
const int LCD_RETURNHOME = 0x02;
const int LCD_ENTRYMODESET = 0x04;
const int LCD_DISPLAYCONTROL = 0x08;
const int LCD_CURSORSHIFT = 0x10;
const int LCD_FUNCTIONSET = 0x20;
const int LCD_SETCGRAMADDR = 0x40;
const int LCD_SETDDRAMADDR = 0x80;

// Latest Command settings
uint8_t entrySetMode = 0x04;
uint8_t displayControl = 0x08;
uint8_t cursorShift = 0x10;
uint8_t functionSet = 0x20;
uint8_t cgRamAddr = 0x40;
uint8_t ddRamAddr = 0x80;

// flags for display entry mode
const int LCD_ENTRYSHIFTINCREMENT = 0x01;
const int LCD_ENTRYLEFT = 0x02;

// flags for display and cursor control
const int LCD_BLINKON = 0x01;
const int LCD_CURSORON = 0x02;
const int LCD_DISPLAYON = 0x04;

// flags for display and cursor shift
const int LCD_MOVERIGHT = 0x04;
const int LCD_DISPLAYMOVE = 0x08;

// flags for function set
const int LCD_5x10DOTS = 0x04;
const int LCD_2LINE = 0x08;
const int LCD_8BITMODE = 0x10;

// flag for backlight control
const int LCD_BACKLIGHT = 0x08;

const int LCD_ENABLE_BIT = 0x04;


static void lcd_1602_i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event){
    switch(event){
        uint8_t data; 
        case I2C_SLAVE_RECEIVE:
            data = i2c_read_byte_raw(i2c);
            if (data == LCD_CLEARDISPLAY){
                printf("Clear display");
            } else if (data == LCD_RETURNHOME){
                printf("Return home");
            } else if ((data & LCD_ENTRYMODESET) == LCD_ENTRYMODESET){

            } else if ((data & LCD_DISPLAYCONTROL) == LCD_DISPLAYCONTROL){

            } else if ((data & LCD_CURSORSHIFT) == LCD_CURSORSHIFT){

            } else if ((data & LCD_FUNCTIONSET) == LCD_FUNCTIONSET){

            } else if ((data & LCD_SETCGRAMADDR) == LCD_SETCGRAMADDR){

            } else if ((data & LCD_SETDDRAMADDR) == LCD_SETDDRAMADDR){

            }
            break;
        case I2C_SLAVE_REQUEST:
            break;
        case I2C_SLAVE_FINISH:
            break;
    }
}

static void setup_slave(){
    gpio_init(SLAVE_SDA_PIN);
    gpio_set_function(SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SLAVE_SDA_PIN);

    gpio_init(SLAVE_SCL_PIN);
    gpio_set_function(SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SLAVE_SCL_PIN);

    i2c_init(i2c0, I2C_BAUDRATE);

    i2c_slave_init(i2c0, SLAVE_ADDR, &lcd_1602_i2c_slave_handler);
}

int main() {
    stdio_init_all();

    while(1){

    }

    return 0;
}