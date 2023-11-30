#include <stdio.h>
#include <pico/stdlib.h> 
#include <pico/i2c_slave.h>
#include <string.h>
#include <hardware/i2c.h>
#include "pico/time.h"
#include "BMP280.h"

// This slave device only mimics I2C 

// I2C slave address
//static const uint SLAVE_ADDR = 0x76;
#define SLAVE_ADDR _u(0x76)
static const uint I2C_BAUDRATE = 100 * 1000;

#define SLAVE_SDA_PIN 0 // i2c0 Pseudo SDA for BMP280, connect master GP4 here
#define SLAVE_SCL_PIN 1 // i2c0 Pseudo SCL for BMP280, connect master GP5 here
#define MASTER_SDA_PIN 6 // i2c1 Temp, fake master SDA
#define MASTER_SCL_PIN 7 // i2c1 Temp, fake master SCL

// Based on https://www.alldatasheet.com/datasheet-pdf/pdf/1132069/BOSCH/BMP280.html
// And built to be suitable for bmp280_i2c

// BMP280 temperature registers
#define REG_TEMP_XLSB _u(0xFC)
#define REG_TEMP_LSB _u(0xFB)
#define REG_TEMP_MSB _u(0xFA)

// Temperature values
uint8_t temp_xlsb = 0x00;
uint8_t temp_lsb = 0xED;
uint8_t temp_msb = 0x7E;

// BMP280 pressure registers
#define REG_PRESSURE_XLSB _u(0xF9)
#define REG_PRESSURE_LSB _u(0xF8)
#define REG_PRESSURE_MSB _u(0xF7)

// Pressure values
uint8_t pressure_xlsb = 0xC0;
uint8_t pressure_lsb = 0x5A;
uint8_t pressure_msb = 0x65;

#define REG_CONFIG _u(0xF5)
#define REG_CTRL_MEAS _u(0xF4)
#define REG_STATUS _u(0xF3)
#define REG_RESET _u(0xE0)
#define REG_ID _u(0xD0)

uint8_t reset = 0x00; // reset triggered on 0xB6
uint8_t id = 0x58;

// Calibration registers
#define REG_DIG_T1_LSB _u(0x88)
#define REG_DIG_T1_MSB _u(0x89)
#define REG_DIG_T2_LSB _u(0x8A)
#define REG_DIG_T2_MSB _u(0x8B)
#define REG_DIG_T3_LSB _u(0x8C)
#define REG_DIG_T3_MSB _u(0x8D)
#define REG_DIG_P1_LSB _u(0x8E)
#define REG_DIG_P1_MSB _u(0x8F)
#define REG_DIG_P2_LSB _u(0x90)
#define REG_DIG_P2_MSB _u(0x91)
#define REG_DIG_P3_LSB _u(0x92)
#define REG_DIG_P3_MSB _u(0x93)
#define REG_DIG_P4_LSB _u(0x94)
#define REG_DIG_P4_MSB _u(0x95)
#define REG_DIG_P5_LSB _u(0x96)
#define REG_DIG_P5_MSB _u(0x97)
#define REG_DIG_P6_LSB _u(0x98)
#define REG_DIG_P6_MSB _u(0x99)
#define REG_DIG_P7_LSB _u(0x9A)
#define REG_DIG_P7_MSB _u(0x9B)
#define REG_DIG_P8_LSB _u(0x9C)
#define REG_DIG_P8_MSB _u(0x9D)
#define REG_DIG_P9_LSB _u(0x9E)
#define REG_DIG_P9_MSB _u(0x9F)

// Temperature calibration parameters
uint8_t dig_t1_lsb = 0x70;
uint8_t dig_t1_msb = 0x6B;
int8_t dig_t2_lsb = 0x43;
int8_t dig_t2_msb = 0x67;
int8_t dig_t3_lsb = 0x18;
int8_t dig_t3_msb = 0xFC;

// Pressure calibration parameters
uint8_t dig_p1_lsb = 0x7D;
uint8_t dig_p1_msb = 0x8E;
int8_t dig_p2_lsb = 0x43;
int8_t dig_p2_msb = 0xD6;
int8_t dig_p3_lsb = 0xD0;
int8_t dig_p3_msb = 0x0B;
int8_t dig_p4_lsb = 0x27;
int8_t dig_p4_msb = 0x0B;
int8_t dig_p5_lsb = 0x8C;
int8_t dig_p5_msb = 0x00;
int8_t dig_p6_lsb = 0xF9;
int8_t dig_p6_msb = 0xFF;
int8_t dig_p7_lsb = 0x8C;
int8_t dig_p7_msb = 0x3C;
int8_t dig_p8_lsb = 0xF8;
int8_t dig_p8_msb = 0xC6;
int8_t dig_p9_lsb = 0x70;
int8_t dig_p9_msb = 0x17;

// Number of calibration registers to be read by master
#define NUM_CALIB_PARAMS 24

// For master to store calibration parameters
struct bmp280_calib_param {
    // temperature params
    uint16_t dig_t1;
    int16_t dig_t2;
    int16_t dig_t3;

    // pressure params
    uint16_t dig_p1;
    int16_t dig_p2;
    int16_t dig_p3;
    int16_t dig_p4;
    int16_t dig_p5;
    int16_t dig_p6;
    int16_t dig_p7;
    int16_t dig_p8;
    int16_t dig_p9;
};

// Memory map of BMP280 
static struct
{
    uint8_t mem[256];
    uint8_t target_mem_address;
    bool address_written;
} BMPMemoryMap;

// Initialise the memory map of BMP280
static void init_memory_map(){
    // Initialising Temperature and Pressure
    BMPMemoryMap.mem[REG_TEMP_XLSB] = temp_xlsb;
    BMPMemoryMap.mem[REG_TEMP_LSB] = temp_lsb;
    BMPMemoryMap.mem[REG_TEMP_MSB] = temp_msb;

    BMPMemoryMap.mem[REG_PRESSURE_XLSB] = pressure_xlsb;
    BMPMemoryMap.mem[REG_PRESSURE_LSB] = pressure_lsb;
    BMPMemoryMap.mem[REG_PRESSURE_MSB] = pressure_msb;

    // Setting id and reset
    BMPMemoryMap.mem[REG_RESET] = reset;
    BMPMemoryMap.mem[REG_ID] = id;

    // Initialising Calibration registers
    BMPMemoryMap.mem[REG_DIG_T1_LSB] = dig_t1_lsb;
    BMPMemoryMap.mem[REG_DIG_T1_MSB] = dig_t1_msb;
    BMPMemoryMap.mem[REG_DIG_T2_LSB] = dig_t2_lsb;
    BMPMemoryMap.mem[REG_DIG_T2_MSB] = dig_t2_msb;
    BMPMemoryMap.mem[REG_DIG_T3_LSB] = dig_t3_lsb;
    BMPMemoryMap.mem[REG_DIG_T3_MSB] = dig_t3_msb;
    BMPMemoryMap.mem[REG_DIG_P1_LSB] = dig_p1_lsb;
    BMPMemoryMap.mem[REG_DIG_P1_MSB] = dig_p1_msb;
    BMPMemoryMap.mem[REG_DIG_P2_LSB] = dig_p2_lsb;
    BMPMemoryMap.mem[REG_DIG_P2_MSB] = dig_p2_msb;
    BMPMemoryMap.mem[REG_DIG_P3_LSB] = dig_p3_lsb;
    BMPMemoryMap.mem[REG_DIG_P3_MSB] = dig_p3_msb;
    BMPMemoryMap.mem[REG_DIG_P4_LSB] = dig_p4_lsb;
    BMPMemoryMap.mem[REG_DIG_P4_MSB] = dig_p4_msb;
    BMPMemoryMap.mem[REG_DIG_P5_LSB] = dig_p5_lsb;
    BMPMemoryMap.mem[REG_DIG_P5_MSB] = dig_p5_msb;
    BMPMemoryMap.mem[REG_DIG_P6_LSB] = dig_p6_lsb;
    BMPMemoryMap.mem[REG_DIG_P6_MSB] = dig_p6_msb;
    BMPMemoryMap.mem[REG_DIG_P7_LSB] = dig_p7_lsb;
    BMPMemoryMap.mem[REG_DIG_P7_MSB] = dig_p7_msb;
    BMPMemoryMap.mem[REG_DIG_P8_LSB] = dig_p8_lsb;
    BMPMemoryMap.mem[REG_DIG_P8_MSB] = dig_p8_msb;
    BMPMemoryMap.mem[REG_DIG_P9_LSB] = dig_p9_lsb;
    BMPMemoryMap.mem[REG_DIG_P9_MSB] = dig_p9_msb;
}

// BMP280 slave handler
static void BMP280_i2c_slave_handler(i2c_inst_t *i2c, i2c_slave_event_t event){
    switch (event){
        case I2C_SLAVE_RECEIVE: // For when master is writing to BMP280
            printf("Recevied \n");
            if (BMPMemoryMap.address_written){
                // Checks that the target address can be written to
                if (BMPMemoryMap.target_mem_address == REG_CONFIG || BMPMemoryMap.target_mem_address == REG_CTRL_MEAS || BMPMemoryMap.target_mem_address == REG_RESET){
                    BMPMemoryMap.mem[BMPMemoryMap.target_mem_address] = i2c_read_byte_raw(i2c);
                }
            } else {
                // First byte sent to device is registered as the address
                BMPMemoryMap.target_mem_address = i2c_read_byte_raw(i2c);
                printf("%02x", BMPMemoryMap.target_mem_address);
                BMPMemoryMap.address_written = true;
            } 
            break;
        case I2C_SLAVE_REQUEST: // For when master is read from BMP280
            if (BMPMemoryMap.target_mem_address != REG_RESET){
                i2c_write_byte_raw(i2c, BMPMemoryMap.mem[BMPMemoryMap.target_mem_address]);
                printf("Slave sending: %02x, from address: %02x\n", BMPMemoryMap.mem[BMPMemoryMap.target_mem_address], BMPMemoryMap.target_mem_address);

                // Increases or decreases the LSB of pressure and temperature with a range of 1-5;
                if (BMPMemoryMap.target_mem_address == REG_PRESSURE_LSB || BMPMemoryMap.target_mem_address == REG_TEMP_LSB){
                    int decision, amount;
                    // Used rand() to randomize the value
                    // A POC to show that the data in a slave emulator can be randomized
                    decision = rand() % 2;
                    amount = rand() % 5 + 1;
                    if (decision){
                        BMPMemoryMap.mem[BMPMemoryMap.target_mem_address] += amount;
                    }else{
                        BMPMemoryMap.mem[BMPMemoryMap.target_mem_address] -= amount;
                    }
                }
                BMPMemoryMap.target_mem_address++;
            }
            break;
        case I2C_SLAVE_FINISH: // For when the transfer ends
            BMPMemoryMap.address_written = false;
            break;
        default:
            break;
    }
}

// Sets up the BMP280 slave device
void setup_slave_BMP280(){
    gpio_init(SLAVE_SDA_PIN);
    gpio_set_function(SLAVE_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SLAVE_SDA_PIN);

    gpio_init(SLAVE_SCL_PIN);
    gpio_set_function(SLAVE_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SLAVE_SCL_PIN);

    i2c_init(i2c0, I2C_BAUDRATE);

    

    init_memory_map();
}

void i2c_slave_init_Caller_BMP280(){
    i2c_slave_init(i2c0, SLAVE_ADDR, &BMP280_i2c_slave_handler);
}


// Initialises hte BMP280
void bmp280_init() {
    // use the "handheld device dynamic" optimal setting (see datasheet)
    uint8_t buf[2];

    // 500ms sampling time, x16 filter
    const uint8_t reg_config_val = ((0x04 << 5) | (0x05 << 2)) & 0xFC;

    // send register number followed by its corresponding value
    buf[0] = REG_CONFIG;
    buf[1] = reg_config_val;
    i2c_write_blocking(i2c1, SLAVE_ADDR, buf, 2, false);

    // osrs_t x1, osrs_p x4, normal mode operation
    const uint8_t reg_ctrl_meas_val = (0x01 << 5) | (0x03 << 2) | (0x03);
    buf[0] = REG_CTRL_MEAS;
    buf[1] = reg_ctrl_meas_val;
    i2c_write_blocking(i2c1, SLAVE_ADDR, buf, 2, false);
}

// Used to read data from BMP280
void bmp280_read_raw(int32_t* temp, int32_t* pressure) {
    // BMP280 data registers are auto-incrementing and we have 3 temperature and
    // pressure registers each, so we start at 0xF7 and read 6 bytes to 0xFC
    // note: normal mode does not require further ctrl_meas and config register writes

    uint8_t buf[6];
    uint8_t reg = REG_PRESSURE_MSB;
    i2c_write_blocking(i2c1, SLAVE_ADDR, &reg, 1, true);  // true to keep master control of bus
    i2c_read_blocking(i2c1, SLAVE_ADDR, buf, 6, false);  // false - finished with bus

    // store the 20 bit read in a 32 bit signed integer for conversion
    *pressure = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
    *temp = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);
}

// Edits the reset register in BMP280
void bmp280_reset() {
    // reset the device with the power-on-reset procedure
    uint8_t buf[2] = { REG_RESET, 0xB6 };
    i2c_write_blocking(i2c1, SLAVE_ADDR, buf, 2, false);
}

// Intermediate function that calculates the fine resolution temperature
// used for both pressure and temperature conversions
int32_t bmp280_convert(int32_t temp, struct bmp280_calib_param* params) {
    // use the 32-bit fixed point compensation implementation given in the
    // datasheet
    int32_t var1, var2;
    var1 = ((((temp >> 3) - ((int32_t)params->dig_t1 << 1))) * ((int32_t)params->dig_t2)) >> 11;
    var2 = (((((temp >> 4) - ((int32_t)params->dig_t1)) * ((temp >> 4) - ((int32_t)params->dig_t1))) >> 12) * ((int32_t)params->dig_t3)) >> 14;
    return var1 + var2;
}

// Converts and returns the proper temperature
int32_t bmp280_convert_temp(int32_t temp, struct bmp280_calib_param* params) {
    // uses the BMP280 calibration parameters to compensate the temperature value read from its registers
    int32_t t_fine = bmp280_convert(temp, params);
    return (t_fine * 5 + 128) >> 8;
}

// Converts and returns the proper pressure
int32_t bmp280_convert_pressure(int32_t pressure, int32_t temp, struct bmp280_calib_param* params) {
    // uses the BMP280 calibration parameters to compensate the pressure value read from its registers
    
    // For more info on this calculation, go to the datasheet page 22 
    int32_t t_fine = bmp280_convert(temp, params);

    int32_t var1, var2;
    uint32_t converted = 0.0;
    var1 = (((int32_t)t_fine) >> 1) - (int32_t)64000;
    var2 = (((var1 >> 2) * (var1 >> 2)) >> 11) * ((int32_t)params->dig_p6);
    var2 += ((var1 * ((int32_t)params->dig_p5)) << 1);
    var2 = (var2 >> 2) + (((int32_t)params->dig_p4) << 16);
    var1 = (((params->dig_p3 * (((var1 >> 2) * (var1 >> 2)) >> 13)) >> 3) + ((((int32_t)params->dig_p2) * var1) >> 1)) >> 18;
    var1 = ((((32768 + var1)) * ((int32_t)params->dig_p1)) >> 15);
    if (var1 == 0) {
        return 0;  // avoid exception caused by division by zero
    }
    converted = (((uint32_t)(((int32_t)1048576) - pressure) - (var2 >> 12))) * 3125;
    if (converted < 0x80000000) {
        converted = (converted << 1) / ((uint32_t)var1);
    } else {
        converted = (converted / (uint32_t)var1) * 2;
    }
    var1 = (((int32_t)params->dig_p9) * ((int32_t)(((converted >> 3) * (converted >> 3)) >> 13))) >> 12;
    var2 = (((int32_t)(converted >> 2)) * ((int32_t)params->dig_p8)) >> 13;
    converted = (uint32_t)((int32_t)converted + ((var1 + var2 + params->dig_p7) >> 4));
    return converted;
}

// Gets the calibration parameters of the BMP280
void bmp280_get_calib_params(struct bmp280_calib_param* params) {
    // raw temp and pressure values need to be calibrated according to
    // parameters generated during the manufacturing of the sensor
    // there are 3 temperature params, and 9 pressure params, each with a LSB
    // and MSB register, so we read from 24 registers

    uint8_t buf[NUM_CALIB_PARAMS] = { 0 };
    uint8_t reg = REG_DIG_T1_LSB;
    i2c_write_blocking(i2c1, SLAVE_ADDR, &reg, 1, true);  // true to keep master control of bus
    // read in one go as register addresses auto-increment
    i2c_read_blocking(i2c1, SLAVE_ADDR, buf, NUM_CALIB_PARAMS, false);  // false, we're done reading

    // store these in a struct for later use
    params->dig_t1 = (uint16_t)(buf[1] << 8) | buf[0];
    params->dig_t2 = (int16_t)(buf[3] << 8) | buf[2];
    params->dig_t3 = (int16_t)(buf[5] << 8) | buf[4];

    params->dig_p1 = (uint16_t)(buf[7] << 8) | buf[6];
    params->dig_p2 = (int16_t)(buf[9] << 8) | buf[8];
    params->dig_p3 = (int16_t)(buf[11] << 8) | buf[10];
    params->dig_p4 = (int16_t)(buf[13] << 8) | buf[12];
    params->dig_p5 = (int16_t)(buf[15] << 8) | buf[14];
    params->dig_p6 = (int16_t)(buf[17] << 8) | buf[16];
    params->dig_p7 = (int16_t)(buf[19] << 8) | buf[18];
    params->dig_p8 = (int16_t)(buf[21] << 8) | buf[20];
    params->dig_p9 = (int16_t)(buf[23] << 8) | buf[22];
}

// Runs a master to interact with BMP280
static void run_BMP280_master(){
    gpio_init(MASTER_SDA_PIN);
    gpio_set_function(MASTER_SDA_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MASTER_SDA_PIN);

    gpio_init(MASTER_SCL_PIN);
    gpio_set_function(MASTER_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(MASTER_SCL_PIN);
    
    i2c_init(i2c1, I2C_BAUDRATE);

    // configure BMP280
    bmp280_init();

    // retrieve fixed compensation params
    struct bmp280_calib_param params;
    bmp280_get_calib_params(&params);

    int32_t raw_temperature;
    int32_t raw_pressure;

    sleep_ms(250); // sleep so that data polling and register update don't collide
    while (1) {
        bmp280_read_raw(&raw_temperature, &raw_pressure);
        int32_t temperature = bmp280_convert_temp(raw_temperature, &params);
        int32_t pressure = bmp280_convert_pressure(raw_pressure, raw_temperature, &params);
        printf("Pressure = %.3f kPa\n", pressure / 1000.f);
        printf("Temp. = %.2f C\n", temperature / 100.f);
        // poll every 500ms
        sleep_ms(500);
    }
}

// int main() {
//     stdio_init_all(); 
    
//     setup_slave();
//     run_BMP280_master();
    
//     while(1){
//         sleep_ms(1000);
//     }

//     return 0;
// }