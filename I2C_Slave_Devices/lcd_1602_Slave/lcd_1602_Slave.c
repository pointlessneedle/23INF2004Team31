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

// Modes for lcd_send_byte
#define LCD_CHARACTER  1
#define LCD_COMMAND    0

#define MAX_LINES      2
#define MAX_CHARS      16

static struct
{
    uint8_t cgram[64];
    uint8_t ddram[128];
    uint8_t target_mem_address;
    bool cgram_address_written;
    bool ddram_address_written;
} lcdMemoryMap;

static struct 
{
    uint8_t high;
    bool high_enable_pins_1;
    bool high_enable_pins_2;
    uint8_t low;
    bool low_enable_pins_1;
    bool low_enable_pins_2;
    bool highFilled;
    bool lowFilled;
} nibbleBytes;

static void lcd_1602_i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event){
    switch(event){
        uint8_t data; 
        uint8_t current;
        case I2C_SLAVE_RECEIVE:
            current = i2c_read_byte_raw(i2c);
            printf("current: %02X\n", current);
            if (nibbleBytes.high_enable_pins_1 && nibbleBytes.high_enable_pins_2){
                if (!nibbleBytes.lowFilled){
                    nibbleBytes.low = current;
                    nibbleBytes.lowFilled = true;
                    printf("nibble low filled\n");
                    break;
                }else if (!nibbleBytes.low_enable_pins_1 && ((nibbleBytes.low | LCD_ENABLE_BIT) == current)){
                    nibbleBytes.low_enable_pins_1 = true;
                    printf("nibble high pin 1\n");
                    if (!nibbleBytes.low_enable_pins_2){
                        break;
                    }
                } else if (!nibbleBytes.low_enable_pins_2 && ((nibbleBytes.low & ~LCD_ENABLE_BIT) == current)){
                    nibbleBytes.low_enable_pins_2 = true;
                    printf("nibble low pin 2\n");
                    if (!nibbleBytes.low_enable_pins_1){
                        break;
                    }
                }

                data = ((nibbleBytes.high & 0xF0) | (nibbleBytes.low >> 4));
                printf("Data received: %02X\n\n", data);
                if (lcdMemoryMap.cgram_address_written){
                    lcdMemoryMap.cgram[lcdMemoryMap.target_mem_address] = data;
                    lcdMemoryMap.target_mem_address++;
                } else if (lcdMemoryMap.ddram_address_written){
                    lcdMemoryMap.ddram[lcdMemoryMap.target_mem_address] = data;
                    lcdMemoryMap.target_mem_address++;
                } else {
                    if (data == LCD_CLEARDISPLAY){
                        //printf("Clear display");
                        lcdMemoryMap.target_mem_address = 0x00;
                        lcdMemoryMap.ddram[lcdMemoryMap.target_mem_address] = 0x20;
                    } else if (data == LCD_RETURNHOME){
                        //printf("Return home");
                        lcdMemoryMap.target_mem_address = 0x00;
                        cursorShift = LCD_CURSORSHIFT;
                    } else if ((data & 0xFC) == LCD_ENTRYMODESET){
                        entrySetMode = (data & 0x07);
                    } else if ((data & 0xF8) == LCD_DISPLAYCONTROL){
                        displayControl = (data & 0x0F);
                    } else if ((data & 0xF0) == LCD_CURSORSHIFT){
                        cursorShift = (data & 0x1F);
                    } else if ((data & 0xE0) == LCD_FUNCTIONSET){
                        functionSet = (data & 0x3F);
                    } else if ((data & 0xC0) == LCD_SETCGRAMADDR){
                        lcdMemoryMap.target_mem_address = (data & 0x3F);
                        lcdMemoryMap.cgram_address_written = true;
                    } else if ((data & 0x80) == LCD_SETDDRAMADDR){
                        lcdMemoryMap.target_mem_address = (data & 0x7F);
                        lcdMemoryMap.ddram_address_written = true;
                    }
                }
            } else {
                if (!nibbleBytes.highFilled){
                    nibbleBytes.high = current;
                    nibbleBytes.highFilled = true;
                    printf("nibble high filled\n");
                } else if (!nibbleBytes.high_enable_pins_1 && ((nibbleBytes.high | LCD_ENABLE_BIT) == current)){
                    nibbleBytes.high_enable_pins_1 = true;
                    printf("nibble high pin 1\n");
                } else if (!nibbleBytes.high_enable_pins_2 && ((nibbleBytes.high & ~LCD_ENABLE_BIT) == current)){
                    nibbleBytes.high_enable_pins_2 = true;
                    printf("nibble high pin 2\n");
                }
            }

            if (nibbleBytes.high_enable_pins_1 && nibbleBytes.high_enable_pins_2 && nibbleBytes.low_enable_pins_1 && nibbleBytes.low_enable_pins_2){
                nibbleBytes.high_enable_pins_1 = false;
                nibbleBytes.high_enable_pins_2 = false;
                nibbleBytes.low_enable_pins_1 = false;
                nibbleBytes.low_enable_pins_2 = false;
                nibbleBytes.highFilled = false;
                nibbleBytes.lowFilled = false;
            }
            break;
        case I2C_SLAVE_REQUEST:
            break;
        case I2C_SLAVE_FINISH:
            lcdMemoryMap.cgram_address_written = false;
            lcdMemoryMap.ddram_address_written = false;
            break;
    }
}

// For master
void i2c_write_byte(uint8_t val) {
    i2c_write_blocking(i2c1, SLAVE_ADDR, &val, 1, false);
    sleep_ms(500);
}

void lcd_toggle_enable(uint8_t val) {
    // Toggle enable pin on LCD display
    // We cannot do this too quickly or things don't work
#define DELAY_US 600
    sleep_us(DELAY_US);
    i2c_write_byte(val | LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
    i2c_write_byte(val & ~LCD_ENABLE_BIT);
    sleep_us(DELAY_US);
}

// The display is sent a byte as two separate nibble transfers
void lcd_send_byte(uint8_t val, int mode) {
    uint8_t high = mode | (val & 0xF0) | LCD_BACKLIGHT;
    uint8_t low = mode | ((val << 4) & 0xF0) | LCD_BACKLIGHT;
    printf("high: %02X, low: %02X, Original value: %02X\n", high, low, val);
    sleep_ms(500);

    i2c_write_byte(high);
    lcd_toggle_enable(high);
    i2c_write_byte(low);
    lcd_toggle_enable(low);
}

void lcd_clear(void) {
    lcd_send_byte(LCD_CLEARDISPLAY, LCD_COMMAND);
}

// go to location on LCD
void lcd_set_cursor(int line, int position) {
    int val = (line == 0) ? 0x80 + position : 0xC0 + position;
    lcd_send_byte(val, LCD_COMMAND);
}

static void inline lcd_char(char val) {
    lcd_send_byte(val, LCD_CHARACTER);
}

void lcd_string(const char *s) {
    while (*s) {
        printf("Sending: %c\n", *s);
        lcd_char(*s++);
        sleep_ms(1000);
    }
}

void lcd_init() {
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x03, LCD_COMMAND);
    lcd_send_byte(0x02, LCD_COMMAND);

    lcd_send_byte(LCD_ENTRYMODESET | LCD_ENTRYLEFT, LCD_COMMAND);
    lcd_send_byte(LCD_FUNCTIONSET | LCD_2LINE, LCD_COMMAND);
    lcd_send_byte(LCD_DISPLAYCONTROL | LCD_DISPLAYON, LCD_COMMAND);
    lcd_clear();
}

static void run_lcd_1602_master(){
    gpio_init(MASTER_SDA_PIN);
    gpio_set_function(MASTER_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MASTER_SDA_PIN);

    gpio_init(MASTER_SCL_PIN);
    gpio_set_function(MASTER_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MASTER_SCL_PIN);
    
    i2c_init(i2c1, I2C_BAUDRATE);

    lcd_init();

    static char *message[] =
            {
                    "RP2040 by", "Raspberry Pi",
                    "A brand new", "microcontroller",
                    "Twin core M0", "Full C SDK",
                    "More power in", "your product",
                    "More beans", "than Heinz!"
            };

    while (1) {
        for (int m = 0; m < sizeof(message) / sizeof(message[0]); m += MAX_LINES) {
            for (int line = 0; line < MAX_LINES; line++) {
                lcd_set_cursor(line, (MAX_CHARS / 2) - strlen(message[m + line]) / 2);
                lcd_string(message[m + line]);
            }
            sleep_ms(2000);
            lcd_clear();
        }
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

    nibbleBytes.high_enable_pins_1 = false;
    nibbleBytes.high_enable_pins_2 = false;
    nibbleBytes.low_enable_pins_1 = false;
    nibbleBytes.low_enable_pins_1 = false;
    nibbleBytes.highFilled = false;
    nibbleBytes.lowFilled = false;

    i2c_slave_init(i2c0, SLAVE_ADDR, &lcd_1602_i2c_slave_handler);
}

int main() {
    stdio_init_all();

    setup_slave();
    run_lcd_1602_master();

    return 0;
}