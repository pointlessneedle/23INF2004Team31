#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include <pico/i2c_slave.h>
#include <ctype.h>

static const uint SLAVE_ADDR = 0x70;
static const uint I2C_BAUDRATE = 100000;

// For the master to run, based on ht16k33.c
#define NUM_DIGITS 4
#define HT16K33_SYSTEM_STANDBY  0x20
#define HT16K33_SYSTEM_RUN      0x21
#define HT16K33_SET_ROW_INT     0xA0
#define HT16K33_BRIGHTNESS      0xE0
#define HT16K33_DISPLAY_SETUP   0x80
#define HT16K33_DISPLAY_OFF     0x0
#define HT16K33_DISPLAY_ON      0x1
#define HT16K33_BLINK_2HZ       0x2
#define HT16K33_BLINK_1HZ       0x4
#define HT16K33_BLINK_0p5HZ     0x6
//----------------------

#define SLAVE_SDA_PIN 0 // Pseudo SDA for ht16k33, connect master GP6 here
#define SLAVE_SCL_PIN 1 // Pseudo SCL for ht16k33, connect master GP7 here
#define MASTER_SDA_PIN 6 // Temp, fake master SDA
#define MASTER_SCL_PIN 7 // Temp, fake master SCL

// Memory addresses of ht16k33
#define DEF_DISPLAY_MEMORY_START _u(0x00)
#define DEF_DISPLAY_MEMORY_END _u(0x0F) // Address, Read and Write, 0x00 - 0x0F

#define DEF_SYSTEM_SETUP _u(0x20)
static uint8_t systemSetUp = 0x20; // Command, Write Only, 0x20 - 0x21

#define DEF_KEY_DATA_ADDRESS_START _u(0x40)
#define DEF_KEY_DATA_ADDRESS_END _u(0x45)
static uint8_t dataAddressPointer = 0x40; // Address, Read only

#define DEF_INT_FLAG_ADDRESS _u(0x60)
static uint8_t intFlagAddress = 0x60; // Address, Read only

#define DEF_DISPLAY_SETUP _u(0x80)
static uint8_t displaySetUp = 0x80; // Command, Write only, 0x80 - 0x87

#define DEF_ROW_INT_SET _u(0xA0)
static uint8_t rowIntSet = 0xA0; // Command, Write only, 0xA0 - 0xA3

#define DEF_DIMMING_SET _u(0xE0) 
static uint8_t dimmingSet = 0xE0; // Command, Write only, 0xE0 - 0xEF

#define DEF_TEST_MODE _u(0xD9)
static uint8_t testMode = 0xD9; // Command, Write only

static struct
{
    uint8_t mem[256];
    uint8_t target_mem_address; // For ht16k33, the target_mem address can be a command or 
} ht16k33MemoryMap; // For display data, key data and INT flag address pointer

bool display_set = false;

static void ht16k33_i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event){
    uint8_t data = i2c_read_byte_raw(i2c);
    switch (event){
        case I2C_SLAVE_RECEIVE:
            if (display_set){ //Writing data to display data address
                ht16k33MemoryMap.mem[ht16k33MemoryMap.target_mem_address] = data;
                ht16k33MemoryMap.target_mem_address++;

                // Resets the display data address to 0x00 if it goes past 0x0F
                if (ht16k33MemoryMap.target_mem_address > DEF_DISPLAY_MEMORY_END){
                    ht16k33MemoryMap.target_mem_address = DEF_DISPLAY_MEMORY_START;
                }
            } else{
                if (data >= DEF_DISPLAY_MEMORY_START && data < DEF_DISPLAY_MEMORY_END){ // Based on datasheet, display data address point 
                    ht16k33MemoryMap.target_mem_address = data;
                    display_set = true;
                } else if ((data ^ DEF_SYSTEM_SETUP) == 0 || (data ^ DEF_SYSTEM_SETUP) == 1){
                    ht16k33MemoryMap.target_mem_address = DEF_SYSTEM_SETUP;
                    systemSetUp = data; // CHANGE THIS
                } else if ((data ^ DEF_DISPLAY_SETUP) >= 0 && (data ^ DEF_DISPLAY_SETUP) <= 7){
                    ht16k33MemoryMap.target_mem_address = DEF_DISPLAY_SETUP;
                    displaySetUp = data; // CHANGE THIS
                } else if ((data ^ DEF_ROW_INT_SET) >= 0 && (data ^ DEF_ROW_INT_SET) <= 3){
                    ht16k33MemoryMap.target_mem_address = DEF_ROW_INT_SET;
                    rowIntSet = data; // CHANGE THIS
                } else if ((data ^ DEF_DIMMING_SET) >= 0 && (data ^ DEF_DIMMING_SET) <= 15){
                    ht16k33MemoryMap.target_mem_address = DEF_DIMMING_SET;
                    dimmingSet = data; // CHANGE THIS
                } else {
                    ht16k33MemoryMap.target_mem_address = data;
                }
            }
            break;
        case I2C_SLAVE_REQUEST:
            if (ht16k33MemoryMap.target_mem_address >= DEF_DISPLAY_MEMORY_START && ht16k33MemoryMap.target_mem_address < DEF_DISPLAY_MEMORY_END){
                i2c_write_byte_raw(i2c, ht16k33MemoryMap.mem[ht16k33MemoryMap.target_mem_address]);
                ht16k33MemoryMap.target_mem_address++;
            } else if (ht16k33MemoryMap.target_mem_address = DEF_KEY_DATA_ADDRESS_START){
                i2c_write_byte_raw(i2c, ht16k33MemoryMap.mem[ht16k33MemoryMap.target_mem_address]);
                ht16k33MemoryMap.target_mem_address++;
            } else if (ht16k33MemoryMap.target_mem_address = DEF_INT_FLAG_ADDRESS){
                i2c_write_byte_raw(i2c, ht16k33MemoryMap.mem[ht16k33MemoryMap.target_mem_address]);
                ht16k33MemoryMap.target_mem_address++;
            }
            break;
        case I2C_SLAVE_FINISH:
            display_set = false;
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

    i2c_slave_init(i2c0, SLAVE_ADDR, &ht16k33_i2c_slave_handler);
}

uint16_t char_to_pattern(char ch) {
// Map, "A" to "Z"
int16_t alpha[] = {
    0xF7,0x128F,0x39,0x120F,0xF9,0xF1,0xBD,0xF6,0x1209,0x1E,0x2470,0x38,0x536,0x2136,
    0x3F,0xF3,0x203F,0x20F3,0x18D,0x1201,0x3E,0xC30,0x2836,0x2D00,0x1500,0xC09
    };

// Map, "0" to "9"
int16_t num[] = {
    0xC3F,0x406,0xDB,0x8F,0xE6,0xED,0xFD,0x1401,0xFF,0xE7
    };

    if (isalpha(ch))
        return alpha[toupper(ch) - 'A'];
    
    if (isdigit(ch))
        return num[ch - '0'];
    
    return 0;
}

/* Quick helper function for single byte transfers */
void i2c_write_byte(uint8_t val) {
    i2c_write_blocking(i2c1, SLAVE_ADDR, &val, 1, false);
}

void ht16k33_init() {
    i2c_write_byte(HT16K33_SYSTEM_RUN);
    i2c_write_byte(HT16K33_SET_ROW_INT);
    i2c_write_byte(HT16K33_DISPLAY_SETUP | HT16K33_DISPLAY_ON);
}

static inline void ht16k33_display_set(int position, uint16_t bin) {
    uint8_t buf[3];
    buf[0] = position * 2;
    buf[1] = bin & 0xff;
    buf[2] = bin >> 8;

    i2c_write_blocking(i2c1, SLAVE_ADDR, buf, count_of(buf), false);
}

static inline void ht16k33_display_char(int position, char ch) {
    ht16k33_display_set(position, char_to_pattern(ch));
}

void ht16k33_display_string(char *str) {
    int digit = 0;
    while (*str && digit <= NUM_DIGITS) {
        ht16k33_display_char(digit++, *str++);
    }
}

void ht16k33_scroll_string(char *str, int interval_ms) {
    int l = strlen(str);

    if (l <= NUM_DIGITS) {
        ht16k33_display_string(str);
    }
    else {
        for (int i = 0; i < l - NUM_DIGITS + 1; i++) {
            ht16k33_display_string(&str[i]);
            sleep_ms(interval_ms);
        }
    }
}

void ht16k33_set_brightness(int bright) {
    i2c_write_byte(HT16K33_BRIGHTNESS | (bright <= 15 ? bright : 15));
}

void ht16k33_set_blink(int blink) {
    int s = 0;
    switch (blink) {
        default: break;
        case 1: s = HT16K33_BLINK_2HZ; break;
        case 2: s = HT16K33_BLINK_1HZ; break;
        case 3: s = HT16K33_BLINK_0p5HZ; break;
    }

    i2c_write_byte(HT16K33_DISPLAY_SETUP | HT16K33_DISPLAY_ON | s);
}

// Runs a master for ht16k33, based on ht16k33_i2c
static void run_ht16k33_master(){
    gpio_init(MASTER_SDA_PIN);
    gpio_set_function(MASTER_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MASTER_SDA_PIN);

    gpio_init(MASTER_SCL_PIN);
    gpio_set_function(MASTER_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MASTER_SCL_PIN);
    
    i2c_init(i2c1, I2C_BAUDRATE);

    ht16k33_init();

    ht16k33_display_set(0, 0);
    ht16k33_display_set(1, 0);
    ht16k33_display_set(2, 0);
    ht16k33_display_set(3, 0);

    while(1){
        while(1){
            sleep_ms(1000);
            printf("test\n");
        }
        ht16k33_scroll_string("Welcome to the Raspberry Pi Pico", 150);

        // Do a speeding up propeller effort using the inner segments
        int bits[] = {0x40, 0x0100, 0x0200, 0x0400, 0x80, 0x2000, 0x1000, 0x0800};
        for (int j = 0;j < 10;j++) {
            for (int i = 0;i< count_of(bits); i++) {
                for (int digit = 0;digit <= NUM_DIGITS; digit++) {
                    ht16k33_display_set(digit, bits[i]);
                }
                sleep_ms(155 - j*15);
            }
        }

        char *strs[] = {
            "Help", "I am", "in a", "Pico", "and ", "Cant", "get ", "out "
        };

        for (int i = 0; i < count_of(strs); i++) {
            ht16k33_display_string(strs[i]);
            sleep_ms(500);
        }

        sleep_ms(1000);

        // Test brightness and blinking

        // Set all segments on all digits on
        ht16k33_display_set(0, 0xffff);
        ht16k33_display_set(1, 0xffff);
        ht16k33_display_set(2, 0xffff);
        ht16k33_display_set(3, 0xffff);

        // Fade up and down
        for (int j=0;j<5;j++) {
            for (int i = 0; i < 15; i++) {
                ht16k33_set_brightness(i);
                sleep_ms(30);
            }

            for (int i = 14; i >=0; i--) {
                ht16k33_set_brightness(i);
                sleep_ms(30);
            }
        }

        ht16k33_set_brightness(15);

        ht16k33_set_blink(1); // 0 for no blink, 1 for 2Hz, 2 for 1Hz, 3 for 0.5Hz
        sleep_ms(5000);
        ht16k33_set_blink(0);
    }
}

int main(){
    stdio_init_all();

    setup_slave();
    run_ht16k33_master();

    while (1);

    return 0;
}