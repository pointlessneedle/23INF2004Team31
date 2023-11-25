#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <pico/i2c_slave.h>

#define SLAVE_ADDR _u(0x18)
static const uint I2C_BAUDRATE = 100 * 1000;

#define SLAVE_SDA_PIN 0 
#define SLAVE_SCL_PIN 1
#define MASTER_SDA_PIN 6
#define MASTER_SCL_PIN 7

const uint8_t STATUS_REG_AUX = 0x07;
const uint8_t OUT_ADC1_L = 0x08;
const uint8_t OUT_ADC1_H = 0x09;
const uint8_t OUT_ADC2_L = 0x0A;
const uint8_t OUT_ADC2_H = 0x0B;
const uint8_t OUT_ADC3_L = 0x0C;
const uint8_t OUT_ADC3_H = 0x0D;
const uint8_t INT_COUNTER_REG = 0x0E;
const uint8_t WHO_AM_I = 0x0F;
const uint8_t TEMP_CFG_REG = 0x1F; // rw
const uint8_t CTRL_REG_1 = 0x20; // rw
const uint8_t CTRL_REG_2 = 0x21; // rw 
const uint8_t CTRL_REG_3 = 0x22; // rw
const uint8_t CTRL_REG_4 = 0x23; // rw
const uint8_t CTRL_REG_5 = 0x24; // rw
const uint8_t CTRL_REG_6 = 0x25; // rw
const uint8_t REFERENCE = 0x26; //rw
const uint8_t STATUS_REG2 = 0x27; //rw
const uint8_t OUT_X_L = 0x28;
const uint8_t OUT_X_H = 0x29;
const uint8_t OUT_Y_L = 0x2A;
const uint8_t OUT_Y_H = 0x2B;
const uint8_t OUT_Z_L = 0x2C;
const uint8_t OUT_Z_H = 0x2D;
const uint8_t FIFO_CTRL_REG = 0x2E; // rw
const uint8_t FIFO_SRC_REG = 0x2F;
const uint8_t INT1_CFG = 0x30; // rw
const uint8_t INT1_SOURCE = 0x31;
const uint8_t INT1_THIS = 0x32; // rw
const uint8_t INT1_DURATION = 0x33; // rw
const uint8_t CLICK_CFG = 0x38;
const uint8_t CLICK_SRC = 0x39;
const uint8_t CLICK_THIS = 0x3A; 

static void lcd_1602_i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event){

}

int main() {
    stdio_init_all();

    setup_slave();
    while (1){

    }

    return 0;
}